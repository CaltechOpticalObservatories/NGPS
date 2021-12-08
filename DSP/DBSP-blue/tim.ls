Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 1



1                                 COMMENT *
2                                  This file is used to generate boot DSP code for
3                                  the 250 MHz fiber optic timing board, SDSU GenII TIMII,
4                                  processor = DSP56303.
5      
6                          Revision History:
7                          --  0.00: 10 Jun 2004 - CRO.
8                                  Original code from Dani Guzman.
9      
10                         --  1.10: 11 Jun 2004 - CRO.
11                                 a.) Cleanup.  Some spanish-english comment translation.
12                                 b.) Y:00000B memory location reserved for NPSHF, the number
13                                     of rows to parallel shift for the PSH command (which
14                                     is new).  This is implimented as part of a focus loop
15                                     in which a series of exposures are made at different
16                                     telescope (or spectrograph) focus positions and the CCD
17                                     charge is shifted some number of rows (NPSHF) between
18                                     each exposure.  At the end of the loop an image with a
19                                     sequence of star images at different telescope focus
20                                     positions is read out.
21     
22                         --  1.20:  23 Jun 2004 - CRO
23                                 a.) Added nested do loops in RDCCD for parallel and serial
24                                     binning.  Added NSBINM1 to Y: memory.  This is the
25                                     serial binning factor minus one, used because he serial
26                                     binning and readout is split into two parts, one
27                                     for serial shift only and the last for a serial shift
28                                     and video processing.  So, the first part is executed
29                                     NSBINM1 times; the last part, once.
30     
31                         --  1.30:  24 Jun 2004 - CRO
32                                 a.) Added version number to Y memory so that it is possible
33                                     to check which version of code is downloaded into
34                                     the timing board DSP by reading that memory location
35                                     Y:00000D.
36                                 b.) Added NPSKP (Y:00000E) and NSSKP (Y:00000F) to Y memory,
37                                     the number of parallel and serial skips for ROI readout.
38                                 c.) Added ROI parallel skipping code just before the start
39                                     of the binned readout.
40     
41                                     In order to specify the number of parallel skips,
42                                     the program looks at the NPSKP value.  If it is
43                                     zero, no skips are performed.  If the NPSKP value
44                                     is greater than zero, that many parallel skips will
45                                     be performed before the code will then read out
46                                     NPR rows.
47     
48                                     So, if it is desired to read out a ROI with ystart=512
49                                     and ylength=1024, NPSKP should be set to 512 and
50                                     NPR should be set to 1024 (divided by the binning
51                                     factor, of course).
52     
53                         --  1.40  29 Jun 2004 - CRO
54                                 a.) Changed serial (binned)read to subroutine called
55                                     from main program getting the serial part of the
56                                     Region of Interest.
57                                 b.) Added subroutine for serial skips, SSKIP.
58                                 c.) To support serial ROI readout, added the following
59                                     parameters to Y: memory:
60                                         NSUND  - Number of pixels in the CCD underscan
61                                                  region.
62                                         NSSKP  - Number of pixels to skip to the ROI
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 2



63                                                  start pixel.
64                                         NSRD   - Number of pixels to read in the ROI.
65                                         NSSKP2 - Number of pixels to skip from the end.
66                                                  of the ROI to the end of the CCD.
67                                         NSOCK  - Number of pixels of overscan to read.
68     
69                                    In order to read out a region of interest (ROI)
70                                    something like the following must be done, for example:
71     
72                                         setroi xstart=512 xlen=1024 ystart=512 ylen=1024
73     
74                                    Then the ROI parameters in Y memory should be set according to the
75                                    following:
76     
77                                       NPSKP  = ystart
78                                       NPR    = ylen/(binning factor)
79                                       NSSKP  = xstart
80                                       NSRD   = xlen/(binning factor)
81                                       NSSKP2 = (CCD size in X) - (xstart + xlen)
82     
83                                    (Question:  should we just use NSR for NSRD and just get rid
84                                     of NSRD?)
85     
86                                    Also, it one wishes to change the number of post or over-
87                                    scan pixels for each row to read, one must update the
88                                    NSOCK parameter for a ROI readout to be correct.
89                                    (Probably could fix the code so that this is not
90                                    necessary.)
91                         --  1.41  10 Aug 2004 - MB
92                                    Added the variable NP2READ (NUmber of Pixels to read), which
93                                    replaces the R5 (or R0) register for storing the amount of pixels
94                                    to be read inside the SREAD and SSKIP subroutines
95                         --  1.42  16 Jan 2005 - MB
96                                    Copied to DBSP directory
97     
98                                 *
99                                   PAGE    132                               ; Printronix page width - 132 columns
100    
101                        ; Include the boot and header files so addressing is easy
102                                  INCLUDE "timhdr.asm"
103                               COMMENT *
104    
105                        This is a header file that is shared between the fiber optic timing board
106                        boot and application code files for Rev. 5 = 250 MHz timing boards
107                                *
108    
109                                  PAGE    132                               ; Printronix page width - 132 columns
110    
111                        ; Various addressing control registers
112       FFFFFB           BCR       EQU     $FFFFFB                           ; Bus Control Register
113       FFFFF9           AAR0      EQU     $FFFFF9                           ; Address Attribute Register, channel 0
114       FFFFF8           AAR1      EQU     $FFFFF8                           ; Address Attribute Register, channel 1
115       FFFFF7           AAR2      EQU     $FFFFF7                           ; Address Attribute Register, channel 2
116       FFFFF6           AAR3      EQU     $FFFFF6                           ; Address Attribute Register, channel 3
117       FFFFFD           PCTL      EQU     $FFFFFD                           ; PLL control register
118       FFFFFE           IPRP      EQU     $FFFFFE                           ; Interrupt Priority register - Peripheral
119       FFFFFF           IPRC      EQU     $FFFFFF                           ; Interrupt Priority register - Core
120    
121                        ; Port E is the Synchronous Communications Interface (SCI) port
122       FFFF9F           PCRE      EQU     $FFFF9F                           ; Port Control Register
123       FFFF9E           PRRE      EQU     $FFFF9E                           ; Port Direction Register
124       FFFF9D           PDRE      EQU     $FFFF9D                           ; Port Data Register
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timhdr.asm  Page 3



125       FFFF9C           SCR       EQU     $FFFF9C                           ; SCI Control Register
126       FFFF9B           SCCR      EQU     $FFFF9B                           ; SCI Clock Control Register
127    
128       FFFF9A           SRXH      EQU     $FFFF9A                           ; SCI Receive Data Register, High byte
129       FFFF99           SRXM      EQU     $FFFF99                           ; SCI Receive Data Register, Middle byte
130       FFFF98           SRXL      EQU     $FFFF98                           ; SCI Receive Data Register, Low byte
131    
132       FFFF97           STXH      EQU     $FFFF97                           ; SCI Transmit Data register, High byte
133       FFFF96           STXM      EQU     $FFFF96                           ; SCI Transmit Data register, Middle byte
134       FFFF95           STXL      EQU     $FFFF95                           ; SCI Transmit Data register, Low byte
135    
136       FFFF94           STXA      EQU     $FFFF94                           ; SCI Transmit Address Register
137       FFFF93           SSR       EQU     $FFFF93                           ; SCI Status Register
138    
139       000009           SCITE     EQU     9                                 ; X:SCR bit set to enable the SCI transmitter
140       000008           SCIRE     EQU     8                                 ; X:SCR bit set to enable the SCI receiver
141       000000           TRNE      EQU     0                                 ; This is set in X:SSR when the transmitter
142                                                                            ;  shift and data registers are both empty
143       000001           TDRE      EQU     1                                 ; This is set in X:SSR when the transmitter
144                                                                            ;  data register is empty
145       000002           RDRF      EQU     2                                 ; X:SSR bit set when receiver register is full
146       00000F           SELSCI    EQU     15                                ; 1 for SCI to backplane, 0 to front connector
147    
148    
149                        ; ESSI Flags
150       000006           TDE       EQU     6                                 ; Set when transmitter data register is empty
151       000007           RDF       EQU     7                                 ; Set when receiver is full of data
152       000010           TE        EQU     16                                ; Transmitter enable
153    
154                        ; Phase Locked Loop initialization
155       050003           PLL_INIT  EQU     $050003                           ; PLL = 25 MHz x 2 = 100 MHz
156    
157                        ; Port B general purpose I/O
158       FFFFC4           HPCR      EQU     $FFFFC4                           ; Control register (bits 1-6 cleared for GPIO)
159       FFFFC9           HDR       EQU     $FFFFC9                           ; Data register
160       FFFFC8           HDDR      EQU     $FFFFC8                           ; Data Direction Register bits (=1 for output)
161    
162                        ; Port C is Enhanced Synchronous Serial Port 0 = ESSI0
163       FFFFBF           PCRC      EQU     $FFFFBF                           ; Port C Control Register
164       FFFFBE           PRRC      EQU     $FFFFBE                           ; Port C Data direction Register
165       FFFFBD           PDRC      EQU     $FFFFBD                           ; Port C GPIO Data Register
166       FFFFBC           TX00      EQU     $FFFFBC                           ; Transmit Data Register #0
167       FFFFB8           RX0       EQU     $FFFFB8                           ; Receive data register
168       FFFFB7           SSISR0    EQU     $FFFFB7                           ; Status Register
169       FFFFB6           CRB0      EQU     $FFFFB6                           ; Control Register B
170       FFFFB5           CRA0      EQU     $FFFFB5                           ; Control Register A
171    
172                        ; Port D is Enhanced Synchronous Serial Port 1 = ESSI1
173       FFFFAF           PCRD      EQU     $FFFFAF                           ; Port D Control Register
174       FFFFAE           PRRD      EQU     $FFFFAE                           ; Port D Data direction Register
175       FFFFAD           PDRD      EQU     $FFFFAD                           ; Port D GPIO Data Register
176       FFFFAC           TX10      EQU     $FFFFAC                           ; Transmit Data Register 0
177       FFFFA7           SSISR1    EQU     $FFFFA7                           ; Status Register
178       FFFFA6           CRB1      EQU     $FFFFA6                           ; Control Register B
179       FFFFA5           CRA1      EQU     $FFFFA5                           ; Control Register A
180    
181                        ; Timer module addresses
182       FFFF8F           TCSR0     EQU     $FFFF8F                           ; Timer control and status register
183       FFFF8E           TLR0      EQU     $FFFF8E                           ; Timer load register = 0
184       FFFF8D           TCPR0     EQU     $FFFF8D                           ; Timer compare register = exposure time
185       FFFF8C           TCR0      EQU     $FFFF8C                           ; Timer count register = elapsed time
186       FFFF83           TPLR      EQU     $FFFF83                           ; Timer prescaler load register => milliseconds
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timhdr.asm  Page 4



187       FFFF82           TPCR      EQU     $FFFF82                           ; Timer prescaler count register
188       000000           TIM_BIT   EQU     0                                 ; Set to enable the timer
189       000009           TRM       EQU     9                                 ; Set to enable the timer preloading
190       000015           TCF       EQU     21                                ; Set when timer counter = compare register
191    
192                        ; Board specific addresses and constants
193       FFFFF1           RDFO      EQU     $FFFFF1                           ; Read incoming fiber optic data byte
194       FFFFF2           WRFO      EQU     $FFFFF2                           ; Write fiber optic data replies
195       FFFFF3           WRSS      EQU     $FFFFF3                           ; Write switch state
196       FFFFF5           WRLATCH   EQU     $FFFFF5                           ; Write to a latch
197       010000           RDAD      EQU     $010000                           ; Read A/D values into the DSP
198       000009           EF        EQU     9                                 ; Serial receiver empty flag
199    
200                        ; DSP port A bit equates
201       000000           PWROK     EQU     0                                 ; Power control board says power is OK
202       000001           LED1      EQU     1                                 ; Control one of two LEDs
203       000002           LVEN      EQU     2                                 ; Low voltage power enable
204       000003           HVEN      EQU     3                                 ; High voltage power enable
205       00000E           SSFHF     EQU     14                                ; Switch state FIFO half full flag
206       00000A           EXT_IN0   EQU     10
207       00000B           EXT_IN1   EQU     11
208       00000C           EXT_OUT0  EQU     12
209       00000D           EXT_OUT1  EQU     13
210    
211                        ; Port D equate
212       000001           SSFEF     EQU     1                                 ; Switch state FIFO empty flag
213    
214                        ; Other equates
215       000002           WRENA     EQU     2                                 ; Enable writing to the EEPROM
216    
217                        ; Latch U12 bit equates
218       000000           CDAC      EQU     0                                 ; Clear the analog board DACs
219       000002           ENCK      EQU     2                                 ; Enable the clock outputs
220       000004           SHUTTER   EQU     4                                 ; Control the shutter
221       000005           TIM_U_RST EQU     5                                 ; Reset the utility board
222    
223                        ; Software status bits, defined at X:<STATUS = X:0
224       000000           ST_RCV    EQU     0                                 ; Set to indicate word is from SCI = utility board
225       000002           IDLMODE   EQU     2                                 ; Set if need to idle after readout
226       000003           ST_SHUT   EQU     3                                 ; Set to indicate shutter is closed, clear for open
227       000004           ST_RDC    EQU     4                                 ; Set if executing 'RDC' command - reading out
228       000005           SPLIT_S   EQU     5                                 ; Set if split serial
229       000006           SPLIT_P   EQU     6                                 ; Set if split parallel
230       000007           MPP       EQU     7                                 ; Set if parallels are in MPP mode
231       000008           NOT_CLR   EQU     8                                 ; Set if not to clear CCD before exposure
232       00000A           TST_IMG   EQU     10                                ; Set if controller is to generate a test image
233       00000B           SHUT      EQU     11                                ; Set if opening shutter at beginning of exposure
234       00000C           ST_DITH   EQU     12                                ; Set if to dither during exposure
235    
236                        ; Address for the table containing the incoming SCI words
237       000400           SCI_TABLE EQU     $400
238    
239    
240                        ; Specify controller configuration bits of the X:STATUS word
241                        ;   to describe the software capabilities of this application file
242                        ; The bit is set (=1) if the capability is supported by the controller
243    
244    
245                                COMMENT *
246    
247                        BIT #'s         FUNCTION
248                        2,1,0           Video Processor
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timhdr.asm  Page 5



249                                                000     CCD Rev. 3
250                                                001     CCD Gen I
251                                                010     IR Rev. 4
252                                                011     IR Coadder
253                                                100     CCD Rev. 5, Differential input
254                                                101     8x IR
255    
256                        4,3             Timing Board
257                                                00      Rev. 4, Gen II
258                                                01      Gen I
259                                                10      Rev. 5, Gen III, 250 MHz
260    
261                        6,5             Utility Board
262                                                00      No utility board
263                                                01      Utility Rev. 3
264    
265                        7               Shutter
266                                                0       No shutter support
267                                                1       Yes shutter support
268    
269                        9,8             Temperature readout
270                                                00      No temperature readout
271                                                01      Polynomial Diode calibration
272                                                10      Linear temperature sensor calibration
273    
274                        10              Subarray readout
275                                                0       Not supported
276                                                1       Yes supported
277    
278                        11              Binning
279                                                0       Not supported
280                                                1       Yes supported
281    
282                        12              Split-Serial readout
283                                                0       Not supported
284                                                1       Yes supported
285    
286                        13              Split-Parallel readout
287                                                0       Not supported
288                                                1       Yes supported
289    
290                        14              MPP = Inverted parallel clocks
291                                                0       Not supported
292                                                1       Yes supported
293    
294                        16,15           Clock Driver Board
295                                                00      Rev. 3
296                                                11      No clock driver board (Gen I)
297    
298                        19,18,17                Special implementations
299                                                000     Somewhere else
300                                                001     Mount Laguna Observatory
301                                                010     NGST Aladdin
302                                                xxx     Other
303                                *
304    
305                        CCDVIDREV3B
306       000000                     EQU     $000000                           ; CCD Video Processor Rev. 3
307       000001           VIDGENI   EQU     $000001                           ; CCD Video Processor Gen I
308       000002           IRREV4    EQU     $000002                           ; IR Video Processor Rev. 4
309       000003           COADDER   EQU     $000003                           ; IR Coadder
310       000004           CCDVIDREV5 EQU    $000004                           ; Differential input CCD video Rev. 5
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timhdr.asm  Page 6



311       000000           TIMREV4   EQU     $000000                           ; Timing Revision 4 = 50 MHz
312       000008           TIMGENI   EQU     $000008                           ; Timing Gen I = 40 MHz
313       000010           TIMREV5   EQU     $000010                           ; Timing Revision 5 = 250 MHz
314       000020           UTILREV3  EQU     $000020                           ; Utility Rev. 3 supported
315       000080           SHUTTER_CC EQU    $000080                           ; Shutter supported
316       000100           TEMP_POLY EQU     $000100                           ; Polynomial calibration
317                        TEMP_LINEAR
318       000200                     EQU     $000200                           ; Linear calibration
319       000400           SUBARRAY  EQU     $000400                           ; Subarray readout supported
320       000800           BINNING   EQU     $000800                           ; Binning supported
321                        SPLIT_SERIAL
322       001000                     EQU     $001000                           ; Split serial supported
323                        SPLIT_PARALLEL
324       002000                     EQU     $002000                           ; Split parallel supported
325       004000           MPP_CC    EQU     $004000                           ; Inverted clocks supported
326       018000           CLKDRVGENI EQU    $018000                           ; No clock driver board - Gen I
327       020000           MLO       EQU     $020000                           ; Set if Mount Laguna Observatory
328       040000           NGST      EQU     $040000                           ; NGST Aladdin implementation
329                                  INCLUDE "timboot.asm"
330                               COMMENT *
331    
332                        This file is used to generate boot DSP code for the 250 MHz fiber optic
333                                timing board using a DSP56303 as its main processor.
334                        This does NOT support utility board operation
335                                *
336                                  PAGE    132                               ; Printronix page width - 132 columns
337    
338                        ; Special address for two words for the DSP to bootstrap code from the EEPROM
339                                  IF      @SCP("HOST","ROM")
346                                  ENDIF
347    
348                                  IF      @SCP("HOST","HOST")
349       P:000000 P:000000                   ORG     P:0,P:0
350       P:000000 P:000000 0C0115            JMP     <INIT
351       P:000001 P:000001 000000            NOP
352                                           ENDIF
353    
354                                 ; *******************  Command Processing  ******************
355    
356                                 ; Read the header and check it for self-consistency
357       P:000002 P:000002 609E00  START     MOVE              X:<IDL_ADR,R0
358       P:000003 P:000003 018FA0            JSET    #TIM_BIT,X:TCSR0,EXPOSING         ; If exposing go check timer
                            0002D3
359       P:000005 P:000005 0A00A4            JSET    #ST_RDC,X:<STATUS,CONTINUE_READING
                            0001D0
360       P:000007 P:000007 0AE080            JMP     (R0)
361    
362       P:000008 P:000008 330700  TST_RCV   MOVE              #<COM_BUF,R3
363       P:000009 P:000009 0D003D            JSR     <GET_RCV                          ; Read the header word into X:(R3)
364       P:00000A P:00000A 0E0009            JCC     *-1
365       P:00000B P:00000B 03FBD6  PRC_RCV   MOVE              X:(R3-1),Y0             ; Put the header into Y0
366       P:00000C P:00000C 0D007F            JSR     <CHK_HDR                          ; Update HEADER and NWORDS
367       P:00000D P:00000D 0E8028            JCS     <ERROR
368    
369                                 ; Read all the remaining words in the command
370       P:00000E P:00000E 578600            MOVE              X:<NWORDS,B
371       P:00000F P:00000F 000000            NOP
372       P:000010 P:000010 01418C            SUB     #1,B                              ; We've already got the header
373       P:000011 P:000011 000000            NOP
374       P:000012 P:000012 06CF00            DO      B,L_GET_COMMAND                   ; Later - put a TIMEOUT here
                            000016
375       P:000014 P:000014 0D003D            JSR     <GET_RCV
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 7



376       P:000015 P:000015 0E0014            JCC     *-1
377       P:000016 P:000016 000000            NOP
378                                 L_GET_COMMAND
379    
380                                 ; Process the receiver entry - is it in the command table ?
381       P:000017 P:000017 63F400  COMMAND   MOVE              #COM_BUF,R3
                            000007
