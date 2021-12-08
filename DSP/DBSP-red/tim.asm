       COMMENT *

This file is used to generate boot DSP code for the 250 MHz fiber optic
	timing board using a DSP56303 as its main processor. It supports 
	split serial and frame transfer, but not split parallel nor binning.
	*
	PAGE    132     ; Printronix page width - 132 columns

; Change history:
; 2011-10-23 (GR): Moved the serial register flush after each parallel skip (was after all parallel skips) to prevent overflowing of the serial reg.
 

; Include the boot and header files so addressing is easy
	INCLUDE "timhdr.asm"
	INCLUDE	"timboot.asm"

	ORG	P:,P:

CC	EQU	CCDVIDREV5+TIMREV5+TEMP_POLY+UTILREV3+SPLIT_SERIAL+SUBARRAY+BINNING+SHUTTER_CC

; Put number of words of application in P: for loading application from EEPROM
	DC	TIMBOOT_X_MEMORY-@LCV(L)-1

; Define CLOCK as a macro to produce in-line code to reduce execution time
CLOCK	MACRO
	JCLR	#SSFHF,X:HDR,*		; Don't overfill the WRSS FIFO
	REP	Y:(R0)+			; Repeat # of times at address Y:(R0)+
	MOVEP	Y:(R0)+,Y:WRSS		; Write the waveform to the FIFO
	ENDM
	
; Set software to IDLE mode
START_IDLE_CLOCKING
        MOVE    #IDLE,R0
        NOP
	MOVE	R0,X:<IDL_ADR
	BSET	#IDLMODE,X:<STATUS	; Idle after readout
	JMP     <FINISH			; Need to send header and 'DON'

; Keep the CCD idling when not reading out
IDLE	
	MOVE	Y:<NSRI,A		; NSERIALS_READ = NSR
	JCLR	#SPLIT_S,X:STATUS,*+3
	ASR	A			; Split serials requires / 2
	NOP
  
  DO      A,IDL1     	; Loop over number of pixels per line
	MOVE    Y:SERIAL_IDLE,R0 	; Serial transfer on pixel
	CLOCK  				; Go to it
	MOVE	#COM_BUF,R3
	JSR	<GET_RCV		; Check for FO or SSI commands
	JCC	<NO_COM			; Continue IDLE if no commands received
	ENDDO
	JMP     <PRC_RCV		; Go process header and command
NO_COM	NOP
IDL1
  	DO      Y:<N_PARALLEL_CLEARS,PAR
	MOVE    Y:<PARALLEL_CLEAR,R0	; Address of parallel clocking waveform
	CLOCK  				; Go clock out the CCD charge
PAR
	JMP     <IDLE


;  *****************  Exposure and readout routines  *****************

; Overall loop - transfer and read NPR lines
RDCCD

; Calculate some readout parameters
	MOVE	Y:<NP_SKIP,A		; NP_SKIP = 0 => full image readout
        NOP
	TST	A
	JNE	<SUB_IMG
	MOVE	Y:<NSR,A		; NSERIALS_READ = NSR
	JCLR	#SPLIT_S,X:STATUS,*+3
	ASR	A			; Split serials requires / 2
	NOP
	MOVE	A,Y:<NSERIALS_READ	; Number of columns in each subimage
	JMP	<WT_CLK

SUB_IMG	MOVE	Y:<NS_RD,A
	NOP
	MOVE	A,Y:<NSERIALS_READ

; Start the loop for parallel shifting desired number of lines
WT_CLK	NOP     ; not using the binning waveform generation at the moment.
	JSR	<WAIT_TO_FINISH_CLOCKING

; Skip over the required number of rows for subimage readout
	MOVE	Y:<NP_SKIP,A		; Number of rows to skip
	TST	A
	JEQ	<CLR_SR
	DO  	Y:<NP_SKIP,L_PSKP	
	DO	Y:<NPBIN,L_PSKIP
	MOVE    Y:<PARALLEL_CLEAR,R0
	CLOCK

; Clear out the accumulated charge from the serial shift register 
CLR_SR	DO      Y:<NSCLR,L_CLRSR	; Loop over number of pixels to clear
        MOVE    Y:<SERIAL_SKIP,R0
	CLOCK    		      	; Go clock out the CCD charge
L_CLRSR NOP		                ; Do loop restriction  

L_PSKIP	NOP
L_PSKP

; Parallel shift the image into the serial shift register
	MOVE	Y:<NPR,A
	NOP
	JCLR  	#SPLIT_P,X:STATUS,P_CLK
  	MOVE 	Y:<NPR,A
  	ASR 	A
  NOP

; This is the main loop over each line to be read out

P_CLK	DO      A1,LPR			; Number of rows to read out

; Exercise the parallel clocks, including binning if needed
	DO	Y:<NPBIN,L_PBIN
	MOVE    Y:<PARALLEL,R0
	CLOCK
L_PBIN

; Check for a command once per line. Only the ABORT command should be issued.
	MOVE	#COM_BUF,R3
	JSR	<GET_RCV		; Was a command received?
	JCC	<CONTINUE_READ		; If no, continue reading out
	JMP	<PRC_RCV		; If yes, go process it

