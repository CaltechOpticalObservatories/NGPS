       COMMENT * 
	This file is used to generate boot DSP code for 
 	the 250 MHz fiber optic timing board, SDSU GenII TIMII, 
	processor = DSP56303.  

Revision History:
--  0.00: 10 Jun 2004 - CRO.
	Original code from Dani Guzman.

--  1.10: 11 Jun 2004 - CRO.
	a.) Cleanup.  Some spanish-english comment translation.
	b.) Y:00000B memory location reserved for NPSHF, the number
	    of rows to parallel shift for the PSH command (which
	    is new).  This is implimented as part of a focus loop
	    in which a series of exposures are made at different 
	    telescope (or spectrograph) focus positions and the CCD 
	    charge is shifted some number of rows (NPSHF) between
	    each exposure.  At the end of the loop an image with a
	    sequence of star images at different telescope focus
	    positions is read out.

--  1.20:  23 Jun 2004 - CRO
	a.) Added nested do loops in RDCCD for parallel and serial
	    binning.  Added NSBINM1 to Y: memory.  This is the
	    serial binning factor minus one, used because he serial
	    binning and readout is split into two parts, one
	    for serial shift only and the last for a serial shift
	    and video processing.  So, the first part is executed
	    NSBINM1 times; the last part, once.

--  1.30:  24 Jun 2004 - CRO
        a.) Added version number to Y memory so that it is possible
            to check which version of code is downloaded into
            the timing board DSP by reading that memory location
            Y:00000D.
        b.) Added NPSKP (Y:00000E) and NSSKP (Y:00000F) to Y memory, 
            the number of parallel and serial skips for ROI readout.
        c.) Added ROI parallel skipping code just before the start
            of the binned readout.

            In order to specify the number of parallel skips,
            the program looks at the NPSKP value.  If it is
            zero, no skips are performed.  If the NPSKP value
            is greater than zero, that many parallel skips will
            be performed before the code will then read out
            NPR rows.

            So, if it is desired to read out a ROI with ystart=512
            and ylength=1024, NPSKP should be set to 512 and
            NPR should be set to 1024 (divided by the binning
            factor, of course).

--  1.40  29 Jun 2004 - CRO
	a.) Changed serial (binned)read to subroutine called
	    from main program getting the serial part of the
	    Region of Interest.
	b.) Added subroutine for serial skips, SSKIP.
	c.) To support serial ROI readout, added the following
	    parameters to Y: memory:
		NSUND  - Number of pixels in the CCD underscan 
			 region.
		NSSKP  - Number of pixels to skip to the ROI 
			 start pixel.
		NSRD   - Number of pixels to read in the ROI.
		NSSKP2 - Number of pixels to skip from the end.
			 of the ROI to the end of the CCD.
		NSOCK  - Number of pixels of overscan to read.	

	   In order to read out a region of interest (ROI)
	   something like the following must be done, for example:

		setroi xstart=512 xlen=1024 ystart=512 ylen=1024

	   Then the ROI parameters in Y memory should be set according to the
	   following:

	      NPSKP  = ystart
	      NPR    = ylen/(binning factor)
	      NSSKP  = xstart 
	      NSRD   = xlen/(binning factor)
	      NSSKP2 = (CCD size in X) - (xstart + xlen)
	  
	   (Question:  should we just use NSR for NSRD and just get rid
	    of NSRD?)

	   Also, it one wishes to change the number of post or over-
	   scan pixels for each row to read, one must update the
	   NSOCK parameter for a ROI readout to be correct.
	   (Probably could fix the code so that this is not 
	   necessary.)
--  1.41  10 Aug 2004 - MB
	   Added the variable NP2READ (NUmber of Pixels to read), which
	   replaces the R5 (or R0) register for storing the amount of pixels
	   to be read inside the SREAD and SSKIP subroutines
--  1.42  16 Jan 2005 - MB
	   Copied to DBSP directory

	*
	PAGE    132     ; Printronix page width - 132 columns

; Include the boot and header files so addressing is easy
	INCLUDE "timhdr.asm"
	INCLUDE	"timboot.asm"

	ORG	P:,P:

CC	EQU	CCDVIDREV5+TIMREV5+UTILREV3+SHUTTER_CC+TEMP_POLY