382       P:000019 P:000019 000000            NOP
383       P:00001A P:00001A 000000            NOP
384       P:00001B P:00001B 0203DF            MOVE              X:(R3+1),B              ; Get the command
385       P:00001C P:00001C 205B00            MOVE              (R3)+
386       P:00001D P:00001D 205B00            MOVE              (R3)+                   ; Point R3 to the first argument
387       P:00001E P:00001E 302200            MOVE              #<COM_TBL,R0            ; Get the command table starting address
388       P:00001F P:00001F 062180            DO      #NUM_COM,END_COM                  ; Loop over the command table
                            000026
389       P:000021 P:000021 47D800            MOVE              X:(R0)+,Y1              ; Get the command table entry
390       P:000022 P:000022 62E07D            CMP     Y1,B      X:(R0),R2               ; Does receiver = table entries address?
391       P:000023 P:000023 0E2026            JNE     <NOT_COM                          ; No, keep looping
392       P:000024 P:000024 00008C            ENDDO                                     ; Restore the DO loop system registers
393       P:000025 P:000025 0AE280            JMP     (R2)                              ; Jump execution to the command
394       P:000026 P:000026 205800  NOT_COM   MOVE              (R0)+                   ; Increment the register past the table addr
ess
395                                 END_COM
396       P:000027 P:000027 0C0028            JMP     <ERROR                            ; The command is not in the table
397    
398                                 ; It's not in the command table - send an error message
399       P:000028 P:000028 479C00  ERROR     MOVE              X:<ERR,Y1               ; Send the message - there was an error
400       P:000029 P:000029 0C002B            JMP     <FINISH1                          ; This protects against unknown commands
401    
402                                 ; Send a reply packet - header and reply
403       P:00002A P:00002A 479700  FINISH    MOVE              X:<DONE,Y1              ; Send 'DON' as the reply
404       P:00002B P:00002B 578500  FINISH1   MOVE              X:<HEADER,B             ; Get header of incoming command
405       P:00002C P:00002C 469B00            MOVE              X:<SMASK,Y0             ; This was the source byte, and is to
406       P:00002D P:00002D 330700            MOVE              #<COM_BUF,R3            ;     become the destination byte
407       P:00002E P:00002E 46925E            AND     Y0,B      X:<TWO,Y0
408       P:00002F P:00002F 0C1ED1            LSR     #8,B                              ; Shift right eight bytes, add it to the
409       P:000030 P:000030 460600            MOVE              Y0,X:<NWORDS            ;     header, and put 2 as the number
410       P:000031 P:000031 469858            ADD     Y0,B      X:<SBRD,Y0              ;     of words in the string
411       P:000032 P:000032 200058            ADD     Y0,B                              ; Add source board's header, set Y1 for abov
e
412       P:000033 P:000033 000000            NOP
413       P:000034 P:000034 575B00            MOVE              B,X:(R3)+               ; Put the new header on the transmitter stac
k
414       P:000035 P:000035 475B00            MOVE              Y1,X:(R3)+              ; Put the value of XO on the transmitter sta
ck
415    
416                                 ; Transmit words to the host computer over the fiber optics link
417       P:000036 P:000036 330700            MOVE              #<COM_BUF,R3
418       P:000037 P:000037 060600            DO      X:<NWORDS,DON_FFO                 ; Transmit all the words in the command
                            00003B
419       P:000039 P:000039 57DB00            MOVE              X:(R3)+,B
420       P:00003A P:00003A 0D006B            JSR     <XMT_WRD
421       P:00003B P:00003B 000000            NOP
422       P:00003C P:00003C 0C0002  DON_FFO   JMP     <START
423    
424                                 ; Read one 32-bit word from the FIFO and put it into B1
425       P:00003D P:00003D 0A8989  GET_RCV   JCLR    #EF,X:HDR,CLR_RTS
                            000067
426    
427                                 ; Because of FIFO metastability require that EF be stable for two tests
428       P:00003F P:00003F 0A8989  TST1      JCLR    #EF,X:HDR,TST2                    ; EF = Low,  Low  => CLR_RTS
                            000042
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 8



429       P:000041 P:000041 0C0045            JMP     <TST3                             ;      High, Low  => try again
430       P:000042 P:000042 0A8989  TST2      JCLR    #EF,X:HDR,CLR_RTS                 ;      Low,  High => try again
                            000067
431       P:000044 P:000044 0C003F            JMP     <TST1                             ;      High, High => read FIFO
432       P:000045 P:000045 0A8989  TST3      JCLR    #EF,X:HDR,TST1
                            00003F
433    
434       P:000047 P:000047 08F4BB            MOVEP             #$028FE2,X:BCR          ; Slow down RDFO access
                            028FE2
435       P:000049 P:000049 000000            NOP
436       P:00004A P:00004A 000000            NOP
437       P:00004B P:00004B 5FF000            MOVE                          Y:RDFO,B
                            FFFFF1
438       P:00004D P:00004D 2B0000            MOVE              #0,B2
439       P:00004E P:00004E 0140CE            AND     #$FF,B
                            0000FF
440       P:000050 P:000050 0140CD            CMP     #>$AC,B                           ; It must be $AC to be a valid word
                            0000AC
441       P:000052 P:000052 0E2067            JNE     <CLR_RTS
442       P:000053 P:000053 4EF000            MOVE                          Y:RDFO,Y0   ; Read the MS byte
                            FFFFF1
443       P:000055 P:000055 0C1951            INSERT  #$008010,Y0,B
                            008010
444       P:000057 P:000057 4EF000            MOVE                          Y:RDFO,Y0   ; Read the middle byte
                            FFFFF1
445       P:000059 P:000059 0C1951            INSERT  #$008008,Y0,B
                            008008
446       P:00005B P:00005B 4EF000            MOVE                          Y:RDFO,Y0   ; Read the LS byte
                            FFFFF1
447       P:00005D P:00005D 0C1951            INSERT  #$008000,Y0,B
                            008000
448       P:00005F P:00005F 000000            NOP
449       P:000060 P:000060 515B00            MOVE              B0,X:(R3)+              ; Put the word into COM_BUF
450       P:000061 P:000061 000000            NOP
451       P:000062 P:000062 000000            NOP
452       P:000063 P:000063 0AF960            BSET    #0,SR                             ; Valid FIFO word => SR carry = 1
453       P:000064 P:000064 08F4BB            MOVEP             #$028FE1,X:BCR          ; Restore RDFO access
                            028FE1
454       P:000066 P:000066 00000C            RTS
455       P:000067 P:000067 0AF940  CLR_RTS   BCLR    #0,SR                             ; Not valid FIFO word => SR carry = 0
456       P:000068 P:000068 08F4BB            MOVEP             #$028FE1,X:BCR          ; Restore RDFO access
                            028FE1
457       P:00006A P:00006A 00000C            RTS
458    
459                                 ; Transmit the word in B1 to the host computer
460       P:00006B P:00006B 08F4BB  XMT_WRD   MOVEP             #$028FE2,X:BCR          ; Slow down RDFO access
                            028FE2
461       P:00006D P:00006D 60F400            MOVE              #FO_HDR+1,R0
                            000002
462       P:00006F P:00006F 060380            DO      #3,XMT_WRD1
                            000073
463       P:000071 P:000071 0C1D91            ASL     #8,B,B
464       P:000072 P:000072 000000            NOP
465       P:000073 P:000073 535800            MOVE              B2,X:(R0)+
466                                 XMT_WRD1
467       P:000074 P:000074 60F400            MOVE              #FO_HDR,R0
                            000001
468       P:000076 P:000076 61F400            MOVE              #WRFO,R1
                            FFFFF2
469       P:000078 P:000078 060480            DO      #4,XMT_WRD2
                            00007B
470       P:00007A P:00007A 46D800            MOVE              X:(R0)+,Y0              ; Should be MOVEP  X:(R0)+,Y:WRFO
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 9



471       P:00007B P:00007B 4E6100            MOVE                          Y0,Y:(R1)
472                                 XMT_WRD2
473       P:00007C P:00007C 08F4BB            MOVEP             #$028FE1,X:BCR          ; Restore RDFO access
                            028FE1
474       P:00007E P:00007E 00000C            RTS
475    
476                                 ; Check the command or reply header in Y0 for self-consistency
477       P:00007F P:00007F 579500  CHK_HDR   MOVE              X:<MASK1,B              ; Test for S.LE.3 and D.LE.3 and N.LE.7
478       P:000080 P:000080 20005E            AND     Y0,B
479       P:000081 P:000081 0E208F            JNE     <CHK_ERR                          ; Test failed
480       P:000082 P:000082 579600            MOVE              X:<MASK2,B              ; Test for either S.NE.0 or D.NE.0
481       P:000083 P:000083 20005E            AND     Y0,B
482       P:000084 P:000084 0EA08F            JEQ     <CHK_ERR                          ; Test failed
483       P:000085 P:000085 579400            MOVE              X:<SEVEN,B
484       P:000086 P:000086 20005E            AND     Y0,B                              ; Extract NWORDS, must be > 0
485       P:000087 P:000087 0EA08F            JEQ     <CHK_ERR
486       P:000088 P:000088 460500            MOVE              Y0,X:<HEADER
487       P:000089 P:000089 550600            MOVE              B1,X:<NWORDS            ; Number of words in the command
488       P:00008A P:00008A 000000            NOP
489       P:00008B P:00008B 000000            NOP
490       P:00008C P:00008C 0AF940            BCLR    #0,SR                             ; Check OK - header is self-consistent
491       P:00008D P:00008D 000000            NOP
492       P:00008E P:00008E 00000C            RTS
493    
494                                 ; CHK_HDR error return
495       P:00008F P:00008F 0AF960  CHK_ERR   BSET    #0,SR
496       P:000090 P:000090 000000            NOP                                       ; SR restriction
497       P:000091 P:000091 00000C            RTS
498    
499                                 ;  *****************  Boot Commands  *******************
500                                 ; Test Data Link - simply return value received after 'TDL'
501       P:000092 P:000092 47DB00  TDL       MOVE              X:(R3)+,Y1              ; Get the data value
502       P:000093 P:000093 0C002B            JMP     <FINISH1                          ; Return from executing TDL command
503    
504                                 ; Read DSP or EEPROM memory ('RDM' address): read memory, reply with value
505       P:000094 P:000094 47DB00  RDMEM     MOVE              X:(R3)+,Y1
506       P:000095 P:000095 20EF00            MOVE              Y1,B
507       P:000096 P:000096 0140CE            AND     #$0FFFFF,B                        ; Bits 23-20 need to be zeroed
                            0FFFFF
508       P:000098 P:000098 21B000            MOVE              B1,R0                   ; Need the address in an address register
509       P:000099 P:000099 20EF00            MOVE              Y1,B
510       P:00009A P:00009A 000000            NOP
511       P:00009B P:00009B 0ACF14            JCLR    #20,B,RDX                         ; Test address bit for Program memory
                            00009F
512       P:00009D P:00009D 07E087            MOVE              P:(R0),Y1               ; Read from Program Memory
513       P:00009E P:00009E 0C002B            JMP     <FINISH1                          ; Send out a header with the value
514       P:00009F P:00009F 0ACF15  RDX       JCLR    #21,B,RDY                         ; Test address bit for X: memory
                            0000A3
515       P:0000A1 P:0000A1 47E000            MOVE              X:(R0),Y1               ; Write to X data memory
516       P:0000A2 P:0000A2 0C002B            JMP     <FINISH1                          ; Send out a header with the value
517       P:0000A3 P:0000A3 0ACF16  RDY       JCLR    #22,B,RDR                         ; Test address bit for Y: memory
                            0000A7
518       P:0000A5 P:0000A5 4FE000            MOVE                          Y:(R0),Y1   ; Read from Y data memory
519       P:0000A6 P:0000A6 0C002B            JMP     <FINISH1                          ; Send out a header with the value
520       P:0000A7 P:0000A7 0ACF17  RDR       JCLR    #23,B,ERROR                       ; Test address bit for read from EEPROM memo
ry
                            000028
521       P:0000A9 P:0000A9 479300            MOVE              X:<THREE,Y1             ; Convert to word address to a byte address
522       P:0000AA P:0000AA 220600            MOVE              R0,Y0                   ; Get 16-bit address in a data register
523       P:0000AB P:0000AB 2000B8            MPY     Y0,Y1,B                           ; Multiply
524       P:0000AC P:0000AC 20002A            ASR     B                                 ; Eliminate zero fill of fractional multiply
525       P:0000AD P:0000AD 213000            MOVE              B0,R0                   ; Need to address memory
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 10



526       P:0000AE P:0000AE 0AD06F            BSET    #15,R0                            ; Set bit so its in EEPROM space
527       P:0000AF P:0000AF 0D00FD            JSR     <RD_WORD                          ; Read word from EEPROM
528       P:0000B0 P:0000B0 21A700            MOVE              B1,Y1                   ; FINISH1 transmits Y1 as its reply
529       P:0000B1 P:0000B1 0C002B            JMP     <FINISH1
530    
531                                 ; Program WRMEM ('WRM' address datum): write to memory, reply 'DON'.
532       P:0000B2 P:0000B2 47DB00  WRMEM     MOVE              X:(R3)+,Y1              ; Get the address to be written to
533       P:0000B3 P:0000B3 20EF00            MOVE              Y1,B
534       P:0000B4 P:0000B4 0140CE            AND     #$0FFFFF,B                        ; Bits 23-20 need to be zeroed
                            0FFFFF
535       P:0000B6 P:0000B6 21B000            MOVE              B1,R0                   ; Need the address in an address register
536       P:0000B7 P:0000B7 20EF00            MOVE              Y1,B
537       P:0000B8 P:0000B8 46DB00            MOVE              X:(R3)+,Y0              ; Get datum into Y0 so MOVE works easily
538       P:0000B9 P:0000B9 0ACF14            JCLR    #20,B,WRX                         ; Test address bit for Program memory
                            0000BD
539       P:0000BB P:0000BB 076086            MOVE              Y0,P:(R0)               ; Write to Program memory
540       P:0000BC P:0000BC 0C002A            JMP     <FINISH
541       P:0000BD P:0000BD 0ACF15  WRX       JCLR    #21,B,WRY                         ; Test address bit for X: memory
                            0000C1
542       P:0000BF P:0000BF 466000            MOVE              Y0,X:(R0)               ; Write to X: memory
543       P:0000C0 P:0000C0 0C002A            JMP     <FINISH
544       P:0000C1 P:0000C1 0ACF16  WRY       JCLR    #22,B,WRR                         ; Test address bit for Y: memory
                            0000C5
545       P:0000C3 P:0000C3 4E6000            MOVE                          Y0,Y:(R0)   ; Write to Y: memory
546       P:0000C4 P:0000C4 0C002A            JMP     <FINISH
547       P:0000C5 P:0000C5 0ACF17  WRR       JCLR    #23,B,ERROR                       ; Test address bit for write to EEPROM
                            000028
548       P:0000C7 P:0000C7 013D02            BCLR    #WRENA,X:PDRC                     ; WR_ENA* = 0 to enable EEPROM writing
549       P:0000C8 P:0000C8 460E00            MOVE              Y0,X:<SV_A1             ; Save the datum to be written
550       P:0000C9 P:0000C9 479300            MOVE              X:<THREE,Y1             ; Convert word address to a byte address
551       P:0000CA P:0000CA 220600            MOVE              R0,Y0                   ; Get 16-bit address in a data register
552       P:0000CB P:0000CB 2000B8            MPY     Y1,Y0,B                           ; Multiply
553       P:0000CC P:0000CC 20002A            ASR     B                                 ; Eliminate zero fill of fractional multiply
554       P:0000CD P:0000CD 213000            MOVE              B0,R0                   ; Need to address memory
555       P:0000CE P:0000CE 0AD06F            BSET    #15,R0                            ; Set bit so its in EEPROM space
556       P:0000CF P:0000CF 558E00            MOVE              X:<SV_A1,B1             ; Get the datum to be written
557       P:0000D0 P:0000D0 060380            DO      #3,L1WRR                          ; Loop over three bytes of the word
                            0000D9
558       P:0000D2 P:0000D2 07588D            MOVE              B1,P:(R0)+              ; Write each EEPROM byte
559       P:0000D3 P:0000D3 0C1C91            ASR     #8,B,B
560       P:0000D4 P:0000D4 469D00            MOVE              X:<C100K,Y0             ; Move right one byte, enter delay = 1 msec
561       P:0000D5 P:0000D5 06C600            DO      Y0,L2WRR                          ; Delay by 12 milliseconds for EEPROM write
                            0000D8
562       P:0000D7 P:0000D7 060CA0            REP     #12                               ; Assume 100 MHz DSP56303
563       P:0000D8 P:0000D8 000000            NOP
564                                 L2WRR
565       P:0000D9 P:0000D9 000000            NOP                                       ; DO loop nesting restriction
566                                 L1WRR
567       P:0000DA P:0000DA 013D22            BSET    #WRENA,X:PDRC                     ; WR_ENA* = 1 to disable EEPROM writing
568       P:0000DB P:0000DB 0C002A            JMP     <FINISH
569    
570                                 ; Load application code from P: memory into its proper locations
571       P:0000DC P:0000DC 47DB00  LDAPPL    MOVE              X:(R3)+,Y1              ; Application number, not used yet
572       P:0000DD P:0000DD 0D00DF            JSR     <LOAD_APPLICATION
573       P:0000DE P:0000DE 0C002A            JMP     <FINISH
574    
575                                 LOAD_APPLICATION
576       P:0000DF P:0000DF 60F400            MOVE              #$8000,R0               ; Starting EEPROM address
                            008000
577       P:0000E1 P:0000E1 0D00FD            JSR     <RD_WORD                          ; Number of words in boot code
578       P:0000E2 P:0000E2 21A600            MOVE              B1,Y0
579       P:0000E3 P:0000E3 479300            MOVE              X:<THREE,Y1
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 11



580       P:0000E4 P:0000E4 2000B8            MPY     Y0,Y1,B
581       P:0000E5 P:0000E5 20002A            ASR     B
582       P:0000E6 P:0000E6 213000            MOVE              B0,R0                   ; EEPROM address of start of P: application
583       P:0000E7 P:0000E7 0AD06F            BSET    #15,R0                            ; To access EEPROM
584       P:0000E8 P:0000E8 0D00FD            JSR     <RD_WORD                          ; Read number of words in application P:
585       P:0000E9 P:0000E9 61F400            MOVE              #(X_BOOT_START+1),R1    ; End of boot P: code that needs keeping
                            000193
586       P:0000EB P:0000EB 06CD00            DO      B1,RD_APPL_P
                            0000EE
587       P:0000ED P:0000ED 0D00FD            JSR     <RD_WORD
588       P:0000EE P:0000EE 07598D            MOVE              B1,P:(R1)+
589                                 RD_APPL_P
590       P:0000EF P:0000EF 0D00FD            JSR     <RD_WORD                          ; Read number of words in application X:
591       P:0000F0 P:0000F0 61F400            MOVE              #END_COMMAND_TABLE,R1
                            000030
592       P:0000F2 P:0000F2 06CD00            DO      B1,RD_APPL_X
                            0000F5
593       P:0000F4 P:0000F4 0D00FD            JSR     <RD_WORD
594       P:0000F5 P:0000F5 555900            MOVE              B1,X:(R1)+
595                                 RD_APPL_X
596       P:0000F6 P:0000F6 0D00FD            JSR     <RD_WORD                          ; Read number of words in application Y:
597       P:0000F7 P:0000F7 310100            MOVE              #1,R1                   ; There is no Y: memory in the boot code
598       P:0000F8 P:0000F8 06CD00            DO      B1,RD_APPL_Y
                            0000FB
599       P:0000FA P:0000FA 0D00FD            JSR     <RD_WORD
600       P:0000FB P:0000FB 5D5900            MOVE                          B1,Y:(R1)+
601                                 RD_APPL_Y
602       P:0000FC P:0000FC 00000C            RTS
603    
604                                 ; Read one word from EEPROM location R0 into accumulator B1
605       P:0000FD P:0000FD 060380  RD_WORD   DO      #3,L_RDBYTE
                            000100
606       P:0000FF P:0000FF 07D88B            MOVE              P:(R0)+,B2
607       P:000100 P:000100 0C1C91            ASR     #8,B,B
608                                 L_RDBYTE
609       P:000101 P:000101 00000C            RTS
610    
611                                 ; Come to here on a 'STP' command so 'DON' can be sent
612                                 STOP_IDLE_CLOCKING
613       P:000102 P:000102 300800            MOVE              #<TST_RCV,R0            ; Execution address when idle => when not
614       P:000103 P:000103 601E00            MOVE              R0,X:<IDL_ADR           ;   processing commands or reading out
615       P:000104 P:000104 0A0002            BCLR    #IDLMODE,X:<STATUS                ; Don't idle after readout
616       P:000105 P:000105 0C002A            JMP     <FINISH
617    
618                                 ; Routines executed after the DSP boots and initializes
619       P:000106 P:000106 300800  STARTUP   MOVE              #<TST_RCV,R0            ; Execution address when idle => when not
620       P:000107 P:000107 601E00            MOVE              R0,X:<IDL_ADR           ;   processing commands or reading out
621       P:000108 P:000108 44F400            MOVE              #50000,X0               ; Delay by 500 milliseconds
                            00C350
