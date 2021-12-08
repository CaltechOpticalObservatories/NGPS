       COMMENT *

This file is used to generate boot DSP code for the 250 MHz fiber optic
	timing board using a DSP56303 as its main processor.
This does NOT support utility board operation
	*
	PAGE    132     ; Printronix page width - 132 columns

; Special address for two words for the DSP to bootstrap code from the EEPROM
	IF	@SCP("DOWNLOAD","ROM")
	ORG	P:0,P:0
	DC	END_ADR				; Number of boot words
	DC	0				; Starting address
	ORG	P:0,P:2
	JMP	<INIT				; Initialize the board
	NOP
	ENDIF

	IF	@SCP("DOWNLOAD","HOST")
	ORG	P:0,P:0
	JMP	<INIT
	NOP
	ENDIF

; *******************  Command Processing  ******************

; Read the header and check it for self-consistency
START	MOVE	X:<IDL_ADR,R0		
	JSET    #TIM_BIT,X:TCSR0,EXPOSING	; If exposing go check timer
	JSET	#ST_RDC,X:<STATUS,CONTINUE_READING
	JMP	(R0)

TST_RCV	MOVE	#<COM_BUF,R3
	JSR	<GET_RCV		; Read the header word into X:(R3)
	JCC	*-1
PRC_RCV	MOVE	X:(R3-1),Y0		; Put the header into Y0
	JSR	<CHK_HDR		; Update HEADER and NWORDS
	JCS	<ERROR

; Read all the remaining words in the command
	MOVE	X:<NWORDS,B
	NOP
	SUB	#1,B			; We've already got the header
	NOP
	DO	B,L_GET_COMMAND		; Later - put a TIMEOUT here
	JSR	<GET_RCV
	JCC	*-1
	NOP
L_GET_COMMAND

; Process the receiver entry - is it in the command table ?
COMMAND	MOVE	#COM_BUF,R3
	NOP
	NOP
	MOVE    X:(R3+1),B	; Get the command
	MOVE	(R3)+
	MOVE	(R3)+		; Point R3 to the first argument
	MOVE	#<COM_TBL,R0 	; Get the command table starting address
	DO      #NUM_COM,END_COM ; Loop over the command table
	MOVE    X:(R0)+,Y1      ; Get the command table entry
	CMP     Y1,B  X:(R0),R2	; Does receiver = table entries address?
	JNE     <NOT_COM        ; No, keep looping
	ENDDO                   ; Restore the DO loop system registers
	JMP     (R2)            ; Jump execution to the command
NOT_COM MOVE    (R0)+           ; Increment the register past the table address
END_COM
	JMP	<ERROR		; The command is not in the table

; It's not in the command table - send an error message
ERROR   MOVE    X:<ERR,Y1	; Send the message - there was an error
        JMP     <FINISH1	; This protects against unknown commands

; Send a reply packet - header and reply
FINISH  MOVE    X:<DONE,Y1	; Send 'DON' as the reply
FINISH1	MOVE    X:<HEADER,B	; Get header of incoming command
	MOVE    X:<SMASK,Y0	; This was the source byte, and is to 
	MOVE	#<COM_BUF,R3	;     become the destination byte
	AND     Y0,B X:<TWO,Y0
	LSR	#8,B		; Shift right eight bytes, add it to the
	MOVE	Y0,X:<NWORDS	;     header, and put 2 as the number
	ADD     Y0,B X:<SBRD,Y0	;     of words in the string
	ADD	Y0,B		; Add source board's header, set Y1 for above
	NOP
	MOVE    B,X:(R3)+       ; Put the new header on the transmitter stack
	MOVE	Y1,X:(R3)+	; Put the value of XO on the transmitter stack

; Transmit words to the host computer over the fiber optics link
	MOVE	#<COM_BUF,R3
	DO	X:<NWORDS,DON_FFO 	; Transmit all the words in the command
	MOVE	X:(R3)+,B
	JSR	<XMT_WRD
	NOP
DON_FFO	JMP	<START

; Read one 32-bit word from the FIFO and put it into B1 
GET_RCV	JCLR    #EF,X:HDR,CLR_RTS

; Because of FIFO metastability require that EF be stable for two tests
TST1	JCLR    #EF,X:HDR,TST2		; EF = Low,  Low  => CLR_RTS
	JMP	<TST3			;      High, Low  => try again