; Put number of words of application in P: for loading application from EEPROM
	DC	TIMBOOT_X_MEMORY-@LCV(L)-1

; senal de control out0
;	BSET	#EXT_OUT0,X:HDR		; sube pata de control out0
;	BCLR	#EXT_OUT0,X:HDR		; baja pata de control out0

; Set software to IDLE mode
START_IDLE_CLOCKING
	MOVE	#IDLE,R0		; Exercise clocks when idling
	MOVE	R0,X:<IDL_ADR
	BSET	#IDLMODE,X:<STATUS	; Idle after readout
	JMP     <FINISH			; Need to send header and 'DON'


; idle routine dumping lines
IDLE	
 	MOVE    #<PREPARE_DUMP,R0	; 
 	JSR     <CLOCK  		; 
 
 	DO      Y:<NPR,END_DUMP  	; 
 					; 
 
 	MOVE    #<PARALLEL_DUMP,R0 	; shift the array by 1 line, havinfg the DUMP barrier down, so it gets dumped
 	JSR     <CLOCK  		; onto the dump line, beyonfd the serial register
 	MOVE	#COM_BUF,R3
 	JSR	<GET_RCV		; Check for FO or SSI commands
 	JCC	<NO_COM1		; Continue IDLE if no commands received
 	ENDDO				; termina ciclo si hay comando
 	JMP     <PRC_RCV		; Go process header and command
NO_COM1	
 	NOP
END_DUMP
 	MOVE    #<CLEAR_READ_REGISTER,R0	; 
 	JSR     <CLOCK  		
 
 	JMP     <IDLE
 
;  *****************  Exposure and readout routines  *****************

RDCCD	
; Check for serial binning and compute "binning minus 1", because we 
; will shift charge binning-1 times and then use the shift+video 
; waveform for the last shift and read.
	MOVE	Y:<NSBIN,A		; Get serial binning factor
	MOVE	X:<ONE,X0		; Constant 1
	SUB	X0,A			; Subtract
	NOP				; pipeline restriction
	MOVE	A1,Y:<NSBINM1		; Put it in Y memory
;
; Clear out the accumulated charge from the serial shift register 
	DO      Y:<NSCLR,*+5		; Loop over number of pixels to skip
	MOVE    #<SERIAL_SKIP,R0      	; Address of serial skipping waveforms
	JSR     <CLOCK          	; Go clock out the CCD charge
	NOP                     	; Do loop restriction  


; For ROI do the parallel skipping
	MOVE	Y:<NPSKP,A		; Get number of lines to skip for each amp
	TST	A			; Is it zero?
	JEQ	<NOSKIP			; Yes, jump around the para. skipping code
	MOVE	#<PREPARE_DUMP,R0	; 
	JSR	<CLOCK			; Clock it out
	DO	A1,LPSKP		; Do loop NPSKP times
	MOVE	#<PARALLEL_DUMP,R0	; dump line
	JSR	<CLOCK			; Clock it out
	NOP
LPSKP
; Clear out the accumulated charge from the serial register again
 	MOVE    #<CLEAR_READ_REGISTER,R0	; 
 	JSR     <CLOCK  		
NOSKIP
;
; Now we're ready to read out the ROI
;
; Move the number of binned lines to shift into A
;	MOVE	#0,B			; just for counting the amount of parallele clocks
;	NOP
;	MOVE	B,Y:<NPTST
;	NOP
;	MOVE	B,Y:<NSTST
;	NOP
	MOVE	Y:<NPR,A
	NOP
;
	DO      A1,LPR			; Number of rows to shift 
;	MOVE	Y:<NPTST,B
;	ADD	#1,B
;	NOP
;	MOVE	B,Y:<NPTST
;	NOP

	DO	Y:<NPBIN,LPBIN		; Parallel binning factor NBPIN
	MOVE    #<PARALLEL_SHIFT,R0	; Parallel shift waveform
	JSR     <CLOCK  		; Clock the parallel transfer
	NOP
LPBIN	

; Check for a command once per line. Only the ABORT command should be issued.
	MOVE	#<COM_BUF,R3
	JSR	<GET_RCV		; Was a command received?
	JCC	<CONT_RD		; If no, continue reading out
	JMP	<CHK_ABORT_COMMAND	; If yes, see if its an abort command