622       P:00010A P:00010A 06C400            DO      X0,L_DELAY
                            00010D
623       P:00010C P:00010C 06E8A3            REP     #1000
624       P:00010D P:00010D 000000            NOP
625                                 L_DELAY
626       P:00010E P:00010E 57F400            MOVE              #$020002,B              ; Normal reply after booting is 'SYR'
                            020002
627       P:000110 P:000110 0D006B            JSR     <XMT_WRD
628       P:000111 P:000111 57F400            MOVE              #'SYR',B
                            535952
629       P:000113 P:000113 0D006B            JSR     <XMT_WRD
630    
631       P:000114 P:000114 0C0002            JMP     <START                            ; Start normal command processing
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 12



632    
633    
634                                 ; *******************  DSP  INITIALIZATION  CODE  **********************
635                                 ; This code initializes the DSP right after booting, and is overwritten
636                                 ;   by application code
637       P:000115 P:000115 08F4BD  INIT      MOVEP             #PLL_INIT,X:PCTL        ; Initialize PLL to 100 MHz
                            050003
638       P:000117 P:000117 000000            NOP
639    
640                                 ; Set operation mode register OMR to normal expanded
641       P:000118 P:000118 0500BA            MOVEC             #$0000,OMR              ; Operating Mode Register = Normal Expanded
642       P:000119 P:000119 0500BB            MOVEC             #0,SP                   ; Reset the Stack Pointer SP
643    
644                                 ; Program the AA = address attribute pins
645       P:00011A P:00011A 08F4B9            MOVEP             #$FFFC21,X:AAR0         ; Y = $FFF000 to $FFFFFF asserts commands
                            FFFC21
646       P:00011C P:00011C 08F4B8            MOVEP             #$008909,X:AAR1         ; P = $008000 to $00FFFF accesses the EEPROM
                            008909
647       P:00011E P:00011E 08F4B7            MOVEP             #$010C11,X:AAR2         ; X = $010000 to $010FFF reads A/D values
                            010C11
648       P:000120 P:000120 08F4B6            MOVEP             #$080621,X:AAR3         ; Y = $080000 to $0BFFFF R/W from SRAM
                            080621
649    
650                                 ; Program the DRAM memory access and addressing
651       P:000122 P:000122 08F4BB            MOVEP             #$028FE2,X:BCR          ; Bus Control Register
                            028FE2
652    
653                                 ; Program the Host port B for parallel I/O
654       P:000124 P:000124 08F484            MOVEP             #>1,X:HPCR              ; All pins enabled as GPIO
                            000001
655       P:000126 P:000126 08F489            MOVEP             #$010C,X:HDR
                            00010C
656       P:000128 P:000128 08F488            MOVEP             #$B10E,X:HDDR           ; Data Direction Register
                            00B10E
657                                                                                     ;  (1 for Output, 0 for Input)
658    
659                                 ; Port B conversion from software bits to schematic labels
660                                 ;       PB0 = PWROK             PB08 = PRSFIFO*
661                                 ;       PB1 = LED1              PB09 = EF*
662                                 ;       PB2 = LVEN              PB10 = EXT-IN0
663                                 ;       PB3 = HVEN              PB11 = EXT-IN1
664                                 ;       PB4 = STATUS0           PB12 = EXT-OUT0
665                                 ;       PB5 = STATUS1           PB13 = EXT-OUT1
666                                 ;       PB6 = STATUS2           PB14 = SSFHF*
667                                 ;       PB7 = STATUS3           PB15 = SELSCI
668    
669                                 ; Program the serial port ESSI0 = Port C for serial communication with
670                                 ;   the utility board
671       P:00012A P:00012A 07F43F            MOVEP             #>0,X:PCRC              ; Software reset of ESSI0
                            000000
672       P:00012C P:00012C 07F435            MOVEP             #$000809,X:CRA0         ; Divide 100 MHz by 20 to get 5.0 MHz
                            000809
673                                                                                     ; DC[4:0] = 0 for non-network operation
674                                                                                     ; WL0-WL2 = ALC = 0 for 2-bit data words
675                                                                                     ; ALC = 0, SSC1 = 0 for SC1 not used
676       P:00012E P:00012E 07F436            MOVEP             #$010130,X:CRB0         ; SCKD = 1 for internally generated clock
                            010130
677                                                                                     ; SHFD = 0 for MSB shifted first
678                                                                                     ; CKP = 0 for rising clock edge transitions
679                                                                                     ; TE0 = 1 to enable transmitter #0
680                                                                                     ; MOD = 0 for normal, non-networked mode
681                                                                                     ; FSL1 = 1, FSL0 = 0 for on-demand transmit
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 13



682       P:000130 P:000130 07F43F            MOVEP             #%000000,X:PCRC         ; Control Register (0 for GPIO, 1 for ESSI)
                            000000
683                                                                                     ; SCK0 = P3, STD0 = P5 are ESSI0, turned OFF
684       P:000132 P:000132 07F43E            MOVEP             #%010111,X:PRRC         ; Data Direction Register (0 for In, 1 for O
ut)
                            000017
685       P:000134 P:000134 07F43D            MOVEP             #%000101,X:PDRC         ; Data Register - ROM/FIFO* = 0, WR_ENA* = 1
,
                            000005
686                                                                                     ;   AUX1 = 0, AUX2 = AUX3 = 1
687    
688                                 ; Conversion from software bits to schematic labels for Port C
689                                 ;       PC0 = SC00 = UTL-T-SCK
690                                 ;       PC1 = SC01 = 2_XMT = SYNC on prototype
691                                 ;       PC2 = SC02 = WR_ENA*
692                                 ;       PC3 = SCK0 = TIM-U-SCK
693                                 ;       PC4 = SRD0 = UTL-T-SCD
694                                 ;       PC5 = STD0 = TIM-U-STD
695    
696                                 ; Program the serial port ESSI1 = Port D for serial transmission to
697                                 ;   the analog boards and two parallel I/O input pins
698       P:000136 P:000136 07F42F            MOVEP             #>0,X:PCRD              ; Software reset of ESSI0
                            000000
699       P:000138 P:000138 07F425            MOVEP             #$000809,X:CRA1         ; Divide 100 MHz by 20 to get 5.0 MHz
                            000809
700                                                                                     ; DC[4:0] = 0
701                                                                                     ; WL[2:0] = 000 for 8-bit data words
702                                                                                     ; SSC1 = 0 for SC1 not used
703       P:00013A P:00013A 07F426            MOVEP             #$000030,X:CRB1         ; SCKD = 1 for internally generated clock
                            000030
704                                                                                     ; SCD2 = 1 so frame sync SC2 is an output
705                                                                                     ; SHFD = 0 for MSB shifted first
706                                                                                     ; CKP = 0 for rising clock edge transitions
707                                                                                     ; TE0 = 0 to NOT enable transmitter #0 yet
708                                                                                     ; MOD = 0 so its not networked mode
709       P:00013C P:00013C 07F42F            MOVEP             #%100000,X:PCRD         ; Control Register (0 for GPIO, 1 for ESSI)
                            000020
710                                                                                     ; PD3 = SCK1, PD5 = STD1 for ESSI
711       P:00013E P:00013E 07F42E            MOVEP             #%000100,X:PRRD         ; Data Direction Register (0 for In, 1 for O
ut)
                            000004
712       P:000140 P:000140 07F42D            MOVEP             #%000100,X:PDRD         ; Data Register: 'not used' = 0 outputs
                            000004
713       P:000142 P:000142 07F42C            MOVEP             #0,X:TX10               ; Initialize the transmitter to zero
                            000000
714       P:000144 P:000144 000000            NOP
715       P:000145 P:000145 000000            NOP
716       P:000146 P:000146 012630            BSET    #TE,X:CRB1                        ; Enable the SSI transmitter
717    
718                                 ; Conversion from software bits to schematic labels for Port D
719                                 ; PD0 = SC10 = 2_XMT_? input
720                                 ; PD1 = SC11 = SSFEF* input
721                                 ; PD2 = SC12 = PWR_EN
722                                 ; PD3 = SCK1 = TIM-A-SCK
723                                 ; PD4 = SRD1 = PWRRST
724                                 ; PD5 = STD1 = TIM-A-STD
725    
726                                 ; Program the SCI port to benign values
727       P:000147 P:000147 07F41F            MOVEP             #%000,X:PCRE            ; Port Control Register = GPIO
                            000000
728       P:000149 P:000149 07F41E            MOVEP             #%010,X:PRRE            ; Port Direction Register (0 = Input)
                            000002
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 14



729       P:00014B P:00014B 07F41D            MOVEP             #%010,X:PDRE            ; Port Data Register
                            000002
730                                 ;       PE0 = RXD
731                                 ;       PE1 = TXD
732                                 ;       PE2 = SCLK
733    
734                                 ; Program one of the three timers as an exposure timer
735       P:00014D P:00014D 07F403            MOVEP             #$C34F,X:TPLR           ; Prescaler to generate millisecond timer,
                            00C34F
736                                                                                     ;  counting from the system clock / 2 = 50 M
Hz
737       P:00014F P:00014F 07F40F            MOVEP             #$208000,X:TCSR0        ; Clear timer complete bit and enable presca
ler
                            208000
738       P:000151 P:000151 07F40E            MOVEP             #0,X:TLR0               ; Timer load register
                            000000
739    
740                                 ; Enable no interrupts
741       P:000153 P:000153 08F4BF            MOVEP             #$000000,X:IPRC         ; No interrupts allowed
                            000000
742       P:000155 P:000155 05F439            MOVE              #$300,SR                ; Mask all interrupts
                            000300
743    
744                                 ; Initialize the fiber optic serial receiver circuitry
745       P:000157 P:000157 061480            DO      #20,L_FO_INIT
                            00015C
746       P:000159 P:000159 5FF000            MOVE                          Y:RDFO,B
                            FFFFF1
747       P:00015B P:00015B 0605A0            REP     #5
748       P:00015C P:00015C 000000            NOP
749                                 L_FO_INIT
750    
751                                 ; Pulse PRSFIFO* low to revive the CMDRST* instruction and reset the FIFO
752       P:00015D P:00015D 44F400            MOVE              #1000000,X0             ; Delay by 10 milliseconds
                            0F4240
753       P:00015F P:00015F 06C400            DO      X0,*+3
                            000161
754       P:000161 P:000161 000000            NOP
755       P:000162 P:000162 0A8908            BCLR    #8,X:HDR
756       P:000163 P:000163 0614A0            REP     #20
757       P:000164 P:000164 000000            NOP
758       P:000165 P:000165 0A8928            BSET    #8,X:HDR
759    
760                                 ; Put all the analog switch inputs to low so they draw minimum current
761       P:000166 P:000166 012F23            BSET    #3,X:PCRD                         ; Turn the serial clock on
762       P:000167 P:000167 56F400            MOVE              #$0C3000,A              ; Value of integrate speed and gain switches
                            0C3000
763       P:000169 P:000169 20001B            CLR     B
764       P:00016A P:00016A 241000            MOVE              #$100000,X0             ; Increment over board numbers for DAC write
s
765       P:00016B P:00016B 45F400            MOVE              #$001000,X1             ; Increment over board numbers for WRSS writ
es
                            001000
766       P:00016D P:00016D 060F80            DO      #15,L_ANALOG                      ; Fifteen video processor boards maximum
                            000175
767       P:00016F P:00016F 0D017C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
768       P:000170 P:000170 200040            ADD     X0,A
769       P:000171 P:000171 5F7000            MOVE                          B,Y:WRSS    ; This is for the fast analog switches
                            FFFFF3
770       P:000173 P:000173 0620A3            REP     #800                              ; Delay for the serial data transmission
771       P:000174 P:000174 000000            NOP
772       P:000175 P:000175 200068            ADD     X1,B                              ; Increment the video and clock driver numbe
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 15



rs
773                                 L_ANALOG
774       P:000176 P:000176 0A0F00            BCLR    #CDAC,X:<LATCH                    ; Enable clearing of DACs
775       P:000177 P:000177 0A0F02            BCLR    #ENCK,X:<LATCH                    ; Disable clock and DAC output switches
776       P:000178 P:000178 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Execute these two operations
                            00000F
777       P:00017A P:00017A 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
778       P:00017B P:00017B 0C0190            JMP     <SKIP
779    
780                                 ; Transmit contents of accumulator A1 over the synchronous serial transmitter
781                                 XMIT_A_WORD
782       P:00017C P:00017C 547000            MOVE              A1,X:SV_A1
                            00000E
783       P:00017E P:00017E 01A786            JCLR    #TDE,X:SSISR1,*                   ; Start bit
                            00017E
784       P:000180 P:000180 07F42C            MOVEP             #$010000,X:TX10
                            010000
785       P:000182 P:000182 060380            DO      #3,L_XMIT
                            000188
786       P:000184 P:000184 01A786            JCLR    #TDE,X:SSISR1,*                   ; Three data bytes
                            000184
787       P:000186 P:000186 04CCCC            MOVEP             A1,X:TX10
788       P:000187 P:000187 0C1E90            LSL     #8,A
789       P:000188 P:000188 000000            NOP
790                                 L_XMIT
791       P:000189 P:000189 01A786            JCLR    #TDE,X:SSISR1,*                   ; Zeroes to bring transmitter low
                            000189
792       P:00018B P:00018B 07F42C            MOVEP             #0,X:TX10
                            000000
793       P:00018D P:00018D 54F000            MOVE              X:SV_A1,A1
                            00000E
794       P:00018F P:00018F 00000C            RTS
795    
796                                 SKIP
797    
798                                           IF      @SCP("HOST","ROM")
806                                           ENDIF
807    
808       P:000190 P:000190 44F400            MOVE              #>$AC,X0
                            0000AC
809       P:000192 P:000192 440100            MOVE              X0,X:<FO_HDR
810    
811       P:000193 P:000193 0C0106            JMP     <STARTUP
812    
813                                 ;  ****************  X: Memory tables  ********************
814    
815                                 ; Define the address in P: space where the table of constants begins
816    
817                                  X_BOOT_START
818       000192                              EQU     @LCV(L)-2
819    
820                                           IF      @SCP("HOST","ROM")
822                                           ENDIF
823                                           IF      @SCP("HOST","HOST")
824       X:000000 X:000000                   ORG     X:0,X:0
825                                           ENDIF
826    
827                                 ; Special storage area - initialization constants and scratch space
828       X:000000 X:000000         STATUS    DC      $1064                             ; Controller status bits
829    
830       000001                    FO_HDR    EQU     STATUS+1                          ; Fiber optic write bytes
831       000005                    HEADER    EQU     FO_HDR+4                          ; Command header
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timboot.asm  Page 16



832       000006                    NWORDS    EQU     HEADER+1                          ; Number of words in the command
833       000007                    COM_BUF   EQU     NWORDS+1                          ; Command buffer
834       00000E                    SV_A1     EQU     COM_BUF+7                         ; Save accumulator A1
835    
836                                           IF      @SCP("HOST","ROM")
841                                           ENDIF
842    
843                                           IF      @SCP("HOST","HOST")
844       X:00000F X:00000F                   ORG     X:$F,X:$F
845                                           ENDIF
846    
847                                 ; Parameter table in P: space to be copied into X: space during
848                                 ;   initialization, and is copied from ROM by the DSP boot
849       X:00000F X:00000F         LATCH     DC      $7A                               ; Starting value in latch chip U25
850                                  EXPOSURE_TIME
851       X:000010 X:000010                   DC      0                                 ; Exposure time in milliseconds
852       X:000011 X:000011         ONE       DC      1                                 ; One
853       X:000012 X:000012         TWO       DC      2                                 ; Two
854       X:000013 X:000013         THREE     DC      3                                 ; Three
855       X:000014 X:000014         SEVEN     DC      7                                 ; Seven
856       X:000015 X:000015         MASK1     DC      $FCFCF8                           ; Mask for checking header
857       X:000016 X:000016         MASK2     DC      $030300                           ; Mask for checking header
858       X:000017 X:000017         DONE      DC      'DON'                             ; Standard reply
859       X:000018 X:000018         SBRD      DC      $020000                           ; Source Identification number
860       X:000019 X:000019         DBRD      DC      $000200                           ; Destination Identification number
861       X:00001A X:00001A         DMASK     DC      $00FF00                           ; Mask to get destination board number
862       X:00001B X:00001B         SMASK     DC      $FF0000                           ; Mask to get source board number
863       X:00001C X:00001C         ERR       DC      'ERR'                             ; An error occurred
864       X:00001D X:00001D         C100K     DC      100000                            ; Delay for WRROM = 1 millisec
865       X:00001E X:00001E         IDL_ADR   DC      TST_RCV                           ; Address of idling routine
866       X:00001F X:00001F         EXP_ADR   DC      0                                 ; Jump to this address during exposures
867       X:000020 X:000020                   DC      0,0
868    
869                                 ; Command table
870       000022                    COM_TBL_R EQU     @LCV(R)
871       X:000022 X:000022         COM_TBL   DC      'TDL',TDL                         ; Test Data Link
872       X:000024 X:000024                   DC      'RDM',RDMEM                       ; Read from DSP or EEPROM memory
873       X:000026 X:000026                   DC      'WRM',WRMEM                       ; Write to DSP memory
874       X:000028 X:000028                   DC      'LDA',LDAPPL                      ; Load application from EEPROM to DSP
875       X:00002A X:00002A                   DC      'STP',STOP_IDLE_CLOCKING
876       X:00002C X:00002C                   DC      'DON',START                       ; Nothing special
877       X:00002E X:00002E                   DC      'ERR',START                       ; Nothing special
878    
879                                  END_COMMAND_TABLE
880       000030                              EQU     @LCV(R)
881    
882                                           IF      @SCP("HOST","ROM")
884                                           ENDIF
885    
886       000030                    END_ADR   EQU     @LCV(L)                           ; End address of P: code written to ROM
887    
888       P:000194 P:000194                   ORG     P:,P:
889    
890       0001B4                    CC        EQU     CCDVIDREV5+TIMREV5+UTILREV3+SHUTTER_CC+TEMP_POLY
891    
892                                 ; Put number of words of application in P: for loading application from EEPROM
893       P:000194 P:000194                   DC      TIMBOOT_X_MEMORY-@LCV(L)-1
894    
895                                 ; senal de control out0
896                                 ;       BSET    #EXT_OUT0,X:HDR         ; sube pata de control out0
897                                 ;       BCLR    #EXT_OUT0,X:HDR         ; baja pata de control out0
898    
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 17



899                                 ; Set software to IDLE mode
900                                 START_IDLE_CLOCKING
901       P:000195 P:000195 60F400            MOVE              #IDLE,R0                ; Exercise clocks when idling
                            00019A
902       P:000197 P:000197 601E00            MOVE              R0,X:<IDL_ADR
903       P:000198 P:000198 0A0022            BSET    #IDLMODE,X:<STATUS                ; Idle after readout
904       P:000199 P:000199 0C002A            JMP     <FINISH                           ; Need to send header and 'DON'
905    
906    
907                                 ; idle routine dumping lines
908                                 IDLE
909       P:00019A P:00019A 302700            MOVE              #<PREPARE_DUMP,R0       ;
910       P:00019B P:00019B 0D037A            JSR     <CLOCK                            ;
911    
912       P:00019C P:00019C 060240            DO      Y:<NPR,END_DUMP                   ;
                            0001A5