TST2	JCLR    #EF,X:HDR,CLR_RTS	;      Low,  High => try again
	JMP	<TST1			;      High, High => read FIFO
TST3	JCLR	#EF,X:HDR,TST1

	MOVEP	#$028FE2,X:BCR		; Slow down RDFO access
	NOP
	NOP
	MOVE	Y:RDFO,B
	MOVE	#0,B2
	AND	#$FF,B
	CMP	#>$AC,B			; It must be $AC to be a valid word
	JNE	<CLR_RTS
	MOVE	Y:RDFO,Y0		; Read the MS byte
	INSERT	#$008010,Y0,B
	MOVE	Y:RDFO,Y0		; Read the middle byte
	INSERT	#$008008,Y0,B
	MOVE	Y:RDFO,Y0		; Read the LS byte
	INSERT	#$008000,Y0,B
	NOP
	MOVE	B0,X:(R3)+		; Put the word into COM_BUF
	NOP
	NOP
	BSET	#0,SR			; Valid FIFO word => SR carry = 1
	MOVEP	#$028FE1,X:BCR		; Restore RDFO access
	RTS
CLR_RTS	BCLR	#0,SR			; Not valid FIFO word => SR carry = 0
	MOVEP	#$028FE1,X:BCR		; Restore RDFO access
	RTS

; Transmit the word in B1 to the host computer
XMT_WRD	MOVEP	#$028FE2,X:BCR		; Slow down RDFO access
	MOVE	#FO_HDR+1,R0
	DO	#3,XMT_WRD1
	ASL	#8,B,B
	NOP
	MOVE	B2,X:(R0)+
XMT_WRD1
	MOVE	#FO_HDR,R0
	MOVE	#WRFO,R1
	DO	#4,XMT_WRD2
	MOVE	X:(R0)+,Y0		; Should be MOVEP  X:(R0)+,Y:WRFO
	MOVE	Y0,Y:(R1)
XMT_WRD2
	MOVEP	#$028FE1,X:BCR		; Restore RDFO access
	RTS

; Check the command or reply header in Y0 for self-consistency
CHK_HDR	MOVE	X:<MASK1,B		; Test for S.LE.3 and D.LE.3 and N.LE.7
	AND     Y0,B		
	JNE     <CHK_ERR		; Test failed
	MOVE	X:<MASK2,B		; Test for either S.NE.0 or D.NE.0
	AND     Y0,B
	JEQ     <CHK_ERR		; Test failed
	MOVE	X:<SEVEN,B
	AND	Y0,B			; Extract NWORDS, must be > 0
	JEQ	<CHK_ERR
	MOVE	Y0,X:<HEADER
	MOVE	B1,X:<NWORDS		; Number of words in the command
	NOP
	NOP
	BCLR	#0,SR			; Check OK - header is self-consistent	
	NOP
	RTS

; CHK_HDR error return
CHK_ERR	BSET	#0,SR
	NOP				; SR restriction
	RTS

;  *****************  Boot Commands  *******************
; Test Data Link - simply return value received after 'TDL'
TDL	MOVE    X:(R3)+,Y1      ; Get the data value
	JMP     <FINISH1	; Return from executing TDL command

; Read DSP or EEPROM memory ('RDM' address): read memory, reply with value
RDMEM	MOVE    X:(R3)+,Y1
	MOVE	Y1,B
	AND	#$0FFFFF,B	; Bits 23-20 need to be zeroed
	MOVE	B1,R0		; Need the address in an address register
	MOVE	Y1,B
	NOP
	JCLR    #20,B,RDX 	; Test address bit for Program memory
	MOVE	P:(R0),Y1	; Read from Program Memory
	JMP     <FINISH1	; Send out a header with the value
RDX	JCLR    #21,B,RDY	; Test address bit for X: memory
	MOVE    X:(R0),Y1	; Write to X data memory
	JMP     <FINISH1	; Send out a header with the value
RDY	JCLR    #22,B,RDR	; Test address bit for Y: memory
	MOVE    Y:(R0),Y1	; Read from Y data memory
	JMP     <FINISH1	; Send out a header with the value