; Abort the readout currently underway
ABR_RDC	JCLR	#ST_RDC,X:<STATUS,ABORT_EXPOSURE
	ENDDO				; Properly terminate readout loop
	JMP	<ABORT_EXPOSURE

CONTINUE_READ

; Read the prescan pixels
F_BIAS	MOVE	Y:<NS_PRE,A		; NS_PRE = 0 => no prescan pixels
	TST	A
	JLE	<END_PRE
	DO      Y:<NS_PRE,F_BRD		; Number of pixels to read out
        MOVE    Y:<SERIAL_READ,R0
	CLOCK  				; Go clock out the CCD charge
F_BRD	NOP
END_PRE	NOP

; Skip over NS_SKP1 columns for subimage readout
	MOVE	Y:<NS_SKP1,A		; Number of columns to skip
	TST	A
	JLE	<L_READ
	DO	Y:<NS_SKP1,L_SKP1	; Number of waveform entries total
	MOVE	Y:<SERIAL_SKIP,R0	; Waveform table starting address
	CLOCK  				; Go clock out the CCD charge
L_SKP1

; Finally read some real pixels
L_READ	DO	Y:<NSERIALS_READ,L_RD
        MOVE    Y:<SERIAL_READ,R0
	CLOCK  				; Go clock out the CCD charge
L_RD

; Skip over NS_SKP2 columns if needed for subimage readout
	MOVE	Y:<NS_SKP2,A		; Number of columns to skip
	TST	A
	JLE	<L_BIAS
	DO	Y:<NS_SKP2,L_SKP2
	MOVE	Y:<SERIAL_SKIP,R0	; Waveform table starting address
	CLOCK  				; Go clock out the CCD charge
L_SKP2

; And read the overscan pixels
L_BIAS	MOVE	Y:<NS_OVR,A		; NS_OVR = 0 => no bias pixels
	TST	A
	JLE	<END_ROW
	DO      Y:<NS_OVR,L_BRD		; Number of pixels to read out
        MOVE    Y:<SERIAL_READ,R0
	CLOCK  				; Go clock out the CCD charge
L_BRD	NOP
END_ROW	NOP
LPR	NOP				; End of parallel loop

; Restore the controller to non-image data transfer and idling if necessary
RDC_END	JCLR	#IDLMODE,X:<STATUS,NO_IDL ; Don't idle after readout
	MOVE	#IDLE,R0
	MOVE	R0,X:<IDL_ADR
	JMP	<RDC_E
NO_IDL	MOVE	#TST_RCV,R0
	MOVE	R0,X:<IDL_ADR
RDC_E	JSR	<WAIT_TO_FINISH_CLOCKING
	BCLR	#ST_RDC,X:<STATUS	; Set status to not reading out
	JMP     <START

; ******  Include many routines not directly needed for readout  *******
	INCLUDE "timCCDmisc.asm"


TIMBOOT_X_MEMORY	EQU	@LCV(L)

;  ****************  Setup memory tables in X: space ********************

; Define the address in P: space where the table of constants begins

	IF	@SCP("DOWNLOAD","HOST")
	ORG     X:END_COMMAND_TABLE,X:END_COMMAND_TABLE
	ENDIF

	IF	@SCP("DOWNLOAD","ROM")
	ORG     X:END_COMMAND_TABLE,P:
	ENDIF

; Application commands
	DC	'PON',POWER_ON
	DC	'POF',POWER_OFF
	DC	'SBV',SET_BIAS_VOLTAGES
	DC	'IDL',START_IDLE_CLOCKING
	DC	'OSH',OPEN_SHUTTER
	DC	'CSH',CLOSE_SHUTTER
	DC	'RDC',RDCCD 			; Begin CCD readout    
	DC	'CLR',CLEAR  			; Fast clear the CCD   

; Exposure and readout control routines
	DC	'SET',SET_EXPOSURE_TIME
	DC	'RET',READ_EXPOSURE_TIME
	DC	'SEX',START_EXPOSURE
	DC	'PEX',PAUSE_EXPOSURE
	DC	'REX',RESUME_EXPOSURE
	DC	'AEX',ABORT_EXPOSURE
	DC	'ABR',ABR_RDC
	DC	'CRD',CONTINUE_READ
        DC      'WSI',SYNTHETIC_IMAGE
        DC      'STR',STR_RDC

; Support routines
	DC	'SGN',ST_GAIN     
	DC	'SBN',SET_BIAS_NUMBER
	DC	'SMX',SET_MUX
	DC	'CSW',CLR_SWS
	DC	'SOS',SELECT_OUTPUT_SOURCE
	DC	'ROI',SET_ROI
	DC	'GEO',SET_GEOMETRY
	DC	'RCC',READ_CONTROLLER_CONFIGURATION 
        DC      'RAW',RAW_COMMAND ; So I can set voltages as I please
        DC      'ERS',ERASE ;Persistent image erase
        DC      'EPG',EPURGE ; E-Purge procedure
        DC      'TAD',TEST_AD
        DC      'CID',CHANGE_IDLE_DIRECTION
        DC      'CIR',CHANGE_IDLE_READ_DIRECTION
        DC      'CPC',CHANGE_NUMBER_PARALLEL_CLEARS
        DC      'NPS',NO_POLARITY_SHIFT
	DC	'BIN',SBINN			; Set binning command