913                                                                                     ;
914    
915       P:00019E P:00019E 302900            MOVE              #<PARALLEL_DUMP,R0      ; shift the array by 1 line, havinfg the DUM
P barrier down, so it gets dumped
916       P:00019F P:00019F 0D037A            JSR     <CLOCK                            ; onto the dump line, beyonfd the serial reg
ister
917       P:0001A0 P:0001A0 330700            MOVE              #COM_BUF,R3
918       P:0001A1 P:0001A1 0D003D            JSR     <GET_RCV                          ; Check for FO or SSI commands
919       P:0001A2 P:0001A2 0E01A5            JCC     <NO_COM1                          ; Continue IDLE if no commands received
920       P:0001A3 P:0001A3 00008C            ENDDO                                     ; termina ciclo si hay comando
921       P:0001A4 P:0001A4 0C000B            JMP     <PRC_RCV                          ; Go process header and command
922                                 NO_COM1
923       P:0001A5 P:0001A5 000000            NOP
924                                 END_DUMP
925       P:0001A6 P:0001A6 303000            MOVE              #<CLEAR_READ_REGISTER,R0 ;
926       P:0001A7 P:0001A7 0D037A            JSR     <CLOCK
927    
928       P:0001A8 P:0001A8 0C019A            JMP     <IDLE
929    
930                                 ;  *****************  Exposure and readout routines  *****************
931    
932                                 RDCCD
933                                 ; Check for serial binning and compute "binning minus 1", because we
934                                 ; will shift charge binning-1 times and then use the shift+video
935                                 ; waveform for the last shift and read.
936       P:0001A9 P:0001A9 5E8600            MOVE                          Y:<NSBIN,A  ; Get serial binning factor
937       P:0001AA P:0001AA 449100            MOVE              X:<ONE,X0               ; Constant 1
938       P:0001AB P:0001AB 200044            SUB     X0,A                              ; Subtract
939       P:0001AC P:0001AC 000000            NOP                                       ; pipeline restriction
940       P:0001AD P:0001AD 5C0C00            MOVE                          A1,Y:<NSBINM1 ; Put it in Y memory
941                                 ;
942                                 ; Clear out the accumulated charge from the serial shift register
943       P:0001AE P:0001AE 060540            DO      Y:<NSCLR,*+5                      ; Loop over number of pixels to skip
                            0001B2
944       P:0001B0 P:0001B0 303300            MOVE              #<SERIAL_SKIP,R0        ; Address of serial skipping waveforms
945       P:0001B1 P:0001B1 0D037A            JSR     <CLOCK                            ; Go clock out the CCD charge
946       P:0001B2 P:0001B2 000000            NOP                                       ; Do loop restriction
947    
948    
949                                 ; For ROI do the parallel skipping
950       P:0001B3 P:0001B3 5E8E00            MOVE                          Y:<NPSKP,A  ; Get number of lines to skip for each amp
951       P:0001B4 P:0001B4 200003            TST     A                                 ; Is it zero?
952       P:0001B5 P:0001B5 0EA1BF            JEQ     <NOSKIP                           ; Yes, jump around the para. skipping code
953       P:0001B6 P:0001B6 302700            MOVE              #<PREPARE_DUMP,R0       ;
954       P:0001B7 P:0001B7 0D037A            JSR     <CLOCK                            ; Clock it out
955       P:0001B8 P:0001B8 06CC00            DO      A1,LPSKP                          ; Do loop NPSKP times
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 18



                            0001BC
956       P:0001BA P:0001BA 302900            MOVE              #<PARALLEL_DUMP,R0      ; dump line
957       P:0001BB P:0001BB 0D037A            JSR     <CLOCK                            ; Clock it out
958       P:0001BC P:0001BC 000000            NOP
959                                 LPSKP
960                                 ; Clear out the accumulated charge from the serial register again
961       P:0001BD P:0001BD 303000            MOVE              #<CLEAR_READ_REGISTER,R0 ;
962       P:0001BE P:0001BE 0D037A            JSR     <CLOCK
963                                 NOSKIP
964                                 ;
965                                 ; Now we're ready to read out the ROI
966                                 ;
967                                 ; Move the number of binned lines to shift into A
968                                 ;       MOVE    #0,B                    ; just for counting the amount of parallele clocks
969                                 ;       NOP
970                                 ;       MOVE    B,Y:<NPTST
971                                 ;       NOP
972                                 ;       MOVE    B,Y:<NSTST
973                                 ;       NOP
974       P:0001BF P:0001BF 5E8200            MOVE                          Y:<NPR,A
975       P:0001C0 P:0001C0 000000            NOP
976                                 ;
977       P:0001C1 P:0001C1 06CC00            DO      A1,LPR                            ; Number of rows to shift
                            0001ED
978                                 ;       MOVE    Y:<NPTST,B
979                                 ;       ADD     #1,B
980                                 ;       NOP
981                                 ;       MOVE    B,Y:<NPTST
982                                 ;       NOP
983    
984       P:0001C3 P:0001C3 060740            DO      Y:<NPBIN,LPBIN                    ; Parallel binning factor NBPIN
                            0001C7
985       P:0001C5 P:0001C5 301F00            MOVE              #<PARALLEL_SHIFT,R0     ; Parallel shift waveform
986       P:0001C6 P:0001C6 0D037A            JSR     <CLOCK                            ; Clock the parallel transfer
987       P:0001C7 P:0001C7 000000            NOP
988                                 LPBIN
989    
990                                 ; Check for a command once per line. Only the ABORT command should be issued.
991       P:0001C8 P:0001C8 330700            MOVE              #<COM_BUF,R3
992       P:0001C9 P:0001C9 0D003D            JSR     <GET_RCV                          ; Was a command received?
993       P:0001CA P:0001CA 0E01D0            JCC     <CONT_RD                          ; If no, continue reading out
994       P:0001CB P:0001CB 0C0312            JMP     <CHK_ABORT_COMMAND                ; If yes, see if its an abort command
995    
996                                 ; Abort the readout currently underway
997       P:0001CC P:0001CC 0A0084  ABR_RDC   JCLR    #ST_RDC,X:<STATUS,ABORT_EXPOSURE
                            000334
998       P:0001CE P:0001CE 00008C            ENDDO                                     ; Properly terminate readout loop
999       P:0001CF P:0001CF 0C0321            JMP     <RDCCD_END_ABORT
1000                                ;
1001                                ; The following procedure reads out one serial row of pixels
1002                                ;    1.  The prescan (underscan) pixels:  shift, read and transmit
1003                                ;    2.  NSSKP pixels:  skip.  This gets us to the beginning of ROI
1004                                ;    3.  NSRD pixels:  Shift, read and transmit.  This is the ROI
1005                                ;    4.  NSSKP2 pixels:  skip.  This gets us to the overscan pixels
1006                                ;    5.  NSOCK pixels:  Shift, read and transmit.  This is the overscan.
1007                                ;
1008                                ;
1009                                ; First check if this is a serial ROI readout
1010                                ;
1011                                CONT_RD
1012                                ;       MOVE    #50,A                   ; prescan pixels
1013                                ;       JSR     <SSKIP                  ; skip all prescan pixels
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 19



1014                                ;       NOP
1015   
1016      P:0001D0 P:0001D0 5E9000            MOVE                          Y:<NSSKP,A  ; Number of serial skips to A
1017      P:0001D1 P:0001D1 200003            TST     A                                 ; zero?
1018      P:0001D2 P:0001D2 0AF0AA            JEQ     NOPRESKP                          ; No predata skips (prescan)
                            0001D6
1019      P:0001D4 P:0001D4 0AF080            JMP     ROI                               ; there is a ROI
                            0001DA
1020                                NOPRESKP
1021      P:0001D6 P:0001D6 5E9200            MOVE                          Y:<NSSKP2,A ; number of serial skips to overs
1022      P:0001D7 P:0001D7 200003            TST     A
1023      P:0001D8 P:0001D8 0AF0AA            JEQ     NOROI                             ; no predata skips, no postdataskips
                            0001EB
1024                                                                                    ; ==> NO ROI
1025                                ;
1026                                ; This is a serial ROI read so here we go
1027                                ; The (binned) pre pixels first
1028                                ;
1029                                ROI
1030      P:0001DA P:0001DA 5E8F00            MOVE                          Y:<NSUND,A  ; Number of underscan pixels
1031      P:0001DB P:0001DB 0D01F9            JSR     <SREAD                            ; Binned read subroutine
1032      P:0001DC P:0001DC 000000            NOP
1033   
1034                                ;
1035                                ; Now Skip to ROI
1036                                ;
1037      P:0001DD P:0001DD 5E9000            MOVE                          Y:<NSSKP,A  ; Number of pixels to ROI
1038      P:0001DE P:0001DE 0D020F            JSR     <SSKIP                            ; Skip
1039      P:0001DF P:0001DF 000000            NOP
1040                                ;
1041                                ; Now the ROI read
1042                                ;
1043      P:0001E0 P:0001E0 5E9100            MOVE                          Y:<NSRD,A   ; Number of pixels in ROI
1044      P:0001E1 P:0001E1 0D01F9            JSR     <SREAD                            ; Binned read sub
1045      P:0001E2 P:0001E2 000000            NOP
1046                                ;
1047                                ; Now skip to overscan pixels
1048                                ;
1049      P:0001E3 P:0001E3 5E9200            MOVE                          Y:<NSSKP2,A ; Number of pixels to end of CCD
1050      P:0001E4 P:0001E4 0D020F            JSR     <SSKIP                            ; Skip subroutine
1051      P:0001E5 P:0001E5 000000            NOP
1052                                ;
1053                                ; Lastly we read the overscan pixels
1054                                ;
1055      P:0001E6 P:0001E6 5E9300            MOVE                          Y:<NSOCK,A  ; Number of (binned) overscan pixels
1056      P:0001E7 P:0001E7 0D01F9            JSR     <SREAD
1057      P:0001E8 P:0001E8 000000            NOP
1058                                ; Done
1059      P:0001E9 P:0001E9 0AF080            JMP     ENDLINE                           ; End of one par-ser cycle
                            0001ED
1060                                ;
1061                                NOROI
1062      P:0001EB P:0001EB 5E8100            MOVE                          Y:<NSR,A    ; Number of (binned) serials
1063      P:0001EC P:0001EC 0D01F9            JSR     <SREAD                            ; serial read subroutine
1064                                ENDLINE
1065      P:0001ED P:0001ED 000000            NOP
1066                                LPR                                                 ; End of parallel loop
1067                                ;
1068                                ; Restore the controller to non-image data transfer and idling if necessary
1069      P:0001EE P:0001EE 0A0082  RDC_END   JCLR    #IDLMODE,X:<STATUS,NO_IDL         ; Don't idle after readout
                            0001F4
1070      P:0001F0 P:0001F0 60F400            MOVE              #IDLE,R0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 20



                            00019A
1071      P:0001F2 P:0001F2 601E00            MOVE              R0,X:<IDL_ADR
1072      P:0001F3 P:0001F3 0C01F6            JMP     <RDC_E
1073      P:0001F4 P:0001F4 300800  NO_IDL    MOVE              #TST_RCV,R0
1074      P:0001F5 P:0001F5 601E00            MOVE              R0,X:<IDL_ADR
1075      P:0001F6 P:0001F6 0D0377  RDC_E     JSR     <WAIT_TO_FINISH_CLOCKING
1076      P:0001F7 P:0001F7 0A0004            BCLR    #ST_RDC,X:<STATUS                 ; Set status to not reading out
1077      P:0001F8 P:0001F8 0C0002            JMP     <START
1078                                ;
1079                                ; Subroutine.
1080                                ; This is where we do the serial binning.  Use the SERIAL_SHIFT
1081                                ; waveform for NSBIN-1 reps then the last rep with SERIAL_READ which
1082                                ; includes video processing.
1083                                ;
1084                                SREAD
1085      P:0001F9 P:0001F9 5C1400            MOVE                          A1,Y:<NP2READ ; Number of pixels to read
1086      P:0001FA P:0001FA 5E8C00            MOVE                          Y:<NSBINM1,A ; binning factor -1 to accumulator
1087      P:0001FB P:0001FB 200003            TST     A                                 ;
1088      P:0001FC P:0001FC 0EA209            JEQ     <NOTBIN                           ; if zero jump to no-binning code
1089                                ;
1090      P:0001FD P:0001FD 061440            DO      Y:<NP2READ,LSR1
                            000207
1091                                ;
1092      P:0001FF P:0001FF 060C40            DO      Y:<NSBINM1,LSBIN                  ; Loop over nsbin-1 pixels
                            000203
1093      P:000201 P:000201 303C00            MOVE              #<SERIAL_SHIFT,R0       ; Pointer to serial shift waveform
1094      P:000202 P:000202 0D037A            JSR     <CLOCK                            ; Clock out the waveform
1095      P:000203 P:000203 000000            NOP
1096                                LSBIN
1097      P:000204 P:000204 304300            MOVE              #<SERIAL_READ,R0        ; Last rep is shift + video proc.
1098      P:000205 P:000205 0D037A            JSR     <CLOCK                            ; Clock it out
1099      P:000206 P:000206 000000            NOP
1100      P:000207 P:000207 000000            NOP
1101                                LSR1
1102      P:000208 P:000208 0C020E            JMP     <NEXTROW                          ; Skip non-binning code.
1103                                NOTBIN
1104      P:000209 P:000209 061440            DO      Y:<NP2READ,LSR2                   ; # pixels to read
                            00020D
1105                                ;       MOVE    Y:<NSTST,B              ; just to count the amount of serial pixels
1106                                ;       ADD     #1,B
1107                                ;       NOP
1108                                ;       MOVE    B,Y:<NSTST
1109                                ;       NOP
1110      P:00020B P:00020B 304300            MOVE              #<SERIAL_READ,R0        ; Waveform table starting address
1111      P:00020C P:00020C 0D037A            JSR     <CLOCK                            ; Go clock out the CCD charge
1112      P:00020D P:00020D 000000            NOP
1113                                LSR2
1114   
1115                                NEXTROW
1116      P:00020E P:00020E 00000C            RTS                                       ; Return from subroutine
1117                                ;
1118                                ; Serial skipping subroutine.  Number of pixels to skip -> R0
1119                                ; The binning appears here only to make up the total amount of skips, since
1120                                ; the skip calculation is based upon binned detector coordinates
1121                                ; Note that the waveform called inside the binning loop is the same skip
1122                                ; routine
1123                                SSKIP
1124      P:00020F P:00020F 5C1400            MOVE                          A1,Y:<NP2READ ; Number of skip pixels
1125      P:000210 P:000210 5E8C00            MOVE                          Y:<NSBINM1,A ; binning factor -1 to accumulator
1126      P:000211 P:000211 200003            TST     A                                 ;
1127      P:000212 P:000212 0EA21E            JEQ     <NOTBINSKP                        ; if zero jump to no-binning code
1128      P:000213 P:000213 061440            DO      Y:<NP2READ,LSK                    ; Do number of pixels to skip
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 21



                            00021C
1129                                ;
1130      P:000215 P:000215 060C40            DO      Y:<NSBINM1,LSBIN2
                            000219
1131      P:000217 P:000217 303300            MOVE              #<SERIAL_SKIP,R0        ; serial skip waveform address
1132      P:000218 P:000218 0D037A            JSR     <CLOCK                            ; Clock it to the hardware
1133      P:000219 P:000219 000000            NOP
1134                                LSBIN2
1135      P:00021A P:00021A 303300            MOVE              #<SERIAL_SKIP,R0        ; serial skip waveform address
1136      P:00021B P:00021B 0D037A            JSR     <CLOCK                            ; Clock it to the hardware
1137      P:00021C P:00021C 000000            NOP                                       ; do loop restriction
1138                                LSK
1139      P:00021D P:00021D 0C0223            JMP     <LKS2
1140                                NOTBINSKP
1141      P:00021E P:00021E 061440            DO      Y:<NP2READ,LKS2                   ; # pixels to read
                            000222
1142      P:000220 P:000220 303300            MOVE              #<SERIAL_SKIP,R0        ; serial skip waveform address
1143      P:000221 P:000221 0D037A            JSR     <CLOCK                            ; Go clock out the CCD charge
1144      P:000222 P:000222 000000            NOP
1145                                LKS2
1146      P:000223 P:000223 00000C            RTS                                       ; Return from subroutine
1147                                ;
1148                                ; ******  Minclude many routines not directly needed for readout  *******
1149                                          INCLUDE "timCCDmisc.asm"
1150                                        COMMENT *
1151                                Miscellaneous CCD control routines, common to all detector types
1152   
1153                                Revision History:
1154                                --  0.01:  11 Jun 2004 - CRO.
1155                                        P_SHIFT routine added.  Shifts the CDD charge by NPSHF rows.
1156                                        NPSHF is stored in Y: memory and can be changed before a shift.
1157                                --  0.02:  29 Jul 2004 - CRO.
1158                                        SBINN - set binning routine added.
1159                                --  0.03:  8 Aug 2004 - MB
1160                                        SET_GEOMETRY - set geometry routine added.
1161                                        SET_ROI - set ROI routine added.
1162                                        CAL_GEOM - Calculate Geometry routine added (calculates NSSKP2)
1163                                --  0.04:  10 Aug 2004 - MB
1164                                        Taken out the command processing inside CLR_CCD, as well as the
1165                                        call to "GET_RCV" after the CLR_CCD routine call inside
1166                                        START_EXPOSURE
1167                                        Added the status bit condition for clearing the array automatically
1168                                        during the exposure (inside START_EXPOSURE). Now the host will need
1169                                        to set the bit NOT_CLR to 1 before doing a focus frame
1170                                --  0.05:  12 Aug 2004 - MB
1171                                        Added SET_GEOMETRY, CALC_GEOM, and SET_ROI subroutines
1172   
1173                                        *
1174                                ; we are not using the last two args: ypre and yov, because in this case
1175                                ; we are not using y extra scans (only data)
1176                                SET_GEOMETRY
1177      P:000224 P:000224 47DB00            MOVE              X:(R3)+,Y1              ; Get first parameter XPRE
1178      P:000225 P:000225 4F0F00            MOVE                          Y1,Y:<NSUND ; Move it to the proper loc in Y:mem
1179      P:000226 P:000226 47DB00            MOVE              X:(R3)+,Y1              ; Get second parameter XDATA
1180      P:000227 P:000227 4F1500            MOVE                          Y1,Y:<NSDATA ; Move it to Y: mem also.
1181      P:000228 P:000228 47DB00            MOVE              X:(R3)+,Y1              ; Get third parameter XOV
1182      P:000229 P:000229 4F1300            MOVE                          Y1,Y:<NSOCK ; Move it to Y: mem also.
1183      P:00022A P:00022A 0D023A            JSR     <CALC_GEOM
1184      P:00022B P:00022B 0C002A            JMP     <FINISH                           ; End, send back 'DON'
1185   
1186   
1187                                ; xstart ystart xlen ylen. We are not using the last param, YLEN
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 22



1188                                ; because we use NPR directly
1189                                SET_ROI
1190      P:00022C P:00022C 56DB00            MOVE              X:(R3)+,A               ; Get first parameter XSTART
1191      P:00022D P:00022D 449100            MOVE              X:<ONE,X0
1192      P:00022E P:00022E 200044            SUB     X0,A                              ; substract 1
1193      P:00022F P:00022F 000000            NOP
1194      P:000230 P:000230 5C1000            MOVE                          A1,Y:<NSSKP ; Move it to the proper loc in Y:mem
1195      P:000231 P:000231 56DB00            MOVE              X:(R3)+,A               ; Get second parameter YSTART
1196      P:000232 P:000232 200044            SUB     X0,A                              ; substract 1
1197      P:000233 P:000233 000000            NOP
1198      P:000234 P:000234 5C0E00            MOVE                          A1,Y:<NPSKP ; Move it to Y: mem also.
1199      P:000235 P:000235 56DB00            MOVE              X:(R3)+,A               ; Get third parameter XLEN
1200      P:000236 P:000236 000000            NOP
1201      P:000237 P:000237 5E1100            MOVE                          A,Y:<NSRD   ; Move it to Y: mem also.
1202      P:000238 P:000238 0D023A            JSR     <CALC_GEOM
1203      P:000239 P:000239 0C002A            JMP     <FINISH                           ; End, send back 'DON'
1204   
1205                                CALC_GEOM
1206      P:00023A P:00023A 5E9500            MOVE                          Y:<NSDATA,A ; number of x data pixesl (binned)
1207      P:00023B P:00023B 4C9000            MOVE                          Y:<NSSKP,X0
1208      P:00023C P:00023C 200044            SUB     X0,A                              ; subtract the xstart
1209      P:00023D P:00023D 4C9100            MOVE                          Y:<NSRD,X0
1210      P:00023E P:00023E 200044            SUB     X0,A                              ; subtract the data to read
1211      P:00023F P:00023F 000000            NOP
1212      P:000240 P:000240 5C1200            MOVE                          A1,Y:<NSSKP2 ; now the remaining is the
1213      P:000241 P:000241 00000C            RTS                                       ; data to skip to reach overscan
1214   
1215   
1216      P:000242 P:000242 47DB00  SBINN     MOVE              X:(R3)+,Y1              ; Get first parameter NSBIN
1217      P:000243 P:000243 4F0600            MOVE                          Y1,Y:<NSBIN ; Move it to the proper loc in Y:mem
1218      P:000244 P:000244 47DB00            MOVE              X:(R3)+,Y1              ; Get second parameter NPBIN
1219      P:000245 P:000245 4F0700            MOVE                          Y1,Y:<NPBIN ; Move it to Y: mem also.
1220      P:000246 P:000246 0C002A            JMP     <FINISH                           ; End, send back 'DON'
1221                                ;
1222   
1223      P:000247 P:000247 5E8B00  P_SHIFT   MOVE                          Y:<NPSHF,A  ; Number of rows to shift
1224      P:000248 P:000248 200023            LSR     A                                 ; Need this for split parallel
1225      P:000249 P:000249 000000            NOP
1226      P:00024A P:00024A 06CC00            DO      A1,LSH                            ; Begin DO LSH loop
                            00024E