RDR	JCLR	#23,B,ERROR	; Test address bit for read from EEPROM memory
	MOVE	X:<THREE,Y1	; Convert to word address to a byte address
	MOVE	R0,Y0		; Get 16-bit address in a data register
	MPY	Y0,Y1,B		; Multiply	
	ASR	B		; Eliminate zero fill of fractional multiply
	MOVE	B0,R0		; Need to address memory
	BSET	#15,R0		; Set bit so its in EEPROM space
	JSR	<RD_WORD	; Read word from EEPROM
	MOVE    B1,Y1		; FINISH1 transmits Y1 as its reply
	JMP     <FINISH1

; Program WRMEM ('WRM' address datum): write to memory, reply 'DON'.
WRMEM	MOVE    X:(R3)+,Y1	; Get the address to be written to
	MOVE	Y1,B
	AND	#$0FFFFF,B	; Bits 23-20 need to be zeroed
	MOVE	B1,R0		; Need the address in an address register
	MOVE	Y1,B
	MOVE    X:(R3)+,Y0	; Get datum into Y0 so MOVE works easily
	JCLR    #20,B,WRX	; Test address bit for Program memory
	MOVE	Y0,P:(R0)	; Write to Program memory
        JMP     <FINISH
WRX	JCLR    #21,B,WRY	; Test address bit for X: memory
	MOVE    Y0,X:(R0)	; Write to X: memory
	JMP     <FINISH
WRY	JCLR    #22,B,WRR	; Test address bit for Y: memory
	MOVE    Y0,Y:(R0)	; Write to Y: memory
	JMP	<FINISH
WRR	JCLR	#23,B,ERROR	; Test address bit for write to EEPROM
	BCLR	#WRENA,X:PDRC	; WR_ENA* = 0 to enable EEPROM writing
	MOVE	Y0,X:<SV_A1	; Save the datum to be written
	MOVE	X:<THREE,Y1	; Convert word address to a byte address
	MOVE	R0,Y0		; Get 16-bit address in a data register
	MPY	Y1,Y0,B		; Multiply	
	ASR	B		; Eliminate zero fill of fractional multiply
	MOVE	B0,R0		; Need to address memory
	BSET	#15,R0		; Set bit so its in EEPROM space
	MOVE    X:<SV_A1,B1	; Get the datum to be written
	DO      #3,L1WRR	; Loop over three bytes of the word
	MOVE    B1,P:(R0)+      ; Write each EEPROM byte
	ASR	#8,B,B
	MOVE	X:<C100K,Y0	; Move right one byte, enter delay = 1 msec
	DO      Y0,L2WRR	; Delay by 12 milliseconds for EEPROM write
	REP	#12		; Assume 100 MHz DSP56303
	NOP
L2WRR
	NOP                     ; DO loop nesting restriction
L1WRR
	BSET	#WRENA,X:PDRC	; WR_ENA* = 1 to disable EEPROM writing
	JMP     <FINISH

; Load application code from P: memory into its proper locations
LDAPPL	MOVE    X:(R3)+,Y1	; Application number, not used yet
	JSR	<LOAD_APPLICATION
	JMP	<FINISH

LOAD_APPLICATION
	MOVE	#$8000,R0	; Starting EEPROM address
	JSR	<RD_WORD	; Number of words in boot code
	MOVE	B1,Y0
	MOVE	X:<THREE,Y1
	MPY	Y0,Y1,B
	ASR	B
	MOVE	B0,R0		; EEPROM address of start of P: application
	BSET	#15,R0		; To access EEPROM
	JSR	<RD_WORD	; Read number of words in application P:
	MOVE	#(X_BOOT_START+1),R1 ; End of boot P: code that needs keeping
	DO	B1,RD_APPL_P
	JSR	<RD_WORD
	MOVE	B1,P:(R1)+
RD_APPL_P
	JSR	<RD_WORD	; Read number of words in application X:
	MOVE	#END_COMMAND_TABLE,R1
	DO	B1,RD_APPL_X
	JSR	<RD_WORD
	MOVE	B1,X:(R1)+
RD_APPL_X
	JSR	<RD_WORD	; Read number of words in application Y:
	MOVE	#1,R1 		; There is no Y: memory in the boot code
	DO	B1,RD_APPL_Y
	JSR	<RD_WORD
	MOVE	B1,Y:(R1)+
RD_APPL_Y
	RTS

; Read one word from EEPROM location R0 into accumulator B1
RD_WORD	DO	#3,L_RDBYTE
	MOVE	P:(R0)+,B2
	ASR	#8,B,B