END_APPLICATION_COMMAND_TABLE	EQU	@LCV(L)

	IF	@SCP("DOWNLOAD","HOST")
NUM_COM			EQU	(@LCV(R)-COM_TBL_R)/2	; Number of boot + 
							;  application commands
EXPOSING		EQU	CHK_TIM			; Address if exposing
CONTINUE_READING	EQU	CONT_RD 		; Address if reading out
	ENDIF

	IF	@SCP("DOWNLOAD","ROM")
	ORG     Y:0,P:
	ENDIF

; Now let's go for the timing waveform tables
	IF	@SCP("DOWNLOAD","HOST")
        ORG     Y:0,Y:0
	ENDIF

GAIN	DC	END_APPLICATON_Y_MEMORY-@LCV(L)-1

NSR     DC      4128            ; 1: Number Serial Read, prescan + image + bias
NPR     DC      2040	     	; 2: Number Parallel Read
NSCLR	DC      NS_CLR  	; 3: To clear the serial register
NPCLR   DC      NP_CLR          ; 4: To clear the parallel register 
NSBIN   DC      1       	; 5: Serial binning parameter
NPBIN   DC      1       	; 6: Parallel binning parameter
TST_DAT	DC      0		; 7: Temporary definition for test images
SHDEL	DC	SH_DEL		; 8: Delay in milliseconds between shutter closing 
				; and image readout
CONFIG	DC	CC		; 9: Controller configuration
NSERIALS_READ	DC	4128    ; 10: Number of serials to read

; Waveform parameters
;OS		DC	'ALL'	; 11: Output source 
OS		DC	'_U1'	; 11: Output source 
FIRST_CLOCKS	DC	0	; 12: Address of first clocks waveforms
CLOCK_LINE	DC	0	; 13: Clock one complete line of charge

; ROI parameters
NP_SKIP		DC	0	; 14: Number of rows to skip
NS_SKP1		DC	0	; 15: Number of serials to clear before read
NS_SKP2		DC	0	; 16: Number of serials to clear after read
NS_PRE		DC	0	; 17: number of prescan pixels to read
NS_OVR		DC	0	; 18: number of overscan pixels to read
NS_RD		DC	0	; 19: number of data pixels to read

; Time parameters for Erase and E-Purge
TIME1   	DC      0       ; 20
TIME2   	DC      0	; 21

; Readout peculiarity parameters. Default values are selected here.
SERIAL_READ     DC      SERIAL_READ_SPLIT  ; 22  ; Serial readout waveforms
SERIAL_SKIP 	DC	SERIAL_SKIP_SPLIT  ; 23  ; Serial skipping waveforms
PARALLEL        DC      PARALLEL_2         ; 24  ; Parallel shifting waveforms
PARALLEL_CLEAR  DC      PARALLEL_CLEAR_2   ; 25
PC_1            DC      PARALLEL_CLEAR_1   ; 26
PC_2            DC      PARALLEL_CLEAR_2   ; 27
SERIAL_IDLE     DC      SERIAL_IDLE_SPLIT  ; 28
SI_L            DC      SERIAL_IDLE_LEFT   ; 29
SI_R            DC      SERIAL_IDLE_RIGHT  ; 30
BYTE_1          DC      0                  ; 31
BYTE_2          DC      0                  ; 32
SR_L            DC      SERIAL_READ_LEFT   ; 33
SR_R            DC      SERIAL_READ_RIGHT  ; 34
P_1             DC      PARALLEL_1         ; 35
P_2             DC      PARALLEL_2         ; 36
N_PARALLEL_CLEARS       DC      1          ; 37
SR_S            DC      SERIAL_READ_SPLIT  ; 38
TMP_1           DC      TMP_PXL_TBL1       ; 39 stuff where I can load a idle without resetting.
TMP_2           DC      TMP_PXL_TBL2       ; 40
TMP_3           DC      TMP_PXL_TBL3       ; 41
PC_S		DC	PARALLEL_CLEAR_SPLIT
NSRI            DC      4128		   ; 43 Total size of serial register
NSR_ACTIVE      DC      4114		   ; 44 Size of active pixels


; Include the waveform table for the designated type of CCD
	INCLUDE "WAVEFORM_FILE" ; Readout and clocking waveform file

        ORG     Y:@LCV(L)+50,Y:@LCV(L)+50

TMP_PXL_TBL1    DC      0

        ORG     Y:@LCV(L)+50,Y:@LCV(L)+50

TMP_PXL_TBL2    DC      0 
        
        ORG     Y:@LCV(L)+50,Y:@LCV(L)+50

TMP_PXL_TBL3    DC      0

END_APPLICATON_Y_MEMORY	EQU	@LCV(L)


; End of program
	END