1227      P:00024C P:00024C 301F00            MOVE              #<PARALLEL_SHIFT,R0     ; Pointer to parallel shift waveform
1228      P:00024D P:00024D 0D037A            JSR     <CLOCK                            ; Go clock out the CCD charge
1229      P:00024E P:00024E 000000            NOP                                       ; Do loop restriction
1230      P:00024F P:00024F 000000  LSH       NOP                                       ; End of parallel shift loop
1231      P:000250 P:000250 0C002A            JMP     <FINISH                           ; End of this routine, send back 'DON'
1232                                ;
1233                                ; End of parallel shift routine
1234                                ;
1235                                POWER_OFF
1236      P:000251 P:000251 0D0287            JSR     <CLEAR_SWITCHES_AND_DACS          ; Clear switches and DACs
1237      P:000252 P:000252 0A8922            BSET    #LVEN,X:HDR
1238      P:000253 P:000253 0A8923            BSET    #HVEN,X:HDR
1239      P:000254 P:000254 0C002A            JMP     <FINISH
1240   
1241                                ; Execute the power-on cycle, as a command
1242                                POWER_ON
1243      P:000255 P:000255 0D0287            JSR     <CLEAR_SWITCHES_AND_DACS          ; Clear switches and DACs
1244   
1245                                ; Turn on the low voltages (+/- 6.5V, +/- 16.5V) and delay
1246      P:000256 P:000256 0A8902            BCLR    #LVEN,X:HDR                       ; Set these signals to DSP outputs
1247      P:000257 P:000257 44F400            MOVE              #2000000,X0
                            1E8480
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 23



1248      P:000259 P:000259 06C400            DO      X0,*+3                            ; Wait 20 millisec for settling
                            00025B
1249      P:00025B P:00025B 000000            NOP
1250   
1251                                ; Turn on the high +36 volt power line and delay
1252      P:00025C P:00025C 0A8903            BCLR    #HVEN,X:HDR                       ; HVEN = Low => Turn on +36V
1253      P:00025D P:00025D 44F400            MOVE              #2000000,X0
                            1E8480
1254      P:00025F P:00025F 06C400            DO      X0,*+3                            ; Wait 20 millisec for settling
                            000261
1255      P:000261 P:000261 000000            NOP
1256   
1257      P:000262 P:000262 0A8980            JCLR    #PWROK,X:HDR,PWR_ERR              ; Test if the power turned on properly
                            000269
1258      P:000264 P:000264 0D026C            JSR     <SET_BIASES                       ; Turn on the DC bias supplies
1259      P:000265 P:000265 60F400            MOVE              #IDLE,R0                ; Put controller in IDLE state
                            00019A
1260      P:000267 P:000267 601E00            MOVE              R0,X:<IDL_ADR
1261      P:000268 P:000268 0C002A            JMP     <FINISH
1262   
1263                                ; The power failed to turn on because of an error on the power control board
1264      P:000269 P:000269 0A8922  PWR_ERR   BSET    #LVEN,X:HDR                       ; Turn off the low voltage emable line
1265      P:00026A P:00026A 0A8923            BSET    #HVEN,X:HDR                       ; Turn off the high voltage emable line
1266      P:00026B P:00026B 0C0028            JMP     <ERROR
1267   
1268                                ; Set all the DC bias voltages and video processor offset values, reading
1269                                ;   them from the 'DACS' table
1270                                SET_BIASES
1271      P:00026C P:00026C 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock
1272      P:00026D P:00026D 0A0F01            BCLR    #1,X:<LATCH                       ; Separate updates of clock driver
1273      P:00026E P:00026E 0A0F20            BSET    #CDAC,X:<LATCH                    ; Disable clearing of DACs
1274      P:00026F P:00026F 0A0F22            BSET    #ENCK,X:<LATCH                    ; Enable clock and DAC output switches
1275      P:000270 P:000270 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Write it to the hardware
                            00000F
1276      P:000272 P:000272 0D0385            JSR     <PAL_DLY                          ; Delay for all this to happen
1277   
1278                                ; Read DAC values from a table, and write them to the DACs
1279      P:000273 P:000273 60F400            MOVE              #DACS,R0                ; Get starting address of DAC values
                            00007E
1280      P:000275 P:000275 000000            NOP
1281      P:000276 P:000276 000000            NOP
1282      P:000277 P:000277 065840            DO      Y:(R0)+,L_DAC                     ; Repeat Y:(R0)+ times
                            00027B
1283      P:000279 P:000279 5ED800            MOVE                          Y:(R0)+,A   ; Read the table entry
1284      P:00027A P:00027A 0D017C            JSR     <XMIT_A_WORD                      ; Transmit it to TIM-A-STD
1285      P:00027B P:00027B 000000            NOP
1286                                L_DAC
1287   
1288                                ; Let the DAC voltages all ramp up before exiting
1289      P:00027C P:00027C 44F400            MOVE              #400000,X0
                            061A80
1290      P:00027E P:00027E 06C400            DO      X0,*+3                            ; 4 millisec delay
                            000280
1291      P:000280 P:000280 000000            NOP
1292      P:000281 P:000281 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1293      P:000282 P:000282 00000C            RTS
1294   
1295                                SET_BIAS_VOLTAGES
1296      P:000283 P:000283 0D026C            JSR     <SET_BIASES
1297      P:000284 P:000284 0C002A            JMP     <FINISH
1298   
1299      P:000285 P:000285 0D0287  CLR_SWS   JSR     <CLEAR_SWITCHES_AND_DACS          ; Clear switches and DACs
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 24



1300      P:000286 P:000286 0C002A            JMP     <FINISH
1301   
1302                                CLEAR_SWITCHES_AND_DACS
1303      P:000287 P:000287 0A0F00            BCLR    #CDAC,X:<LATCH                    ; Clear all the DACs
1304      P:000288 P:000288 0A0F02            BCLR    #ENCK,X:<LATCH                    ; Disable all the output switches
1305      P:000289 P:000289 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Write it to the hardware
                            00000F
1306      P:00028B P:00028B 012F23            BSET    #3,X:PCRD                         ; Turn the serial clock on
1307      P:00028C P:00028C 56F400            MOVE              #$0C3000,A              ; Value of integrate speed and gain switches
                            0C3000
1308      P:00028E P:00028E 20001B            CLR     B
1309      P:00028F P:00028F 241000            MOVE              #$100000,X0             ; Increment over board numbers for DAC write
s
1310      P:000290 P:000290 45F400            MOVE              #$001000,X1             ; Increment over board numbers for WRSS writ
es
                            001000
1311      P:000292 P:000292 060F80            DO      #15,L_VIDEO                       ; Fifteen video processor boards maximum
                            000299
1312      P:000294 P:000294 0D017C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1313      P:000295 P:000295 200040            ADD     X0,A
1314      P:000296 P:000296 5F7000            MOVE                          B,Y:WRSS
                            FFFFF3
1315      P:000298 P:000298 0D0385            JSR     <PAL_DLY                          ; Delay for the serial data transmission
1316      P:000299 P:000299 200068            ADD     X1,B
1317                                L_VIDEO
1318      P:00029A P:00029A 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1319      P:00029B P:00029B 00000C            RTS
1320   
1321                                ; Modify the current shutter state.
1322                                ;       'X0' contains the bits specifying the desired shutter state
1323                                SET_SHUTTER_STATE
1324      P:00029C P:00029C 568F00            MOVE              X:LATCH,A
1325      P:00029D P:00029D 0140C6            AND     #$FFEF,A
                            00FFEF
1326      P:00029F P:00029F 200042            OR      X0,A
1327      P:0002A0 P:0002A0 000000            NOP
1328      P:0002A1 P:0002A1 540F00            MOVE              A1,X:LATCH
1329      P:0002A2 P:0002A2 09CC35            MOVEP             A1,Y:WRLATCH
1330      P:0002A3 P:0002A3 00000C            RTS
1331   
1332                                ; Open the shutter from the timing board, executed as a command (positive logic)
1333                                OPEN_SHUTTER
1334      P:0002A4 P:0002A4 240000            MOVE              #0,X0
1335      P:0002A5 P:0002A5 0D029C            JSR     <SET_SHUTTER_STATE
1336      P:0002A6 P:0002A6 0C002A            JMP     <FINISH
1337   
1338                                ; Close the shutter from the timing board, executed as a command (positive logic)
1339                                CLOSE_SHUTTER
1340      P:0002A7 P:0002A7 44F400            MOVE              #>$10,X0
                            000010
1341      P:0002A9 P:0002A9 0D029C            JSR     <SET_SHUTTER_STATE
1342      P:0002AA P:0002AA 0C002A            JMP     <FINISH
1343   
1344                                ; Open the shutter from the timing board, executed as a command (negative logic)
1345                                ;OPEN_SHUTTER
1346                                ;       MOVE    #>$10,X0
1347                                ;       JSR     <SET_SHUTTER_STATE
1348                                ;       BSET    #ST_SHUT,X:<STATUS
1349                                ;       JMP     <FINISH
1350   
1351                                ; Close the shutter from the timing board, executed as a command (negative logic)
1352                                ;CLOSE_SHUTTER
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 25



1353                                ;       MOVE    #0,X0
1354                                ;       JSR     <SET_SHUTTER_STATE
1355                                ;       BCLR    #ST_SHUT,X:<STATUS
1356                                ;       JMP     <FINISH
1357   
1358                                ; Shutter subroutines for positive logic
1359   
1360      P:0002AB P:0002AB 240000  OSHUT     MOVE              #0,X0
1361      P:0002AC P:0002AC 0D029C            JSR     <SET_SHUTTER_STATE
1362      P:0002AD P:0002AD 00000C            RTS
1363   
1364      P:0002AE P:0002AE 44F400  CSHUT     MOVE              #>$10,X0
                            000010
1365      P:0002B0 P:0002B0 0D029C            JSR     <SET_SHUTTER_STATE
1366      P:0002B1 P:0002B1 00000C            RTS
1367   
1368                                ; Shutter subroutines for negative logic
1369                                ;OSHUT  MOVE    #>$10,X0
1370                                ;       JSR     <SET_SHUTTER_STATE
1371                                ;       BSET    #ST_SHUT,X:<STATUS
1372                                ;       RTS
1373   
1374                                ;CSHUT  MOVE    #0,X0
1375                                ;       JSR     <SET_SHUTTER_STATE
1376                                ;       BCLR    #ST_SHUT,X:<STATUS
1377                                ;       RTS
1378   
1379   
1380                                ; Fast clear of CCD, executed as a command
1381      P:0002B2 P:0002B2 0D02B4  CLEAR     JSR     <CLR_CCD
1382      P:0002B3 P:0002B3 0C002A            JMP     <FINISH
1383   
1384                                ; Default fast clearing routine with serial clocks inactive
1385                                ; Fast clear image before each exposure, executed as a subroutine
1386      P:0002B4 P:0002B4 4C8400  CLR_CCD   MOVE                          Y:<NPCLR,X0
1387      P:0002B5 P:0002B5 06C400            DO      X0,LPCLR2                         ; Loop over number of lines in image
                            0002BA
1388      P:0002B7 P:0002B7 60F400            MOVE              #PARALLEL_CLEAR,R0      ; Address of parallel transfer waveform
                            000018
1389      P:0002B9 P:0002B9 0D037A            JSR     <CLOCK
1390      P:0002BA P:0002BA 000000            NOP
1391      P:0002BB P:0002BB 060540  LPCLR2    DO      Y:<NSCLR,LSCLR2                   ; Clear out the serial shift register
                            0002BF
1392      P:0002BD P:0002BD 303300            MOVE              #<SERIAL_SKIP,R0
1393      P:0002BE P:0002BE 0D037A            JSR     <CLOCK
1394      P:0002BF P:0002BF 000000            NOP
1395                                LSCLR2
1396      P:0002C0 P:0002C0 0D003D            JSR     <GET_RCV                          ; Check for FO command
1397      P:0002C1 P:0002C1 00000C            RTS
1398   
1399                                ; Start the exposure timer and monitor its progress
1400      P:0002C2 P:0002C2 07F40E  EXPOSE    MOVEP             #0,X:TLR0               ; Load 0 into counter timer
                            000000
1401      P:0002C4 P:0002C4 579000            MOVE              X:<EXPOSURE_TIME,B
1402      P:0002C5 P:0002C5 20000B            TST     B                                 ; Special test for zero exposure time
1403      P:0002C6 P:0002C6 0EA2D7            JEQ     <ZERO_EXP                         ; Don't even start an exposure
1404      P:0002C7 P:0002C7 0A008B            JCLR    #SHUT,X:STATUS,L_SEX0             ; open the shutter if needed
                            0002CA
1405      P:0002C9 P:0002C9 0D02AB            JSR     <OSHUT                            ; Open the shutter if needed
1406                                L_SEX0
1407      P:0002CA P:0002CA 01418C            SUB     #1,B                              ; Timer counts from X:TCPR0+1 to zero
1408      P:0002CB P:0002CB 010F20            BSET    #TIM_BIT,X:TCSR0                  ; Enable the timer #0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 26



1409      P:0002CC P:0002CC 577000            MOVE              B,X:TCPR0
                            FFFF8D
1410      P:0002CE P:0002CE 330700  CHK_RCV   MOVE              #<COM_BUF,R3            ; The beginning of the command buffer
1411      P:0002CF P:0002CF 0A8989            JCLR    #EF,X:HDR,EXP1                    ; Simple test for fast execution
                            0002D3
1412      P:0002D1 P:0002D1 0D003D            JSR     <GET_RCV                          ; Check for an incoming command
1413      P:0002D2 P:0002D2 0E800B            JCS     <PRC_RCV                          ; If command is received, go check it
1414                                EXP1
1415                                ;       JCLR    #ST_DITH,X:STATUS,CHK_TIM
1416                                ;       MOVE    #SERIAL_SKIP,R0
1417                                ;       JSR     <CLOCK
1418   
1419      P:0002D3 P:0002D3 018F95  CHK_TIM   JCLR    #TCF,X:TCSR0,CHK_RCV              ; Wait for timer to equal compare value
                            0002CE
1420      P:0002D5 P:0002D5 010F00  END_EXP   BCLR    #TIM_BIT,X:TCSR0                  ; Disable the timer
1421      P:0002D6 P:0002D6 0AE780            JMP     (R7)                              ; This contains the return address
1422   
1423                                ZERO_EXP
1424      P:0002D7 P:0002D7 44F400            MOVE              #20000,X0               ; delay = 200 microsec, #100 per microsec
                            004E20
1425      P:0002D9 P:0002D9 06C400            DO      X0,END_DELAY
                            0002DB
1426      P:0002DB P:0002DB 000000            NOP
1427                                END_DELAY
1428      P:0002DC P:0002DC 0C02D5            JMP     <END_EXP
1429   
1430                                ; Start the exposure, operate the shutter, and initiate CCD readout
1431                                START_EXPOSURE
1432      P:0002DD P:0002DD 57F400            MOVE              #$020102,B
                            020102
1433      P:0002DF P:0002DF 0D006B            JSR     <XMT_WRD
1434      P:0002E0 P:0002E0 57F400            MOVE              #'IIA',B
                            494941
1435      P:0002E2 P:0002E2 0D006B            JSR     <XMT_WRD
1436      P:0002E3 P:0002E3 0D023A            JSR     <CALC_GEOM
1437   
1438                                ; Clear the CCD and process commands from the host
1439      P:0002E4 P:0002E4 0A00A8            JSET    #NOT_CLR,X:STATUS,NOT_CLR_ARRAY   ; do not clr array if set
                            0002E7
1440      P:0002E6 P:0002E6 0D02B4            JSR     <CLR_CCD                          ; Clear out the CCD
1441                                NOT_CLR_ARRAY
1442                                ; Continue on with exposure
1443      P:0002E7 P:0002E7 300800            MOVE              #TST_RCV,R0             ; Process commands, don't idle,
1444      P:0002E8 P:0002E8 601E00            MOVE              R0,X:<IDL_ADR           ;    during the exposure
1445      P:0002E9 P:0002E9 67F400            MOVE              #L_SEX1,R7              ; Return address at end of exposure
                            0002EC
1446      P:0002EB P:0002EB 0C02C2            JMP     <EXPOSE                           ; Delay for specified exposure time
1447                                L_SEX1
1448   
1449                                ; Now we really start the CCD readout, alerting the PCI board, closing the
1450                                ;  shutter, waiting for it to close and then reading out
1451      P:0002EC P:0002EC 0D036A  STR_RDC   JSR     <PCI_READ_IMAGE                   ; Get the PCI board reading the image
1452      P:0002ED P:0002ED 0A0024            BSET    #ST_RDC,X:<STATUS                 ; Set status to reading out
1453      P:0002EE P:0002EE 0A008B            JCLR    #SHUT,X:STATUS,S_DEL0
                            0002FC
1454      P:0002F0 P:0002F0 0D02AE            JSR     <CSHUT                            ; Close the shutter if necessary
1455   
1456                                ; Delay readout until the shutter has fully closed
1457      P:0002F1 P:0002F1 5E8900            MOVE                          Y:<SHDEL,A
1458      P:0002F2 P:0002F2 200003            TST     A
1459      P:0002F3 P:0002F3 0EF2FC            JLE     <S_DEL0
1460      P:0002F4 P:0002F4 44F400            MOVE              #100000,X0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 27



                            0186A0
1461      P:0002F6 P:0002F6 06CE00            DO      A,S_DEL0                          ; Delay by Y:SHDEL milliseconds
                            0002FB
1462      P:0002F8 P:0002F8 06C400            DO      X0,S_DEL1
                            0002FA
1463      P:0002FA P:0002FA 000000            NOP
1464      P:0002FB P:0002FB 000000  S_DEL1    NOP
1465                                S_DEL0
1466      P:0002FC P:0002FC 0A00AA            JSET    #TST_IMG,X:STATUS,SYNTHETIC_IMAGE
                            000337
1467      P:0002FE P:0002FE 0C01A9            JMP     <RDCCD                            ; Finally, go read out the CCD
1468   
1469                                ; Set the desired exposure time
1470                                SET_EXPOSURE_TIME
1471      P:0002FF P:0002FF 46DB00            MOVE              X:(R3)+,Y0
1472      P:000300 P:000300 461000            MOVE              Y0,X:EXPOSURE_TIME
1473      P:000301 P:000301 07F00D            MOVEP             X:EXPOSURE_TIME,X:TCPR0
                            000010
1474      P:000303 P:000303 0C002A            JMP     <FINISH
1475   
1476                                ; Read the time remaining until the exposure ends
1477                                READ_EXPOSURE_TIME
1478      P:000304 P:000304 47F000            MOVE              X:TCR0,Y1               ; Read elapsed exposure time
                            FFFF8C
1479      P:000306 P:000306 0C002B            JMP     <FINISH1
1480   
1481                                ; Pause the exposure - close the shutter, and stop the timer
1482                                PAUSE_EXPOSURE
1483                                ;       MOVEP   X:TCR0,X:EXPOSURE_TIME  ; Save the elapsed exposure time
1484      P:000307 P:000307 010F00            BCLR    #TIM_BIT,X:TCSR0                  ; Disable the DSP exposure timer
1485      P:000308 P:000308 0D02AE            JSR     <CSHUT                            ; Close the shutter
1486      P:000309 P:000309 0C002A            JMP     <FINISH
1487   
1488                                ; Resume the exposure - open the shutter if needed and restart the timer
1489                                RESUME_EXPOSURE
1490      P:00030A P:00030A 010F29            BSET    #TRM,X:TCSR0                      ; To be sure it will load TLR0
1491      P:00030B P:00030B 07700C            MOVEP             X:TCR0,X:TLR0           ; Restore elapsed exposure time
                            FFFF8E