L_RDBYTE
	RTS

; Come to here on a 'STP' command so 'DON' can be sent
STOP_IDLE_CLOCKING
	MOVE	#<TST_RCV,R0	; Execution address when idle => when not
	MOVE	R0,X:<IDL_ADR	;   processing commands or reading out
	BCLR	#IDLMODE,X:<STATUS	; Don't idle after readout
	JMP     <FINISH

; Routines executed after the DSP boots and initializes
STARTUP	MOVE	#<TST_RCV,R0	; Execution address when idle => when not
	MOVE	R0,X:<IDL_ADR	;   processing commands or reading out
	MOVE	#50000,X0	; Delay by 500 milliseconds
	DO	X0,L_DELAY
	REP	#1000
	NOP
L_DELAY
	MOVE	#$020002,B	; Normal reply after booting is 'SYR'
	JSR	<XMT_WRD
	MOVE	#'SYR',B
	JSR	<XMT_WRD

	JMP	<START		; Start normal command processing


; *******************  DSP  INITIALIZATION  CODE  **********************
; This code initializes the DSP right after booting, and is overwritten 
;   by application code
INIT	MOVEP	#PLL_INIT,X:PCTL	; Initialize PLL to 100 MHz
	NOP

; Set operation mode register OMR to normal expanded
	MOVEC   #$0000,OMR	; Operating Mode Register = Normal Expanded
	MOVEC	#0,SP		; Reset the Stack Pointer SP

; Program the AA = address attribute pins
	MOVEP	#$FFFC21,X:AAR0	; Y = $FFF000 to $FFFFFF asserts commands
	MOVEP	#$008909,X:AAR1	; P = $008000 to $00FFFF accesses the EEPROM 
	MOVEP	#$010C11,X:AAR2	; X = $010000 to $010FFF reads A/D values
	MOVEP	#$080621,X:AAR3	; Y = $080000 to $0BFFFF R/W from SRAM

; Program the DRAM memory access and addressing
	MOVEP	#$028FE2,X:BCR			; Bus Control Register

; Program the Host port B for parallel I/O
	MOVEP	#>1,X:HPCR			; All pins enabled as GPIO
	MOVEP	#$010C,X:HDR	
	MOVEP	#$B10E,X:HDDR			; Data Direction Register
						;  (1 for Output, 0 for Input)

; Port B conversion from software bits to schematic labels
;	PB0 = PWROK		PB08 = PRSFIFO*
;	PB1 = LED1		PB09 = EF*
;	PB2 = LVEN		PB10 = EXT-IN0
;	PB3 = HVEN		PB11 = EXT-IN1
;	PB4 = STATUS0		PB12 = EXT-OUT0
;	PB5 = STATUS1		PB13 = EXT-OUT1
;	PB6 = STATUS2		PB14 = SSFHF*
;	PB7 = STATUS3		PB15 = SELSCI
	
; Program the serial port ESSI0 = Port C for serial communication with 
;   the utility board
	MOVEP	#>0,X:PCRC	; Software reset of ESSI0
	MOVEP	#$000809,X:CRA0	; Divide 100 MHz by 20 to get 5.0 MHz
				; DC[4:0] = 0 for non-network operation
				; WL0-WL2 = ALC = 0 for 2-bit data words
				; ALC = 0, SSC1 = 0 for SC1 not used
	MOVEP	#$010130,X:CRB0	; SCKD = 1 for internally generated clock
				; SHFD = 0 for MSB shifted first
				; CKP = 0 for rising clock edge transitions
				; TE0 = 1 to enable transmitter #0
				; MOD = 0 for normal, non-networked mode
				; FSL1 = 1, FSL0 = 0 for on-demand transmit
	MOVEP	#%000000,X:PCRC	; Control Register (0 for GPIO, 1 for ESSI)
				; SCK0 = P3, STD0 = P5 are ESSI0, turned OFF
	MOVEP	#%010111,X:PRRC	; Data Direction Register (0 for In, 1 for Out)
	MOVEP	#%000101,X:PDRC	; Data Register - ROM/FIFO* = 0, WR_ENA* = 1,
				;   AUX1 = 0, AUX2 = AUX3 = 1