; Abort the readout currently underway
ABR_RDC	JCLR	#ST_RDC,X:<STATUS,ABORT_EXPOSURE
	ENDDO				; Properly terminate readout loop
	JMP	<RDCCD_END_ABORT
;
; The following procedure reads out one serial row of pixels
;    1.  The prescan (underscan) pixels:  shift, read and transmit
;    2.  NSSKP pixels:  skip.  This gets us to the beginning of ROI
;    3.  NSRD pixels:  Shift, read and transmit.  This is the ROI
;    4.  NSSKP2 pixels:  skip.  This gets us to the overscan pixels
;    5.  NSOCK pixels:  Shift, read and transmit.  This is the overscan.
;
;
; First check if this is a serial ROI readout
;
CONT_RD
;	MOVE	#50,A			; prescan pixels
;	JSR	<SSKIP			; skip all prescan pixels
;	NOP

	MOVE	Y:<NSSKP,A		; Number of serial skips to A
	TST	A			; zero?
	JEQ	NOPRESKP		; No predata skips (prescan)
	JMP	ROI			; there is a ROI
NOPRESKP
	MOVE	Y:<NSSKP2,A		; number of serial skips to overs
	TST	A
	JEQ	NOROI			; no predata skips, no postdataskips
					; ==> NO ROI
;
; This is a serial ROI read so here we go
; The (binned) pre pixels first
;
ROI
	MOVE	Y:<NSUND,A		; Number of underscan pixels
	JSR	<SREAD			; Binned read subroutine
	NOP
	
;
; Now Skip to ROI
;
	MOVE	Y:<NSSKP,A		; Number of pixels to ROI
	JSR	<SSKIP			; Skip
	NOP
;
; Now the ROI read
;
	MOVE	Y:<NSRD,A		; Number of pixels in ROI
	JSR	<SREAD			; Binned read sub
	NOP
;
; Now skip to overscan pixels
;
	MOVE	Y:<NSSKP2,A		; Number of pixels to end of CCD
	JSR	<SSKIP			; Skip subroutine	
	NOP
;
; Lastly we read the overscan pixels
;
	MOVE	Y:<NSOCK,A		; Number of (binned) overscan pixels
	JSR	<SREAD
	NOP
; Done
	JMP	ENDLINE			; End of one par-ser cycle 
;
NOROI
	MOVE	Y:<NSR,A		; Number of (binned) serials 
	JSR	<SREAD			; serial read subroutine
ENDLINE
	NOP
LPR					; End of parallel loop
;
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
;
; Subroutine.
; This is where we do the serial binning.  Use the SERIAL_SHIFT
; waveform for NSBIN-1 reps then the last rep with SERIAL_READ which
; includes video processing.
;
SREAD
	MOVE	A1,Y:<NP2READ		; Number of pixels to read
	MOVE	Y:<NSBINM1,A		; binning factor -1 to accumulator
	TST	A			;
	JEQ	<NOTBIN			; if zero jump to no-binning code
;
	DO	Y:<NP2READ,LSR1
;
 	DO	Y:<NSBINM1,LSBIN	; Loop over nsbin-1 pixels
 	MOVE	#<SERIAL_SHIFT,R0	; Pointer to serial shift waveform
 	JSR	<CLOCK			; Clock out the waveform
 	NOP
LSBIN
 	MOVE	#<SERIAL_READ,R0	; Last rep is shift + video proc.
 	JSR	<CLOCK			; Clock it out
 	NOP
	NOP
LSR1
 	JMP	<NEXTROW		; Skip non-binning code.
NOTBIN
	DO	Y:<NP2READ,LSR2		; # pixels to read 
;	MOVE	Y:<NSTST,B		; just to count the amount of serial pixels
;	ADD	#1,B
; 	NOP
; 	MOVE	B,Y:<NSTST
; 	NOP
 	MOVE	#<SERIAL_READ,R0	; Waveform table starting address
 	JSR     <CLOCK  		; Go clock out the CCD charge
 	NOP
LSR2

NEXTROW
	RTS				; Return from subroutine