1492      P:00030D P:00030D 010F20            BSET    #TIM_BIT,X:TCSR0                  ; Re-enable the DSP exposure timer
1493      P:00030E P:00030E 0A008B            JCLR    #SHUT,X:STATUS,L_RES
                            000311
1494      P:000310 P:000310 0D02AB            JSR     <OSHUT                            ; Open the shutter ir necessary
1495      P:000311 P:000311 0C002A  L_RES     JMP     <FINISH
1496   
1497                                ; See if the command issued during readout is a 'ABR'. If not continue readout
1498                                CHK_ABORT_COMMAND
1499      P:000312 P:000312 44DB00            MOVE              X:(R3)+,X0              ; Get candidate header
1500      P:000313 P:000313 56F400            MOVE              #$000202,A
                            000202
1501      P:000315 P:000315 200045            CMP     X0,A
1502      P:000316 P:000316 0E231E            JNE     <RD_CONT
1503      P:000317 P:000317 0D003D  WT_COM    JSR     <GET_RCV                          ; Get the command
1504      P:000318 P:000318 0E0317            JCC     <WT_COM
1505      P:000319 P:000319 44DB00            MOVE              X:(R3)+,X0              ; Get candidate header
1506      P:00031A P:00031A 56F400            MOVE              #'ABR',A
                            414252
1507      P:00031C P:00031C 200045            CMP     X0,A
1508      P:00031D P:00031D 0EA1CC            JEQ     <ABR_RDC
1509      P:00031E P:00031E 330700  RD_CONT   MOVE              #<COM_BUF,R3            ; Continue reading out the CCD
1510      P:00031F P:00031F 227400            MOVE              R3,R4
1511      P:000320 P:000320 0C01D0            JMP     <CONT_RD
1512   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 28



1513                                ; Special ending after abort command to send a 'DON' to the host computer
1514                                RDCCD_END_ABORT
1515      P:000321 P:000321 44F400            MOVE              #100000,X0
                            0186A0
1516      P:000323 P:000323 06C400            DO      X0,*+3                            ; Wait one millisec
                            000325
1517      P:000325 P:000325 000000            NOP
1518      P:000326 P:000326 0A0082            JCLR    #IDLMODE,X:<STATUS,NO_IDL2        ; Don't idle after readout
                            00032C
1519      P:000328 P:000328 60F400            MOVE              #IDLE,R0
                            00019A
1520      P:00032A P:00032A 601E00            MOVE              R0,X:<IDL_ADR
1521      P:00032B P:00032B 0C032E            JMP     <RDC_E2
1522      P:00032C P:00032C 300800  NO_IDL2   MOVE              #TST_RCV,R0
1523      P:00032D P:00032D 601E00            MOVE              R0,X:<IDL_ADR
1524      P:00032E P:00032E 0D0377  RDC_E2    JSR     <WAIT_TO_FINISH_CLOCKING
1525      P:00032F P:00032F 0A0004            BCLR    #ST_RDC,X:<STATUS                 ; Set status to not reading out
1526   
1527      P:000330 P:000330 44F400            MOVE              #$000202,X0             ; Send 'DON' to the host computer
                            000202
1528      P:000332 P:000332 440500            MOVE              X0,X:<HEADER
1529      P:000333 P:000333 0C002A            JMP     <FINISH
1530   
1531                                ; Abort exposure - close the shutter, stop the timer and resume idle mode
1532                                ABORT_EXPOSURE
1533      P:000334 P:000334 010F00            BCLR    #TIM_BIT,X:TCSR0                  ; Disable the DSP exposure timer
1534      P:000335 P:000335 0D02AE            JSR     <CSHUT                            ; Close the shutter
1535      P:000336 P:000336 0C01EE            JMP     <RDC_END
1536   
1537                                ; Generate a synthetic image by simply incrementing the pixel counts
1538                                SYNTHETIC_IMAGE
1539      P:000337 P:000337 200013            CLR     A
1540      P:000338 P:000338 060240            DO      Y:<NPR,LPR_TST                    ; Loop over each line readout
                            000343
1541      P:00033A P:00033A 060140            DO      Y:<NSR,LSR_TST                    ; Loop over number of pixels per line
                            000342
1542      P:00033C P:00033C 0614A0            REP     #20                               ; #20 => 1.0 microsec per pixel
1543      P:00033D P:00033D 000000            NOP
1544      P:00033E P:00033E 014180            ADD     #1,A                              ; Pixel data = Pixel data + 1
1545      P:00033F P:00033F 000000            NOP
1546      P:000340 P:000340 21CF00            MOVE              A,B
1547      P:000341 P:000341 0D0345            JSR     <XMT_PIX                          ;  transmit them
1548      P:000342 P:000342 000000            NOP
1549                                LSR_TST
1550      P:000343 P:000343 000000            NOP
1551                                LPR_TST
1552      P:000344 P:000344 0C01EE            JMP     <RDC_END                          ; Normal exit
1553   
1554                                ; Transmit the 16-bit pixel datum in B1 to the host computer
1555      P:000345 P:000345 0C1DA1  XMT_PIX   ASL     #16,B,B
1556      P:000346 P:000346 000000            NOP
1557      P:000347 P:000347 216500            MOVE              B2,X1
1558      P:000348 P:000348 0C1D91            ASL     #8,B,B
1559      P:000349 P:000349 000000            NOP
1560      P:00034A P:00034A 216400            MOVE              B2,X0
1561      P:00034B P:00034B 000000            NOP
1562      P:00034C P:00034C 09C532            MOVEP             X1,Y:WRFO
1563      P:00034D P:00034D 09C432            MOVEP             X0,Y:WRFO
1564      P:00034E P:00034E 00000C            RTS
1565   
1566                                ; Test the hardware to read A/D values directly into the DSP instead
1567                                ;   of using the SXMIT option, A/Ds #2 and 3.
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 29



1568      P:00034F P:00034F 57F000  READ_AD   MOVE              X:(RDAD+2),B
                            010002
1569      P:000351 P:000351 0C1DA1            ASL     #16,B,B
1570      P:000352 P:000352 000000            NOP
1571      P:000353 P:000353 216500            MOVE              B2,X1
1572      P:000354 P:000354 0C1D91            ASL     #8,B,B
1573      P:000355 P:000355 000000            NOP
1574      P:000356 P:000356 216400            MOVE              B2,X0
1575      P:000357 P:000357 000000            NOP
1576      P:000358 P:000358 09C532            MOVEP             X1,Y:WRFO
1577      P:000359 P:000359 09C432            MOVEP             X0,Y:WRFO
1578      P:00035A P:00035A 060AA0            REP     #10
1579      P:00035B P:00035B 000000            NOP
1580      P:00035C P:00035C 57F000            MOVE              X:(RDAD+3),B
                            010003
1581      P:00035E P:00035E 0C1DA1            ASL     #16,B,B
1582      P:00035F P:00035F 000000            NOP
1583      P:000360 P:000360 216500            MOVE              B2,X1
1584      P:000361 P:000361 0C1D91            ASL     #8,B,B
1585      P:000362 P:000362 000000            NOP
1586      P:000363 P:000363 216400            MOVE              B2,X0
1587      P:000364 P:000364 000000            NOP
1588      P:000365 P:000365 09C532            MOVEP             X1,Y:WRFO
1589      P:000366 P:000366 09C432            MOVEP             X0,Y:WRFO
1590      P:000367 P:000367 060AA0            REP     #10
1591      P:000368 P:000368 000000            NOP
1592      P:000369 P:000369 00000C            RTS
1593   
1594                                ; Alert the PCI interface board that images are coming soon
1595                                PCI_READ_IMAGE
1596      P:00036A P:00036A 57F400            MOVE              #$020104,B              ; Send header word to the FO transmitter
                            020104
1597      P:00036C P:00036C 0D006B            JSR     <XMT_WRD
1598      P:00036D P:00036D 57F400            MOVE              #'RDA',B
                            524441
1599      P:00036F P:00036F 0D006B            JSR     <XMT_WRD
1600      P:000370 P:000370 5FF000            MOVE                          Y:NSR,B     ; Number of columns to read
                            000001
1601      P:000372 P:000372 0D006B            JSR     <XMT_WRD
1602      P:000373 P:000373 5FF000            MOVE                          Y:NPR,B     ; Number of rows to read
                            000002
1603      P:000375 P:000375 0D006B            JSR     <XMT_WRD
1604      P:000376 P:000376 00000C            RTS
1605   
1606                                ; Wait for the clocking to be complete before proceeding
1607                                WAIT_TO_FINISH_CLOCKING
1608      P:000377 P:000377 01ADA1            JSET    #SSFEF,X:PDRD,*                   ; Wait for the SS FIFO to be empty
                            000377
1609      P:000379 P:000379 00000C            RTS
1610   
1611                                ; This MOVEP instruction executes in 30 nanosec, 20 nanosec for the MOVEP,
1612                                ;   and 10 nanosec for the wait state that is required for SRAM writes and
1613                                ;   FIFO setup times. It looks reliable, so will be used for now.
1614   
1615                                ; Core subroutine for clocking out CCD charge
1616      P:00037A P:00037A 0A898E  CLOCK     JCLR    #SSFHF,X:HDR,*                    ; Only write to FIFO if < half full
                            00037A
1617      P:00037C P:00037C 000000            NOP
1618      P:00037D P:00037D 0A898E            JCLR    #SSFHF,X:HDR,CLOCK                ; Guard against metastability
                            00037A
1619      P:00037F P:00037F 4CD800            MOVE                          Y:(R0)+,X0  ; # of waveform entries
1620      P:000380 P:000380 06C400            DO      X0,CLK1                           ; Repeat X0 times
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 30



                            000382
1621      P:000382 P:000382 09D8F3            MOVEP             Y:(R0)+,Y:WRSS          ; 30 nsec Write the waveform to the SS
1622                                CLK1
1623      P:000383 P:000383 000000            NOP
1624      P:000384 P:000384 00000C            RTS                                       ; Return from subroutine
1625   
1626                                ; Work on later !!!
1627                                ; This will execute in 20 nanosec, 10 nanosec for the MOVE and 10 nanosec
1628                                ;   the one wait state that is required for SRAM writes and FIFO setup times.
1629                                ;   However, the assembler gives a WARNING about pipeline problems if its
1630                                ;   put in a DO loop. This problem needs to be resolved later, and in the
1631                                ;   meantime I'll be using the MOVEP instruction.
1632   
1633                                ;       MOVE    #$FFFF03,R6             ; Write switch states, X:(R6)
1634                                ;       MOVE    Y:(R0)+,A  A,X:(R6)
1635   
1636   
1637                                ; Delay for serial writes to the PALs and DACs by 8 microsec
1638      P:000385 P:000385 062083  PAL_DLY   DO      #800,DLY                          ; Wait 8 usec for serial data transmission
                            000387
1639      P:000387 P:000387 000000            NOP
1640      P:000388 P:000388 000000  DLY       NOP
1641      P:000389 P:000389 00000C            RTS
1642   
1643                                ; Let the host computer read the controller configuration
1644                                READ_CONTROLLER_CONFIGURATION
1645      P:00038A P:00038A 4F8A00            MOVE                          Y:<CONFIG,Y1 ; Just transmit the configuration
1646      P:00038B P:00038B 0C002B            JMP     <FINISH1
1647   
1648                                ; Set the video processor gain and integrator speed for all video boards
1649                                ;  Command syntax is  SGN  #GAIN  #SPEED, #GAIN = 1, 2, 5 or 10
1650                                ;                                         #SPEED = 0 for slow, 1 for fast
1651      P:00038C P:00038C 012F23  ST_GAIN   BSET    #3,X:PCRD                         ; Turn the serial clock on
1652      P:00038D P:00038D 56DB00            MOVE              X:(R3)+,A               ; Gain value (1,2,5 or 10)
1653      P:00038E P:00038E 44F400            MOVE              #>1,X0
                            000001
1654      P:000390 P:000390 200045            CMP     X0,A                              ; Check for gain = x1
1655      P:000391 P:000391 0E2395            JNE     <STG2
1656      P:000392 P:000392 57F400            MOVE              #>$77,B
                            000077
1657      P:000394 P:000394 0C03A9            JMP     <STG_A
1658      P:000395 P:000395 44F400  STG2      MOVE              #>2,X0                  ; Check for gain = x2
                            000002
1659      P:000397 P:000397 200045            CMP     X0,A
1660      P:000398 P:000398 0E239C            JNE     <STG5
1661      P:000399 P:000399 57F400            MOVE              #>$BB,B
                            0000BB
1662      P:00039B P:00039B 0C03A9            JMP     <STG_A
1663      P:00039C P:00039C 44F400  STG5      MOVE              #>5,X0                  ; Check for gain = x5
                            000005
1664      P:00039E P:00039E 200045            CMP     X0,A
1665      P:00039F P:00039F 0E23A3            JNE     <STG10
1666      P:0003A0 P:0003A0 57F400            MOVE              #>$DD,B
                            0000DD
1667      P:0003A2 P:0003A2 0C03A9            JMP     <STG_A
1668      P:0003A3 P:0003A3 44F400  STG10     MOVE              #>10,X0                 ; Check for gain = x10
                            00000A
1669      P:0003A5 P:0003A5 200045            CMP     X0,A
1670      P:0003A6 P:0003A6 0E2028            JNE     <ERROR
1671      P:0003A7 P:0003A7 57F400            MOVE              #>$EE,B
                            0000EE
1672   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 31



1673      P:0003A9 P:0003A9 56DB00  STG_A     MOVE              X:(R3)+,A               ; Integrator Speed (0 for slow, 1 for fast)
1674      P:0003AA P:0003AA 000000            NOP
1675      P:0003AB P:0003AB 0ACC00            JCLR    #0,A1,STG_B
                            0003B0
1676      P:0003AD P:0003AD 0ACD68            BSET    #8,B1
1677      P:0003AE P:0003AE 000000            NOP
1678      P:0003AF P:0003AF 0ACD69            BSET    #9,B1
1679      P:0003B0 P:0003B0 44F400  STG_B     MOVE              #$0C3C00,X0
                            0C3C00
1680      P:0003B2 P:0003B2 20004A            OR      X0,B
1681      P:0003B3 P:0003B3 000000            NOP
1682      P:0003B4 P:0003B4 5F0000            MOVE                          B,Y:<GAIN   ; Store the GAIN value for later use
1683   
1684                                ; Send this same value to 15 video processor boards whether they exist or not
1685      P:0003B5 P:0003B5 241000            MOVE              #$100000,X0             ; Increment value
1686      P:0003B6 P:0003B6 21EE00            MOVE              B,A
1687      P:0003B7 P:0003B7 060F80            DO      #15,STG_LOOP
                            0003BB
1688      P:0003B9 P:0003B9 0D017C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1689      P:0003BA P:0003BA 0D0385            JSR     <PAL_DLY                          ; Wait for SSI and PAL to be empty
1690      P:0003BB P:0003BB 200048            ADD     X0,B                              ; Increment the video processor board number
1691                                STG_LOOP
1692      P:0003BC P:0003BC 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1693      P:0003BD P:0003BD 0C002A            JMP     <FINISH
1694      P:0003BE P:0003BE 56DB00  ERR_SGN   MOVE              X:(R3)+,A
1695      P:0003BF P:0003BF 0C0028            JMP     <ERROR
1696   
1697                                ; Set the video processor boards in DC-coupled diagnostic mode or not
1698                                ; Command syntax is  SDC #      # = 0 for normal operation
1699                                ;                               # = 1 for DC coupled diagnostic mode
1700      P:0003C0 P:0003C0 012F23  SET_DC    BSET    #3,X:PCRD                         ; Turn the serial clock on
1701      P:0003C1 P:0003C1 44DB00            MOVE              X:(R3)+,X0
1702      P:0003C2 P:0003C2 0AC420            JSET    #0,X0,SDC_1
                            0003C7
1703      P:0003C4 P:0003C4 0A004A            BCLR    #10,Y:<GAIN
1704      P:0003C5 P:0003C5 0A004B            BCLR    #11,Y:<GAIN
1705      P:0003C6 P:0003C6 0C03C9            JMP     <SDC_A
1706      P:0003C7 P:0003C7 0A006A  SDC_1     BSET    #10,Y:<GAIN
1707      P:0003C8 P:0003C8 0A006B            BSET    #11,Y:<GAIN
1708      P:0003C9 P:0003C9 241000  SDC_A     MOVE              #$100000,X0             ; Increment value
1709      P:0003CA P:0003CA 060F80            DO      #15,SDC_LOOP
                            0003CF
1710      P:0003CC P:0003CC 5E8000            MOVE                          Y:<GAIN,A
1711      P:0003CD P:0003CD 0D017C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1712      P:0003CE P:0003CE 0D0385            JSR     <PAL_DLY                          ; Wait for SSI and PAL to be empty
1713      P:0003CF P:0003CF 200048            ADD     X0,B                              ; Increment the video processor board number
1714                                SDC_LOOP
1715      P:0003D0 P:0003D0 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1716      P:0003D1 P:0003D1 0C002A            JMP     <FINISH
1717   
1718                                ; Set a particular DAC numbers, for setting DC bias voltages, clock driver
1719                                ;   voltages and video processor offset
1720                                ;
1721                                ; SBN  #BOARD  #DAC  ['CLK' or 'VID']  voltage
1722                                ;
1723                                ;                               #BOARD is from 0 to 15
1724                                ;                               #DAC number
1725                                ;                               #voltage is from 0 to 4095
1726   
1727                                SET_BIAS_NUMBER                                     ; Set bias number
1728      P:0003D2 P:0003D2 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock
1729      P:0003D3 P:0003D3 56DB00            MOVE              X:(R3)+,A               ; First argument is board number, 0 to 15
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 32



1730      P:0003D4 P:0003D4 0614A0            REP     #20
1731      P:0003D5 P:0003D5 200033            LSL     A
1732      P:0003D6 P:0003D6 000000            NOP
1733      P:0003D7 P:0003D7 21C400            MOVE              A,X0
1734      P:0003D8 P:0003D8 56DB00            MOVE              X:(R3)+,A               ; Second argument is DAC number
1735      P:0003D9 P:0003D9 060EA0            REP     #14
1736      P:0003DA P:0003DA 200033            LSL     A
1737      P:0003DB P:0003DB 200042            OR      X0,A
1738      P:0003DC P:0003DC 57DB00            MOVE              X:(R3)+,B               ; Third argument is 'VID' or 'CLK' string
1739      P:0003DD P:0003DD 44F400            MOVE              #'VID',X0
                            564944
1740      P:0003DF P:0003DF 20004D            CMP     X0,B
1741      P:0003E0 P:0003E0 0E23E5            JNE     <CLK_DRV
1742      P:0003E1 P:0003E1 0ACC73            BSET    #19,A1                            ; Set bits to mean video processor DAC
1743      P:0003E2 P:0003E2 000000            NOP
1744      P:0003E3 P:0003E3 0ACC72            BSET    #18,A1
1745      P:0003E4 P:0003E4 0C03E9            JMP     <VID_BRD
1746      P:0003E5 P:0003E5 44F400  CLK_DRV   MOVE              #'CLK',X0
                            434C4B
1747      P:0003E7 P:0003E7 20004D            CMP     X0,B
1748      P:0003E8 P:0003E8 0E23F3            JNE     <ERR_SBN
1749      P:0003E9 P:0003E9 21C400  VID_BRD   MOVE              A,X0
1750      P:0003EA P:0003EA 56DB00            MOVE              X:(R3)+,A               ; Fourth argument is voltage value, 0 to $ff
f
1751      P:0003EB P:0003EB 46F400            MOVE              #$000FFF,Y0             ; Mask off just 12 bits to be sure
                            000FFF
1752      P:0003ED P:0003ED 200056            AND     Y0,A
1753      P:0003EE P:0003EE 200042            OR      X0,A
1754      P:0003EF P:0003EF 0D017C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1755      P:0003F0 P:0003F0 0D0385            JSR     <PAL_DLY                          ; Wait for the number to be sent
1756      P:0003F1 P:0003F1 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1757      P:0003F2 P:0003F2 0C002A            JMP     <FINISH
1758      P:0003F3 P:0003F3 56DB00  ERR_SBN   MOVE              X:(R3)+,A               ; Read and discard the fourth argument
1759      P:0003F4 P:0003F4 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1760      P:0003F5 P:0003F5 0C0028            JMP     <ERROR
1761   
1762                                ; Specify the MUX value to be output on the clock driver board
1763                                ; Command syntax is  SMX  #clock_driver_board #MUX1 #MUX2
1764                                ;                               #clock_driver_board from 0 to 15
1765                                ;                               #MUX1, #MUX2 from 0 to 23
1766   
1767      P:0003F6 P:0003F6 012F23  SET_MUX   BSET    #3,X:PCRD                         ; Turn on the serial clock
1768      P:0003F7 P:0003F7 56DB00            MOVE              X:(R3)+,A               ; Clock driver board number
1769      P:0003F8 P:0003F8 0614A0            REP     #20
1770      P:0003F9 P:0003F9 200033            LSL     A
1771      P:0003FA P:0003FA 44F400            MOVE              #$003000,X0
                            003000