; Conversion from software bits to schematic labels for Port C
;	PC0 = SC00 = UTL-T-SCK
;	PC1 = SC01 = 2_XMT = SYNC on prototype
;	PC2 = SC02 = WR_ENA*
;	PC3 = SCK0 = TIM-U-SCK
;	PC4 = SRD0 = UTL-T-SCD
;	PC5 = STD0 = TIM-U-STD

; Program the serial port ESSI1 = Port D for serial transmission to 
;   the analog boards and two parallel I/O input pins
	MOVEP	#>0,X:PCRD	; Software reset of ESSI0
	MOVEP	#$000809,X:CRA1	; Divide 100 MHz by 20 to get 5.0 MHz
				; DC[4:0] = 0
				; WL[2:0] = 000 for 8-bit data words
				; SSC1 = 0 for SC1 not used
	MOVEP	#$000030,X:CRB1	; SCKD = 1 for internally generated clock
				; SCD2 = 1 so frame sync SC2 is an output
				; SHFD = 0 for MSB shifted first
				; CKP = 0 for rising clock edge transitions
				; TE0 = 0 to NOT enable transmitter #0 yet
				; MOD = 0 so its not networked mode
	MOVEP	#%100000,X:PCRD	; Control Register (0 for GPIO, 1 for ESSI)
				; PD3 = SCK1, PD5 = STD1 for ESSI
	MOVEP	#%000100,X:PRRD	; Data Direction Register (0 for In, 1 for Out)
	MOVEP	#%000100,X:PDRD	; Data Register: 'not used' = 0 outputs
	MOVEP	#0,X:TX10	; Initialize the transmitter to zero
	NOP
	NOP
	BSET	#TE,X:CRB1	; Enable the SSI transmitter

; Conversion from software bits to schematic labels for Port D
; PD0 = SC10 = 2_XMT_? input
; PD1 = SC11 = SSFEF* input
; PD2 = SC12 = PWR_EN
; PD3 = SCK1 = TIM-A-SCK
; PD4 = SRD1 = PWRRST
; PD5 = STD1 = TIM-A-STD

; Program the SCI port to benign values
	MOVEP	#%000,X:PCRE	; Port Control Register = GPIO
	MOVEP	#%010,X:PRRE	; Port Direction Register (0 = Input)
	MOVEP	#%010,X:PDRE	; Port Data Register
;	PE0 = RXD
;	PE1 = TXD
;	PE2 = SCLK

; Program one of the three timers as an exposure timer
	MOVEP	#$C34F,X:TPLR	; Prescaler to generate millisecond timer,
				;  counting from the system clock / 2 = 50 MHz
	MOVEP	#$208000,X:TCSR0 ; Clear timer complete bit and enable prescaler
	MOVEP	#0,X:TLR0	; Timer load register

; Enable no interrupts
	MOVEP	#$000000,X:IPRC	; No interrupts allowed
	MOVE	#$300,SR	; Mask all interrupts

; Initialize the fiber optic serial receiver circuitry
	DO	#20,L_FO_INIT
	MOVE	Y:RDFO,B
	REP	#5
	NOP
L_FO_INIT

; Pulse PRSFIFO* low to revive the CMDRST* instruction and reset the FIFO
	MOVE	#1000000,X0	; Delay by 10 milliseconds
	DO	X0,*+3
	NOP
	BCLR	#8,X:HDR
	REP	#20
	NOP
	BSET	#8,X:HDR

; Put all the analog switch inputs to low so they draw minimum current
	BSET	#3,X:PCRD	; Turn the serial clock on
	MOVE	#$0C3000,A	; Value of integrate speed and gain switches
	CLR	B
	MOVE	#$100000,X0	; Increment over board numbers for DAC writes
	MOVE	#$001000,X1	; Increment over board numbers for WRSS writes
	DO	#15,L_ANALOG	; Fifteen video processor boards maximum
	JSR	<XMIT_A_WORD	; Transmit A to TIM-A-STD
	ADD	X0,A
	MOVE	B,Y:WRSS	; This is for the fast analog switches
	REP	#800		; Delay for the serial data transmission
	NOP
	ADD	X1,B		; Increment the video and clock driver numbers
L_ANALOG	
	BCLR	#CDAC,X:<LATCH		; Enable clearing of DACs
	BCLR	#ENCK,X:<LATCH		; Disable clock and DAC output switches
	MOVEP	X:LATCH,Y:WRLATCH 	; Execute these two operations
	BCLR	#3,X:PCRD		; Turn the serial clock off
	JMP	<SKIP