;
; Serial skipping subroutine.  Number of pixels to skip -> R0
; The binning appears here only to make up the total amount of skips, since
; the skip calculation is based upon binned detector coordinates
; Note that the waveform called inside the binning loop is the same skip
; routine
SSKIP
	MOVE	A1,Y:<NP2READ		; Number of skip pixels
	MOVE	Y:<NSBINM1,A		; binning factor -1 to accumulator
	TST	A			;
	JEQ	<NOTBINSKP			; if zero jump to no-binning code
	DO	Y:<NP2READ,LSK		; Do number of pixels to skip
;
	DO      Y:<NSBINM1,LSBIN2
	MOVE	#<SERIAL_SKIP,R0	; serial skip waveform address
	JSR	<CLOCK			; Clock it to the hardware
	NOP
LSBIN2
	MOVE	#<SERIAL_SKIP,R0	; serial skip waveform address
	JSR	<CLOCK			; Clock it to the hardware
	NOP				; do loop restriction
LSK
	JMP	<LKS2
NOTBINSKP
	DO	Y:<NP2READ,LKS2		; # pixels to read 
	MOVE	#<SERIAL_SKIP,R0	; serial skip waveform address
 	JSR     <CLOCK  		; Go clock out the CCD charge
 	NOP
LKS2
	RTS				; Return from subroutine
;
; ******  Minclude many routines not directly needed for readout  *******
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
	DC	'RDC',STR_RDC   
	DC	'CLR',CLEAR   
	DC	'PSH',P_SHIFT   
	DC	'BIN',SBINN  
	DC	'GEO',SET_GEOMETRY  
	DC	'ROI',SET_ROI 

; Exposure and readout control routines
	DC	'SET',SET_EXPOSURE_TIME
	DC	'RET',READ_EXPOSURE_TIME
	DC	'SEX',START_EXPOSURE
	DC	'PEX',PAUSE_EXPOSURE
	DC	'REX',RESUME_EXPOSURE
	DC	'AEX',ABORT_EXPOSURE
	DC	'ABR',ABR_RDC
	DC	'CRD',CONT_RD

; Support routines
	DC	'SGN',ST_GAIN
	DC	'SDC',SET_DC
	DC	'SBN',SET_BIAS_NUMBER
	DC	'SMX',SET_MUX
	DC	'CSW',CLR_SWS
	DC	'RCC',READ_CONTROLLER_CONFIGURATION

END_APPLICATON_COMMAND_TABLE	EQU	@LCV(L)

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

NSR	DC	2136		; number of serial transfers
NPR	DC	2048		; number of parallel transfers 
NS_DEL	DC	60
NPCLR   DC      NP_CLR    	; To clear the parallels
NSCLR	DC      NS_CLR  	; To clear the serial register
NSBIN   DC      1       	; Serial binning parameter
NPBIN   DC      1       	; Parallel binning parameter
TST_DAT	DC	0		; Temporary definition for test images
SHDEL	DC	SH_DEL		; Delay in milliseconds between shutter closing 
				;   and image readout
CONFIG	DC	CC		; Controller configuration
NPSHF	DC	64		; default # of parallels to shift w/ PSH command.
NSBINM1	DC	0		; Serial binning factor minus 1
VERSION DC	$00008C		; Version number of this code. (0x8C==140=>1.4)
NPSKP	DC	0		; number of lines to skip to get to ROI
NSUND	DC	24		; number of underscan (prescan) pixels
NSSKP	DC	0		; number of pixels to skip to get to ROI
NSRD	DC	2048		; number of pixels to read in the ROI
NSSKP2	DC	0		; number of pixels to skip to get to overscan
NSOCK	DC	64		; number of overscan (bias) pixels
NP2READ	DC	0		; number of overscan (bias) pixels
NSDATA	DC	2048		; number of data (bias) pixels
NSTST	DC	0		; number of data (bias) pixels
NPTST	DC	0		; number of data (bias) pixels
;
; Note:  NSR = 2048 + NSUND (default=24) + NSOCK (default=64)
;

; Include the waveform table for the designated type of CCD
	INCLUDE "CCD44-82.waveforms" ; Readout and clocking waveform file

END_APPLICATON_Y_MEMORY	EQU	@LCV(L)

; End of program
	END