1772      P:0003FC P:0003FC 200042            OR      X0,A
1773      P:0003FD P:0003FD 000000            NOP
1774      P:0003FE P:0003FE 21C500            MOVE              A,X1                    ; Move here for storage
1775   
1776                                ; Get the first MUX number
1777      P:0003FF P:0003FF 56DB00            MOVE              X:(R3)+,A               ; Get the first MUX number
1778      P:000400 P:000400 0AF0A9            JLT     ERR_SM1
                            000444
1779      P:000402 P:000402 44F400            MOVE              #>24,X0                 ; Check for argument less than 32
                            000018
1780      P:000404 P:000404 200045            CMP     X0,A
1781      P:000405 P:000405 0AF0A1            JGE     ERR_SM1
                            000444
1782      P:000407 P:000407 21CF00            MOVE              A,B
1783      P:000408 P:000408 44F400            MOVE              #>7,X0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 33



                            000007
1784      P:00040A P:00040A 20004E            AND     X0,B
1785      P:00040B P:00040B 44F400            MOVE              #>$18,X0
                            000018
1786      P:00040D P:00040D 200046            AND     X0,A
1787      P:00040E P:00040E 0E2411            JNE     <SMX_1                            ; Test for 0 <= MUX number <= 7
1788      P:00040F P:00040F 0ACD63            BSET    #3,B1
1789      P:000410 P:000410 0C041C            JMP     <SMX_A
1790      P:000411 P:000411 44F400  SMX_1     MOVE              #>$08,X0
                            000008
1791      P:000413 P:000413 200045            CMP     X0,A                              ; Test for 8 <= MUX number <= 15
1792      P:000414 P:000414 0E2417            JNE     <SMX_2
1793      P:000415 P:000415 0ACD64            BSET    #4,B1
1794      P:000416 P:000416 0C041C            JMP     <SMX_A
1795      P:000417 P:000417 44F400  SMX_2     MOVE              #>$10,X0
                            000010
1796      P:000419 P:000419 200045            CMP     X0,A                              ; Test for 16 <= MUX number <= 23
1797      P:00041A P:00041A 0E2444            JNE     <ERR_SM1
1798      P:00041B P:00041B 0ACD65            BSET    #5,B1
1799      P:00041C P:00041C 20006A  SMX_A     OR      X1,B1                             ; Add prefix to MUX numbers
1800      P:00041D P:00041D 000000            NOP
1801      P:00041E P:00041E 21A700            MOVE              B1,Y1
1802   
1803                                ; Add on the second MUX number
1804      P:00041F P:00041F 56DB00            MOVE              X:(R3)+,A               ; Get the next MUX number
1805      P:000420 P:000420 0E9028            JLT     <ERROR
1806      P:000421 P:000421 44F400            MOVE              #>24,X0                 ; Check for argument less than 32
                            000018
1807      P:000423 P:000423 200045            CMP     X0,A
1808      P:000424 P:000424 0E1028            JGE     <ERROR
1809      P:000425 P:000425 0606A0            REP     #6
1810      P:000426 P:000426 200033            LSL     A
1811      P:000427 P:000427 000000            NOP
1812      P:000428 P:000428 21CF00            MOVE              A,B
1813      P:000429 P:000429 44F400            MOVE              #$1C0,X0
                            0001C0
1814      P:00042B P:00042B 20004E            AND     X0,B
1815      P:00042C P:00042C 44F400            MOVE              #>$600,X0
                            000600
1816      P:00042E P:00042E 200046            AND     X0,A
1817      P:00042F P:00042F 0E2432            JNE     <SMX_3                            ; Test for 0 <= MUX number <= 7
1818      P:000430 P:000430 0ACD69            BSET    #9,B1
1819      P:000431 P:000431 0C043D            JMP     <SMX_B
1820      P:000432 P:000432 44F400  SMX_3     MOVE              #>$200,X0
                            000200
1821      P:000434 P:000434 200045            CMP     X0,A                              ; Test for 8 <= MUX number <= 15
1822      P:000435 P:000435 0E2438            JNE     <SMX_4
1823      P:000436 P:000436 0ACD6A            BSET    #10,B1
1824      P:000437 P:000437 0C043D            JMP     <SMX_B
1825      P:000438 P:000438 44F400  SMX_4     MOVE              #>$400,X0
                            000400
1826      P:00043A P:00043A 200045            CMP     X0,A                              ; Test for 16 <= MUX number <= 23
1827      P:00043B P:00043B 0E2028            JNE     <ERROR
1828      P:00043C P:00043C 0ACD6B            BSET    #11,B1
1829      P:00043D P:00043D 200078  SMX_B     ADD     Y1,B                              ; Add prefix to MUX numbers
1830      P:00043E P:00043E 000000            NOP
1831      P:00043F P:00043F 21AE00            MOVE              B1,A
1832      P:000440 P:000440 0D017C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1833      P:000441 P:000441 0D0385            JSR     <PAL_DLY                          ; Delay for all this to happen
1834      P:000442 P:000442 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1835      P:000443 P:000443 0C002A            JMP     <FINISH
1836      P:000444 P:000444 56DB00  ERR_SM1   MOVE              X:(R3)+,A
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  timCCDmisc.asm  Page 34



1837      P:000445 P:000445 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1838      P:000446 P:000446 0C0028            JMP     <ERROR
1839   
1840   
1841   
1842   
1843                                 TIMBOOT_X_MEMORY
1844      000447                              EQU     @LCV(L)
1845   
1846                                ;  ****************  Setup memory tables in X: space ********************
1847   
1848                                ; Define the address in P: space where the table of constants begins
1849   
1850                                          IF      @SCP("HOST","HOST")
1851      X:000030 X:000030                   ORG     X:END_COMMAND_TABLE,X:END_COMMAND_TABLE
1852                                          ENDIF
1853   
1854                                          IF      @SCP("HOST","ROM")
1856                                          ENDIF
1857   
1858                                ; Application commands
1859      X:000030 X:000030                   DC      'PON',POWER_ON
1860      X:000032 X:000032                   DC      'POF',POWER_OFF
1861      X:000034 X:000034                   DC      'SBV',SET_BIAS_VOLTAGES
1862      X:000036 X:000036                   DC      'IDL',START_IDLE_CLOCKING
1863      X:000038 X:000038                   DC      'OSH',OPEN_SHUTTER
1864      X:00003A X:00003A                   DC      'CSH',CLOSE_SHUTTER
1865      X:00003C X:00003C                   DC      'RDC',STR_RDC
1866      X:00003E X:00003E                   DC      'CLR',CLEAR
1867      X:000040 X:000040                   DC      'PSH',P_SHIFT
1868      X:000042 X:000042                   DC      'BIN',SBINN
1869      X:000044 X:000044                   DC      'GEO',SET_GEOMETRY
1870      X:000046 X:000046                   DC      'ROI',SET_ROI
1871   
1872                                ; Exposure and readout control routines
1873      X:000048 X:000048                   DC      'SET',SET_EXPOSURE_TIME
1874      X:00004A X:00004A                   DC      'RET',READ_EXPOSURE_TIME
1875      X:00004C X:00004C                   DC      'SEX',START_EXPOSURE
1876      X:00004E X:00004E                   DC      'PEX',PAUSE_EXPOSURE
1877      X:000050 X:000050                   DC      'REX',RESUME_EXPOSURE
1878      X:000052 X:000052                   DC      'AEX',ABORT_EXPOSURE
1879      X:000054 X:000054                   DC      'ABR',ABR_RDC
1880      X:000056 X:000056                   DC      'CRD',CONT_RD
1881   
1882                                ; Support routines
1883      X:000058 X:000058                   DC      'SGN',ST_GAIN
1884      X:00005A X:00005A                   DC      'SDC',SET_DC
1885      X:00005C X:00005C                   DC      'SBN',SET_BIAS_NUMBER
1886      X:00005E X:00005E                   DC      'SMX',SET_MUX
1887      X:000060 X:000060                   DC      'CSW',CLR_SWS
1888      X:000062 X:000062                   DC      'RCC',READ_CONTROLLER_CONFIGURATION
1889   
1890                                 END_APPLICATON_COMMAND_TABLE
1891      000064                              EQU     @LCV(L)
1892   
1893                                          IF      @SCP("HOST","HOST")
1894      000021                    NUM_COM   EQU     (@LCV(R)-COM_TBL_R)/2             ; Number of boot +
1895                                                                                    ;  application commands
1896      0002D3                    EXPOSING  EQU     CHK_TIM                           ; Address if exposing
1897                                 CONTINUE_READING
1898      0001D0                              EQU     CONT_RD                           ; Address if reading out
1899                                          ENDIF
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  tim.asm  Page 35



1900   
1901                                          IF      @SCP("HOST","ROM")
1903                                          ENDIF
1904   
1905                                ; Now let's go for the timing waveform tables
1906                                          IF      @SCP("HOST","HOST")
1907      Y:000000 Y:000000                   ORG     Y:0,Y:0
1908                                          ENDIF
1909   
1910      Y:000000 Y:000000         GAIN      DC      END_APPLICATON_Y_MEMORY-@LCV(L)-1
1911   
1912      Y:000001 Y:000001         NSR       DC      2136                              ; number of serial transfers
1913      Y:000002 Y:000002         NPR       DC      2048                              ; number of parallel transfers
1914      Y:000003 Y:000003         NS_DEL    DC      60
1915      Y:000004 Y:000004         NPCLR     DC      NP_CLR                            ; To clear the parallels
1916      Y:000005 Y:000005         NSCLR     DC      NS_CLR                            ; To clear the serial register
1917      Y:000006 Y:000006         NSBIN     DC      1                                 ; Serial binning parameter
1918      Y:000007 Y:000007         NPBIN     DC      1                                 ; Parallel binning parameter
1919      Y:000008 Y:000008         TST_DAT   DC      0                                 ; Temporary definition for test images
1920      Y:000009 Y:000009         SHDEL     DC      SH_DEL                            ; Delay in milliseconds between shutter clos
ing
1921                                                                                    ;   and image readout
1922      Y:00000A Y:00000A         CONFIG    DC      CC                                ; Controller configuration
1923      Y:00000B Y:00000B         NPSHF     DC      64                                ; default # of parallels to shift w/ PSH com
mand.
1924      Y:00000C Y:00000C         NSBINM1   DC      0                                 ; Serial binning factor minus 1
1925      Y:00000D Y:00000D         VERSION   DC      $00008C                           ; Version number of this code. (0x8C==140=>1
.4)
1926      Y:00000E Y:00000E         NPSKP     DC      0                                 ; number of lines to skip to get to ROI
1927      Y:00000F Y:00000F         NSUND     DC      24                                ; number of underscan (prescan) pixels
1928      Y:000010 Y:000010         NSSKP     DC      0                                 ; number of pixels to skip to get to ROI
1929      Y:000011 Y:000011         NSRD      DC      2048                              ; number of pixels to read in the ROI
1930      Y:000012 Y:000012         NSSKP2    DC      0                                 ; number of pixels to skip to get to oversca
n
1931      Y:000013 Y:000013         NSOCK     DC      64                                ; number of overscan (bias) pixels
1932      Y:000014 Y:000014         NP2READ   DC      0                                 ; number of overscan (bias) pixels
1933      Y:000015 Y:000015         NSDATA    DC      2048                              ; number of data (bias) pixels
1934      Y:000016 Y:000016         NSTST     DC      0                                 ; number of data (bias) pixels
1935      Y:000017 Y:000017         NPTST     DC      0                                 ; number of data (bias) pixels
1936                                ;
1937                                ; Note:  NSR = 2048 + NSUND (default=24) + NSOCK (default=64)
1938                                ;
1939   
1940                                ; Include the waveform table for the designated type of CCD
1941                                          INCLUDE "CCD44-82.waveforms"              ; Readout and clocking waveform file
1942                                       COMMENT *
1943   
1944                                        *
1945   
1946                                ; LEFT CHANNEL READOUT
1947   
1948                                ; Miscellaneous definitions
1949      200000                    CLKAD     EQU     $200000
1950      000000                    VIDEO     EQU     $000000                           ; Video processor board select = 0
1951      002000                    CLK2      EQU     $002000                           ; Clock driver board lower half
1952      003000                    CLK3      EQU     $003000                           ; Clock driver board upper half
1953   
1954                                ; Delay numbers for clocking
1955      B80000                    P_DLY     EQU     $B80000                           ; Parallel clock delay
1956      090000                    S_DLY     EQU     $090000                           ; Serial shift time per state.
1957                                                                                    ; Gets noisy if shorter (or is it the shorte
r dwell?)
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  CCD44-82.waveforms  Page 36



1958                                ;SS_DLY EQU     $310000 ; Serial skip time per state
1959      030000                    SS_DLY    EQU     $030000                           ; Serial skip time per state
1960      040000                    SW_DLY    EQU     $040000                           ; summing well settling time.  2 may be ok.
1961      000834                    NS_CLR    EQU     2100                              ; Serial clocks to clear
1962      000400                    NP_CLR    EQU     1024                              ; Parallel clocks to clear (per amplifier)
1963      000032                    SH_DEL    EQU     50                                ; Shutter delay in milliseconds
1964      070000                    ADC_DLY   EQU     $070000                           ; ADC sample time = 300 ns, but some slew oc
curs during integration.
1965      040000                    RG_DLY    EQU     $040000                           ; reset gate settling before DC restore
1966   
1967      110000                    CNV_DLY   EQU     $110000                           ; RG_DLY+CNV_DLY+40ns >> 815 ns, say 880ns.
1968      030000                    DCR_DLY   EQU     $030000                           ; DC Restore, settle time after xmit > 160ns
1969   
1970                                ;INT_DLY EQU    $000000 ; integration delay -> measured 2.48 usecs/pix
1971                                ;INT_DLY EQU    $140000 ; adjust to extend dwell time  -> measured 3.28 usecs/pix
1972                                ;INT_DLY EQU    $260000 ; adjust to extend dwell time  -> measured 4.00 usecs/pix
1973      3F0000                    INT_DLY   EQU     $3F0000                           ; adjust to extend dwell time  -> measured 5
.00 usecs/pix
1974                                ;INT_DLY EQU    $580000 ; adjust to extend dwell time  -> measured 6.00 usecs/pix
1975                                ;INT_DLY EQU    $7D0000 ; adjust to extend dwell time  -> measured 7.00 usecs/pix
1976                                ;INT_DLY2 EQU   $310000 ; add to 7us dwell time to get 9.00 usecs/pix
1977                                ;INT_DLY EQU    $130000 ;
1978                                ;INT_DLY2 EQU   $130000 ;
1979   
1980                                ; clocking voltages
1981      1.000000E+000             P_HI      EQU     +1.0                              ; Parallel clock        ; datasheet +10
1982      -9.000000E+000            P_LO      EQU     -9.0                              ; datasheet 0.0
1983      3.000000E+000             RG_HI     EQU     +3.0                              ; Reset Gate            ; datasheet +12.0
1984      -3.000000E+000            RG_LO     EQU     -3.0                              ; datasheet 0.0
1985      2.000000E+000             S_HI      EQU     +2.0                              ; Serial clocks         ; datasheet +11.0
1986      -8.000000E+000            S_LO      EQU     -8.0                              ; datasheet 1.0
1987      2.000000E+000             SW_HI     EQU     +2.0                              ; Summing well          ; datasheet +11.0
1988      -9.000000E+000            SW_LO     EQU     -9.0                              ; datasheet 0.0
1989      3.000000E+000             DG_HI     EQU     3.0                               ; Dump Gate
1990      -9.000000E+000            DG_LO     EQU     -9.0                              ; Dump Gate
1991      -9.000000E+000            ZERO      EQU     -9.0
1992      1.240000E+001             CLKmax    EQU     +12.4
1993   
1994                                ; DC Bias definition, in volts
1995      2.000000E+001             BOD       EQU     +20.0                             ; Output Drain both     ; datasheet +29
1996      000016                    BJD       EQU     +22                               ; Jfet Drain both       ; datasheet +31
1997      8.000000E+000             BRDL      EQU     +8.0                              ; Reset Drain           ; datasheet +17
1998      8.000000E+000             BRDR      EQU     +8.0
1999      1.500000E+001             BDD       EQU     +15.0                             ; Dump Drain            ; datasheet +24
2000      -6.000000E+000            BOG1      EQU     -6.0                              ; Output gate 1         ; datasheet +3
2001      -3.000000E+000            BOG2      EQU     -3.0                              ; Output gate 2         ; datasheet +4
2002   
2003                                ; Output video offset parameters
2004      0005DF                    OFFSET0   EQU     1503
2005      000641                    OFFSET1   EQU     1601
2006   
2007                                ; ******************************************************
2008                                ; ******************************************************
2009   
2010                                ; Define switch state bits for the CCD clocks - CLK2, which is lower bank
2011      000001                    P1        EQU     1                                 ; Parallel shift register phase #1, L & R am
ps
2012      000002                    P2        EQU     2                                 ; Parallel shift register phase #2, L & R am
ps
2013      000004                    P3        EQU     4                                 ; Parallel shift register phase #3, L & R am
ps
2014      000008                    H_DG      EQU     8                                 ; Dump Gate
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  CCD44-82.waveforms  Page 37



2015      000000                    L_DG      EQU     0                                 ; Dump Gate
2016   
2017                                ; Define switch state bits for the CCD clocks - CLK3, which is upper bank
2018      000001                    S1L       EQU     1                                 ; Serial shift register phase #1, left quadr
ant
2019      000002                    S2L       EQU     2                                 ; Serial shift register phase #2, left
2020      000004                    SWL       EQU     4                                 ; Summing well, Left quadrant
2021      000008                    S1R       EQU     8                                 ; Serial shift register phase #1, right quad
rant
2022      000010                    S2R       EQU     $10                               ; Serial shift register phase #2, right
2023      000020                    SWR       EQU     $20                               ; Summing well, Right quadrant
2024      000040                    S3        EQU     $40                               ; Serial shift register phase #3, both
2025      000080                    RGL       EQU     $80                               ; Reset Gate left
2026      000100                    RGR       EQU     $100                              ; Reset Gate right
2027   
2028   
2029                                ;  ***  Definitions for Y: memory waveform tables  *****
2030   
2031                                ; waveforms para el borrado paralelo (ie parallel erase.)
2032                                PARALLEL_CLEAR
2033      Y:000018 Y:000018                   DC      END_PARALLEL_CLEAR-PARALLEL_CLEAR-1
2034      Y:000019 Y:000019                   DC      CLK2+P_DLY+00+P2+P3+H_DG
2035      Y:00001A Y:00001A                   DC      CLK2+P_DLY+00+00+P3+H_DG
2036      Y:00001B Y:00001B                   DC      CLK2+P_DLY+P1+00+P3+H_DG
2037      Y:00001C Y:00001C                   DC      CLK2+P_DLY+P1+00+00+H_DG
2038      Y:00001D Y:00001D                   DC      CLK2+P_DLY+P1+P2+00+H_DG
2039      Y:00001E Y:00001E                   DC      CLK2+P_DLY+00+P2+00+H_DG
2040                                END_PARALLEL_CLEAR
2041   
2042                                ; Clock the image down
2043                                PARALLEL_SHIFT
2044      Y:00001F Y:00001F                   DC      END_PARALLEL_SHIFT-PARALLEL_SHIFT-1
2045      Y:000020 Y:000020                   DC      CLK3+S_DLY+S1L+S1R+S2L+S2R+00+RGL+RGR+SWR+SWL
2046      Y:000021 Y:000021                   DC      CLK2+P_DLY+00+P2+P3+L_DG
2047      Y:000022 Y:000022                   DC      CLK2+P_DLY+00+00+P3+L_DG
2048      Y:000023 Y:000023                   DC      CLK2+P_DLY+P1+00+P3+L_DG
2049      Y:000024 Y:000024                   DC      CLK2+P_DLY+P1+00+00+L_DG
2050      Y:000025 Y:000025                   DC      CLK2+P_DLY+P1+P2+00+L_DG
2051      Y:000026 Y:000026                   DC      CLK2+P_DLY+00+P2+00+L_DG
2052                                END_PARALLEL_SHIFT
2053   
2054                                PREPARE_DUMP
2055      Y:000027 Y:000027                   DC      END_PREPARE_DUMP-PREPARE_DUMP-1
2056      Y:000028 Y:000028                   DC      CLK3+S_DLY+S1L+S1R+S2L+S2R+00+RGL+RGR+SWR+SWL
2057                                END_PREPARE_DUMP
2058   
2059                                PARALLEL_DUMP
2060      Y:000029 Y:000029                   DC      END_PARALLEL_DUMP-PARALLEL_DUMP-1
2061      Y:00002A Y:00002A                   DC      CLK2+P_DLY+00+P2+P3+H_DG
2062      Y:00002B Y:00002B                   DC      CLK2+P_DLY+00+00+P3+H_DG
2063      Y:00002C Y:00002C                   DC      CLK2+P_DLY+P1+00+P3+H_DG
2064      Y:00002D Y:00002D                   DC      CLK2+P_DLY+P1+00+00+H_DG
2065      Y:00002E Y:00002E                   DC      CLK2+P_DLY+P1+P2+00+H_DG
2066      Y:00002F Y:00002F                   DC      CLK2+P_DLY+00+P2+00+H_DG
2067                                END_PARALLEL_DUMP
2068   
2069                                CLEAR_READ_REGISTER
2070      Y:000030 Y:000030                   DC      END_CLEAR_READ_REGISTER-CLEAR_READ_REGISTER-1
2071      Y:000031 Y:000031                   DC      CLK3+S_DLY+000+000+000+000+00+RGL+RGR+000+000
2072      Y:000032 Y:000032                   DC      CLK2+P_DLY+00+P2+00+L_DG
2073                                END_CLEAR_READ_REGISTER
2074   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  CCD44-82.waveforms  Page 38