; Transmit contents of accumulator A1 over the synchronous serial transmitter
XMIT_A_WORD
	MOVE	A1,X:SV_A1
	JCLR	#TDE,X:SSISR1,*		; Start bit
	MOVEP	#$010000,X:TX10
	DO	#3,L_XMIT
	JCLR	#TDE,X:SSISR1,*		; Three data bytes	
	MOVEP	A1,X:TX10
	LSL	#8,A
	NOP
L_XMIT
	JCLR	#TDE,X:SSISR1,*		; Zeroes to bring transmitter low
	MOVEP	#0,X:TX10
	MOVE	X:SV_A1,A1
	RTS

SKIP

	IF	@SCP("DOWNLOAD","ROM")	
; Move the table of boot constants from P: space to X: space
	MOVE	#X_BOOT_START,R1 		; Start of table of constants
	MOVE	#0,R0 
	DO	#X_BOOT_LENGTH,L_X_WRITE
	MOVE	P:(R1)+,X0
	MOVE	X0,X:(R0)+			; Write the constants to X:
L_X_WRITE
	ENDIF

	MOVE	#>$AC,X0
	MOVE	X0,X:<FO_HDR

	JMP	<STARTUP

;  ****************  X: Memory tables  ********************

; Define the address in P: space where the table of constants begins

X_BOOT_START	EQU	@LCV(L)-2

	IF	@SCP("DOWNLOAD","ROM")
        ORG     X:0,P:
	ENDIF
	IF	@SCP("DOWNLOAD","HOST")
        ORG     X:0,X:0
	ENDIF

; Special storage area - initialization constants and scratch space
STATUS		DC	$1064		; Controller status bits

FO_HDR	EQU	STATUS+1		; Fiber optic write bytes
HEADER	EQU	FO_HDR+4		; Command header
NWORDS	EQU	HEADER+1		; Number of words in the command	
COM_BUF	EQU	NWORDS+1		; Command buffer
SV_A1	EQU	COM_BUF+7		; Save accumulator A1

	IF	@SCP("DOWNLOAD","ROM")
NUM_COM			EQU	7		; Number of boot commands
EXPOSING		EQU	TST_RCV		; Address if exposing
CONTINUE_READING	EQU	TST_RCV 	; Address if reading out
        ORG     X:$F,P:@LCV(L)+14
	ENDIF

	IF	@SCP("DOWNLOAD","HOST")
        ORG     X:$F,X:$F
	ENDIF

; Parameter table in P: space to be copied into X: space during 
;   initialization, and is copied from ROM by the DSP boot
LATCH		DC      $7A		; Starting value in latch chip U25
EXPOSURE_TIME	DC	0		; Exposure time in milliseconds
ONE		DC	1		; One
TWO		DC	2		; Two
THREE		DC	3		; Three
SEVEN		DC	7		; Seven
MASK1		DC	$FCFCF8		; Mask for checking header
MASK2		DC	$030300		; Mask for checking header
DONE		DC	'DON'		; Standard reply
SBRD		DC	$020000 	; Source Identification number
DBRD		DC	$000200 	; Destination Identification number
DMASK		DC	$00FF00 	; Mask to get destination board number
SMASK   	DC	$FF0000 	; Mask to get source board number
ERR		DC	'ERR'		; An error occurred
C100K		DC	100000		; Delay for WRROM = 1 millisec
IDL_ADR		DC	TST_RCV		; Address of idling routine
EXP_ADR		DC	0		; Jump to this address during exposures
		DC	0,0

; Command table
COM_TBL_R	EQU	@LCV(R)
COM_TBL	DC      'TDL',TDL		; Test Data Link
	DC      'RDM',RDMEM		; Read from DSP or EEPROM memory
	DC      'WRM',WRMEM		; Write to DSP memory        
	DC	'LDA',LDAPPL		; Load application from EEPROM to DSP
	DC	'STP',STOP_IDLE_CLOCKING
	DC	'DON',START		; Nothing special
	DC      'ERR',START		; Nothing special

END_COMMAND_TABLE	EQU	@LCV(R)

	IF	@SCP("DOWNLOAD","ROM")
X_BOOT_LENGTH	EQU	@LCV(L)-2-X_BOOT_START
	ENDIF

END_ADR	EQU	@LCV(L)			; End address of P: code written to ROM