2075                                ; Serial clocking waveform for skipping
2076                                SERIAL_SKIP
2077      Y:000033 Y:000033                   DC      END_SERIAL_SKIP-SERIAL_SKIP-1
2078      Y:000034 Y:000034                   DC      VIDEO+$000000+%1110100            ; DC Restore(!b1)
2079      Y:000035 Y:000035                   DC      CLK3+SS_DLY+S1L+S2R+S2L+S1R+00+000+000+000+000
2080      Y:000036 Y:000036                   DC      CLK3+SS_DLY+000+000+S2L+S1R+00+000+000+000+000
2081      Y:000037 Y:000037                   DC      VIDEO+$000000+%1110110            ; end DC restore
2082      Y:000038 Y:000038                   DC      CLK3+SS_DLY+000+000+S2L+S1R+S3+RGL+RGR+SWL+SWR
2083      Y:000039 Y:000039                   DC      CLK3+SS_DLY+000+000+000+000+S3+RGL+RGR+SWL+SWR
2084      Y:00003A Y:00003A                   DC      CLK3+SS_DLY+S1L+S2R+000+000+S3+RGL+RGR+SWL+SWR
2085      Y:00003B Y:00003B                   DC      CLK3+SS_DLY+S1L+S2R+000+000+00+000+000+000+000
2086                                END_SERIAL_SKIP
2087   
2088                                ; Serial clocking waveform for binning (shift only) (SW high and RG low)
2089                                SERIAL_SHIFT
2090      Y:00003C Y:00003C                   DC      END_SERIAL_SHIFT-SERIAL_SHIFT-1
2091      Y:00003D Y:00003D                   DC      CLK3+S_DLY+S1L+S2R+S2L+S1R+00+000+000+SWR+SWL
2092      Y:00003E Y:00003E                   DC      CLK3+S_DLY+000+000+S2L+S1R+00+000+000+SWR+SWL
2093      Y:00003F Y:00003F                   DC      CLK3+S_DLY+000+000+S2L+S1R+S3+000+000+SWL+SWR
2094      Y:000040 Y:000040                   DC      CLK3+S_DLY+000+000+000+000+S3+000+000+SWL+SWR
2095      Y:000041 Y:000041                   DC      CLK3+S_DLY+S1L+S2R+000+000+S3+000+000+SWL+SWR
2096      Y:000042 Y:000042                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2097                                END_SERIAL_SHIFT
2098   
2099                                ; Serial clocking waveform for reading (shift plus video proc)
2100                                ; CLK3 = Delay+S1L+S1R+S2L+S2L+S3+RGL+RGR+SWL+SWR
2101                                ;
2102                                SERIAL_READ
2103      Y:000043 Y:000043                   DC      END_SERIAL_READ-SERIAL_READ-1
2104      Y:000044 Y:000044                   DC      VIDEO+$000000+%0010111            ; Stop resetting integrator
2105   
2106      Y:000045 Y:000045                   DC      VIDEO+000000+%0000111             ; Integrate
2107      Y:000046 Y:000046                   DC      CLK3+S_DLY+S1L+S2R+S2L+S1R+00+000+000+SWR+SWL
2108      Y:000047 Y:000047                   DC      CLK3+S_DLY+000+000+S2L+S1R+00+000+000+SWR+SWL
2109      Y:000048 Y:000048                   DC      CLK3+S_DLY+000+000+S2L+S1R+S3+000+000+SWL+SWR
2110      Y:000049 Y:000049                   DC      CLK3+S_DLY+000+000+000+000+S3+000+000+SWL+SWR
2111      Y:00004A Y:00004A                   DC      CLK3+S_DLY+S1L+S2R+000+000+S3+000+000+SWL+SWR
2112      Y:00004B Y:00004B                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2113      Y:00004C Y:00004C                   DC      CLK3+INT_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2114                                ;       DC      CLK3+INT_DLY2+S1L+S2R+000+000+00+000+000+SWL+SWR
2115      Y:00004D Y:00004D                   DC      VIDEO+$000000+%0011011            ; Stop Integrate
2116   
2117      Y:00004E Y:00004E                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+000+000
2118      Y:00004F Y:00004F                   DC      CLK3+SW_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2119   
2120      Y:000050 Y:000050                   DC      VIDEO+$00000+%0001011             ; Integrate
2121      Y:000051 Y:000051                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2122      Y:000052 Y:000052                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2123      Y:000053 Y:000053                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2124      Y:000054 Y:000054                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2125      Y:000055 Y:000055                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2126      Y:000056 Y:000056                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2127      Y:000057 Y:000057                   DC      CLK3+INT_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2128                                ;       DC      CLK3+INT_DLY2+S1L+S2R+000+000+00+000+000+SWL+SWR
2129      Y:000058 Y:000058                   DC      VIDEO+$000000+%0011011            ; Stop integrator
2130   
2131      Y:000059 Y:000059                   DC      CLK3+ADC_DLY+S1L+S2R+000+000+00+RGL+RGR+SWL+SWR ; Reset Gate, ADC sample>300 n
s
2132      Y:00005A Y:00005A                   DC      VIDEO+$000000+%0111011            ; Start convert=hold (lo to hi on bit 5 -SS5
-)
2133      Y:00005B Y:00005B                   DC      VIDEO+$000000+%0111011            ; redundant write manage beat frequencies
2134      Y:00005C Y:00005C                   DC      VIDEO+$000000+%0111011            ; redundant write manage beat frequencies
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  CCD44-82.waveforms  Page 39



2135      Y:00005D Y:00005D                   DC      CLK3+$000000+S1L+S2R+000+000+00+000+000+SWR+SWL ; end of RG pulse
2136      Y:00005E Y:00005E                   DC      VIDEO+RG_DLY+%1110110             ; Reset Integrator(!b0), RG settling
2137      Y:00005F Y:00005F                   DC      VIDEO+CNV_DLY+%1110100            ; DC Restore(!b1)
2138      Y:000060 Y:000060                   DC      $00F000                           ; Transmit A/D data to host
2139      Y:000061 Y:000061                   DC      CLK3+DCR_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR ; DC restore & settle after xm
it
2140                                END_SERIAL_READ
2141   
2142                                DUMMY_READ
2143      Y:000062 Y:000062                   DC      END_DUMMY_READ-DUMMY_READ-1
2144      Y:000063 Y:000063                   DC      VIDEO+$000000+%0010111            ; Stop resetting integrator
2145                                                                                    ;      DC      VIDEO+$000000+%0010111       
   ; Stop resetting integrator (unnecessary?)
2146   
2147      Y:000064 Y:000064                   DC      VIDEO+00000+%0000111              ; Integrate
2148      Y:000065 Y:000065                   DC      CLK3+S_DLY+S1L+S2R+S2L+S1R+00+000+000+SWR+SWL
2149      Y:000066 Y:000066                   DC      CLK3+S_DLY+000+000+S2L+S1R+00+000+000+SWR+SWL
2150      Y:000067 Y:000067                   DC      CLK3+S_DLY+000+000+S2L+S1R+S3+000+000+SWL+SWR
2151      Y:000068 Y:000068                   DC      CLK3+S_DLY+000+000+000+000+S3+000+000+SWL+SWR
2152      Y:000069 Y:000069                   DC      CLK3+S_DLY+S1L+S2R+000+000+S3+000+000+SWL+SWR
2153      Y:00006A Y:00006A                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2154      Y:00006B Y:00006B                   DC      CLK3+INT_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2155                                ;       DC      CLK3+INT_DLY2+S1L+S2R+000+000+00+000+000+SWL+SWR
2156      Y:00006C Y:00006C                   DC      VIDEO+$000000+%0011011            ; Stop Integrate
2157   
2158      Y:00006D Y:00006D                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+000+000
2159      Y:00006E Y:00006E                   DC      CLK3+SW_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2160   
2161      Y:00006F Y:00006F                   DC      VIDEO+$000000+%0001011            ; Integrate
2162      Y:000070 Y:000070                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2163      Y:000071 Y:000071                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2164      Y:000072 Y:000072                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2165      Y:000073 Y:000073                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2166      Y:000074 Y:000074                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2167      Y:000075 Y:000075                   DC      CLK3+S_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2168      Y:000076 Y:000076                   DC      CLK3+INT_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR
2169                                ;       DC      CLK3+INT_DLY2+S1L+S2R+000+000+00+000+000+SWL+SWR
2170      Y:000077 Y:000077                   DC      VIDEO+$00000+%0011011             ; Stop integrator
2171   
2172      Y:000078 Y:000078                   DC      CLK3+ADC_DLY+S1L+S2R+000+000+00+RGL+RGR+SWL+SWR ; Reset Gate, ADC sample>300 n
s
2173      Y:000079 Y:000079                   DC      VIDEO+$000000+%0111011            ; Start convert=hold (lo to hi on bit 5 -SS5
-)
2174      Y:00007A Y:00007A                   DC      CLK3+$000000+S1L+S2R+000+000+00+000+000+SWR+SWL ; end of RG pulse
2175      Y:00007B Y:00007B                   DC      VIDEO+RG_DLY+%1110110             ; Reset Integrator(!b0), RG settling
2176      Y:00007C Y:00007C                   DC      VIDEO+CNV_DLY+%1110100            ; DC Restore(!b1)
2177                                                                                    ; transmit >>815 ns after start convert.
2178      Y:00007D Y:00007D                   DC      CLK3+DCR_DLY+S1L+S2R+000+000+00+000+000+SWL+SWR ; DC restore & settle after xm
it
2179                                END_DUMMY_READ
2180   
2181   
2182   
2183                                ; Initialization of clock driver and video processor DACs and switches
2184      Y:00007E Y:00007E         DACS      DC      END_DACS-DACS-1
2185      Y:00007F Y:00007F                   DC      CLKAD+$0A0080
2186      Y:000080 Y:000080                   DC      CLKAD+$000100+@CVI((P_HI+CLKmax)/(2*CLKmax)*255) ; Pin #1, P1
2187      Y:000081 Y:000081                   DC      CLKAD+$000200+@CVI((P_LO+CLKmax)/(2*CLKmax)*255)
2188      Y:000082 Y:000082                   DC      CLKAD+$000400+@CVI((P_HI+CLKmax)/(2*CLKmax)*255) ; Pin #2, P2
2189      Y:000083 Y:000083                   DC      CLKAD+$000800+@CVI((P_LO+CLKmax)/(2*CLKmax)*255)
2190      Y:000084 Y:000084                   DC      CLKAD+$002000+@CVI((P_HI+CLKmax)/(2*CLKmax)*255) ; Pin #3, P3
2191      Y:000085 Y:000085                   DC      CLKAD+$004000+@CVI((P_LO+CLKmax)/(2*CLKmax)*255)
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  CCD44-82.waveforms  Page 40



2192      Y:000086 Y:000086                   DC      CLKAD+$008000+@CVI((DG_HI+CLKmax)/(2*CLKmax)*255) ; Pin #4, DG
2193      Y:000087 Y:000087                   DC      CLKAD+$010000+@CVI((DG_LO+CLKmax)/(2*CLKmax)*255)
2194      Y:000088 Y:000088                   DC      CLKAD+$020100+@CVI((P_HI+CLKmax)/(2*CLKmax)*255) ; Pin #5, ??
2195      Y:000089 Y:000089                   DC      CLKAD+$020200+@CVI((P_LO+CLKmax)/(2*CLKmax)*255)
2196   
2197   
2198      Y:00008A Y:00008A                   DC      CLKAD+$020400+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #6, Unused
2199      Y:00008B Y:00008B                   DC      CLKAD+$020800+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2200      Y:00008C Y:00008C                   DC      CLKAD+$022000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #7, Unused
2201      Y:00008D Y:00008D                   DC      CLKAD+$024000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2202      Y:00008E Y:00008E                   DC      CLKAD+$028000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #8, Unused
2203      Y:00008F Y:00008F                   DC      CLKAD+$030000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2204      Y:000090 Y:000090                   DC      CLKAD+$040100+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #9, Unused
2205      Y:000091 Y:000091                   DC      CLKAD+$040200+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2206      Y:000092 Y:000092                   DC      CLKAD+$040400+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #10, Unused
2207      Y:000093 Y:000093                   DC      CLKAD+$040800+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2208      Y:000094 Y:000094                   DC      CLKAD+$042000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #11, Unused
2209      Y:000095 Y:000095                   DC      CLKAD+$044000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2210      Y:000096 Y:000096                   DC      CLKAD+$048000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #12, Unused
2211      Y:000097 Y:000097                   DC      CLKAD+$050000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2212   
2213   
2214      Y:000098 Y:000098                   DC      CLKAD+$060100+@CVI((S_HI+CLKmax)/(2*CLKmax)*255) ; Pin #13, S1L
2215      Y:000099 Y:000099                   DC      CLKAD+$060200+@CVI((S_LO+CLKmax)/(2*CLKmax)*255)
2216      Y:00009A Y:00009A                   DC      CLKAD+$060400+@CVI((S_HI+CLKmax)/(2*CLKmax)*255) ; Pin #14, S2L
2217      Y:00009B Y:00009B                   DC      CLKAD+$060800+@CVI((S_LO+CLKmax)/(2*CLKmax)*255)
2218      Y:00009C Y:00009C                   DC      CLKAD+$062000+@CVI((SW_HI+CLKmax)/(2*CLKmax)*255) ; Pin #15, SWL
2219      Y:00009D Y:00009D                   DC      CLKAD+$064000+@CVI((SW_LO+CLKmax)/(2*CLKmax)*255)
2220      Y:00009E Y:00009E                   DC      CLKAD+$068000+@CVI((S_HI+CLKmax)/(2*CLKmax)*255) ; Pin #16, S1R
2221      Y:00009F Y:00009F                   DC      CLKAD+$070000+@CVI((S_LO+CLKmax)/(2*CLKmax)*255)
2222      Y:0000A0 Y:0000A0                   DC      CLKAD+$080100+@CVI((S_HI+CLKmax)/(2*CLKmax)*255) ; Pin #17, S2R
2223      Y:0000A1 Y:0000A1                   DC      CLKAD+$080200+@CVI((S_LO+CLKmax)/(2*CLKmax)*255)
2224      Y:0000A2 Y:0000A2                   DC      CLKAD+$080400+@CVI((SW_HI+CLKmax)/(2*CLKmax)*255) ; Pin #18, SWR
2225      Y:0000A3 Y:0000A3                   DC      CLKAD+$080800+@CVI((SW_LO+CLKmax)/(2*CLKmax)*255)
2226      Y:0000A4 Y:0000A4                   DC      CLKAD+$082000+@CVI((S_HI+CLKmax)/(2*CLKmax)*255) ; Pin #19, S3
2227      Y:0000A5 Y:0000A5                   DC      CLKAD+$084000+@CVI((S_LO+CLKmax)/(2*CLKmax)*255)
2228      Y:0000A6 Y:0000A6                   DC      CLKAD+$088000+@CVI((RG_HI+CLKmax)/(2*CLKmax)*255) ; Pin #33, RGL
2229      Y:0000A7 Y:0000A7                   DC      CLKAD+$090000+@CVI((RG_LO+CLKmax)/(2*CLKmax)*255)
2230      Y:0000A8 Y:0000A8                   DC      CLKAD+$0A0100+@CVI((RG_HI+CLKmax)/(2*CLKmax)*255) ; Pin #34, RGR
2231      Y:0000A9 Y:0000A9                   DC      CLKAD+$0A0200+@CVI((RG_LO+CLKmax)/(2*CLKmax)*255)
2232   
2233   
2234      Y:0000AA Y:0000AA                   DC      CLKAD+$0A0400+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #35, Unused
2235      Y:0000AB Y:0000AB                   DC      CLKAD+$0A0800+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2236      Y:0000AC Y:0000AC                   DC      CLKAD+$0A2000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #36, Unused
2237      Y:0000AD Y:0000AD                   DC      CLKAD+$0A4000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2238      Y:0000AE Y:0000AE                   DC      CLKAD+$0A8000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255) ; Pin #37, Unused
2239      Y:0000AF Y:0000AF                   DC      CLKAD+$0B0000+@CVI((ZERO+CLKmax)/(2*CLKmax)*255)
2240   
2241   
2242   
2243                                ; Set gain and integrator speed. (77, bb, dd, ee; low gain to high)
2244                                ;       DC      $0c3c77                 ; Gain x1, slow integ. speed, board #0
2245      Y:0000B0 Y:0000B0                   DC      $0c3f77                           ; Gain x1, fast integrate speed
2246                                ;       DC      $0c3fbb                 ; Gain x2
2247                                ;       DC      $0c3fdd                 ; Gain x4.75
2248                                ;       DC      $0c3fee                 ; Gain x9.50
2249   
2250                                ;       DC      $0c3fbb                 ; Default x2 gain, fast integration
2251                                ;       DC      $0c33bb                 ; Default x2 gain, fast integration (to read the sawtoot
h waveform)
2252   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  12:03:41  CCD44-82.waveforms  Page 41



2253                                ; Output offset voltages
2254      Y:0000B1 Y:0000B1                   DC      $0c4000+OFFSET0                   ; Output video offset, channel 0
2255      Y:0000B2 Y:0000B2                   DC      $0cc000+OFFSET1
2256   
2257                                ; DC bias voltages. Gain and offsets adjusted for video boards SN205 and SN304. Feb. 24, 2005
2258                                ;  (if it is wrong, blame on DG, if it is right, applauses to MB)
2259   
2260      Y:0000B3 Y:0000B3                   DC      $0d0000+@CVI((BOD-6.3)/23.7*4095) ; pin #1, VID0
2261      Y:0000B4 Y:0000B4                   DC      $0d4000+@CVI((BJD-6.3)/23.7*4095) ; pin #2, VID0
2262      Y:0000B5 Y:0000B5                   DC      $0d8000+@CVI((BRDL-3.9)/16.1*4095) ; pin #3, VID0
2263      Y:0000B6 Y:0000B6                   DC      $0dc000+@CVI((BRDR-3.9)/16.1*4095) ; pin #4, VID0
2264      Y:0000B7 Y:0000B7                   DC      $0e0000+@CVI((BDD-3.9)/16.1*4095) ; pin #5, VID0
2265   
2266      Y:0000B8 Y:0000B8                   DC      $0f8000+@CVI((BOG1+10.0)/20.0*4095) ; pin #11, VID0
2267      Y:0000B9 Y:0000B9                   DC      $0fc000+@CVI((BOG2+10.0)/20.0*4095) ; pin #12, VID0
2268   
2269                                END_DACS
2270   
2271                                 END_APPLICATON_Y_MEMORY
2272      0000BA                              EQU     @LCV(L)
2273   
2274                                ; End of program
2275                                          END

0    Errors
0    Warnings


