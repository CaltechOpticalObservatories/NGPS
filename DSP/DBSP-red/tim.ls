Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 1



1                                 COMMENT *
2      
3                          This file is used to generate boot DSP code for the 250 MHz fiber optic
4                                  timing board using a DSP56303 as its main processor. It supports
5                                  split serial and frame transfer, but not split parallel nor binning.
6                                  *
7                                    PAGE    132                               ; Printronix page width - 132 columns
8      
9                          ; Change history:
10                         ; 2011-10-23 (GR): Moved the serial register flush after each parallel skip (was after all parallel skips
) to prevent overflowing of the serial reg.
11     
12     
13                         ; Include the boot and header files so addressing is easy
14                                   INCLUDE "timhdr.asm"
15                                COMMENT *
16     
17                         This is a header file that is shared between the fiber optic timing board
18                         boot and application code files for Rev. 5 = 250 MHz timing boards
19                                 *
20     
21                                   PAGE    132                               ; Printronix page width - 132 columns
22     
23                         ; Various addressing control registers
24        FFFFFB           BCR       EQU     $FFFFFB                           ; Bus Control Register
25        FFFFF9           AAR0      EQU     $FFFFF9                           ; Address Attribute Register, channel 0
26        FFFFF8           AAR1      EQU     $FFFFF8                           ; Address Attribute Register, channel 1
27        FFFFF7           AAR2      EQU     $FFFFF7                           ; Address Attribute Register, channel 2
28        FFFFF6           AAR3      EQU     $FFFFF6                           ; Address Attribute Register, channel 3
29        FFFFFD           PCTL      EQU     $FFFFFD                           ; PLL control register
30        FFFFFE           IPRP      EQU     $FFFFFE                           ; Interrupt Priority register - Peripheral
31        FFFFFF           IPRC      EQU     $FFFFFF                           ; Interrupt Priority register - Core
32     
33                         ; Port E is the Synchronous Communications Interface (SCI) port
34        FFFF9F           PCRE      EQU     $FFFF9F                           ; Port Control Register
35        FFFF9E           PRRE      EQU     $FFFF9E                           ; Port Direction Register
36        FFFF9D           PDRE      EQU     $FFFF9D                           ; Port Data Register
37        FFFF9C           SCR       EQU     $FFFF9C                           ; SCI Control Register
38        FFFF9B           SCCR      EQU     $FFFF9B                           ; SCI Clock Control Register
39     
40        FFFF9A           SRXH      EQU     $FFFF9A                           ; SCI Receive Data Register, High byte
41        FFFF99           SRXM      EQU     $FFFF99                           ; SCI Receive Data Register, Middle byte
42        FFFF98           SRXL      EQU     $FFFF98                           ; SCI Receive Data Register, Low byte
43     
44        FFFF97           STXH      EQU     $FFFF97                           ; SCI Transmit Data register, High byte
45        FFFF96           STXM      EQU     $FFFF96                           ; SCI Transmit Data register, Middle byte
46        FFFF95           STXL      EQU     $FFFF95                           ; SCI Transmit Data register, Low byte
47     
48        FFFF94           STXA      EQU     $FFFF94                           ; SCI Transmit Address Register
49        FFFF93           SSR       EQU     $FFFF93                           ; SCI Status Register
50     
51        000009           SCITE     EQU     9                                 ; X:SCR bit set to enable the SCI transmitter
52        000008           SCIRE     EQU     8                                 ; X:SCR bit set to enable the SCI receiver
53        000000           TRNE      EQU     0                                 ; This is set in X:SSR when the transmitter
54                                                                             ;  shift and data registers are both empty
55        000001           TDRE      EQU     1                                 ; This is set in X:SSR when the transmitter
56                                                                             ;  data register is empty
57        000002           RDRF      EQU     2                                 ; X:SSR bit set when receiver register is full
58        00000F           SELSCI    EQU     15                                ; 1 for SCI to backplane, 0 to front connector
59     
60     
61                         ; ESSI Flags
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timhdr.asm  Page 2



62        000006           TDE       EQU     6                                 ; Set when transmitter data register is empty
63        000007           RDF       EQU     7                                 ; Set when receiver is full of data
64        000010           TE        EQU     16                                ; Transmitter enable
65     
66                         ; Phase Locked Loop initialization
67        050003           PLL_INIT  EQU     $050003                           ; PLL = 25 MHz x 2 = 100 MHz
68     
69                         ; Port B general purpose I/O
70        FFFFC4           HPCR      EQU     $FFFFC4                           ; Control register (bits 1-6 cleared for GPIO)
71        FFFFC9           HDR       EQU     $FFFFC9                           ; Data register
72        FFFFC8           HDDR      EQU     $FFFFC8                           ; Data Direction Register bits (=1 for output)
73     
74                         ; Port C is Enhanced Synchronous Serial Port 0 = ESSI0
75        FFFFBF           PCRC      EQU     $FFFFBF                           ; Port C Control Register
76        FFFFBE           PRRC      EQU     $FFFFBE                           ; Port C Data direction Register
77        FFFFBD           PDRC      EQU     $FFFFBD                           ; Port C GPIO Data Register
78        FFFFBC           TX00      EQU     $FFFFBC                           ; Transmit Data Register #0
79        FFFFB8           RX0       EQU     $FFFFB8                           ; Receive data register
80        FFFFB7           SSISR0    EQU     $FFFFB7                           ; Status Register
81        FFFFB6           CRB0      EQU     $FFFFB6                           ; Control Register B
82        FFFFB5           CRA0      EQU     $FFFFB5                           ; Control Register A
83     
84                         ; Port D is Enhanced Synchronous Serial Port 1 = ESSI1
85        FFFFAF           PCRD      EQU     $FFFFAF                           ; Port D Control Register
86        FFFFAE           PRRD      EQU     $FFFFAE                           ; Port D Data direction Register
87        FFFFAD           PDRD      EQU     $FFFFAD                           ; Port D GPIO Data Register
88        FFFFAC           TX10      EQU     $FFFFAC                           ; Transmit Data Register 0
89        FFFFA7           SSISR1    EQU     $FFFFA7                           ; Status Register
90        FFFFA6           CRB1      EQU     $FFFFA6                           ; Control Register B
91        FFFFA5           CRA1      EQU     $FFFFA5                           ; Control Register A
92     
93                         ; Timer module addresses
94        FFFF8F           TCSR0     EQU     $FFFF8F                           ; Timer control and status register
95        FFFF8E           TLR0      EQU     $FFFF8E                           ; Timer load register = 0
96        FFFF8D           TCPR0     EQU     $FFFF8D                           ; Timer compare register = exposure time
97        FFFF8C           TCR0      EQU     $FFFF8C                           ; Timer count register = elapsed time
98        FFFF83           TPLR      EQU     $FFFF83                           ; Timer prescaler load register => milliseconds
99        FFFF82           TPCR      EQU     $FFFF82                           ; Timer prescaler count register
100       000000           TIM_BIT   EQU     0                                 ; Set to enable the timer
101       000009           TRM       EQU     9                                 ; Set to enable the timer preloading
102       000015           TCF       EQU     21                                ; Set when timer counter = compare register
103    
104                        ; Board specific addresses and constants
105       FFFFF1           RDFO      EQU     $FFFFF1                           ; Read incoming fiber optic data byte
106       FFFFF2           WRFO      EQU     $FFFFF2                           ; Write fiber optic data replies
107       FFFFF3           WRSS      EQU     $FFFFF3                           ; Write switch state
108       FFFFF5           WRLATCH   EQU     $FFFFF5                           ; Write to a latch
109       010000           RDAD      EQU     $010000                           ; Read A/D values into the DSP
110       000009           EF        EQU     9                                 ; Serial receiver empty flag
111    
112                        ; DSP port A bit equates
113       000000           PWROK     EQU     0                                 ; Power control board says power is OK
114       000001           LED1      EQU     1                                 ; Control one of two LEDs
115       000002           LVEN      EQU     2                                 ; Low voltage power enable
116       000003           HVEN      EQU     3                                 ; High voltage power enable
117       00000E           SSFHF     EQU     14                                ; Switch state FIFO half full flag
118       00000A           EXT_IN0   EQU     10                                ; External digital I/O to the timing board
119       00000B           EXT_IN1   EQU     11
120       00000C           EXT_OUT0  EQU     12
121       00000D           EXT_OUT1  EQU     13
122    
123                        ; Port D equate
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timhdr.asm  Page 3



124       000000           SLAVE     EQU     0                                 ; Set if not a master by having jumper 2 not installe
d
125       000001           SSFEF     EQU     1                                 ; Switch state FIFO empty flag
126    
127                        ; Other equates
128       000002           WRENA     EQU     2                                 ; Enable writing to the EEPROM
129    
130                        ; Latch U25 bit equates
131       000000           CDAC      EQU     0                                 ; Clear the analog board DACs
132       000002           ENCK      EQU     2                                 ; Enable the clock outputs
133       000004           SHUTTER   EQU     4                                 ; Control the shutter
134       000005           TIM_U_RST EQU     5                                 ; Reset the utility board
135    
136                        ; Software status bits, defined at X:<STATUS = X:0
137       000000           ST_RCV    EQU     0                                 ; Set to indicate word is from SCI = utility board
138       000002           IDLMODE   EQU     2                                 ; Set if need to idle after readout
139       000003           ST_SHUT   EQU     3                                 ; Set to indicate shutter is closed, clear for open
140       000004           ST_RDC    EQU     4                                 ; Set if executing 'RDC' command - reading out
141       000005           SPLIT_S   EQU     5                                 ; Set if split serial
142       000006           SPLIT_P   EQU     6                                 ; Set if split parallel
143       000007           MPP       EQU     7                                 ; Set if parallels are in MPP mode
144       000008           NOT_CLR   EQU     8                                 ; Set if not to clear CCD before exposure
145       00000A           TST_IMG   EQU     10                                ; Set if controller is to generate a test image
146       00000B           SHUT      EQU     11                                ; Set if opening shutter at beginning of exposure
147       00000C           ST_DITH   EQU     12                                ; Set if to dither during exposure
148       00000D           ST_SYNC   EQU     13                                ; Set if starting exposure on SYNC = high signal
149       00000E           ST_CNRD   EQU     14                                ; Set if in continous readout mode
150                        ; didn't need to commenct out above as we have a 24-bit status word...
151       00000F           POWERST   EQU     15                                ; current power state
152    
153                        ; Address for the table containing the incoming SCI words
154       000400           SCI_TABLE EQU     $400
155    
156    
157                        ; Specify controller configuration bits of the X:STATUS word
158                        ;   to describe the software capabilities of this application file
159                        ; The bit is set (=1) if the capability is supported by the controller
160    
161    
162                                COMMENT *
163    
164                        BIT #'s         FUNCTION
165                        2,1,0           Video Processor
166                                                000     CCD Rev. 3
167                                                001     CCD Gen I
168                                                010     IR Rev. 4
169                                                011     IR Coadder
170                                                100     CCD Rev. 5, Differential input
171                                                101     8x IR
172    
173                        4,3             Timing Board
174                                                00      Rev. 4, Gen II
175                                                01      Gen I
176                                                10      Rev. 5, Gen III, 250 MHz
177    
178                        6,5             Utility Board
179                                                00      No utility board
180                                                01      Utility Rev. 3
181    
182                        7               Shutter
183                                                0       No shutter support
184                                                1       Yes shutter support
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timhdr.asm  Page 4



185    
186                        9,8             Temperature readout
187                                                00      No temperature readout
188                                                01      Polynomial Diode calibration
189                                                10      Linear temperature sensor calibration
190    
191                        10              Subarray readout
192                                                0       Not supported
193                                                1       Yes supported
194    
195                        11              Binning
196                                                0       Not supported
197                                                1       Yes supported
198    
199                        12              Split-Serial readout
200                                                0       Not supported
201                                                1       Yes supported
202    
203                        13              Split-Parallel readout
204                                                0       Not supported
205                                                1       Yes supported
206    
207                        14              MPP = Inverted parallel clocks
208                                                0       Not supported
209                                                1       Yes supported
210    
211                        16,15           Clock Driver Board
212                                                00      Rev. 3
213                                                11      No clock driver board (Gen I)
214    
215                        19,18,17                Special implementations
216                                                000     Somewhere else
217                                                001     Mount Laguna Observatory
218                                                010     NGST Aladdin
219                                                xxx     Other
220                                *
221    
222                        CCDVIDREV3B
223       000000                     EQU     $000000                           ; CCD Video Processor Rev. 3
224       000001           VIDGENI   EQU     $000001                           ; CCD Video Processor Gen I
225       000002           IRREV4    EQU     $000002                           ; IR Video Processor Rev. 4
226       000003           COADDER   EQU     $000003                           ; IR Coadder
227       000004           CCDVIDREV5 EQU    $000004                           ; Differential input CCD video Rev. 5
228       000000           TIMREV4   EQU     $000000                           ; Timing Revision 4 = 50 MHz
229       000008           TIMGENI   EQU     $000008                           ; Timing Gen I = 40 MHz
230       000010           TIMREV5   EQU     $000010                           ; Timing Revision 5 = 250 MHz
231       000020           UTILREV3  EQU     $000020                           ; Utility Rev. 3 supported
232       000080           SHUTTER_CC EQU    $000080                           ; Shutter supported
233       000100           TEMP_POLY EQU     $000100                           ; Polynomial calibration
234                        TEMP_LINEAR
235       000200                     EQU     $000200                           ; Linear calibration
236       000400           SUBARRAY  EQU     $000400                           ; Subarray readout supported
237       000800           BINNING   EQU     $000800                           ; Binning supported
238                        SPLIT_SERIAL
239       001000                     EQU     $001000                           ; Split serial supported
240                        SPLIT_PARALLEL
241       002000                     EQU     $002000                           ; Split parallel supported
242       004000           MPP_CC    EQU     $004000                           ; Inverted clocks supported
243       018000           CLKDRVGENI EQU    $018000                           ; No clock driver board - Gen I
244       020000           MLO       EQU     $020000                           ; Set if Mount Laguna Observatory
245       040000           NGST      EQU     $040000                           ; NGST Aladdin implementation
246       100000           CONT_RD   EQU     $100000                           ; Continuous readout implemented
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timhdr.asm  Page 5



247    
248                                  INCLUDE "timboot.asm"
249                               COMMENT *
250    
251                        This file is used to generate boot DSP code for the 250 MHz fiber optic
252                                timing board using a DSP56303 as its main processor.
253                        Added utility board support Dec. 2002
254                                *
255                                  PAGE    132                               ; Printronix page width - 132 columns
256    
257                        ; Special address for two words for the DSP to bootstrap code from the EEPROM
258                                  IF      @SCP("HOST","ROM")
265                                  ENDIF
266    
267                                  IF      @SCP("HOST","HOST")
268       P:000000 P:000000                   ORG     P:0,P:0
269       P:000000 P:000000 0C0190            JMP     <INIT
270       P:000001 P:000001 000000            NOP
271                                           ENDIF
272    
273                                 ;  This ISR receives serial words a byte at a time over the asynchronous
274                                 ;    serial link (SCI) and squashes them into a single 24-bit word
275       P:000002 P:000002 602400  SCI_RCV   MOVE              R0,X:<SAVE_R0           ; Save R0
276       P:000003 P:000003 052139            MOVEC             SR,X:<SAVE_SR           ; Save Status Register
277       P:000004 P:000004 60A700            MOVE              X:<SCI_R0,R0            ; Restore R0 = pointer to SCI receive regist
er
278       P:000005 P:000005 542300            MOVE              A1,X:<SAVE_A1           ; Save A1
279       P:000006 P:000006 452200            MOVE              X1,X:<SAVE_X1           ; Save X1
280       P:000007 P:000007 54A600            MOVE              X:<SCI_A1,A1            ; Get SRX value of accumulator contents
281       P:000008 P:000008 45E000            MOVE              X:(R0),X1               ; Get the SCI byte
282       P:000009 P:000009 0AD041            BCLR    #1,R0                             ; Test for the address being $FFF6 = last by
te
283       P:00000A P:00000A 000000            NOP
284       P:00000B P:00000B 000000            NOP
285       P:00000C P:00000C 000000            NOP
286       P:00000D P:00000D 205862            OR      X1,A      (R0)+                   ; Add the byte into the 24-bit word
287       P:00000E P:00000E 0E0013            JCC     <MID_BYT                          ; Not the last byte => only restore register
s
288       P:00000F P:00000F 545C00  END_BYT   MOVE              A1,X:(R4)+              ; Put the 24-bit word into the SCI buffer
289       P:000010 P:000010 60F400            MOVE              #SRXL,R0                ; Re-establish first address of SCI interfac
e
                            FFFF98
290       P:000012 P:000012 2C0000            MOVE              #0,A1                   ; For zeroing out SCI_A1
291       P:000013 P:000013 602700  MID_BYT   MOVE              R0,X:<SCI_R0            ; Save the SCI receiver address
292       P:000014 P:000014 542600            MOVE              A1,X:<SCI_A1            ; Save A1 for next interrupt
293       P:000015 P:000015 05A139            MOVEC             X:<SAVE_SR,SR           ; Restore Status Register
294       P:000016 P:000016 54A300            MOVE              X:<SAVE_A1,A1           ; Restore A1
295       P:000017 P:000017 45A200            MOVE              X:<SAVE_X1,X1           ; Restore X1
296       P:000018 P:000018 60A400            MOVE              X:<SAVE_R0,R0           ; Restore R0
297       P:000019 P:000019 000004            RTI                                       ; Return from interrupt service
298    
299                                 ; Clear error condition and interrupt on SCI receiver
300       P:00001A P:00001A 077013  CLR_ERR   MOVEP             X:SSR,X:RCV_ERR         ; Read SCI status register
                            000025
301       P:00001C P:00001C 077018            MOVEP             X:SRXL,X:RCV_ERR        ; This clears any error
                            000025
302       P:00001E P:00001E 000004            RTI
303    
304       P:00001F P:00001F                   DC      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
305       P:000030 P:000030                   DC      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
306       P:000040 P:000040                   DC      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
307    
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 6



308                                 ;Tune the table so the following instruction is at P:$50 exactly.
309       P:000050 P:000050 0D0002            JSR     SCI_RCV                           ; SCI receive data interrupt
310       P:000051 P:000051 000000            NOP
311       P:000052 P:000052 0D001A            JSR     CLR_ERR                           ; SCI receive error interrupt
312       P:000053 P:000053 000000            NOP
313    
314                                 ; *******************  Command Processing  ******************
315    
316                                 ; Read the header and check it for self-consistency
317       P:000054 P:000054 609F00  START     MOVE              X:<IDL_ADR,R0
318       P:000055 P:000055 018FA0            JSET    #TIM_BIT,X:TCSR0,EXPOSING         ; If exposing go check the timer
                            000367
319       P:000057 P:000057 0A00A4            JSET    #ST_RDC,X:<STATUS,CONTINUE_READING
                            100000
320       P:000059 P:000059 0AE080            JMP     (R0)
321    
322       P:00005A P:00005A 330700  TST_RCV   MOVE              #<COM_BUF,R3
323       P:00005B P:00005B 0D00A5            JSR     <GET_RCV
324       P:00005C P:00005C 0E005B            JCC     *-1
325    
326                                 ; Check the header and read all the remaining words in the command
327       P:00005D P:00005D 0C00FF  PRC_RCV   JMP     <CHK_HDR                          ; Update HEADER and NWORDS
328       P:00005E P:00005E 578600  PR_RCV    MOVE              X:<NWORDS,B             ; Read this many words total in the command
329       P:00005F P:00005F 000000            NOP
330       P:000060 P:000060 01418C            SUB     #1,B                              ; We've already read the header
331       P:000061 P:000061 000000            NOP
332       P:000062 P:000062 06CF00            DO      B,RD_COM
                            00006A
333       P:000064 P:000064 205B00            MOVE              (R3)+                   ; Increment past what's been read already
334       P:000065 P:000065 0B0080  GET_WRD   JSCLR   #ST_RCV,X:STATUS,CHK_FO
                            0000A9
335       P:000067 P:000067 0B00A0            JSSET   #ST_RCV,X:STATUS,CHK_SCI
                            0000D5
336       P:000069 P:000069 0E0065            JCC     <GET_WRD
337       P:00006A P:00006A 000000            NOP
338       P:00006B P:00006B 330700  RD_COM    MOVE              #<COM_BUF,R3            ; Restore R3 = beginning of the command
339    
340                                 ; Is this command for the timing board?
341       P:00006C P:00006C 448500            MOVE              X:<HEADER,X0
342       P:00006D P:00006D 579B00            MOVE              X:<DMASK,B
343       P:00006E P:00006E 459A4E            AND     X0,B      X:<TIM_DRB,X1           ; Extract destination byte
344       P:00006F P:00006F 20006D            CMP     X1,B                              ; Does header = timing board number?
345       P:000070 P:000070 0EA080            JEQ     <COMMAND                          ; Yes, process it here
346       P:000071 P:000071 0E909D            JLT     <FO_XMT                           ; Send it to fiber optic transmitter
347    
348                                 ; Transmit the command to the utility board over the SCI port
349       P:000072 P:000072 060600            DO      X:<NWORDS,DON_XMT                 ; Transmit NWORDS
                            00007E
350       P:000074 P:000074 60F400            MOVE              #STXL,R0                ; SCI first byte address
                            FFFF95
351       P:000076 P:000076 44DB00            MOVE              X:(R3)+,X0              ; Get the 24-bit word to transmit
352       P:000077 P:000077 060380            DO      #3,SCI_SPT
                            00007D
353       P:000079 P:000079 019381            JCLR    #TDRE,X:SSR,*                     ; Continue ONLY if SCI XMT is empty
                            000079
354       P:00007B P:00007B 445800            MOVE              X0,X:(R0)+              ; Write to SCI, byte pointer + 1
355       P:00007C P:00007C 000000            NOP                                       ; Delay for the status flag to be set
356       P:00007D P:00007D 000000            NOP
357                                 SCI_SPT
358       P:00007E P:00007E 000000            NOP
359                                 DON_XMT
360       P:00007F P:00007F 0C0054            JMP     <START
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 7



361    
362                                 ; Process the receiver entry - is it in the command table ?
363       P:000080 P:000080 0203DF  COMMAND   MOVE              X:(R3+1),B              ; Get the command
364       P:000081 P:000081 205B00            MOVE              (R3)+
365       P:000082 P:000082 205B00            MOVE              (R3)+                   ; Point R3 to the first argument
366       P:000083 P:000083 302800            MOVE              #<COM_TBL,R0            ; Get the command table starting address
367       P:000084 P:000084 062A80            DO      #NUM_COM,END_COM                  ; Loop over the command table
                            00008B
368       P:000086 P:000086 47D800            MOVE              X:(R0)+,Y1              ; Get the command table entry
369       P:000087 P:000087 62E07D            CMP     Y1,B      X:(R0),R2               ; Does receiver = table entries address?
370       P:000088 P:000088 0E208B            JNE     <NOT_COM                          ; No, keep looping
371       P:000089 P:000089 00008C            ENDDO                                     ; Restore the DO loop system registers
372       P:00008A P:00008A 0AE280            JMP     (R2)                              ; Jump execution to the command
373       P:00008B P:00008B 205800  NOT_COM   MOVE              (R0)+                   ; Increment the register past the table addr
ess
374                                 END_COM
375       P:00008C P:00008C 0C008D            JMP     <ERROR                            ; The command is not in the table
376    
377                                 ; It's not in the command table - send an error message
378       P:00008D P:00008D 479D00  ERROR     MOVE              X:<ERR,Y1               ; Send the message - there was an error
379       P:00008E P:00008E 0C0090            JMP     <FINISH1                          ; This protects against unknown commands
380    
381                                 ; Send a reply packet - header and reply
382       P:00008F P:00008F 479800  FINISH    MOVE              X:<DONE,Y1              ; Send 'DON' as the reply
383    
384       P:000090 P:000090 578500  FINISH1   MOVE              X:<HEADER,B             ; Get header of incoming command
385       P:000091 P:000091 469C00            MOVE              X:<SMASK,Y0             ; This was the source byte, and is to
386       P:000092 P:000092 330700            MOVE              #<COM_BUF,R3            ;     become the destination byte
387       P:000093 P:000093 46935E            AND     Y0,B      X:<TWO,Y0
388       P:000094 P:000094 0C1ED1            LSR     #8,B                              ; Shift right eight bytes, add it to the
389       P:000095 P:000095 460600            MOVE              Y0,X:<NWORDS            ;     header, and put 2 as the number
390       P:000096 P:000096 469958            ADD     Y0,B      X:<SBRD,Y0              ;     of words in the string
391       P:000097 P:000097 200058            ADD     Y0,B                              ; Add source board's header, set Y1 for abov
e
392       P:000098 P:000098 000000            NOP
393       P:000099 P:000099 575B00            MOVE              B,X:(R3)+               ; Put the new header on the transmitter stac
k
394       P:00009A P:00009A 475B00            MOVE              Y1,X:(R3)+              ; Put the argument on the transmitter stack
395       P:00009B P:00009B 570500            MOVE              B,X:<HEADER
396       P:00009C P:00009C 0C006B            JMP     <RD_COM                           ; Decide where to send the reply, and do it
397    
398                                 ; Transmit words to the host computer over the fiber optics link
399       P:00009D P:00009D 63F400  FO_XMT    MOVE              #COM_BUF,R3
                            000007
400       P:00009F P:00009F 060600            DO      X:<NWORDS,DON_FFO                 ; Transmit all the words in the command
                            0000A3
401       P:0000A1 P:0000A1 57DB00            MOVE              X:(R3)+,B
402       P:0000A2 P:0000A2 0D00EB            JSR     <XMT_WRD
403       P:0000A3 P:0000A3 000000            NOP
404       P:0000A4 P:0000A4 0C0054  DON_FFO   JMP     <START
405    
406                                 ; Check for commands from the fiber optic FIFO and the utility board (SCI)
407       P:0000A5 P:0000A5 0D00A9  GET_RCV   JSR     <CHK_FO                           ; Check for fiber optic command from FIFO
408       P:0000A6 P:0000A6 0E80A8            JCS     <RCV_RTS                          ; If there's a command, check the header
409       P:0000A7 P:0000A7 0D00D5            JSR     <CHK_SCI                          ; Check for an SCI command
410       P:0000A8 P:0000A8 00000C  RCV_RTS   RTS
411    
412                                 ; Because of FIFO metastability require that EF be stable for two tests
413       P:0000A9 P:0000A9 0A8989  CHK_FO    JCLR    #EF,X:HDR,TST2                    ; EF = Low,  Low  => CLR SR, return
                            0000AC
414       P:0000AB P:0000AB 0C00AF            JMP     <TST3                             ;      High, Low  => try again
415       P:0000AC P:0000AC 0A8989  TST2      JCLR    #EF,X:HDR,CLR_CC                  ;      Low,  High => try again
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 8



                            0000D1
416       P:0000AE P:0000AE 0C00A9            JMP     <CHK_FO                           ;      High, High => read FIFO
417       P:0000AF P:0000AF 0A8989  TST3      JCLR    #EF,X:HDR,CHK_FO
                            0000A9
418    
419       P:0000B1 P:0000B1 08F4BB            MOVEP             #$028FE2,X:BCR          ; Slow down RDFO access
                            028FE2
420       P:0000B3 P:0000B3 000000            NOP
421       P:0000B4 P:0000B4 000000            NOP
422       P:0000B5 P:0000B5 5FF000            MOVE                          Y:RDFO,B
                            FFFFF1
423       P:0000B7 P:0000B7 2B0000            MOVE              #0,B2
424       P:0000B8 P:0000B8 0140CE            AND     #$FF,B
                            0000FF
425       P:0000BA P:0000BA 0140CD            CMP     #>$AC,B                           ; It must be $AC to be a valid word
                            0000AC
426       P:0000BC P:0000BC 0E20D1            JNE     <CLR_CC
427       P:0000BD P:0000BD 4EF000            MOVE                          Y:RDFO,Y0   ; Read the MS byte
                            FFFFF1
428       P:0000BF P:0000BF 0C1951            INSERT  #$008010,Y0,B
                            008010
429       P:0000C1 P:0000C1 4EF000            MOVE                          Y:RDFO,Y0   ; Read the middle byte
                            FFFFF1
430       P:0000C3 P:0000C3 0C1951            INSERT  #$008008,Y0,B
                            008008
431       P:0000C5 P:0000C5 4EF000            MOVE                          Y:RDFO,Y0   ; Read the LS byte
                            FFFFF1
432       P:0000C7 P:0000C7 0C1951            INSERT  #$008000,Y0,B
                            008000
433       P:0000C9 P:0000C9 000000            NOP
434       P:0000CA P:0000CA 516300            MOVE              B0,X:(R3)               ; Put the word into COM_BUF
435       P:0000CB P:0000CB 0A0000            BCLR    #ST_RCV,X:<STATUS                 ; Its a command from the host computer
436       P:0000CC P:0000CC 000000  SET_CC    NOP
437       P:0000CD P:0000CD 0AF960            BSET    #0,SR                             ; Valid word => SR carry bit = 1
438       P:0000CE P:0000CE 08F4BB            MOVEP             #$028FE1,X:BCR          ; Restore RDFO access
                            028FE1
439       P:0000D0 P:0000D0 00000C            RTS
440       P:0000D1 P:0000D1 0AF940  CLR_CC    BCLR    #0,SR                             ; Not valid word => SR carry bit = 0
441       P:0000D2 P:0000D2 08F4BB            MOVEP             #$028FE1,X:BCR          ; Restore RDFO access
                            028FE1
442       P:0000D4 P:0000D4 00000C            RTS
443    
444                                 ; Test the SCI (= synchronous communications interface) for new words
445       P:0000D5 P:0000D5 44F000  CHK_SCI   MOVE              X:(SCI_TABLE+33),X0
                            000421
446       P:0000D7 P:0000D7 228E00            MOVE              R4,A
447       P:0000D8 P:0000D8 209000            MOVE              X0,R0
448       P:0000D9 P:0000D9 200045            CMP     X0,A
449       P:0000DA P:0000DA 0EA0D1            JEQ     <CLR_CC                           ; There is no new SCI word
450       P:0000DB P:0000DB 44D800            MOVE              X:(R0)+,X0
451       P:0000DC P:0000DC 446300            MOVE              X0,X:(R3)
452       P:0000DD P:0000DD 220E00            MOVE              R0,A
453       P:0000DE P:0000DE 0140C5            CMP     #(SCI_TABLE+32),A                 ; Wrap it around the circular
                            000420
454       P:0000E0 P:0000E0 0EA0E4            JEQ     <INIT_PROCESSED_SCI               ;   buffer boundary
455       P:0000E1 P:0000E1 547000            MOVE              A1,X:(SCI_TABLE+33)
                            000421
456       P:0000E3 P:0000E3 0C00E9            JMP     <SCI_END
457                                 INIT_PROCESSED_SCI
458       P:0000E4 P:0000E4 56F400            MOVE              #SCI_TABLE,A
                            000400
459       P:0000E6 P:0000E6 000000            NOP
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 9



460       P:0000E7 P:0000E7 567000            MOVE              A,X:(SCI_TABLE+33)
                            000421
461       P:0000E9 P:0000E9 0A0020  SCI_END   BSET    #ST_RCV,X:<STATUS                 ; Its a utility board (SCI) word
462       P:0000EA P:0000EA 0C00CC            JMP     <SET_CC
463    
464                                 ; Transmit the word in B1 to the host computer over the fiber optic data link
465                                 XMT_WRD
466       P:0000EB P:0000EB 08F4BB            MOVEP             #$028FE2,X:BCR          ; Slow down RDFO access
                            028FE2
467       P:0000ED P:0000ED 60F400            MOVE              #FO_HDR+1,R0
                            000002
468       P:0000EF P:0000EF 060380            DO      #3,XMT_WRD1
                            0000F3
469       P:0000F1 P:0000F1 0C1D91            ASL     #8,B,B
470       P:0000F2 P:0000F2 000000            NOP
471       P:0000F3 P:0000F3 535800            MOVE              B2,X:(R0)+
472                                 XMT_WRD1
473       P:0000F4 P:0000F4 60F400            MOVE              #FO_HDR,R0
                            000001
474       P:0000F6 P:0000F6 61F400            MOVE              #WRFO,R1
                            FFFFF2
475       P:0000F8 P:0000F8 060480            DO      #4,XMT_WRD2
                            0000FB
476       P:0000FA P:0000FA 46D800            MOVE              X:(R0)+,Y0              ; Should be MOVEP  X:(R0)+,Y:WRFO
477       P:0000FB P:0000FB 4E6100            MOVE                          Y0,Y:(R1)
478                                 XMT_WRD2
479       P:0000FC P:0000FC 08F4BB            MOVEP             #$028FE1,X:BCR          ; Restore RDFO access
                            028FE1
480       P:0000FE P:0000FE 00000C            RTS
481    
482                                 ; Check the command or reply header in X:(R3) for self-consistency
483       P:0000FF P:0000FF 46E300  CHK_HDR   MOVE              X:(R3),Y0
484       P:000100 P:000100 579600            MOVE              X:<MASK1,B              ; Test for S.LE.3 and D.LE.3 and N.LE.7
485       P:000101 P:000101 20005E            AND     Y0,B
486       P:000102 P:000102 0E208D            JNE     <ERROR                            ; Test failed
487       P:000103 P:000103 579700            MOVE              X:<MASK2,B              ; Test for either S.NE.0 or D.NE.0
488       P:000104 P:000104 20005E            AND     Y0,B
489       P:000105 P:000105 0EA08D            JEQ     <ERROR                            ; Test failed
490       P:000106 P:000106 579500            MOVE              X:<SEVEN,B
491       P:000107 P:000107 20005E            AND     Y0,B                              ; Extract NWORDS, must be > 0
492       P:000108 P:000108 0EA08D            JEQ     <ERROR
493       P:000109 P:000109 44E300            MOVE              X:(R3),X0
494       P:00010A P:00010A 440500            MOVE              X0,X:<HEADER            ; Its a correct header
495       P:00010B P:00010B 550600            MOVE              B1,X:<NWORDS            ; Number of words in the command
496       P:00010C P:00010C 0C005E            JMP     <PR_RCV
497    
498                                 ;  *****************  Boot Commands  *******************
499    
500                                 ; Test Data Link - simply return value received after 'TDL'
501       P:00010D P:00010D 47DB00  TDL       MOVE              X:(R3)+,Y1              ; Get the data value
502       P:00010E P:00010E 0C0090            JMP     <FINISH1                          ; Return from executing TDL command
503    
504                                 ; Read DSP or EEPROM memory ('RDM' address): read memory, reply with value
505       P:00010F P:00010F 47DB00  RDMEM     MOVE              X:(R3)+,Y1
506       P:000110 P:000110 20EF00            MOVE              Y1,B
507       P:000111 P:000111 0140CE            AND     #$0FFFFF,B                        ; Bits 23-20 need to be zeroed
                            0FFFFF
508       P:000113 P:000113 21B000            MOVE              B1,R0                   ; Need the address in an address register
509       P:000114 P:000114 20EF00            MOVE              Y1,B
510       P:000115 P:000115 000000            NOP
511       P:000116 P:000116 0ACF14            JCLR    #20,B,RDX                         ; Test address bit for Program memory
                            00011A
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 10



512       P:000118 P:000118 07E087            MOVE              P:(R0),Y1               ; Read from Program Memory
513       P:000119 P:000119 0C0090            JMP     <FINISH1                          ; Send out a header with the value
514       P:00011A P:00011A 0ACF15  RDX       JCLR    #21,B,RDY                         ; Test address bit for X: memory
                            00011E
515       P:00011C P:00011C 47E000            MOVE              X:(R0),Y1               ; Write to X data memory
516       P:00011D P:00011D 0C0090            JMP     <FINISH1                          ; Send out a header with the value
517       P:00011E P:00011E 0ACF16  RDY       JCLR    #22,B,RDR                         ; Test address bit for Y: memory
                            000122
518       P:000120 P:000120 4FE000            MOVE                          Y:(R0),Y1   ; Read from Y data memory
519       P:000121 P:000121 0C0090            JMP     <FINISH1                          ; Send out a header with the value
520       P:000122 P:000122 0ACF17  RDR       JCLR    #23,B,ERROR                       ; Test address bit for read from EEPROM memo
ry
                            00008D
521       P:000124 P:000124 479400            MOVE              X:<THREE,Y1             ; Convert to word address to a byte address
522       P:000125 P:000125 220600            MOVE              R0,Y0                   ; Get 16-bit address in a data register
523       P:000126 P:000126 2000B8            MPY     Y0,Y1,B                           ; Multiply
524       P:000127 P:000127 20002A            ASR     B                                 ; Eliminate zero fill of fractional multiply
525       P:000128 P:000128 213000            MOVE              B0,R0                   ; Need to address memory
526       P:000129 P:000129 0AD06F            BSET    #15,R0                            ; Set bit so its in EEPROM space
527       P:00012A P:00012A 0D0178            JSR     <RD_WORD                          ; Read word from EEPROM
528       P:00012B P:00012B 21A700            MOVE              B1,Y1                   ; FINISH1 transmits Y1 as its reply
529       P:00012C P:00012C 0C0090            JMP     <FINISH1
530    
531                                 ; Program WRMEM ('WRM' address datum): write to memory, reply 'DON'.
532       P:00012D P:00012D 47DB00  WRMEM     MOVE              X:(R3)+,Y1              ; Get the address to be written to
533       P:00012E P:00012E 20EF00            MOVE              Y1,B
534       P:00012F P:00012F 0140CE            AND     #$0FFFFF,B                        ; Bits 23-20 need to be zeroed
                            0FFFFF
535       P:000131 P:000131 21B000            MOVE              B1,R0                   ; Need the address in an address register
536       P:000132 P:000132 20EF00            MOVE              Y1,B
537       P:000133 P:000133 46DB00            MOVE              X:(R3)+,Y0              ; Get datum into Y0 so MOVE works easily
538       P:000134 P:000134 0ACF14            JCLR    #20,B,WRX                         ; Test address bit for Program memory
                            000138
539       P:000136 P:000136 076086            MOVE              Y0,P:(R0)               ; Write to Program memory
540       P:000137 P:000137 0C008F            JMP     <FINISH
541       P:000138 P:000138 0ACF15  WRX       JCLR    #21,B,WRY                         ; Test address bit for X: memory
                            00013C
542       P:00013A P:00013A 466000            MOVE              Y0,X:(R0)               ; Write to X: memory
543       P:00013B P:00013B 0C008F            JMP     <FINISH
544       P:00013C P:00013C 0ACF16  WRY       JCLR    #22,B,WRR                         ; Test address bit for Y: memory
                            000140
545       P:00013E P:00013E 4E6000            MOVE                          Y0,Y:(R0)   ; Write to Y: memory
546       P:00013F P:00013F 0C008F            JMP     <FINISH
547       P:000140 P:000140 0ACF17  WRR       JCLR    #23,B,ERROR                       ; Test address bit for write to EEPROM
                            00008D
548       P:000142 P:000142 013D02            BCLR    #WRENA,X:PDRC                     ; WR_ENA* = 0 to enable EEPROM writing
549       P:000143 P:000143 460E00            MOVE              Y0,X:<SV_A1             ; Save the datum to be written
550       P:000144 P:000144 479400            MOVE              X:<THREE,Y1             ; Convert word address to a byte address
551       P:000145 P:000145 220600            MOVE              R0,Y0                   ; Get 16-bit address in a data register
552       P:000146 P:000146 2000B8            MPY     Y1,Y0,B                           ; Multiply
553       P:000147 P:000147 20002A            ASR     B                                 ; Eliminate zero fill of fractional multiply
554       P:000148 P:000148 213000            MOVE              B0,R0                   ; Need to address memory
555       P:000149 P:000149 0AD06F            BSET    #15,R0                            ; Set bit so its in EEPROM space
556       P:00014A P:00014A 558E00            MOVE              X:<SV_A1,B1             ; Get the datum to be written
557       P:00014B P:00014B 060380            DO      #3,L1WRR                          ; Loop over three bytes of the word
                            000154
558       P:00014D P:00014D 07588D            MOVE              B1,P:(R0)+              ; Write each EEPROM byte
559       P:00014E P:00014E 0C1C91            ASR     #8,B,B
560       P:00014F P:00014F 469E00            MOVE              X:<C100K,Y0             ; Move right one byte, enter delay = 1 msec
561       P:000150 P:000150 06C600            DO      Y0,L2WRR                          ; Delay by 12 milliseconds for EEPROM write
                            000153
562       P:000152 P:000152 060CA0            REP     #12                               ; Assume 100 MHz DSP56303
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 11



563       P:000153 P:000153 000000            NOP
564                                 L2WRR
565       P:000154 P:000154 000000            NOP                                       ; DO loop nesting restriction
566                                 L1WRR
567       P:000155 P:000155 013D22            BSET    #WRENA,X:PDRC                     ; WR_ENA* = 1 to disable EEPROM writing
568    
569       P:000156 P:000156 0C008F            JMP     <FINISH
570    
571                                 ; Load application code from P: memory into its proper locations
572       P:000157 P:000157 47DB00  LDAPPL    MOVE              X:(R3)+,Y1              ; Application number, not used yet
573       P:000158 P:000158 0D015A            JSR     <LOAD_APPLICATION
574       P:000159 P:000159 0C008F            JMP     <FINISH
575    
576                                 LOAD_APPLICATION
577       P:00015A P:00015A 60F400            MOVE              #$8000,R0               ; Starting EEPROM address
                            008000
578       P:00015C P:00015C 0D0178            JSR     <RD_WORD                          ; Number of words in boot code
579       P:00015D P:00015D 21A600            MOVE              B1,Y0
580       P:00015E P:00015E 479400            MOVE              X:<THREE,Y1
581       P:00015F P:00015F 2000B8            MPY     Y0,Y1,B
582       P:000160 P:000160 20002A            ASR     B
583       P:000161 P:000161 213000            MOVE              B0,R0                   ; EEPROM address of start of P: application
584       P:000162 P:000162 0AD06F            BSET    #15,R0                            ; To access EEPROM
585       P:000163 P:000163 0D0178            JSR     <RD_WORD                          ; Read number of words in application P:
586       P:000164 P:000164 61F400            MOVE              #(X_BOOT_START+1),R1    ; End of boot P: code that needs keeping
                            00022B
587       P:000166 P:000166 06CD00            DO      B1,RD_APPL_P
                            000169
588       P:000168 P:000168 0D0178            JSR     <RD_WORD
589       P:000169 P:000169 07598D            MOVE              B1,P:(R1)+
590                                 RD_APPL_P
591       P:00016A P:00016A 0D0178            JSR     <RD_WORD                          ; Read number of words in application X:
592       P:00016B P:00016B 61F400            MOVE              #END_COMMAND_TABLE,R1
                            000036
593       P:00016D P:00016D 06CD00            DO      B1,RD_APPL_X
                            000170
594       P:00016F P:00016F 0D0178            JSR     <RD_WORD
595       P:000170 P:000170 555900            MOVE              B1,X:(R1)+
596                                 RD_APPL_X
597       P:000171 P:000171 0D0178            JSR     <RD_WORD                          ; Read number of words in application Y:
598       P:000172 P:000172 310100            MOVE              #1,R1                   ; There is no Y: memory in the boot code
599       P:000173 P:000173 06CD00            DO      B1,RD_APPL_Y
                            000176
600       P:000175 P:000175 0D0178            JSR     <RD_WORD
601       P:000176 P:000176 5D5900            MOVE                          B1,Y:(R1)+
602                                 RD_APPL_Y
603       P:000177 P:000177 00000C            RTS
604    
605                                 ; Read one word from EEPROM location R0 into accumulator B1
606       P:000178 P:000178 060380  RD_WORD   DO      #3,L_RDBYTE
                            00017B
607       P:00017A P:00017A 07D88B            MOVE              P:(R0)+,B2
608       P:00017B P:00017B 0C1C91            ASR     #8,B,B
609                                 L_RDBYTE
610       P:00017C P:00017C 00000C            RTS
611    
612                                 ; Come to here on a 'STP' command so 'DON' can be sent
613                                 STOP_IDLE_CLOCKING
614       P:00017D P:00017D 305A00            MOVE              #<TST_RCV,R0            ; Execution address when idle => when not
615       P:00017E P:00017E 601F00            MOVE              R0,X:<IDL_ADR           ;   processing commands or reading out
616       P:00017F P:00017F 0A0002            BCLR    #IDLMODE,X:<STATUS                ; Don't idle after readout
617       P:000180 P:000180 0C008F            JMP     <FINISH
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 12



618    
619                                 ; Routines executed after the DSP boots and initializes
620       P:000181 P:000181 305A00  STARTUP   MOVE              #<TST_RCV,R0            ; Execution address when idle => when not
621       P:000182 P:000182 601F00            MOVE              R0,X:<IDL_ADR           ;   processing commands or reading out
622       P:000183 P:000183 44F400            MOVE              #50000,X0               ; Delay by 500 milliseconds
                            00C350
623       P:000185 P:000185 06C400            DO      X0,L_DELAY
                            000188
624       P:000187 P:000187 06E8A3            REP     #1000
625       P:000188 P:000188 000000            NOP
626                                 L_DELAY
627       P:000189 P:000189 57F400            MOVE              #$020002,B              ; Normal reply after booting is 'SYR'
                            020002
628       P:00018B P:00018B 0D00EB            JSR     <XMT_WRD
629       P:00018C P:00018C 57F400            MOVE              #'SYR',B
                            535952
630       P:00018E P:00018E 0D00EB            JSR     <XMT_WRD
631    
632       P:00018F P:00018F 0C0054            JMP     <START                            ; Start normal command processing
633    
634                                 ; *******************  DSP  INITIALIZATION  CODE  **********************
635                                 ; This code initializes the DSP right after booting, and is overwritten
636                                 ;   by application code
637       P:000190 P:000190 08F4BD  INIT      MOVEP             #PLL_INIT,X:PCTL        ; Initialize PLL to 100 MHz
                            050003
638       P:000192 P:000192 000000            NOP
639    
640                                 ; Set operation mode register OMR to normal expanded
641       P:000193 P:000193 0500BA            MOVEC             #$0000,OMR              ; Operating Mode Register = Normal Expanded
642       P:000194 P:000194 0500BB            MOVEC             #0,SP                   ; Reset the Stack Pointer SP
643    
644                                 ; Program the AA = address attribute pins
645       P:000195 P:000195 08F4B9            MOVEP             #$FFFC21,X:AAR0         ; Y = $FFF000 to $FFFFFF asserts commands
                            FFFC21
646       P:000197 P:000197 08F4B8            MOVEP             #$008909,X:AAR1         ; P = $008000 to $00FFFF accesses the EEPROM
                            008909
647       P:000199 P:000199 08F4B7            MOVEP             #$010C11,X:AAR2         ; X = $010000 to $010FFF reads A/D values
                            010C11
648       P:00019B P:00019B 08F4B6            MOVEP             #$080621,X:AAR3         ; Y = $080000 to $0BFFFF R/W from SRAM
                            080621
649    
650                                 ; Program the DRAM memory access and addressing
651       P:00019D P:00019D 08F4BB            MOVEP             #$028FE1,X:BCR          ; Bus Control Register
                            028FE1
652    
653                                 ; Program the Host port B for parallel I/O
654       P:00019F P:00019F 08F484            MOVEP             #>1,X:HPCR              ; All pins enabled as GPIO
                            000001
655       P:0001A1 P:0001A1 08F489            MOVEP             #$810C,X:HDR
                            00810C
656       P:0001A3 P:0001A3 08F488            MOVEP             #$B10E,X:HDDR           ; Data Direction Register
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
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 13



667                                 ;       PB7 = STATUS3           PB15 = SELSCI
668    
669                                 ; Program the serial port ESSI0 = Port C for serial communication with
670                                 ;   the utility board
671       P:0001A5 P:0001A5 07F43F            MOVEP             #>0,X:PCRC              ; Software reset of ESSI0
                            000000
672       P:0001A7 P:0001A7 07F435            MOVEP             #$180809,X:CRA0         ; Divide 100 MHz by 20 to get 5.0 MHz
                            180809
673                                                                                     ; DC[4:0] = 0 for non-network operation
674                                                                                     ; WL0-WL2 = 3 for 24-bit data words
675                                                                                     ; SSC1 = 0 for SC1 not used
676       P:0001A9 P:0001A9 07F436            MOVEP             #$020020,X:CRB0         ; SCKD = 1 for internally generated clock
                            020020
677                                                                                     ; SCD2 = 0 so frame sync SC2 is an output
678                                                                                     ; SHFD = 0 for MSB shifted first
679                                                                                     ; FSL = 0, frame sync length not used
680                                                                                     ; CKP = 0 for rising clock edge transitions
681                                                                                     ; SYN = 0 for asynchronous
682                                                                                     ; TE0 = 1 to enable transmitter #0
683                                                                                     ; MOD = 0 for normal, non-networked mode
684                                                                                     ; TE0 = 0 to NOT enable transmitter #0 yet
685                                                                                     ; RE = 1 to enable receiver
686       P:0001AB P:0001AB 07F43F            MOVEP             #%111001,X:PCRC         ; Control Register (0 for GPIO, 1 for ESSI)
                            000039
687       P:0001AD P:0001AD 07F43E            MOVEP             #%000110,X:PRRC         ; Data Direction Register (0 for In, 1 for O
ut)
                            000006
688       P:0001AF P:0001AF 07F43D            MOVEP             #%000100,X:PDRC         ; Data Register - WR_ENA* = 1
                            000004
689    
690                                 ; Port C version = Analog boards
691                                 ;       MOVEP   #$000809,X:CRA0 ; Divide 100 MHz by 20 to get 5.0 MHz
692                                 ;       MOVEP   #$000030,X:CRB0 ; SCKD = 1 for internally generated clock
693                                 ;       MOVEP   #%100000,X:PCRC ; Control Register (0 for GPIO, 1 for ESSI)
694                                 ;       MOVEP   #%000100,X:PRRC ; Data Direction Register (0 for In, 1 for Out)
695                                 ;       MOVEP   #%000000,X:PDRC ; Data Register: 'not used' = 0 outputs
696    
697       P:0001B1 P:0001B1 07F43C            MOVEP             #0,X:TX00               ; Initialize the transmitter to zero
                            000000
698       P:0001B3 P:0001B3 000000            NOP
699       P:0001B4 P:0001B4 000000            NOP
700       P:0001B5 P:0001B5 013630            BSET    #TE,X:CRB0                        ; Enable the SSI transmitter
701    
702                                 ; Conversion from software bits to schematic labels for Port C
703                                 ;       PC0 = SC00 = UTL-T-SCK
704                                 ;       PC1 = SC01 = 2_XMT = SYNC on prototype
705                                 ;       PC2 = SC02 = WR_ENA*
706                                 ;       PC3 = SCK0 = TIM-U-SCK
707                                 ;       PC4 = SRD0 = UTL-T-STD
708                                 ;       PC5 = STD0 = TIM-U-STD
709    
710                                 ; Program the serial port ESSI1 = Port D for serial transmission to
711                                 ;   the analog boards and two parallel I/O input pins
712       P:0001B6 P:0001B6 07F42F            MOVEP             #>0,X:PCRD              ; Software reset of ESSI0
                            000000
713       P:0001B8 P:0001B8 07F425            MOVEP             #$000809,X:CRA1         ; Divide 100 MHz by 20 to get 5.0 MHz
                            000809
714                                                                                     ; DC[4:0] = 0
715                                                                                     ; WL[2:0] = ALC = 0 for 8-bit data words
716                                                                                     ; SSC1 = 0 for SC1 not used
717       P:0001BA P:0001BA 07F426            MOVEP             #$000030,X:CRB1         ; SCKD = 1 for internally generated clock
                            000030
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 14



718                                                                                     ; SCD2 = 1 so frame sync SC2 is an output
719                                                                                     ; SHFD = 0 for MSB shifted first
720                                                                                     ; CKP = 0 for rising clock edge transitions
721                                                                                     ; TE0 = 0 to NOT enable transmitter #0 yet
722                                                                                     ; MOD = 0 so its not networked mode
723       P:0001BC P:0001BC 07F42F            MOVEP             #%100000,X:PCRD         ; Control Register (0 for GPIO, 1 for ESSI)
                            000020
724                                                                                     ; PD3 = SCK1, PD5 = STD1 for ESSI
725       P:0001BE P:0001BE 07F42E            MOVEP             #%000100,X:PRRD         ; Data Direction Register (0 for In, 1 for O
ut)
                            000004
726       P:0001C0 P:0001C0 07F42D            MOVEP             #%000100,X:PDRD         ; Data Register: 'not used' = 0 outputs
                            000004
727       P:0001C2 P:0001C2 07F42C            MOVEP             #0,X:TX10               ; Initialize the transmitter to zero
                            000000
728       P:0001C4 P:0001C4 000000            NOP
729       P:0001C5 P:0001C5 000000            NOP
730       P:0001C6 P:0001C6 012630            BSET    #TE,X:CRB1                        ; Enable the SSI transmitter
731    
732                                 ; Conversion from software bits to schematic labels for Port D
733                                 ; PD0 = SC10 = 2_XMT_? input
734                                 ; PD1 = SC11 = SSFEF* input
735                                 ; PD2 = SC12 = PWR_EN
736                                 ; PD3 = SCK1 = TIM-A-SCK
737                                 ; PD4 = SRD1 = PWRRST
738                                 ; PD5 = STD1 = TIM-A-STD
739    
740                                 ; Program the SCI port to communicate with the utility board
741       P:0001C7 P:0001C7 07F41C            MOVEP             #$0B04,X:SCR            ; SCI programming: 11-bit asynchronous
                            000B04
742                                                                                     ;   protocol (1 start, 8 data, 1 even parity
,
743                                                                                     ;   1 stop); LSB before MSB; enable receiver
744                                                                                     ;   and its interrupts; transmitter interrup
ts
745                                                                                     ;   disabled.
746       P:0001C9 P:0001C9 07F41B            MOVEP             #$0003,X:SCCR           ; SCI clock: utility board data rate =
                            000003
747                                                                                     ;   (390,625 kbits/sec); internal clock.
748       P:0001CB P:0001CB 07F41F            MOVEP             #%011,X:PCRE            ; Port Control Register = RXD, TXD enabled
                            000003
749       P:0001CD P:0001CD 07F41E            MOVEP             #%000,X:PRRE            ; Port Direction Register (0 = Input)
                            000000
750    
751                                 ;       PE0 = RXD
752                                 ;       PE1 = TXD
753                                 ;       PE2 = SCLK
754    
755                                 ; Program one of the three timers as an exposure timer
756       P:0001CF P:0001CF 07F403            MOVEP             #$C34F,X:TPLR           ; Prescaler to generate millisecond timer,
                            00C34F
757                                                                                     ;  counting from the system clock / 2 = 50 M
Hz
758       P:0001D1 P:0001D1 07F40F            MOVEP             #$208200,X:TCSR0        ; Clear timer complete bit and enable presca
ler
                            208200
759       P:0001D3 P:0001D3 07F40E            MOVEP             #0,X:TLR0               ; Timer load register
                            000000
760    
761                                 ; Enable interrupts for the SCI port only
762       P:0001D5 P:0001D5 08F4BF            MOVEP             #$000000,X:IPRC         ; No interrupts allowed
                            000000
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 15



763       P:0001D7 P:0001D7 08F4BE            MOVEP             #>$80,X:IPRP            ; Enable SCI interrupt only, IPR = 1
                            000080
764       P:0001D9 P:0001D9 00FCB8            ANDI    #$FC,MR                           ; Unmask all interrupt levels
765    
766                                 ; Initialize the fiber optic serial receiver circuitry
767       P:0001DA P:0001DA 061480            DO      #20,L_FO_INIT
                            0001DF
768       P:0001DC P:0001DC 5FF000            MOVE                          Y:RDFO,B
                            FFFFF1
769       P:0001DE P:0001DE 0605A0            REP     #5
770       P:0001DF P:0001DF 000000            NOP
771                                 L_FO_INIT
772    
773                                 ; Pulse PRSFIFO* low to revive the CMDRST* instruction and reset the FIFO
774       P:0001E0 P:0001E0 44F400            MOVE              #1000000,X0             ; Delay by 10 milliseconds
                            0F4240
775       P:0001E2 P:0001E2 06C400            DO      X0,*+3
                            0001E4
776       P:0001E4 P:0001E4 000000            NOP
777       P:0001E5 P:0001E5 0A8908            BCLR    #8,X:HDR
778       P:0001E6 P:0001E6 0614A0            REP     #20
779       P:0001E7 P:0001E7 000000            NOP
780       P:0001E8 P:0001E8 0A8928            BSET    #8,X:HDR
781    
782                                 ; Reset the utility board
783       P:0001E9 P:0001E9 0A0F05            BCLR    #5,X:<LATCH
784       P:0001EA P:0001EA 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Clear reset utility board bit
                            00000F
785       P:0001EC P:0001EC 06C8A0            REP     #200                              ; Delay by RESET* low time
786       P:0001ED P:0001ED 000000            NOP
787       P:0001EE P:0001EE 0A0F25            BSET    #5,X:<LATCH
788       P:0001EF P:0001EF 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Clear reset utility board bit
                            00000F
789       P:0001F1 P:0001F1 56F400            MOVE              #200000,A               ; Delay 2 msec for utility boot
                            030D40
790       P:0001F3 P:0001F3 06CE00            DO      A,*+3
                            0001F5
791       P:0001F5 P:0001F5 000000            NOP
792    
793                                 ; Put all the analog switch inputs to low so they draw minimum current
794       P:0001F6 P:0001F6 012F23            BSET    #3,X:PCRD                         ; Turn the serial clock on
795       P:0001F7 P:0001F7 56F400            MOVE              #$0C3000,A              ; Value of integrate speed and gain switches
                            0C3000
796       P:0001F9 P:0001F9 20001B            CLR     B
797       P:0001FA P:0001FA 241000            MOVE              #$100000,X0             ; Increment over board numbers for DAC write
s
798       P:0001FB P:0001FB 45F400            MOVE              #$001000,X1             ; Increment over board numbers for WRSS writ
es
                            001000
799       P:0001FD P:0001FD 060F80            DO      #15,L_ANALOG                      ; Fifteen video processor boards maximum
                            000205
800       P:0001FF P:0001FF 0D020C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
801       P:000200 P:000200 200040            ADD     X0,A
802       P:000201 P:000201 5F7000            MOVE                          B,Y:WRSS    ; This is for the fast analog switches
                            FFFFF3
803       P:000203 P:000203 0620A3            REP     #800                              ; Delay for the serial data transmission
804       P:000204 P:000204 000000            NOP
805       P:000205 P:000205 200068            ADD     X1,B                              ; Increment the video and clock driver numbe
rs
806                                 L_ANALOG
807       P:000206 P:000206 0A0F00            BCLR    #CDAC,X:<LATCH                    ; Enable clearing of DACs
808       P:000207 P:000207 0A0F02            BCLR    #ENCK,X:<LATCH                    ; Disable clock and DAC output switches
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 16



809       P:000208 P:000208 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Execute these two operations
                            00000F
810       P:00020A P:00020A 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
811       P:00020B P:00020B 0C0223            JMP     <SKIP
812    
813                                 ; Transmit contents of accumulator A1 over the synchronous serial transmitter
814                                 XMIT_A_WORD
815       P:00020C P:00020C 07F42C            MOVEP             #0,X:TX10               ; This helps, don't know why
                            000000
816       P:00020E P:00020E 547000            MOVE              A1,X:SV_A1
                            00000E
817       P:000210 P:000210 000000            NOP
818       P:000211 P:000211 01A786            JCLR    #TDE,X:SSISR1,*                   ; Start bit
                            000211
819       P:000213 P:000213 07F42C            MOVEP             #$010000,X:TX10
                            010000
820       P:000215 P:000215 060380            DO      #3,L_X
                            00021B
821       P:000217 P:000217 01A786            JCLR    #TDE,X:SSISR1,*                   ; Three data bytes
                            000217
822       P:000219 P:000219 04CCCC            MOVEP             A1,X:TX10
823       P:00021A P:00021A 0C1E90            LSL     #8,A
824       P:00021B P:00021B 000000            NOP
825                                 L_X
826       P:00021C P:00021C 01A786            JCLR    #TDE,X:SSISR1,*                   ; Zeroes to bring transmitter low
                            00021C
827       P:00021E P:00021E 07F42C            MOVEP             #0,X:TX10
                            000000
828       P:000220 P:000220 54F000            MOVE              X:SV_A1,A1
                            00000E
829       P:000222 P:000222 00000C            RTS
830    
831                                 SKIP
832    
833                                 ; Set up the circular SCI buffer, 32 words in size
834       P:000223 P:000223 64F400            MOVE              #SCI_TABLE,R4
                            000400
835       P:000225 P:000225 051FA4            MOVE              #31,M4
836       P:000226 P:000226 647000            MOVE              R4,X:(SCI_TABLE+33)
                            000421
837    
838                                           IF      @SCP("HOST","ROM")
846                                           ENDIF
847    
848       P:000228 P:000228 44F400            MOVE              #>$AC,X0
                            0000AC
849       P:00022A P:00022A 440100            MOVE              X0,X:<FO_HDR
850    
851       P:00022B P:00022B 0C0181            JMP     <STARTUP
852    
853                                 ;  ****************  X: Memory tables  ********************
854    
855                                 ; Define the address in P: space where the table of constants begins
856    
857                                  X_BOOT_START
858       00022A                              EQU     @LCV(L)-2
859    
860                                           IF      @SCP("HOST","ROM")
862                                           ENDIF
863                                           IF      @SCP("HOST","HOST")
864       X:000000 X:000000                   ORG     X:0,X:0
865                                           ENDIF
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 17



866    
867                                 ; Special storage area - initialization constants and scratch space
868       X:000000 X:000000         STATUS    DC      $1064                             ; Controller status bits
869    
870       000001                    FO_HDR    EQU     STATUS+1                          ; Fiber optic write bytes
871       000005                    HEADER    EQU     FO_HDR+4                          ; Command header
872       000006                    NWORDS    EQU     HEADER+1                          ; Number of words in the command
873       000007                    COM_BUF   EQU     NWORDS+1                          ; Command buffer
874       00000E                    SV_A1     EQU     COM_BUF+7                         ; Save accumulator A1
875    
876                                           IF      @SCP("HOST","ROM")
881                                           ENDIF
882    
883                                           IF      @SCP("HOST","HOST")
884       X:00000F X:00000F                   ORG     X:$F,X:$F
885                                           ENDIF
886    
887                                 ; Parameter table in P: space to be copied into X: space during
888                                 ;   initialization, and is copied from ROM by the DSP boot
889       X:00000F X:00000F         LATCH     DC      $7A                               ; Starting value in latch chip U25
890                                  EXPOSURE_TIME
891       X:000010 X:000010                   DC      0                                 ; Exposure time in milliseconds
892                                  ELAPSED_TIME
893       X:000011 X:000011                   DC      0                                 ; Time elapsed so far in the exposure
894       X:000012 X:000012         ONE       DC      1                                 ; One
895       X:000013 X:000013         TWO       DC      2                                 ; Two
896       X:000014 X:000014         THREE     DC      3                                 ; Three
897       X:000015 X:000015         SEVEN     DC      7                                 ; Seven
898       X:000016 X:000016         MASK1     DC      $FCFCF8                           ; Mask for checking header
899       X:000017 X:000017         MASK2     DC      $030300                           ; Mask for checking header
900       X:000018 X:000018         DONE      DC      'DON'                             ; Standard reply
901       X:000019 X:000019         SBRD      DC      $020000                           ; Source Identification number
902       X:00001A X:00001A         TIM_DRB   DC      $000200                           ; Destination = timing board number
903       X:00001B X:00001B         DMASK     DC      $00FF00                           ; Mask to get destination board number
904       X:00001C X:00001C         SMASK     DC      $FF0000                           ; Mask to get source board number
905       X:00001D X:00001D         ERR       DC      'ERR'                             ; An error occurred
906       X:00001E X:00001E         C100K     DC      100000                            ; Delay for WRROM = 1 millisec
907       X:00001F X:00001F         IDL_ADR   DC      TST_RCV                           ; Address of idling routine
908       X:000020 X:000020         EXP_ADR   DC      0                                 ; Jump to this address during exposures
909    
910    
911                                 ; Places for saving register values
912       X:000021 X:000021         SAVE_SR   DC      0                                 ; Status Register
913       X:000022 X:000022         SAVE_X1   DC      0
914       X:000023 X:000023         SAVE_A1   DC      0
915       X:000024 X:000024         SAVE_R0   DC      0
916       X:000025 X:000025         RCV_ERR   DC      0
917       X:000026 X:000026         SCI_A1    DC      0                                 ; Contents of accumulator A1 in RCV ISR
918       X:000027 X:000027         SCI_R0    DC      SRXL
919    
920                                 ; Command table
921       000028                    COM_TBL_R EQU     @LCV(R)
922       X:000028 X:000028         COM_TBL   DC      'TDL',TDL                         ; Test Data Link
923       X:00002A X:00002A                   DC      'RDM',RDMEM                       ; Read from DSP or EEPROM memory
924       X:00002C X:00002C                   DC      'WRM',WRMEM                       ; Write to DSP memory
925       X:00002E X:00002E                   DC      'LDA',LDAPPL                      ; Load application from EEPROM to DSP
926       X:000030 X:000030                   DC      'STP',STOP_IDLE_CLOCKING
927       X:000032 X:000032                   DC      'DON',START                       ; Nothing special
928       X:000034 X:000034                   DC      'ERR',START                       ; Nothing special
929    
930                                  END_COMMAND_TABLE
931       000036                              EQU     @LCV(R)
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timboot.asm  Page 18



932    
933                                 ; The table at SCI_TABLE is for words received from the utility board, written by
934                                 ;   the interrupt service routine SCI_RCV. Note that it is 32 words long,
935                                 ;   hard coded, and the 33rd location contains the pointer to words that have
936                                 ;   been processed by moving them from the SCI_TABLE to the COM_BUF.
937    
938                                           IF      @SCP("HOST","ROM")
940                                           ENDIF
941    
942       000036                    END_ADR   EQU     @LCV(L)                           ; End address of P: code written to ROM
943    
944       P:00022C P:00022C                   ORG     P:,P:
945    
946       001DB4                    CC        EQU     CCDVIDREV5+TIMREV5+TEMP_POLY+UTILREV3+SPLIT_SERIAL+SUBARRAY+BINNING+SHUTTER_CC
947    
948                                 ; Put number of words of application in P: for loading application from EEPROM
949       P:00022C P:00022C                   DC      TIMBOOT_X_MEMORY-@LCV(L)-1
950    
951                                 ; Define CLOCK as a macro to produce in-line code to reduce execution time
952                                 CLOCK     MACRO
953  m                                        JCLR    #SSFHF,X:HDR,*                    ; Don't overfill the WRSS FIFO
954  m                                        REP     Y:(R0)+                           ; Repeat # of times at address Y:(R0)+
955  m                                        MOVEP   Y:(R0)+,Y:WRSS                    ; Write the waveform to the FIFO
956  m                                        ENDM
957    
958                                 ; Set software to IDLE mode
959                                 START_IDLE_CLOCKING
960       P:00022D P:00022D 60F400            MOVE              #IDLE,R0
                            000233
961       P:00022F P:00022F 000000            NOP
962       P:000230 P:000230 601F00            MOVE              R0,X:<IDL_ADR
963       P:000231 P:000231 0A0022            BSET    #IDLMODE,X:<STATUS                ; Idle after readout
964       P:000232 P:000232 0C008F            JMP     <FINISH                           ; Need to send header and 'DON'
965    
966                                 ; Keep the CCD idling when not reading out
967                                 IDLE
968       P:000233 P:000233 5EAB00            MOVE                          Y:<NSRI,A   ; NSERIALS_READ = NSR
969       P:000234 P:000234 0A0085            JCLR    #SPLIT_S,X:STATUS,*+3
                            000237
970       P:000236 P:000236 200022            ASR     A                                 ; Split serials requires / 2
971       P:000237 P:000237 000000            NOP
972    
973       P:000238 P:000238 06CE00            DO      A,IDL1                            ; Loop over number of pixels per line
                            000245
974       P:00023A P:00023A 68F000            MOVE                          Y:SERIAL_IDLE,R0 ; Serial transfer on pixel
                            00001C
975                                           CLOCK                                     ; Go to it
979       P:000240 P:000240 330700            MOVE              #COM_BUF,R3
980       P:000241 P:000241 0D00A5            JSR     <GET_RCV                          ; Check for FO or SSI commands
981       P:000242 P:000242 0E0245            JCC     <NO_COM                           ; Continue IDLE if no commands received
982       P:000243 P:000243 00008C            ENDDO
983       P:000244 P:000244 0C005D            JMP     <PRC_RCV                          ; Go process header and command
984       P:000245 P:000245 000000  NO_COM    NOP
985                                 IDL1
986       P:000246 P:000246 062540            DO      Y:<N_PARALLEL_CLEARS,PAR
                            00024C
987       P:000248 P:000248 689900            MOVE                          Y:<PARALLEL_CLEAR,R0 ; Address of parallel clocking wave
form
988                                           CLOCK                                     ; Go clock out the CCD charge
992                                 PAR
993       P:00024D P:00024D 0C0233            JMP     <IDLE
994    
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 19



995    
996                                 ;  *****************  Exposure and readout routines  *****************
997    
998                                 ; Overall loop - transfer and read NPR lines
999                                 RDCCD
1000   
1001                                ; Calculate some readout parameters
1002      P:00024E P:00024E 5E8E00            MOVE                          Y:<NP_SKIP,A ; NP_SKIP = 0 => full image readout
1003      P:00024F P:00024F 000000            NOP
1004      P:000250 P:000250 200003            TST     A
1005      P:000251 P:000251 0E2259            JNE     <SUB_IMG
1006      P:000252 P:000252 5E8100            MOVE                          Y:<NSR,A    ; NSERIALS_READ = NSR
1007      P:000253 P:000253 0A0085            JCLR    #SPLIT_S,X:STATUS,*+3
                            000256
1008      P:000255 P:000255 200022            ASR     A                                 ; Split serials requires / 2
1009      P:000256 P:000256 000000            NOP
1010      P:000257 P:000257 5E0A00            MOVE                          A,Y:<NSERIALS_READ ; Number of columns in each subimage
1011      P:000258 P:000258 0C025C            JMP     <WT_CLK
1012   
1013      P:000259 P:000259 5E9300  SUB_IMG   MOVE                          Y:<NS_RD,A
1014      P:00025A P:00025A 000000            NOP
1015      P:00025B P:00025B 5E0A00            MOVE                          A,Y:<NSERIALS_READ
1016   
1017                                ; Start the loop for parallel shifting desired number of lines
1018      P:00025C P:00025C 000000  WT_CLK    NOP                                       ; not using the binning waveform generation 
at the moment.
1019      P:00025D P:00025D 0D040B            JSR     <WAIT_TO_FINISH_CLOCKING
1020   
1021                                ; Skip over the required number of rows for subimage readout
1022      P:00025E P:00025E 5E8E00            MOVE                          Y:<NP_SKIP,A ; Number of rows to skip
1023      P:00025F P:00025F 200003            TST     A
1024      P:000260 P:000260 0EA26A            JEQ     <CLR_SR
1025      P:000261 P:000261 060E40            DO      Y:<NP_SKIP,L_PSKP
                            000272
1026      P:000263 P:000263 060640            DO      Y:<NPBIN,L_PSKIP
                            000271
1027      P:000265 P:000265 689900            MOVE                          Y:<PARALLEL_CLEAR,R0
1028                                          CLOCK
1032   
1033                                ; Clear out the accumulated charge from the serial shift register
1034      P:00026A P:00026A 060340  CLR_SR    DO      Y:<NSCLR,L_CLRSR                  ; Loop over number of pixels to clear
                            000270
1035      P:00026C P:00026C 689700            MOVE                          Y:<SERIAL_SKIP,R0
1036                                          CLOCK                                     ; Go clock out the CCD charge
1040      P:000271 P:000271 000000  L_CLRSR   NOP                                       ; Do loop restriction
1041   
1042      P:000272 P:000272 000000  L_PSKIP   NOP
1043                                L_PSKP
1044   
1045                                ; Parallel shift the image into the serial shift register
1046      P:000273 P:000273 5E8200            MOVE                          Y:<NPR,A
1047      P:000274 P:000274 000000            NOP
1048      P:000275 P:000275 0A0086            JCLR    #SPLIT_P,X:STATUS,P_CLK
                            00027A
1049      P:000277 P:000277 5E8200            MOVE                          Y:<NPR,A
1050      P:000278 P:000278 200022            ASR     A
1051      P:000279 P:000279 000000            NOP
1052   
1053                                ; This is the main loop over each line to be read out
1054   
1055      P:00027A P:00027A 06CC00  P_CLK     DO      A1,LPR                            ; Number of rows to read out
                            0002BD
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 20



1056   
1057                                ; Exercise the parallel clocks, including binning if needed
1058      P:00027C P:00027C 060640            DO      Y:<NPBIN,L_PBIN
                            000282
1059      P:00027E P:00027E 689800            MOVE                          Y:<PARALLEL,R0
1060                                          CLOCK
1064                                L_PBIN
1065   
1066                                ; Check for a command once per line. Only the ABORT command should be issued.
1067      P:000283 P:000283 330700            MOVE              #COM_BUF,R3
1068      P:000284 P:000284 0D00A5            JSR     <GET_RCV                          ; Was a command received?
1069      P:000285 P:000285 0E028B            JCC     <CONTINUE_READ                    ; If no, continue reading out
1070      P:000286 P:000286 0C005D            JMP     <PRC_RCV                          ; If yes, go process it
1071   
1072                                ; Abort the readout currently underway
1073      P:000287 P:000287 0A0084  ABR_RDC   JCLR    #ST_RDC,X:<STATUS,ABORT_EXPOSURE
                            0003B9
1074      P:000289 P:000289 00008C            ENDDO                                     ; Properly terminate readout loop
1075      P:00028A P:00028A 0C03B9            JMP     <ABORT_EXPOSURE
1076   
1077                                CONTINUE_READ
1078   
1079                                ; Read the prescan pixels
1080      P:00028B P:00028B 5E9100  F_BIAS    MOVE                          Y:<NS_PRE,A ; NS_PRE = 0 => no prescan pixels
1081      P:00028C P:00028C 200003            TST     A
1082      P:00028D P:00028D 0EF296            JLE     <END_PRE
1083      P:00028E P:00028E 061140            DO      Y:<NS_PRE,F_BRD                   ; Number of pixels to read out
                            000294
1084      P:000290 P:000290 689600            MOVE                          Y:<SERIAL_READ,R0
1085                                          CLOCK                                     ; Go clock out the CCD charge
1089      P:000295 P:000295 000000  F_BRD     NOP
1090      P:000296 P:000296 000000  END_PRE   NOP
1091   
1092                                ; Skip over NS_SKP1 columns for subimage readout
1093      P:000297 P:000297 5E8F00            MOVE                          Y:<NS_SKP1,A ; Number of columns to skip
1094      P:000298 P:000298 200003            TST     A
1095      P:000299 P:000299 0EF2A1            JLE     <L_READ
1096      P:00029A P:00029A 060F40            DO      Y:<NS_SKP1,L_SKP1                 ; Number of waveform entries total
                            0002A0
1097      P:00029C P:00029C 689700            MOVE                          Y:<SERIAL_SKIP,R0 ; Waveform table starting address
1098                                          CLOCK                                     ; Go clock out the CCD charge
1102                                L_SKP1
1103   
1104                                ; Finally read some real pixels
1105      P:0002A1 P:0002A1 060A40  L_READ    DO      Y:<NSERIALS_READ,L_RD
                            0002A7
1106      P:0002A3 P:0002A3 689600            MOVE                          Y:<SERIAL_READ,R0
1107                                          CLOCK                                     ; Go clock out the CCD charge
1111                                L_RD
1112   
1113                                ; Skip over NS_SKP2 columns if needed for subimage readout
1114      P:0002A8 P:0002A8 5E9000            MOVE                          Y:<NS_SKP2,A ; Number of columns to skip
1115      P:0002A9 P:0002A9 200003            TST     A
1116      P:0002AA P:0002AA 0EF2B2            JLE     <L_BIAS
1117      P:0002AB P:0002AB 061040            DO      Y:<NS_SKP2,L_SKP2
                            0002B1
1118      P:0002AD P:0002AD 689700            MOVE                          Y:<SERIAL_SKIP,R0 ; Waveform table starting address
1119                                          CLOCK                                     ; Go clock out the CCD charge
1123                                L_SKP2
1124   
1125                                ; And read the overscan pixels
1126      P:0002B2 P:0002B2 5E9200  L_BIAS    MOVE                          Y:<NS_OVR,A ; NS_OVR = 0 => no bias pixels
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 21



1127      P:0002B3 P:0002B3 200003            TST     A
1128      P:0002B4 P:0002B4 0EF2BD            JLE     <END_ROW
1129      P:0002B5 P:0002B5 061240            DO      Y:<NS_OVR,L_BRD                   ; Number of pixels to read out
                            0002BB
1130      P:0002B7 P:0002B7 689600            MOVE                          Y:<SERIAL_READ,R0
1131                                          CLOCK                                     ; Go clock out the CCD charge
1135      P:0002BC P:0002BC 000000  L_BRD     NOP
1136      P:0002BD P:0002BD 000000  END_ROW   NOP
1137      P:0002BE P:0002BE 000000  LPR       NOP                                       ; End of parallel loop
1138   
1139                                ; Restore the controller to non-image data transfer and idling if necessary
1140      P:0002BF P:0002BF 0A0082  RDC_END   JCLR    #IDLMODE,X:<STATUS,NO_IDL         ; Don't idle after readout
                            0002C5
1141      P:0002C1 P:0002C1 60F400            MOVE              #IDLE,R0
                            000233
1142      P:0002C3 P:0002C3 601F00            MOVE              R0,X:<IDL_ADR
1143      P:0002C4 P:0002C4 0C02C7            JMP     <RDC_E
1144      P:0002C5 P:0002C5 305A00  NO_IDL    MOVE              #TST_RCV,R0
1145      P:0002C6 P:0002C6 601F00            MOVE              R0,X:<IDL_ADR
1146      P:0002C7 P:0002C7 0D040B  RDC_E     JSR     <WAIT_TO_FINISH_CLOCKING
1147      P:0002C8 P:0002C8 0A0004            BCLR    #ST_RDC,X:<STATUS                 ; Set status to not reading out
1148      P:0002C9 P:0002C9 0C0054            JMP     <START
1149   
1150                                ; ******  Include many routines not directly needed for readout  *******
1151                                          INCLUDE "timCCDmisc.asm"
1152                                ; Miscellaneous CCD control routines, common to all detector types
1153   
1154                                ; 2011-10-21 (GR): Added SBINN to for binning (only parallel binning implemented so far)
1155                                ; 2011-10-23 (GR): Replaced SERIAL_SKIP waveforms for _U1 with SERIAL_SKIP_SPLIT for faster flus
h of serial reg after each parallel skip
1156   
1157   
1158                                POWER_OFF
1159      P:0002CA P:0002CA 0D0312            JSR     <CLEAR_SWITCHES                   ; Clear all analog switches
1160      P:0002CB P:0002CB 0A8922            BSET    #LVEN,X:HDR
1161      P:0002CC P:0002CC 0A8923            BSET    #HVEN,X:HDR
1162      P:0002CD P:0002CD 0A000F            BCLR    #POWERST,X:<STATUS                ; Set the power state in the X: status word
1163      P:0002CE P:0002CE 0C008F            JMP     <FINISH
1164   
1165                                ; Execute the power-on cycle, as a command
1166                                POWER_ON
1167      P:0002CF P:0002CF 0D0312            JSR     <CLEAR_SWITCHES                   ; Clear all analog switches
1168      P:0002D0 P:0002D0 0D02E4            JSR     <PON                              ; Turn on the power control board
1169      P:0002D1 P:0002D1 0A8980            JCLR    #PWROK,X:HDR,PWR_ERR              ; Test if the power turned on properly
                            0002E1
1170      P:0002D3 P:0002D3 0D02F7            JSR     <SET_BIASES                       ; Turn on the DC bias supplies
1171      P:0002D4 P:0002D4 0D052B            JSR     <SEL_OS                           ; Set up readout parameters
1172      P:0002D5 P:0002D5 60F400            MOVE              #IDLE,R0                ; Put controller in IDLE state
                            000233
1173                                ;       MOVE    #TST_RCV,R0             ; Put controller in non-IDLE state
1174      P:0002D7 P:0002D7 601F00            MOVE              R0,X:<IDL_ADR
1175      P:0002D8 P:0002D8 0A002F            BSET    #POWERST,X:<STATUS                ; Set the power state bit in the X: status w
ord
1176   
1177                                                                                    ; get the gain setting and put it into the a
ppropriate place in Y memory
1178      P:0002D9 P:0002D9 5EF000            MOVE                          Y:GAIN_SETTING,A
                            000211
1179      P:0002DB P:0002DB 240D00            MOVE              #$0D0000,X0
1180      P:0002DC P:0002DC 200044            SUB     X0,A
1181      P:0002DD P:0002DD 000000            NOP
1182      P:0002DE P:0002DE 5E0000            MOVE                          A,Y:<GAIN
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 22



1183      P:0002DF P:0002DF 0A002F            BSET    #POWERST,X:<STATUS                ; Set the power state bit in the X: status w
ord
1184      P:0002E0 P:0002E0 0C008F            JMP     <FINISH
1185   
1186                                ; The power failed to turn on because of an error on the power control board
1187      P:0002E1 P:0002E1 0A8922  PWR_ERR   BSET    #LVEN,X:HDR                       ; Turn off the low voltage enable line
1188      P:0002E2 P:0002E2 0A8923            BSET    #HVEN,X:HDR                       ; Turn off the high voltage enable line
1189      P:0002E3 P:0002E3 0C008D            JMP     <ERROR
1190   
1191                                ; As a subroutine, turn on the low voltages (+/- 6.5V, +/- 16.5V) and delay
1192      P:0002E4 P:0002E4 0A8902  PON       BCLR    #LVEN,X:HDR                       ; Set these signals to DSP outputs
1193      P:0002E5 P:0002E5 44F400            MOVE              #2000000,X0
                            1E8480
1194      P:0002E7 P:0002E7 06C400            DO      X0,*+3                            ; Wait 20 millisec for settling
                            0002E9
1195      P:0002E9 P:0002E9 000000            NOP
1196   
1197                                ; Turn on the high +36 volt power line and then delay
1198      P:0002EA P:0002EA 0A8903            BCLR    #HVEN,X:HDR                       ; HVEN = Low => Turn on +36V
1199      P:0002EB P:0002EB 44F400            MOVE              #10000000,X0
                            989680
1200      P:0002ED P:0002ED 06C400            DO      X0,*+3                            ; Wait 100 millisec for settling
                            0002EF
1201      P:0002EF P:0002EF 000000            NOP
1202      P:0002F0 P:0002F0 00000C            RTS
1203   
1204   
1205                                RAW_COMMAND
1206      P:0002F1 P:0002F1 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock
1207      P:0002F2 P:0002F2 56DB00            MOVE              X:(R3)+,A               ; Get the command which should just be a wor
d
1208      P:0002F3 P:0002F3 0D020C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1209      P:0002F4 P:0002F4 0D040E            JSR     <PAL_DLY                          ; Wait for the number to be sent
1210      P:0002F5 P:0002F5 012F03            BCLR    #3,X:PCRD                         ; Turn off the serial clock
1211      P:0002F6 P:0002F6 0C008F            JMP     <FINISH
1212   
1213                                ; Set all the DC bias voltages and video processor offset values, reading
1214                                ;   them from the 'DACS' table
1215                                SET_BIASES
1216      P:0002F7 P:0002F7 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock
1217      P:0002F8 P:0002F8 0A0F01            BCLR    #1,X:<LATCH                       ; Separate updates of clock driver
1218      P:0002F9 P:0002F9 0A0F20            BSET    #CDAC,X:<LATCH                    ; Disable clearing of DACs
1219      P:0002FA P:0002FA 0A0F22            BSET    #ENCK,X:<LATCH                    ; Enable clock and DAC output switches
1220      P:0002FB P:0002FB 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Write it to the hardware
                            00000F
1221      P:0002FD P:0002FD 0D040E            JSR     <PAL_DLY                          ; Delay for all this to happen
1222   
1223                                ; Read DAC values from a table, and write them to the DACs
1224      P:0002FE P:0002FE 60F400            MOVE              #DACS,R0                ; Get starting address of DAC values
                            0001CF
1225      P:000300 P:000300 000000            NOP
1226      P:000301 P:000301 000000            NOP
1227      P:000302 P:000302 065840            DO      Y:(R0)+,L_DAC                     ; Repeat Y:(R0)+ times
                            000306
1228      P:000304 P:000304 5ED800            MOVE                          Y:(R0)+,A   ; Read the table entry
1229      P:000305 P:000305 0D020C            JSR     <XMIT_A_WORD                      ; Transmit it to TIM-A-STD
1230      P:000306 P:000306 000000            NOP
1231                                L_DAC
1232   
1233                                ; Let the DAC voltages all ramp up before exiting
1234      P:000307 P:000307 44F400            MOVE              #400000,X0
                            061A80
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 23



1235      P:000309 P:000309 06C400            DO      X0,*+3                            ; 4 millisec delay
                            00030B
1236      P:00030B P:00030B 000000            NOP
1237      P:00030C P:00030C 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1238      P:00030D P:00030D 00000C            RTS
1239   
1240                                SET_BIAS_VOLTAGES
1241      P:00030E P:00030E 0D02F7            JSR     <SET_BIASES
1242      P:00030F P:00030F 0C008F            JMP     <FINISH
1243   
1244      P:000310 P:000310 0D0312  CLR_SWS   JSR     <CLEAR_SWITCHES
1245      P:000311 P:000311 0C008F            JMP     <FINISH
1246   
1247                                ; Clear all video processor analog switches to lower their power dissipation
1248                                CLEAR_SWITCHES
1249      P:000312 P:000312 012F23            BSET    #3,X:PCRD                         ; Turn the serial clock on
1250      P:000313 P:000313 56F400            MOVE              #$0C3000,A              ; Value of integrate speed and gain switches
                            0C3000
1251      P:000315 P:000315 20001B            CLR     B
1252      P:000316 P:000316 241000            MOVE              #$100000,X0             ; Increment over board numbers for DAC write
s
1253      P:000317 P:000317 45F400            MOVE              #$001000,X1             ; Increment over board numbers for WRSS writ
es
                            001000
1254      P:000319 P:000319 060F80            DO      #15,L_VIDEO                       ; Fifteen video processor boards maximum
                            000320
1255      P:00031B P:00031B 0D020C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1256      P:00031C P:00031C 200040            ADD     X0,A
1257      P:00031D P:00031D 5F7000            MOVE                          B,Y:WRSS
                            FFFFF3
1258      P:00031F P:00031F 0D040E            JSR     <PAL_DLY                          ; Delay for the serial data transmission
1259      P:000320 P:000320 200068            ADD     X1,B
1260                                L_VIDEO
1261      P:000321 P:000321 0A0F00            BCLR    #CDAC,X:<LATCH                    ; Enable clearing of DACs
1262      P:000322 P:000322 0A0F02            BCLR    #ENCK,X:<LATCH                    ; Disable clock and DAC output switches
1263      P:000323 P:000323 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Execute these two operations
                            00000F
1264      P:000325 P:000325 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1265      P:000326 P:000326 00000C            RTS
1266   
1267                                SET_SHUTTER_STATE
1268      P:000327 P:000327 568F00            MOVE              X:LATCH,A
1269      P:000328 P:000328 0140C6            AND     #$FFEF,A
                            00FFEF
1270      P:00032A P:00032A 200042            OR      X0,A
1271      P:00032B P:00032B 000000            NOP
1272      P:00032C P:00032C 540F00            MOVE              A1,X:LATCH
1273      P:00032D P:00032D 09CC35            MOVEP             A1,Y:WRLATCH
1274      P:00032E P:00032E 00000C            RTS
1275   
1276                                ; Open the shutter from the timing board, executed as a command
1277                                OPEN_SHUTTER
1278      P:00032F P:00032F 0A0023            BSET    #ST_SHUT,X:<STATUS                ; Set status bit to mean shutter open
1279      P:000330 P:000330 240000            MOVE              #0,X0
1280      P:000331 P:000331 0D0327            JSR     <SET_SHUTTER_STATE
1281      P:000332 P:000332 0C008F            JMP     <FINISH
1282   
1283                                ; Close the shutter from the timing board, executed as a command
1284                                CLOSE_SHUTTER
1285      P:000333 P:000333 0A0003            BCLR    #ST_SHUT,X:<STATUS                ; Clear status to mean shutter closed
1286      P:000334 P:000334 44F400            MOVE              #>$10,X0
                            000010
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 24



1287      P:000336 P:000336 0D0327            JSR     <SET_SHUTTER_STATE
1288      P:000337 P:000337 0C008F            JMP     <FINISH
1289   
1290                                ; Shutter subroutines
1291      P:000338 P:000338 0A0023  OSHUT     BSET    #ST_SHUT,X:<STATUS                ; Set status bit to mean shutter open
1292      P:000339 P:000339 240000            MOVE              #0,X0
1293      P:00033A P:00033A 0D0327            JSR     <SET_SHUTTER_STATE
1294      P:00033B P:00033B 00000C            RTS
1295   
1296      P:00033C P:00033C 0A0003  CSHUT     BCLR    #ST_SHUT,X:<STATUS                ; Clear status to mean shutter closed
1297      P:00033D P:00033D 44F400            MOVE              #>$10,X0
                            000010
1298      P:00033F P:00033F 0D0327            JSR     <SET_SHUTTER_STATE
1299      P:000340 P:000340 00000C            RTS
1300   
1301                                ; Clear the CCD, executed as a command
1302      P:000341 P:000341 0D0343  CLEAR     JSR     <CLR_CCD
1303      P:000342 P:000342 0C008F            JMP     <FINISH
1304   
1305                                ; Default clearing routine with serial clocks inactive
1306                                ; Fast clear image before each exposure, executed as a subroutine
1307      P:000343 P:000343 060440  CLR_CCD   DO      Y:<NPCLR,LPCLR2                   ; Loop over number of lines in image
                            000353
1308      P:000345 P:000345 689900            MOVE                          Y:<PARALLEL_CLEAR,R0 ; Address of parallel transfer wave
form
1309                                          CLOCK
1313      P:00034A P:00034A 0A8989            JCLR    #EF,X:HDR,LPCLR1                  ; Simple test for fast execution
                            000353
1314      P:00034C P:00034C 330700            MOVE              #COM_BUF,R3
1315      P:00034D P:00034D 0D00A5            JSR     <GET_RCV                          ; Check for FO command
1316      P:00034E P:00034E 0E0353            JCC     <LPCLR1                           ; Continue no commands received
1317   
1318      P:00034F P:00034F 60F400            MOVE              #LPCLR1,R0
                            000353
1319      P:000351 P:000351 601F00            MOVE              R0,X:<IDL_ADR
1320      P:000352 P:000352 0C005D            JMP     <PRC_RCV
1321      P:000353 P:000353 000000  LPCLR1    NOP
1322                                LPCLR2
1323      P:000354 P:000354 330700            MOVE              #COM_BUF,R3
1324      P:000355 P:000355 0D00A5            JSR     <GET_RCV                          ; Check for FO command
1325      P:000356 P:000356 00000C            RTS
1326   
1327                                ; Start the exposure timer and monitor its progress
1328      P:000357 P:000357 07F40E  EXPOSE    MOVEP             #0,X:TLR0               ; Load 0 into counter timer
                            000000
1329      P:000359 P:000359 240000            MOVE              #0,X0
1330      P:00035A P:00035A 441100            MOVE              X0,X:<ELAPSED_TIME      ; Set elapsed exposure time to zero
1331      P:00035B P:00035B 579000            MOVE              X:<EXPOSURE_TIME,B
1332      P:00035C P:00035C 20000B            TST     B                                 ; Special test for zero exposure time
1333      P:00035D P:00035D 0EA369            JEQ     <END_EXP                          ; Don't even start an exposure
1334      P:00035E P:00035E 01418C            SUB     #1,B                              ; Timer counts from X:TCPR0+1 to zero
1335      P:00035F P:00035F 010F20            BSET    #TIM_BIT,X:TCSR0                  ; Enable the timer #0
1336      P:000360 P:000360 577000            MOVE              B,X:TCPR0
                            FFFF8D
1337      P:000362 P:000362 0A8989  CHK_RCV   JCLR    #EF,X:HDR,CHK_TIM                 ; Simple test for fast execution
                            000367
1338      P:000364 P:000364 330700            MOVE              #COM_BUF,R3             ; The beginning of the command buffer
1339      P:000365 P:000365 0D00A5            JSR     <GET_RCV                          ; Check for an incoming command
1340      P:000366 P:000366 0E805D            JCS     <PRC_RCV                          ; If command is received, go check it
1341      P:000367 P:000367 018F95  CHK_TIM   JCLR    #TCF,X:TCSR0,CHK_RCV              ; Wait for timer to equal compare value
                            000362
1342      P:000369 P:000369 010F00  END_EXP   BCLR    #TIM_BIT,X:TCSR0                  ; Disable the timer
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 25



1343      P:00036A P:00036A 0AE780            JMP     (R7)                              ; This contains the return address
1344   
1345                                ; Start the exposure, operate the shutter, and initiate the CCD readout
1346                                START_EXPOSURE
1347      P:00036B P:00036B 57F400            MOVE              #$020102,B
                            020102
1348      P:00036D P:00036D 0D00EB            JSR     <XMT_WRD
1349      P:00036E P:00036E 57F400            MOVE              #'IIA',B                ; Initialize the PCI image address
                            494941
1350      P:000370 P:000370 0D00EB            JSR     <XMT_WRD
1351      P:000371 P:000371 0B0088            JSCLR   #NOT_CLR,X:STATUS,CLR_CCD         ; Jump to clear out routine if bit set
                            000343
1352      P:000373 P:000373 330700            MOVE              #COM_BUF,R3             ; The beginning of the command buffer
1353      P:000374 P:000374 0D00A5            JSR     <GET_RCV                          ; Check for FO command
1354      P:000375 P:000375 0E805D            JCS     <PRC_RCV                          ; Process the command
1355      P:000376 P:000376 305A00            MOVE              #TST_RCV,R0             ; Process commands during the exposure
1356      P:000377 P:000377 601F00            MOVE              R0,X:<IDL_ADR
1357      P:000378 P:000378 0D040B            JSR     <WAIT_TO_FINISH_CLOCKING
1358   
1359                                ; Operate the shutter if needed and begin exposure
1360      P:000379 P:000379 0A008B            JCLR    #SHUT,X:STATUS,L_SEX0
                            00037C
1361      P:00037B P:00037B 0D0338            JSR     <OSHUT                            ; Open the shutter if needed
1362      P:00037C P:00037C 67F400  L_SEX0    MOVE              #L_SEX1,R7              ; Return address at end of exposure
                            00037F
1363   
1364      P:00037E P:00037E 0C0357            JMP     <EXPOSE                           ; Delay for specified exposure time
1365                                L_SEX1
1366   
1367                                ; Now we really start the CCD readout, alerting the PCI board, closing the
1368                                ;  shutter, waiting for it to close and then reading out
1369      P:00037F P:00037F 0D03FE  STR_RDC   JSR     <PCI_READ_IMAGE                   ; Get the PCI board reading the image
1370      P:000380 P:000380 0A0024            BSET    #ST_RDC,X:<STATUS                 ; Set status to reading out
1371      P:000381 P:000381 0A008B            JCLR    #SHUT,X:STATUS,TST_SYN
                            000384
1372      P:000383 P:000383 0D033C            JSR     <CSHUT                            ; Close the shutter if necessary
1373      P:000384 P:000384 0A00AA  TST_SYN   JSET    #TST_IMG,X:STATUS,SYNTHETIC_IMAGE
                            0003C9
1374   
1375                                ; Delay readout until the shutter has fully closed
1376      P:000386 P:000386 5E8800            MOVE                          Y:<SHDEL,A
1377      P:000387 P:000387 200003            TST     A
1378      P:000388 P:000388 0EF391            JLE     <S_DEL0
1379      P:000389 P:000389 44F400            MOVE              #100000,X0
                            0186A0
1380      P:00038B P:00038B 06CE00            DO      A,S_DEL0                          ; Delay by Y:SHDEL milliseconds
                            000390
1381      P:00038D P:00038D 06C400            DO      X0,S_DEL1
                            00038F
1382      P:00038F P:00038F 000000            NOP
1383      P:000390 P:000390 000000  S_DEL1    NOP
1384      P:000391 P:000391 000000  S_DEL0    NOP
1385   
1386      P:000392 P:000392 0C024E            JMP     <RDCCD                            ; Finally, go read out the CCD
1387   
1388                                TEST_AD
1389      P:000393 P:000393 57F000            MOVE              X:(RDAD+1),B
                            010001
1390      P:000395 P:000395 0C1DA1            ASL     #16,B,B
1391      P:000396 P:000396 000000            NOP
1392      P:000397 P:000397 216500            MOVE              B2,X1
1393      P:000398 P:000398 0C1D91            ASL     #8,B,B
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 26



1394      P:000399 P:000399 000000            NOP
1395      P:00039A P:00039A 216400            MOVE              B2,X0
1396      P:00039B P:00039B 000000            NOP
1397      P:00039C P:00039C 20A700            MOVE              X1,Y1
1398      P:00039D P:00039D 208600            MOVE              X0,Y0
1399      P:00039E P:00039E 4F1F00            MOVE                          Y1,Y:<BYTE_1
1400      P:00039F P:00039F 4E2000            MOVE                          Y0,Y:<BYTE_2
1401   
1402      P:0003A0 P:0003A0 0C008F            JMP     <FINISH
1403   
1404                                ; Set the desired exposure time
1405                                SET_EXPOSURE_TIME
1406      P:0003A1 P:0003A1 46DB00            MOVE              X:(R3)+,Y0
1407      P:0003A2 P:0003A2 461000            MOVE              Y0,X:EXPOSURE_TIME
1408      P:0003A3 P:0003A3 04C68D            MOVEP             Y0,X:TCPR0
1409      P:0003A4 P:0003A4 0C008F            JMP     <FINISH
1410   
1411                                ; Read the time remaining until the exposure ends
1412                                READ_EXPOSURE_TIME
1413      P:0003A5 P:0003A5 018FA0            JSET    #TIM_BIT,X:TCSR0,RD_TIM           ; Read DSP timer if its running
                            0003A9
1414      P:0003A7 P:0003A7 479100            MOVE              X:<ELAPSED_TIME,Y1
1415      P:0003A8 P:0003A8 0C0090            JMP     <FINISH1
1416      P:0003A9 P:0003A9 47F000  RD_TIM    MOVE              X:TCR0,Y1               ; Read elapsed exposure time
                            FFFF8C
1417      P:0003AB P:0003AB 0C0090            JMP     <FINISH1
1418   
1419                                ; Pause the exposure - close the shutter and stop the timer
1420                                PAUSE_EXPOSURE
1421      P:0003AC P:0003AC 07700C            MOVEP             X:TCR0,X:ELAPSED_TIME   ; Save the elapsed exposure time
                            000011
1422      P:0003AE P:0003AE 010F00            BCLR    #TIM_BIT,X:TCSR0                  ; Disable the DSP exposure timer
1423      P:0003AF P:0003AF 0D033C            JSR     <CSHUT                            ; Close the shutter
1424      P:0003B0 P:0003B0 0C008F            JMP     <FINISH
1425   
1426                                ; Resume the exposure - open the shutter if needed and restart the timer
1427                                RESUME_EXPOSURE
1428      P:0003B1 P:0003B1 010F29            BSET    #TRM,X:TCSR0                      ; To be sure it will load TLR0
1429      P:0003B2 P:0003B2 07700C            MOVEP             X:TCR0,X:TLR0           ; Restore elapsed exposure time
                            FFFF8E
1430      P:0003B4 P:0003B4 010F20            BSET    #TIM_BIT,X:TCSR0                  ; Re-enable the DSP exposure timer
1431      P:0003B5 P:0003B5 0A008B            JCLR    #SHUT,X:STATUS,L_RES
                            0003B8
1432      P:0003B7 P:0003B7 0D0338            JSR     <OSHUT                            ; Open the shutter if necessary
1433      P:0003B8 P:0003B8 0C008F  L_RES     JMP     <FINISH
1434   
1435                                ; Abort exposure - close the shutter, stop the timer and resume idle mode
1436                                ABORT_EXPOSURE
1437      P:0003B9 P:0003B9 0D033C            JSR     <CSHUT                            ; Close the shutter
1438      P:0003BA P:0003BA 010F00            BCLR    #TIM_BIT,X:TCSR0                  ; Disable the DSP exposure timer
1439      P:0003BB P:0003BB 0A0082            JCLR    #IDLMODE,X:<STATUS,NO_IDL2        ; Don't idle after readout
                            0003C1
1440      P:0003BD P:0003BD 60F400            MOVE              #IDLE,R0
                            000233
1441      P:0003BF P:0003BF 601F00            MOVE              R0,X:<IDL_ADR
1442      P:0003C0 P:0003C0 0C03C3            JMP     <RDC_E2
1443      P:0003C1 P:0003C1 305A00  NO_IDL2   MOVE              #TST_RCV,R0
1444      P:0003C2 P:0003C2 601F00            MOVE              R0,X:<IDL_ADR
1445      P:0003C3 P:0003C3 0D040B  RDC_E2    JSR     <WAIT_TO_FINISH_CLOCKING
1446      P:0003C4 P:0003C4 0A0004            BCLR    #ST_RDC,X:<STATUS                 ; Set status to not reading out
1447      P:0003C5 P:0003C5 06A08F            DO      #4000,*+3                         ; Wait 40 microsec for the fiber
                            0003C7
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 27



1448      P:0003C7 P:0003C7 000000            NOP                                       ;  optic to clear out
1449      P:0003C8 P:0003C8 0C008F            JMP     <FINISH
1450                                ; Generate a synthetic image by simply incrementing the pixel counts
1451                                SYNTHETIC_IMAGE
1452                                ;       JSR     <PCI_READ_IMAGE         ; Get the PCI board reading the image
1453      P:0003C9 P:0003C9 0A0024            BSET    #ST_RDC,X:<STATUS                 ; Set status to reading out
1454      P:0003CA P:0003CA 0D040E            JSR     <PAL_DLY
1455      P:0003CB P:0003CB 200013            CLR     A
1456      P:0003CC P:0003CC 060240            DO      Y:<NPR,LPR_TST                    ; Loop over each line readout
                            0003D7
1457      P:0003CE P:0003CE 060140            DO      Y:<NSR,LSR_TST                    ; Loop over number of pixels per line
                            0003D6
1458      P:0003D0 P:0003D0 0614A0            REP     #20                               ; #20 => 1.0 microsec per pixel
1459      P:0003D1 P:0003D1 000000            NOP
1460      P:0003D2 P:0003D2 014180            ADD     #1,A                              ; Pixel data = Pixel data + 1
1461      P:0003D3 P:0003D3 000000            NOP
1462      P:0003D4 P:0003D4 21CF00            MOVE              A,B
1463      P:0003D5 P:0003D5 0D03D9            JSR     <XMT_PIX                          ;  transmit them
1464      P:0003D6 P:0003D6 000000            NOP
1465                                LSR_TST
1466      P:0003D7 P:0003D7 000000            NOP
1467                                LPR_TST
1468      P:0003D8 P:0003D8 0C02BF            JMP     <RDC_END                          ; Normal exit
1469   
1470                                ; Transmit the 16-bit pixel datum in B1 to the host computer
1471      P:0003D9 P:0003D9 0C1DA1  XMT_PIX   ASL     #16,B,B
1472      P:0003DA P:0003DA 000000            NOP
1473      P:0003DB P:0003DB 216500            MOVE              B2,X1
1474      P:0003DC P:0003DC 0C1D91            ASL     #8,B,B
1475      P:0003DD P:0003DD 000000            NOP
1476      P:0003DE P:0003DE 216400            MOVE              B2,X0
1477      P:0003DF P:0003DF 000000            NOP
1478      P:0003E0 P:0003E0 09C532            MOVEP             X1,Y:WRFO
1479      P:0003E1 P:0003E1 09C432            MOVEP             X0,Y:WRFO
1480      P:0003E2 P:0003E2 00000C            RTS
1481   
1482                                ; Test the hardware to read A/D values directly into the DSP instead
1483                                ;   of using the SXMIT option, A/Ds #2 and 3.
1484      P:0003E3 P:0003E3 57F000  READ_AD   MOVE              X:(RDAD+2),B
                            010002
1485      P:0003E5 P:0003E5 0C1DA1            ASL     #16,B,B
1486      P:0003E6 P:0003E6 000000            NOP
1487      P:0003E7 P:0003E7 216500            MOVE              B2,X1
1488      P:0003E8 P:0003E8 0C1D91            ASL     #8,B,B
1489      P:0003E9 P:0003E9 000000            NOP
1490      P:0003EA P:0003EA 216400            MOVE              B2,X0
1491      P:0003EB P:0003EB 000000            NOP
1492      P:0003EC P:0003EC 09C532            MOVEP             X1,Y:WRFO
1493      P:0003ED P:0003ED 09C432            MOVEP             X0,Y:WRFO
1494      P:0003EE P:0003EE 060AA0            REP     #10
1495      P:0003EF P:0003EF 000000            NOP
1496      P:0003F0 P:0003F0 57F000            MOVE              X:(RDAD+3),B
                            010003
1497      P:0003F2 P:0003F2 0C1DA1            ASL     #16,B,B
1498      P:0003F3 P:0003F3 000000            NOP
1499      P:0003F4 P:0003F4 216500            MOVE              B2,X1
1500      P:0003F5 P:0003F5 0C1D91            ASL     #8,B,B
1501      P:0003F6 P:0003F6 000000            NOP
1502      P:0003F7 P:0003F7 216400            MOVE              B2,X0
1503      P:0003F8 P:0003F8 000000            NOP
1504      P:0003F9 P:0003F9 09C532            MOVEP             X1,Y:WRFO
1505      P:0003FA P:0003FA 09C432            MOVEP             X0,Y:WRFO
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 28



1506      P:0003FB P:0003FB 060AA0            REP     #10
1507      P:0003FC P:0003FC 000000            NOP
1508      P:0003FD P:0003FD 00000C            RTS
1509   
1510                                ; Alert the PCI interface board that images are coming soon
1511                                PCI_READ_IMAGE
1512      P:0003FE P:0003FE 57F400            MOVE              #$020104,B              ; Send header word to the FO xmtr
                            020104
1513      P:000400 P:000400 0D00EB            JSR     <XMT_WRD
1514      P:000401 P:000401 57F400            MOVE              #'RDA',B
                            524441
1515      P:000403 P:000403 0D00EB            JSR     <XMT_WRD
1516      P:000404 P:000404 5FF000            MOVE                          Y:NSR,B     ; Number of columns to read
                            000001
1517      P:000406 P:000406 0D00EB            JSR     <XMT_WRD
1518      P:000407 P:000407 5FF000            MOVE                          Y:NPR,B     ; Number of rows to read
                            000002
1519      P:000409 P:000409 0D00EB            JSR     <XMT_WRD
1520      P:00040A P:00040A 00000C            RTS
1521   
1522                                ; Wait for the clocking to be complete before proceeding
1523                                WAIT_TO_FINISH_CLOCKING
1524      P:00040B P:00040B 01ADA1            JSET    #SSFEF,X:PDRD,*                   ; Wait for the SS FIFO to be empty
                            00040B
1525      P:00040D P:00040D 00000C            RTS
1526   
1527                                ; Delay for serial writes to the PALs and DACs by 8 microsec
1528      P:00040E P:00040E 062083  PAL_DLY   DO      #800,*+3                          ; Wait 8 usec for serial data xmit
                            000410
1529      P:000410 P:000410 000000            NOP
1530      P:000411 P:000411 00000C            RTS
1531   
1532                                ; Let the host computer read the controller configuration
1533                                READ_CONTROLLER_CONFIGURATION
1534      P:000412 P:000412 4F8900            MOVE                          Y:<CONFIG,Y1 ; Just transmit the configuration
1535      P:000413 P:000413 0C0090            JMP     <FINISH1
1536   
1537                                ; Set the video processor gain and integrator speed for all video boards
1538                                ;                                         #SPEED = 0 for slow, 1 for fast
1539      P:000414 P:000414 012F23  ST_GAIN   BSET    #3,X:PCRD                         ; Turn on the serial clock
1540      P:000415 P:000415 56DB00            MOVE              X:(R3)+,A               ; Gain value (1,2,5 or 10)
1541      P:000416 P:000416 44F400            MOVE              #>1,X0
                            000001
1542      P:000418 P:000418 20001B            CLR     B
1543   
1544      P:000419 P:000419 060F80            DO      #15,CHK_GAIN
                            00041E
1545      P:00041B P:00041B 200005            CMP     B,A
1546      P:00041C P:00041C 0EA420            JEQ     <STG_A
1547      P:00041D P:00041D 200048            ADD     X0,B
1548      P:00041E P:00041E 000000            NOP
1549                                CHK_GAIN
1550      P:00041F P:00041F 0C042A            JMP     <ERR_SGN
1551   
1552      P:000420 P:000420 5E0000  STG_A     MOVE                          A,Y:<GAIN   ; Store the GAIN value for later use
1553      P:000421 P:000421 240D00            MOVE              #$0D0000,X0
1554      P:000422 P:000422 200042            OR      X0,A
1555      P:000423 P:000423 000000            NOP
1556   
1557                                ; Send this same value to 15 video processor boards whether they exist or not
1558                                ;       MOVE    #$100000,X0     ; Increment value
1559                                ;       DO      #15,STG_LOOP
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 29



1560                                ; DO NOT LOOP!  You will send out a word which is 0x2D000x where x is the
1561                                ; gain settings.  This will set one of the reset drain voltages to ~0V and
1562                                ; so mess up the CCD depletion.  You can't set the HV board address to get
1563                                ; around this...  If you end up with more than one video card in a box
1564                                ; then you'll have to manually set this.
1565      P:000424 P:000424 0D020C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1566      P:000425 P:000425 0D040E            JSR     <PAL_DLY                          ; Wait for SSI and PAL to be empty
1567      P:000426 P:000426 200040            ADD     X0,A                              ; Increment the video processor board number
1568      P:000427 P:000427 000000            NOP
1569                                STG_LOOP
1570      P:000428 P:000428 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1571      P:000429 P:000429 0C008F            JMP     <FINISH
1572   
1573      P:00042A P:00042A 56DB00  ERR_SGN   MOVE              X:(R3)+,A
1574      P:00042B P:00042B 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1575      P:00042C P:00042C 0C008D            JMP     <ERROR
1576   
1577                                ; Set a particular DAC numbers, for setting DC bias voltages, clock driver
1578                                ;   voltages and video processor offset
1579                                ; This is code for the ARC32 clock driver and ARC45 CCD video processor
1580                                ;
1581                                ; SBN  #BOARD  #DAC  ['CLK' or 'VID'] voltage
1582                                ;
1583                                ;                               #BOARD is from 0 to 15
1584                                ;                               #DAC number
1585                                ;                               #voltage is from 0 to 4095
1586   
1587                                SET_BIAS_NUMBER                                     ; Set bias number
1588      P:00042D P:00042D 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock
1589      P:00042E P:00042E 56DB00            MOVE              X:(R3)+,A               ; First argument is board number, 0 to 15
1590      P:00042F P:00042F 0614A0            REP     #20
1591      P:000430 P:000430 200033            LSL     A
1592      P:000431 P:000431 000000            NOP
1593      P:000432 P:000432 21C500            MOVE              A,X1                    ; Save the board number
1594      P:000433 P:000433 56DB00            MOVE              X:(R3)+,A               ; Second argument is DAC number
1595      P:000434 P:000434 57E300            MOVE              X:(R3),B                ; Third argument is 'VID' or 'CLK' string
1596      P:000435 P:000435 0140CD            CMP     #'VID',B
                            564944
1597      P:000437 P:000437 0E243F            JNE     <CLK_DRV
1598      P:000438 P:000438 060EA0            REP     #14
1599      P:000439 P:000439 200033            LSL     A
1600      P:00043A P:00043A 000000            NOP
1601      P:00043B P:00043B 0ACC73            BSET    #19,A1                            ; Set bits to mean video processor DAC
1602      P:00043C P:00043C 000000            NOP
1603      P:00043D P:00043D 0ACC72            BSET    #18,A1
1604      P:00043E P:00043E 0C0469            JMP     <BD_SET
1605      P:00043F P:00043F 0140CD  CLK_DRV   CMP     #'CLK',B
                            434C4B
1606      P:000441 P:000441 0E247E            JNE     <ERR_SBN
1607   
1608                                ; For ARC32 do some trickiness to set the chip select and address bits
1609      P:000442 P:000442 218F00            MOVE              A1,B
1610      P:000443 P:000443 060EA0            REP     #14
1611      P:000444 P:000444 200033            LSL     A
1612      P:000445 P:000445 240E00            MOVE              #$0E0000,X0
1613      P:000446 P:000446 200046            AND     X0,A
1614      P:000447 P:000447 44F400            MOVE              #>7,X0
                            000007
1615      P:000449 P:000449 20004E            AND     X0,B                              ; Get 3 least significant bits of clock #
1616      P:00044A P:00044A 01408D            CMP     #0,B
1617      P:00044B P:00044B 0E244E            JNE     <CLK_1
1618      P:00044C P:00044C 0ACE68            BSET    #8,A
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 30



1619      P:00044D P:00044D 0C0469            JMP     <BD_SET
1620      P:00044E P:00044E 01418D  CLK_1     CMP     #1,B
1621      P:00044F P:00044F 0E2452            JNE     <CLK_2
1622      P:000450 P:000450 0ACE69            BSET    #9,A
1623      P:000451 P:000451 0C0469            JMP     <BD_SET
1624      P:000452 P:000452 01428D  CLK_2     CMP     #2,B
1625      P:000453 P:000453 0E2456            JNE     <CLK_3
1626      P:000454 P:000454 0ACE6A            BSET    #10,A
1627      P:000455 P:000455 0C0469            JMP     <BD_SET
1628      P:000456 P:000456 01438D  CLK_3     CMP     #3,B
1629      P:000457 P:000457 0E245A            JNE     <CLK_4
1630      P:000458 P:000458 0ACE6B            BSET    #11,A
1631      P:000459 P:000459 0C0469            JMP     <BD_SET
1632      P:00045A P:00045A 01448D  CLK_4     CMP     #4,B
1633      P:00045B P:00045B 0E245E            JNE     <CLK_5
1634      P:00045C P:00045C 0ACE6D            BSET    #13,A
1635      P:00045D P:00045D 0C0469            JMP     <BD_SET
1636      P:00045E P:00045E 01458D  CLK_5     CMP     #5,B
1637      P:00045F P:00045F 0E2462            JNE     <CLK_6
1638      P:000460 P:000460 0ACE6E            BSET    #14,A
1639      P:000461 P:000461 0C0469            JMP     <BD_SET
1640      P:000462 P:000462 01468D  CLK_6     CMP     #6,B
1641      P:000463 P:000463 0E2466            JNE     <CLK_7
1642      P:000464 P:000464 0ACE6F            BSET    #15,A
1643      P:000465 P:000465 0C0469            JMP     <BD_SET
1644      P:000466 P:000466 01478D  CLK_7     CMP     #7,B
1645      P:000467 P:000467 0E2469            JNE     <BD_SET
1646      P:000468 P:000468 0ACE70            BSET    #16,A
1647   
1648      P:000469 P:000469 200062  BD_SET    OR      X1,A                              ; Add on the board number
1649      P:00046A P:00046A 000000            NOP
1650      P:00046B P:00046B 21C400            MOVE              A,X0
1651      P:00046C P:00046C 57DB00            MOVE              X:(R3)+,B               ; Third argument (again) is 'VID' or 'CLK' s
tring
1652      P:00046D P:00046D 0140CD            CMP     #'VID',B
                            564944
1653      P:00046F P:00046F 0EA478            JEQ     <VID
1654      P:000470 P:000470 56DB00            MOVE              X:(R3)+,A               ; Fourth argument is voltage value, 0 to $ff
f
1655      P:000471 P:000471 0604A0            REP     #4
1656      P:000472 P:000472 200023            LSR     A                                 ; Convert 12 bits to 8 bits for ARC32
1657      P:000473 P:000473 46F400            MOVE              #>$FF,Y0                ; Mask off just 8 bits
                            0000FF
1658      P:000475 P:000475 200056            AND     Y0,A
1659      P:000476 P:000476 200042            OR      X0,A
1660      P:000477 P:000477 0C047A            JMP     <XMT_SBN
1661      P:000478 P:000478 56DB00  VID       MOVE              X:(R3)+,A               ; Fourth argument is voltage value for ARC45
, 12 bits
1662      P:000479 P:000479 200042            OR      X0,A
1663   
1664      P:00047A P:00047A 0D020C  XMT_SBN   JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1665      P:00047B P:00047B 0D040E            JSR     <PAL_DLY                          ; Wait for the number to be sent
1666      P:00047C P:00047C 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1667      P:00047D P:00047D 0C008F            JMP     <FINISH
1668      P:00047E P:00047E 56DB00  ERR_SBN   MOVE              X:(R3)+,A               ; Read and discard the fourth argument
1669      P:00047F P:00047F 012F03            BCLR    #3,X:PCRD                         ; Turn the serial clock off
1670      P:000480 P:000480 0C008D            JMP     <ERROR
1671   
1672   
1673                                ; Specify the MUX value to be output on the clock driver board
1674                                ; Command syntax is  SMX  #clock_driver_board #MUX1 #MUX2
1675                                ;                               #clock_driver_board from 0 to 15
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 31



1676                                ;                               #MUX1, #MUX2 from 0 to 23
1677   
1678      P:000481 P:000481 012F23  SET_MUX   BSET    #3,X:PCRD                         ; Turn on the serial clock
1679      P:000482 P:000482 56DB00            MOVE              X:(R3)+,A               ; Clock driver board number
1680      P:000483 P:000483 0614A0            REP     #20
1681      P:000484 P:000484 200033            LSL     A
1682      P:000485 P:000485 44F400            MOVE              #$003000,X0
                            003000
1683      P:000487 P:000487 200042            OR      X0,A
1684      P:000488 P:000488 000000            NOP
1685      P:000489 P:000489 21C500            MOVE              A,X1                    ; Move here for storage
1686   
1687                                ; Get the first MUX number
1688      P:00048A P:00048A 56DB00            MOVE              X:(R3)+,A               ; Get the first MUX number
1689      P:00048B P:00048B 0AF0A9            JLT     ERR_SM1
                            0004CF
1690      P:00048D P:00048D 44F400            MOVE              #>24,X0                 ; Check for argument less than 32
                            000018
1691      P:00048F P:00048F 200045            CMP     X0,A
1692      P:000490 P:000490 0AF0A1            JGE     ERR_SM1
                            0004CF
1693      P:000492 P:000492 21CF00            MOVE              A,B
1694      P:000493 P:000493 44F400            MOVE              #>7,X0
                            000007
1695      P:000495 P:000495 20004E            AND     X0,B
1696      P:000496 P:000496 44F400            MOVE              #>$18,X0
                            000018
1697      P:000498 P:000498 200046            AND     X0,A
1698      P:000499 P:000499 0E249C            JNE     <SMX_1                            ; Test for 0 <= MUX number <= 7
1699      P:00049A P:00049A 0ACD63            BSET    #3,B1
1700      P:00049B P:00049B 0C04A7            JMP     <SMX_A
1701      P:00049C P:00049C 44F400  SMX_1     MOVE              #>$08,X0
                            000008
1702      P:00049E P:00049E 200045            CMP     X0,A                              ; Test for 8 <= MUX number <= 15
1703      P:00049F P:00049F 0E24A2            JNE     <SMX_2
1704      P:0004A0 P:0004A0 0ACD64            BSET    #4,B1
1705      P:0004A1 P:0004A1 0C04A7            JMP     <SMX_A
1706      P:0004A2 P:0004A2 44F400  SMX_2     MOVE              #>$10,X0
                            000010
1707      P:0004A4 P:0004A4 200045            CMP     X0,A                              ; Test for 16 <= MUX number <= 23
1708      P:0004A5 P:0004A5 0E24CF            JNE     <ERR_SM1
1709      P:0004A6 P:0004A6 0ACD65            BSET    #5,B1
1710      P:0004A7 P:0004A7 20006A  SMX_A     OR      X1,B1                             ; Add prefix to MUX numbers
1711      P:0004A8 P:0004A8 000000            NOP
1712      P:0004A9 P:0004A9 21A700            MOVE              B1,Y1
1713   
1714                                ; Add on the second MUX number
1715      P:0004AA P:0004AA 56DB00            MOVE              X:(R3)+,A               ; Get the next MUX number
1716      P:0004AB P:0004AB 0E908D            JLT     <ERROR
1717      P:0004AC P:0004AC 44F400            MOVE              #>24,X0                 ; Check for argument less than 32
                            000018
1718      P:0004AE P:0004AE 200045            CMP     X0,A
1719      P:0004AF P:0004AF 0E108D            JGE     <ERROR
1720      P:0004B0 P:0004B0 0606A0            REP     #6
1721      P:0004B1 P:0004B1 200033            LSL     A
1722      P:0004B2 P:0004B2 000000            NOP
1723      P:0004B3 P:0004B3 21CF00            MOVE              A,B
1724      P:0004B4 P:0004B4 44F400            MOVE              #$1C0,X0
                            0001C0
1725      P:0004B6 P:0004B6 20004E            AND     X0,B
1726      P:0004B7 P:0004B7 44F400            MOVE              #>$600,X0
                            000600
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 32



1727      P:0004B9 P:0004B9 200046            AND     X0,A
1728      P:0004BA P:0004BA 0E24BD            JNE     <SMX_3                            ; Test for 0 <= MUX number <= 7
1729      P:0004BB P:0004BB 0ACD69            BSET    #9,B1
1730      P:0004BC P:0004BC 0C04C8            JMP     <SMX_B
1731      P:0004BD P:0004BD 44F400  SMX_3     MOVE              #>$200,X0
                            000200
1732      P:0004BF P:0004BF 200045            CMP     X0,A                              ; Test for 8 <= MUX number <= 15
1733      P:0004C0 P:0004C0 0E24C3            JNE     <SMX_4
1734      P:0004C1 P:0004C1 0ACD6A            BSET    #10,B1
1735      P:0004C2 P:0004C2 0C04C8            JMP     <SMX_B
1736      P:0004C3 P:0004C3 44F400  SMX_4     MOVE              #>$400,X0
                            000400
1737      P:0004C5 P:0004C5 200045            CMP     X0,A                              ; Test for 16 <= MUX number <= 23
1738      P:0004C6 P:0004C6 0E208D            JNE     <ERROR
1739      P:0004C7 P:0004C7 0ACD6B            BSET    #11,B1
1740      P:0004C8 P:0004C8 200078  SMX_B     ADD     Y1,B                              ; Add prefix to MUX numbers
1741      P:0004C9 P:0004C9 000000            NOP
1742      P:0004CA P:0004CA 21AE00            MOVE              B1,A
1743      P:0004CB P:0004CB 0D020C            JSR     <XMIT_A_WORD                      ; Transmit A to TIM-A-STD
1744      P:0004CC P:0004CC 0D040E            JSR     <PAL_DLY                          ; Delay for all this to happen
1745      P:0004CD P:0004CD 012F03            BCLR    #3,X:PCRD                         ; Turn off the serial clock
1746      P:0004CE P:0004CE 0C008F            JMP     <FINISH
1747      P:0004CF P:0004CF 56DB00  ERR_SM1   MOVE              X:(R3)+,A
1748      P:0004D0 P:0004D0 012F03            BCLR    #3,X:PCRD                         ; Turn off the serial clock
1749      P:0004D1 P:0004D1 0C008D            JMP     <ERROR
1750   
1751                                ; GEOMETRY subroutine
1752                                SET_GEOMETRY
1753      P:0004D2 P:0004D2 47DB00            MOVE              X:(R3)+,Y1              ; Get first parameter (prescan pixels)
1754      P:0004D3 P:0004D3 4F1100            MOVE                          Y1,Y:<NS_PRE ; Move it to Ymem
1755      P:0004D4 P:0004D4 47DB00            MOVE              X:(R3)+,Y1              ; Get second parameter XDATA
1756      P:0004D5 P:0004D5 4F1300            MOVE                          Y1,Y:<NS_RD ;
1757      P:0004D6 P:0004D6 47DB00            MOVE              X:(R3)+,Y1              ; Get third parameter (overscan pixels)
1758      P:0004D7 P:0004D7 4F1200            MOVE                          Y1,Y:<NS_OVR ; Move it to Ymem
1759      P:0004D8 P:0004D8 0D04E8            JSR     <CALC_GEOM
1760      P:0004D9 P:0004D9 0C008F            JMP     <FINISH                           ; End, send back 'DON'
1761   
1762   
1763                                ; ROI parameters: xstart ystart xlen ylen (ylen not used)
1764                                SET_ROI
1765      P:0004DA P:0004DA 56DB00            MOVE              X:(R3)+,A               ; Get first parameter XSTART
1766      P:0004DB P:0004DB 449200            MOVE              X:<ONE,X0
1767      P:0004DC P:0004DC 200044            SUB     X0,A                              ; substract 1
1768      P:0004DD P:0004DD 000000            NOP
1769      P:0004DE P:0004DE 5C0F00            MOVE                          A1,Y:<NS_SKP1 ; Move it to the proper loc in Y:mem
1770      P:0004DF P:0004DF 56DB00            MOVE              X:(R3)+,A               ; Get second parameter YSTART
1771      P:0004E0 P:0004E0 200044            SUB     X0,A                              ; substract 1
1772      P:0004E1 P:0004E1 000000            NOP
1773      P:0004E2 P:0004E2 5C0E00            MOVE                          A1,Y:<NP_SKIP ; Move it to Y: mem also.
1774      P:0004E3 P:0004E3 56DB00            MOVE              X:(R3)+,A               ; Get third parameter XLEN
1775      P:0004E4 P:0004E4 000000            NOP
1776      P:0004E5 P:0004E5 5E1300            MOVE                          A,Y:<NS_RD  ; Move it to Y: mem also.
1777   
1778      P:0004E6 P:0004E6 0D04E8            JSR     <CALC_GEOM
1779      P:0004E7 P:0004E7 0C008F            JMP     <FINISH                           ; End, send back 'DON'
1780   
1781                                CALC_GEOM
1782      P:0004E8 P:0004E8 5EAC00            MOVE                          Y:<NSR_ACTIVE,A ; number of x data pixels (binned)
1783      P:0004E9 P:0004E9 4C8F00            MOVE                          Y:<NS_SKP1,X0
1784      P:0004EA P:0004EA 200044            SUB     X0,A                              ; subtract the xstart
1785      P:0004EB P:0004EB 4C9300            MOVE                          Y:<NS_RD,X0
1786      P:0004EC P:0004EC 200044            SUB     X0,A                              ; subtract the data to read
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 33



1787      P:0004ED P:0004ED 000000            NOP
1788      P:0004EE P:0004EE 5C1000            MOVE                          A1,Y:<NS_SKP2 ; now the remaining is the
1789      P:0004EF P:0004EF 00000C            RTS                                       ; data to skip to reach overscan
1790   
1791   
1792                                ; Generate the serial readout waveform table for the chosen
1793                                ;   value of readout and serial binning.
1794   
1795                                GENERATE_SERIAL_WAVEFORM
1796      P:0004F0 P:0004F0 60F400            MOVE              #CCD_RESET,R0
                            0000DD
1797      P:0004F2 P:0004F2 61F400            MOVE              #(PXL_TBL+1),R1
                            00021B
1798      P:0004F4 P:0004F4 5ED800            MOVE                          Y:(R0)+,A
1799      P:0004F5 P:0004F5 000000            NOP
1800      P:0004F6 P:0004F6 06CC00            DO      A1,L_CCD_RESET
                            0004F9
1801      P:0004F8 P:0004F8 4CD800            MOVE                          Y:(R0)+,X0
1802      P:0004F9 P:0004F9 4C5900            MOVE                          X0,Y:(R1)+
1803                                L_CCD_RESET
1804   
1805                                ; Generate the first set of clocks
1806      P:0004FA P:0004FA 68F000            MOVE                          Y:FIRST_CLOCKS,R0
                            00000C
1807      P:0004FC P:0004FC 000000            NOP
1808      P:0004FD P:0004FD 000000            NOP
1809      P:0004FE P:0004FE 5ED800            MOVE                          Y:(R0)+,A
1810      P:0004FF P:0004FF 000000            NOP
1811      P:000500 P:000500 06CC00            DO      A1,L_FIRST_CLOCKS
                            000503
1812      P:000502 P:000502 4CD800            MOVE                          Y:(R0)+,X0
1813      P:000503 P:000503 4C5900            MOVE                          X0,Y:(R1)+
1814                                L_FIRST_CLOCKS
1815   
1816   
1817                                ; Generate the binning waveforms if needed
1818      P:000504 P:000504 5E8500            MOVE                          Y:<NSBIN,A
1819      P:000505 P:000505 014184            SUB     #1,A
1820      P:000506 P:000506 0EF514            JLE     <GEN_VID
1821      P:000507 P:000507 06CC00            DO      A1,L_BIN
                            000513
1822      P:000509 P:000509 68F000            MOVE                          Y:CLOCK_LINE,R0
                            00000D
1823      P:00050B P:00050B 000000            NOP
1824      P:00050C P:00050C 000000            NOP
1825      P:00050D P:00050D 5ED800            MOVE                          Y:(R0)+,A
1826      P:00050E P:00050E 000000            NOP
1827      P:00050F P:00050F 06CC00            DO      A1,L_CLOCK_LINE
                            000512
1828      P:000511 P:000511 4CD800            MOVE                          Y:(R0)+,X0
1829      P:000512 P:000512 4C5900            MOVE                          X0,Y:(R1)+
1830                                L_CLOCK_LINE
1831      P:000513 P:000513 000000            NOP
1832                                L_BIN
1833   
1834                                ; Generate the video processor waveforms
1835      P:000514 P:000514 60F400  GEN_VID   MOVE              #VIDEO_PROCESS,R0
                            0000FA
1836      P:000516 P:000516 000000            NOP
1837      P:000517 P:000517 000000            NOP
1838      P:000518 P:000518 5ED800            MOVE                          Y:(R0)+,A
1839      P:000519 P:000519 000000            NOP
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 34



1840      P:00051A P:00051A 06CC00            DO      A1,L_VID
                            00051D
1841      P:00051C P:00051C 4CD800            MOVE                          Y:(R0)+,X0
1842      P:00051D P:00051D 4C5900            MOVE                          X0,Y:(R1)+
1843                                L_VID
1844   
1845                                ; Finally, calculate the number of entries in the waveform table just generated
1846      P:00051E P:00051E 44F400            MOVE              #PXL_TBL,X0
                            00021A
1847      P:000520 P:000520 209000            MOVE              X0,R0
1848      P:000521 P:000521 222E00            MOVE              R1,A
1849      P:000522 P:000522 200044            SUB     X0,A
1850      P:000523 P:000523 014184            SUB     #1,A
1851      P:000524 P:000524 000000            NOP
1852      P:000525 P:000525 5C6000            MOVE                          A1,Y:(R0)
1853      P:000526 P:000526 00000C            RTS
1854   
1855                                ; Select which readouts to process
1856                                ;   'SOS'  Amplifier_name
1857                                ;       Amplifier_name = '_L2' '_U2' '_L1' '_L2' '__1' '__2' 'ALL'
1858   
1859                                SELECT_OUTPUT_SOURCE
1860      P:000527 P:000527 46DB00            MOVE              X:(R3)+,Y0
1861      P:000528 P:000528 4E0B00            MOVE                          Y0,Y:<OS
1862      P:000529 P:000529 0D052B            JSR     <SEL_OS
1863      P:00052A P:00052A 0C008F            JMP     <FINISH
1864   
1865      P:00052B P:00052B 4E8B00  SEL_OS    MOVE                          Y:<OS,Y0
1866      P:00052C P:00052C 56F400            MOVE              #'_U1',A
                            5F5531
1867      P:00052E P:00052E 200055            CMP     Y0,A
1868      P:00052F P:00052F 0E254B            JNE     <COMP_U2
1869   
1870      P:000530 P:000530 44F400            MOVE              #$F0C3,X0
                            00F0C3
1871      P:000532 P:000532 4C7000            MOVE                          X0,Y:SXL
                            00010C
1872      P:000534 P:000534 44F400            MOVE              #PARALLEL_1,X0
                            00004B
1873      P:000536 P:000536 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
1874      P:000538 P:000538 44F400            MOVE              #SERIAL_READ_LEFT,X0
                            000103
1875                                ;        MOVE    #SERIAL_READ_RIGHT,X0          ; Clock away from the amplifier (reverse clockin
g) for Test Only
1876                                ;        MOVE    #SERIAL_READ_LEFT_NULL,X0      ; Permanent video reset for Test Only.
1877      P:00053A P:00053A 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
1878                                ;       MOVE    #SERIAL_SKIP_LEFT,X0
1879      P:00053C P:00053C 44F400            MOVE              #SERIAL_SKIP_SPLIT,X0
                            000171
1880      P:00053E P:00053E 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
1881      P:000540 P:000540 44F400            MOVE              #SERIAL_IDLE_LEFT,X0
                            000073
1882      P:000542 P:000542 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
1883      P:000544 P:000544 44F400            MOVE              #PARALLEL_CLEAR_1,X0
                            000055
1884      P:000546 P:000546 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
1885      P:000548 P:000548 0A0005            BCLR    #SPLIT_S,X:STATUS
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 35



1886      P:000549 P:000549 0A0006            BCLR    #SPLIT_P,X:STATUS
1887      P:00054A P:00054A 00000C            RTS
1888   
1889      P:00054B P:00054B 56F400  COMP_U2   MOVE              #'_U2',A
                            5F5532
1890      P:00054D P:00054D 200055            CMP     Y0,A
1891      P:00054E P:00054E 0E256A            JNE     <COMP_L1
1892   
1893      P:00054F P:00054F 44F400            MOVE              #$F041,X0
                            00F041
1894      P:000551 P:000551 4C7000            MOVE                          X0,Y:SXR
                            000144
1895      P:000553 P:000553 44F400            MOVE              #PARALLEL_2,X0
                            000041
1896      P:000555 P:000555 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
1897      P:000557 P:000557 44F400            MOVE              #SERIAL_READ_RIGHT,X0
                            00013B
1898      P:000559 P:000559 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
1899      P:00055B P:00055B 44F400            MOVE              #SERIAL_SKIP_RIGHT,X0
                            000168
1900      P:00055D P:00055D 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
1901      P:00055F P:00055F 44F400            MOVE              #SERIAL_IDLE_RIGHT,X0
                            000095
1902      P:000561 P:000561 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
1903      P:000563 P:000563 44F400            MOVE              #PARALLEL_CLEAR_2,X0
                            00005F
1904      P:000565 P:000565 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
1905      P:000567 P:000567 0A0005            BCLR    #SPLIT_S,X:STATUS
1906      P:000568 P:000568 0A0006            BCLR    #SPLIT_P,X:STATUS
1907      P:000569 P:000569 00000C            RTS
1908   
1909      P:00056A P:00056A 56F400  COMP_L1   MOVE              #'_L1',A
                            5F4C31
1910      P:00056C P:00056C 200055            CMP     Y0,A
1911      P:00056D P:00056D 0E2589            JNE     <COMP_L2
1912   
1913   
1914      P:00056E P:00056E 44F400            MOVE              #$F082,X0
                            00F082
1915      P:000570 P:000570 4C7000            MOVE                          X0,Y:SXR
                            000144
1916      P:000572 P:000572 44F400            MOVE              #PARALLEL_1,X0
                            00004B
1917      P:000574 P:000574 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
1918      P:000576 P:000576 44F400            MOVE              #SERIAL_READ_RIGHT,X0
                            00013B
1919      P:000578 P:000578 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
1920      P:00057A P:00057A 44F400            MOVE              #SERIAL_SKIP_RIGHT,X0
                            000168
1921      P:00057C P:00057C 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
1922      P:00057E P:00057E 44F400            MOVE              #SERIAL_IDLE_RIGHT,X0
                            000095
1923      P:000580 P:000580 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 36



1924      P:000582 P:000582 44F400            MOVE              #PARALLEL_CLEAR_1,X0
                            000055
1925      P:000584 P:000584 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
1926      P:000586 P:000586 0A0005            BCLR    #SPLIT_S,X:STATUS
1927      P:000587 P:000587 0A0006            BCLR    #SPLIT_P,X:STATUS
1928      P:000588 P:000588 00000C            RTS
1929   
1930      P:000589 P:000589 56F400  COMP_L2   MOVE              #'_L2',A
                            5F4C32
1931      P:00058B P:00058B 200055            CMP     Y0,A
1932      P:00058C P:00058C 0E25A8            JNE     <COMP_2
1933   
1934      P:00058D P:00058D 44F400            MOVE              #$F000,X0
                            00F000
1935      P:00058F P:00058F 4C7000            MOVE                          X0,Y:SXL
                            00010C
1936      P:000591 P:000591 44F400            MOVE              #PARALLEL_2,X0
                            000041
1937      P:000593 P:000593 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
1938      P:000595 P:000595 44F400            MOVE              #SERIAL_READ_LEFT,X0
                            000103
1939      P:000597 P:000597 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
1940      P:000599 P:000599 44F400            MOVE              #SERIAL_SKIP_LEFT,X0
                            00015F
1941      P:00059B P:00059B 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
1942      P:00059D P:00059D 44F400            MOVE              #SERIAL_IDLE_LEFT,X0
                            000073
1943      P:00059F P:00059F 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
1944      P:0005A1 P:0005A1 44F400            MOVE              #PARALLEL_CLEAR_2,X0
                            00005F
1945      P:0005A3 P:0005A3 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
1946      P:0005A5 P:0005A5 0A0005            BCLR    #SPLIT_S,X:STATUS
1947      P:0005A6 P:0005A6 0A0006            BCLR    #SPLIT_P,X:STATUS
1948      P:0005A7 P:0005A7 00000C            RTS
1949   
1950      P:0005A8 P:0005A8 56F400  COMP_2    MOVE              #'__2',A
                            5F5F32
1951      P:0005AA P:0005AA 200055            CMP     Y0,A
1952      P:0005AB P:0005AB 0E25C7            JNE     <COMP_1
1953   
1954                                ;        MOVE    #$F040,X0
1955      P:0005AC P:0005AC 44F400            MOVE              #$F041,X0
                            00F041
1956      P:0005AE P:0005AE 4C7000            MOVE                          X0,Y:SXRL
                            00011E
1957      P:0005B0 P:0005B0 44F400            MOVE              #PARALLEL_2,X0
                            000041
1958      P:0005B2 P:0005B2 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
1959                                ;        MOVE    #SERIAL_READ_SPLIT,X0
1960      P:0005B4 P:0005B4 44F400            MOVE              #SERIAL_READ_SPLIT_SPECIAL,X0
                            000127
1961      P:0005B6 P:0005B6 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
1962      P:0005B8 P:0005B8 44F400            MOVE              #SERIAL_SKIP_SPLIT,X0
                            000171
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 37



1963      P:0005BA P:0005BA 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
1964      P:0005BC P:0005BC 44F400            MOVE              #SERIAL_IDLE_SPLIT,X0
                            0000B9
1965      P:0005BE P:0005BE 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
1966      P:0005C0 P:0005C0 44F400            MOVE              #PARALLEL_CLEAR_2,X0
                            00005F
1967      P:0005C2 P:0005C2 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
1968      P:0005C4 P:0005C4 0A0025            BSET    #SPLIT_S,X:STATUS
1969      P:0005C5 P:0005C5 0A0006            BCLR    #SPLIT_P,X:STATUS
1970      P:0005C6 P:0005C6 00000C            RTS
1971   
1972      P:0005C7 P:0005C7 56F400  COMP_1    MOVE              #'__1',A
                            5F5F31
1973      P:0005C9 P:0005C9 200055            CMP     Y0,A
1974      P:0005CA P:0005CA 0E25E6            JNE     <COMP_ALL
1975   
1976      P:0005CB P:0005CB 44F400            MOVE              #$F0C2,X0
                            00F0C2
1977      P:0005CD P:0005CD 4C7000            MOVE                          X0,Y:SXRL
                            00011E
1978      P:0005CF P:0005CF 44F400            MOVE              #PARALLEL_1,X0
                            00004B
1979      P:0005D1 P:0005D1 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
1980      P:0005D3 P:0005D3 44F400            MOVE              #SERIAL_READ_SPLIT,X0
                            000115
1981      P:0005D5 P:0005D5 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
1982      P:0005D7 P:0005D7 44F400            MOVE              #SERIAL_SKIP_SPLIT,X0
                            000171
1983      P:0005D9 P:0005D9 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
1984      P:0005DB P:0005DB 44F400            MOVE              #SERIAL_IDLE_SPLIT,X0
                            0000B9
1985      P:0005DD P:0005DD 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
1986      P:0005DF P:0005DF 44F400            MOVE              #PARALLEL_CLEAR_1,X0
                            000055
1987      P:0005E1 P:0005E1 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
1988      P:0005E3 P:0005E3 0A0025            BSET    #SPLIT_S,X:STATUS
1989      P:0005E4 P:0005E4 0A0006            BCLR    #SPLIT_P,X:STATUS
1990      P:0005E5 P:0005E5 00000C            RTS
1991   
1992      P:0005E6 P:0005E6 56F400  COMP_ALL  MOVE              #'ALL',A
                            414C4C
1993      P:0005E8 P:0005E8 200055            CMP     Y0,A
1994      P:0005E9 P:0005E9 0E208D            JNE     <ERROR
1995   
1996      P:0005EA P:0005EA 44F400            MOVE              #$F0C0,X0
                            00F0C0
1997      P:0005EC P:0005EC 4C7000            MOVE                          X0,Y:SXRL
                            00011E
1998      P:0005EE P:0005EE 44F400            MOVE              #PARALLEL_SPLIT,X0
                            000037
1999      P:0005F0 P:0005F0 4C7000            MOVE                          X0,Y:PARALLEL
                            000018
2000      P:0005F2 P:0005F2 44F400            MOVE              #SERIAL_READ_SPLIT,X0
                            000115
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 38



2001      P:0005F4 P:0005F4 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
2002      P:0005F6 P:0005F6 44F400            MOVE              #SERIAL_SKIP_SPLIT,X0
                            000171
2003      P:0005F8 P:0005F8 4C7000            MOVE                          X0,Y:SERIAL_SKIP
                            000017
2004      P:0005FA P:0005FA 44F400            MOVE              #SERIAL_IDLE_SPLIT,X0
                            0000B9
2005      P:0005FC P:0005FC 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2006      P:0005FE P:0005FE 44F400            MOVE              #PARALLEL_CLEAR_SPLIT,X0
                            000069
2007      P:000600 P:000600 4C7000            MOVE                          X0,Y:PARALLEL_CLEAR
                            000019
2008      P:000602 P:000602 0A0025            BSET    #SPLIT_S,X:STATUS
2009      P:000603 P:000603 0A0026            BSET    #SPLIT_P,X:STATUS
2010      P:000604 P:000604 00000C            RTS
2011   
2012                                NO_POLARITY_SHIFT
2013      P:000605 P:000605 4E8B00            MOVE                          Y:<OS,Y0
2014      P:000606 P:000606 56F400            MOVE              #'_U1',A
                            5F5531
2015      P:000608 P:000608 200055            CMP     Y0,A
2016      P:000609 P:000609 0E260F            JNE     <COMP_U2_NO_POL
2017   
2018      P:00060A P:00060A 44F400            MOVE              #SERIAL_IDLE_LEFT_NO_POL,X0
                            000084
2019      P:00060C P:00060C 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2020      P:00060E P:00060E 0C008F            JMP     <FINISH
2021   
2022                                COMP_U2_NO_POL
2023      P:00060F P:00060F 56F400            MOVE              #'_U2',A
                            5F5532
2024      P:000611 P:000611 200055            CMP     Y0,A
2025      P:000612 P:000612 0E2618            JNE     <COMP_L1_NO_POL
2026   
2027      P:000613 P:000613 44F400            MOVE              #SERIAL_IDLE_RIGHT_NO_POL,X0
                            0000A7
2028      P:000615 P:000615 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2029      P:000617 P:000617 0C008F            JMP     <FINISH
2030   
2031                                COMP_L1_NO_POL
2032      P:000618 P:000618 56F400            MOVE              #'_L1',A
                            5F4C31
2033      P:00061A P:00061A 200055            CMP     Y0,A
2034      P:00061B P:00061B 0E2621            JNE     <COMP_L2_NO_POL
2035   
2036      P:00061C P:00061C 44F400            MOVE              #SERIAL_IDLE_RIGHT_NO_POL,X0
                            0000A7
2037      P:00061E P:00061E 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2038      P:000620 P:000620 0C008F            JMP     <FINISH
2039   
2040                                COMP_L2_NO_POL
2041      P:000621 P:000621 56F400            MOVE              #'_L2',A
                            5F4C32
2042      P:000623 P:000623 200055            CMP     Y0,A
2043      P:000624 P:000624 0E262A            JNE     <COMP_2_NO_POL
2044   
2045      P:000625 P:000625 44F400            MOVE              #SERIAL_IDLE_LEFT_NO_POL,X0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 39



                            000084
2046      P:000627 P:000627 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2047      P:000629 P:000629 0C008F            JMP     <FINISH
2048   
2049                                COMP_2_NO_POL
2050      P:00062A P:00062A 56F400            MOVE              #'__2',A
                            5F5F32
2051      P:00062C P:00062C 200055            CMP     Y0,A
2052      P:00062D P:00062D 0E2633            JNE     <COMP_1_NO_POL
2053   
2054      P:00062E P:00062E 44F400            MOVE              #SERIAL_IDLE_SPLIT_NO_POL,X0
                            0000CB
2055      P:000630 P:000630 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2056      P:000632 P:000632 0C008F            JMP     <FINISH
2057   
2058                                COMP_1_NO_POL
2059      P:000633 P:000633 56F400            MOVE              #'__1',A
                            5F5F31
2060      P:000635 P:000635 200055            CMP     Y0,A
2061      P:000636 P:000636 0E263C            JNE     <COMP_ALL_NO_POL
2062   
2063      P:000637 P:000637 44F400            MOVE              #SERIAL_IDLE_SPLIT_NO_POL,X0
                            0000CB
2064      P:000639 P:000639 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2065      P:00063B P:00063B 0C008F            JMP     <FINISH
2066   
2067                                 COMP_ALL_NO_POL
2068      P:00063C P:00063C 56F400            MOVE              #'ALL',A
                            414C4C
2069      P:00063E P:00063E 200055            CMP     Y0,A
2070      P:00063F P:00063F 0E208D            JNE     <ERROR
2071   
2072      P:000640 P:000640 44F400            MOVE              #SERIAL_IDLE_SPLIT_NO_POL,X0
                            0000CB
2073      P:000642 P:000642 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2074      P:000644 P:000644 0C008F            JMP     <FINISH
2075   
2076   
2077   
2078   
2079      P:000645 P:000645 44DB00  ERASE     MOVE              X:(R3)+,X0              ; Get TIME1 off the command buffer
2080      P:000646 P:000646 4C1400            MOVE                          X0,Y:<TIME1 ; Move it to the address in tim.asm
2081      P:000647 P:000647 44DB00            MOVE              X:(R3)+,X0              ; Ditto for TIME2
2082      P:000648 P:000648 4C1500            MOVE                          X0,Y:<TIME2
2083   
2084                                                                                    ;TODO: Get rid of this horrible hack
2085                                                                                    ;Horrible hack to get over time out on comma
nd return
2086      P:000649 P:000649 0D0680            JSR     <HACK_FINISH
2087      P:00064A P:00064A 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock - this should alr
eady be the case
2088      P:00064B P:00064B 0A0F01            BCLR    #1,X:<LATCH                       ; Separate updates of clock driver
2089      P:00064C P:00064C 0A0F20            BSET    #CDAC,X:<LATCH                    ; Disable clearing of DACs
2090      P:00064D P:00064D 0A0F22            BSET    #ENCK,X:<LATCH                    ; Enable clock and DAC output switches
2091      P:00064E P:00064E 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Write it to the hardware
                            00000F
2092      P:000650 P:000650 0D040E            JSR     <PAL_DLY                          ; Delay for all this to happen
2093   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 40



2094                                ; Read DAC values from a table, and write them to the DACs
2095      P:000651 P:000651 60F400            MOVE              #ERHI,R0                ; Get starting address of erase clock values
                            00017B
2096      P:000653 P:000653 000000            NOP
2097      P:000654 P:000654 000000            NOP
2098      P:000655 P:000655 065840            DO      Y:(R0)+,E_DAC                     ; Repeat Y:(R0)+ times
                            000659
2099      P:000657 P:000657 5ED800            MOVE                          Y:(R0)+,A   ; Read the table entry
2100      P:000658 P:000658 0D020C            JSR     <XMIT_A_WORD                      ; Transmit it to TIM-A-STD
2101      P:000659 P:000659 000000            NOP
2102                                E_DAC
2103   
2104                                ; Let the DAC voltages all ramp up before exiting
2105      P:00065A P:00065A 44F400            MOVE              #400000,X0
                            061A80
2106      P:00065C P:00065C 06C400            DO      X0,*+3                            ; 4 millisec delay
                            00065E
2107      P:00065E P:00065E 000000            NOP
2108   
2109                                ; Start the delay loop
2110   
2111      P:00065F P:00065F 061440            DO      Y:<TIME1,ER_T1                    ; Delay TIME1 msec
                            000662
2112      P:000661 P:000661 0D067A            JSR     <LNG_DLY
2113      P:000662 P:000662 000000            NOP
2114   
2115      P:000663 P:000663 5EF000  ER_T1     MOVE                          Y:VSUBN,A   ; Reset the Vsub value
                            00017A
2116      P:000665 P:000665 0D020C            JSR     <XMIT_A_WORD
2117      P:000666 P:000666 0D040E            JSR     <PAL_DLY                          ; Wait for SSI and PAL to empty
2118   
2119      P:000667 P:000667 061540            DO      Y:<TIME2,ER_T2                    ; Delay TIME2 msec
                            00066A
2120      P:000669 P:000669 0D067A            JSR     <LNG_DLY
2121      P:00066A P:00066A 000000            NOP
2122   
2123      P:00066B P:00066B 60F400  ER_T2     MOVE              #ERHI_END,R0            ; Get the original clock values back.
                            000195
2124                                ;        MOVE    #DACS,R0               ; This line would do the same job -- pointing back to th
e
2125                                                                                    ; original clocking values, but you'd be set
ting everything.
2126   
2127                                ; Read DAC values from a table, and write them to the DACs
2128      P:00066D P:00066D 000000            NOP
2129      P:00066E P:00066E 000000            NOP
2130      P:00066F P:00066F 065840            DO      Y:(R0)+,END_DAC_RESTORE           ; Repeat Y:(R0)+ times
                            000673
2131      P:000671 P:000671 5ED800            MOVE                          Y:(R0)+,A   ; Read the table entry
2132      P:000672 P:000672 0D020C            JSR     <XMIT_A_WORD                      ; Transmit it to TIM-A-STD
2133      P:000673 P:000673 000000            NOP
2134                                END_DAC_RESTORE
2135   
2136                                ; Let the DAC voltages all ramp up before exiting
2137      P:000674 P:000674 44F400            MOVE              #400000,X0
                            061A80
2138      P:000676 P:000676 06C400            DO      X0,*+3                            ; 4 millisec delay
                            000678
2139      P:000678 P:000678 000000            NOP
2140      P:000679 P:000679 0C0054            JMP     <START
2141   
2142                                ; Routine to delay for 1msec
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 41



2143                                ; 40ns for DO and NOP line.  1ms/80ns = 125000
2144   
2145      P:00067A P:00067A 44F400  LNG_DLY   MOVE              #100000,X0
                            0186A0
2146      P:00067C P:00067C 06C400            DO      X0,*+3                            ; Wait 1 millisec for settling
                            00067E
2147      P:00067E P:00067E 000000            NOP
2148      P:00067F P:00067F 00000C            RTS
2149   
2150                                ; Hack finish subroutine to send 'DON' reply back to the PIC
2151                                ; card.  Some of this should be redundant -- such as jumping to the
2152                                ; command processing.
2153                                 HACK_FINISH
2154      P:000680 P:000680 479800            MOVE              X:<DONE,Y1              ; Send 'DON' as the reply
2155                                 HACK_FINISH1
2156      P:000681 P:000681 578500            MOVE              X:<HEADER,B             ; Get header of incoming command
2157      P:000682 P:000682 469C00            MOVE              X:<SMASK,Y0             ; This was the source byte, and is to
2158      P:000683 P:000683 330700            MOVE              #<COM_BUF,R3            ;     become the destination byte
2159      P:000684 P:000684 46935E            AND     Y0,B      X:<TWO,Y0
2160      P:000685 P:000685 0C1ED1            LSR     #8,B                              ; Shift right eight bytes, add it to the
2161      P:000686 P:000686 460600            MOVE              Y0,X:<NWORDS            ;     header, and put 2 as the number
2162      P:000687 P:000687 469958            ADD     Y0,B      X:<SBRD,Y0              ;     of words in the string
2163      P:000688 P:000688 200058            ADD     Y0,B                              ; Add source board's header, set Y1 for abov
e
2164      P:000689 P:000689 000000            NOP
2165      P:00068A P:00068A 575B00            MOVE              B,X:(R3)+               ; Put the new header on the transmitter stac
k
2166      P:00068B P:00068B 475B00            MOVE              Y1,X:(R3)+              ; Put the argument on the transmitter stack
2167      P:00068C P:00068C 570500            MOVE              B,X:<HEADER
2168      P:00068D P:00068D 330700            MOVE              #<COM_BUF,R3            ; Restore R3 = beginning of the command
2169   
2170                                ; Is this command for the timing board?
2171      P:00068E P:00068E 448500            MOVE              X:<HEADER,X0
2172      P:00068F P:00068F 579B00            MOVE              X:<DMASK,B
2173      P:000690 P:000690 459A4E            AND     X0,B      X:<TIM_DRB,X1           ; Extract destination byte
2174      P:000691 P:000691 20006D            CMP     X1,B                              ; Does header = timing board number?
2175      P:000692 P:000692 0EA080            JEQ     <COMMAND                          ; Yes, process it here
2176      P:000693 P:000693 0E9694            JLT     <HACK_FO_XMT                      ; Send it to fiber optic transmitter
2177                                 HACK_FO_XMT
2178      P:000694 P:000694 330700            MOVE              #COM_BUF,R3
2179      P:000695 P:000695 060600            DO      X:<NWORDS,HACK_DON_FFO            ; Transmit all the words in the command
                            000699
2180      P:000697 P:000697 57DB00            MOVE              X:(R3)+,B
2181      P:000698 P:000698 0D00EB            JSR     <XMT_WRD
2182      P:000699 P:000699 000000            NOP
2183                                 HACK_DON_FFO
2184      P:00069A P:00069A 00000C            RTS
2185   
2186      P:00069B P:00069B 44DB00  EPURGE    MOVE              X:(R3)+,X0              ; Get TIME1 off the command buffer
2187      P:00069C P:00069C 4C1400            MOVE                          X0,Y:<TIME1 ; Move it to the address in tim.asm
2188   
2189                                                                                    ;TODO: Get rid of this horrible hack
2190                                                                                    ;Horrible hack to get over time out on comma
nd return
2191      P:00069D P:00069D 0D0680            JSR     <HACK_FINISH
2192      P:00069E P:00069E 012F23            BSET    #3,X:PCRD                         ; Turn on the serial clock - this should alr
eady be the case
2193      P:00069F P:00069F 0A0F01            BCLR    #1,X:<LATCH                       ; Separate updates of clock driver
2194      P:0006A0 P:0006A0 0A0F20            BSET    #CDAC,X:<LATCH                    ; Disable clearing of DACs
2195      P:0006A1 P:0006A1 0A0F22            BSET    #ENCK,X:<LATCH                    ; Enable clock and DAC output switches
2196      P:0006A2 P:0006A2 09F0B5            MOVEP             X:LATCH,Y:WRLATCH       ; Write it to the hardware
                            00000F
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 42



2197      P:0006A4 P:0006A4 0D040E            JSR     <PAL_DLY                          ; Delay for all this to happen
2198   
2199                                ; Read DAC values from a table, and write them to the DACs
2200      P:0006A5 P:0006A5 60F400            MOVE              #EPUR,R0                ; Get starting address of erase clock values
                            0001B2
2201      P:0006A7 P:0006A7 000000            NOP
2202      P:0006A8 P:0006A8 000000            NOP
2203      P:0006A9 P:0006A9 065840            DO      Y:(R0)+,E_DAC2                    ; Repeat Y:(R0)+ times
                            0006AD
2204      P:0006AB P:0006AB 5ED800            MOVE                          Y:(R0)+,A   ; Read the table entry
2205      P:0006AC P:0006AC 0D020C            JSR     <XMIT_A_WORD                      ; Transmit it to TIM-A-STD
2206      P:0006AD P:0006AD 000000            NOP
2207                                E_DAC2
2208   
2209                                ; Let the DAC voltages all ramp before exiting
2210      P:0006AE P:0006AE 44F400            MOVE              #400000,X0
                            061A80
2211      P:0006B0 P:0006B0 06C400            DO      X0,*+3                            ; 4 millisec delay
                            0006B2
2212      P:0006B2 P:0006B2 000000            NOP
2213   
2214                                ; Start the delay loop
2215   
2216      P:0006B3 P:0006B3 061440            DO      Y:<TIME1,EPUR_T1                  ; Delay TIME1 msec
                            0006B6
2217      P:0006B5 P:0006B5 0D067A            JSR     <LNG_DLY
2218      P:0006B6 P:0006B6 000000            NOP
2219   
2220      P:0006B7 P:0006B7 60F400  EPUR_T1   MOVE              #ERHI_END,R0            ; Get the original clock values back.
                            000195
2221      P:0006B9 P:0006B9 000000            NOP
2222      P:0006BA P:0006BA 000000            NOP
2223      P:0006BB P:0006BB 065840            DO      Y:(R0)+,END_DAC_RESTORE2          ; Repeat Y:(R0)+ times
                            0006BF
2224      P:0006BD P:0006BD 5ED800            MOVE                          Y:(R0)+,A   ; Read the table entry
2225      P:0006BE P:0006BE 0D020C            JSR     <XMIT_A_WORD                      ; Transmit it to TIM-A-STD
2226      P:0006BF P:0006BF 000000            NOP
2227                                END_DAC_RESTORE2
2228   
2229                                ; Let the DAC voltages all ramp up before exiting
2230      P:0006C0 P:0006C0 44F400            MOVE              #400000,X0
                            061A80
2231      P:0006C2 P:0006C2 06C400            DO      X0,*+3                            ; 4 millisec delay
                            0006C4
2232      P:0006C4 P:0006C4 000000            NOP
2233      P:0006C5 P:0006C5 0C0054            JMP     <START
2234   
2235                                ; change the idling direction.  CID
2236                                CHANGE_IDLE_DIRECTION
2237      P:0006C6 P:0006C6 46DB00            MOVE              X:(R3)+,Y0
2238      P:0006C7 P:0006C7 0D06C9            JSR     <CHG_IDL
2239      P:0006C8 P:0006C8 0C008F            JMP     <FINISH
2240   
2241      P:0006C9 P:0006C9 56F400  CHG_IDL   MOVE              #'__L',A                ; Shift right
                            5F5F4C
2242      P:0006CB P:0006CB 200055            CMP     Y0,A
2243      P:0006CC P:0006CC 0E26D3            JNE     <COMP_IR
2244   
2245      P:0006CD P:0006CD 44F400            MOVE              #SERIAL_IDLE_LEFT,X0
                            000073
2246      P:0006CF P:0006CF 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 43



2247      P:0006D1 P:0006D1 0A0005            BCLR    #SPLIT_S,X:STATUS
2248      P:0006D2 P:0006D2 00000C            RTS
2249   
2250      P:0006D3 P:0006D3 56F400  COMP_IR   MOVE              #'__R',A
                            5F5F52
2251      P:0006D5 P:0006D5 200055            CMP     Y0,A
2252      P:0006D6 P:0006D6 0E26DD            JNE     <COMP_IS
2253   
2254      P:0006D7 P:0006D7 44F400            MOVE              #SERIAL_IDLE_RIGHT,X0
                            000095
2255      P:0006D9 P:0006D9 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2256      P:0006DB P:0006DB 0A0005            BCLR    #SPLIT_S,X:STATUS
2257      P:0006DC P:0006DC 00000C            RTS
2258   
2259      P:0006DD P:0006DD 56F400  COMP_IS   MOVE              #'__S',A
                            5F5F53
2260      P:0006DF P:0006DF 200055            CMP     Y0,A
2261      P:0006E0 P:0006E0 0E208D            JNE     <ERROR
2262   
2263      P:0006E1 P:0006E1 44F400            MOVE              #SERIAL_IDLE_SPLIT,X0
                            0000B9
2264      P:0006E3 P:0006E3 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2265      P:0006E5 P:0006E5 0A0025            BSET    #SPLIT_S,X:STATUS
2266      P:0006E6 P:0006E6 00000C            RTS
2267   
2268                                CHANGE_NUMBER_PARALLEL_CLEARS
2269      P:0006E7 P:0006E7 46DB00            MOVE              X:(R3)+,Y0
2270      P:0006E8 P:0006E8 4E2500            MOVE                          Y0,Y:<N_PARALLEL_CLEARS
2271      P:0006E9 P:0006E9 0C008F            JMP     <FINISH
2272   
2273   
2274                                CHANGE_IDLE_READ_DIRECTION
2275      P:0006EA P:0006EA 46DB00            MOVE              X:(R3)+,Y0
2276      P:0006EB P:0006EB 0D06ED            JSR     <CHG_IDL_READ
2277      P:0006EC P:0006EC 0C008F            JMP     <FINISH
2278   
2279                                CHG_IDL_READ
2280      P:0006ED P:0006ED 56F400            MOVE              #'__L',A                ; Shift right
                            5F5F4C
2281      P:0006EF P:0006EF 200055            CMP     Y0,A
2282      P:0006F0 P:0006F0 0E26FB            JNE     <COMP_I_R_R
2283   
2284      P:0006F1 P:0006F1 44F400            MOVE              #SERIAL_IDLE_LEFT,X0
                            000073
2285      P:0006F3 P:0006F3 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2286      P:0006F5 P:0006F5 44F400            MOVE              #SERIAL_READ_LEFT,X0
                            000103
2287      P:0006F7 P:0006F7 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
2288      P:0006F9 P:0006F9 0A0005            BCLR    #SPLIT_S,X:STATUS
2289      P:0006FA P:0006FA 00000C            RTS
2290   
2291                                COMP_I_R_R
2292      P:0006FB P:0006FB 56F400            MOVE              #'__R',A
                            5F5F52
2293      P:0006FD P:0006FD 200055            CMP     Y0,A
2294      P:0006FE P:0006FE 0E26DD            JNE     <COMP_IS
2295   
2296      P:0006FF P:0006FF 44F400            MOVE              #SERIAL_IDLE_RIGHT,X0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  timCCDmisc.asm  Page 44



                            000095
2297      P:000701 P:000701 4C7000            MOVE                          X0,Y:SERIAL_IDLE
                            00001C
2298      P:000703 P:000703 44F400            MOVE              #SERIAL_READ_RIGHT,X0
                            00013B
2299      P:000705 P:000705 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
2300      P:000707 P:000707 0A0005            BCLR    #SPLIT_S,X:STATUS
2301      P:000708 P:000708 00000C            RTS
2302   
2303                                COMP_I_R_S
2304      P:000709 P:000709 56F400            MOVE              #'__S',A
                            5F5F53
2305      P:00070B P:00070B 200055            CMP     Y0,A
2306      P:00070C P:00070C 0E208D            JNE     <ERROR
2307   
2308      P:00070D P:00070D 44F400            MOVE              #SERIAL_READ_SPLIT,X0
                            000115
2309      P:00070F P:00070F 4C7000            MOVE                          X0,Y:SERIAL_READ
                            000016
2310      P:000711 P:000711 0A0025            BSET    #SPLIT_S,X:STATUS
2311      P:000712 P:000712 00000C            RTS
2312   
2313      P:000713 P:000713 47DB00  SBINN     MOVE              X:(R3)+,Y1              ; Get first parameter NSBIN
2314                                ;        MOVE    Y1,Y:<NSBIN             ; Move it to the proper loc in Y:mem (not implemented y
et)
2315      P:000714 P:000714 47DB00            MOVE              X:(R3)+,Y1              ; Get second parameter NPBIN
2316      P:000715 P:000715 4F0600            MOVE                          Y1,Y:<NPBIN ; Move it to Y: mem also.
2317      P:000716 P:000716 0C008F            JMP     <FINISH                           ; End, send back 'DON'
2318                                ;
2319   
2320   
2321   
2322                                 TIMBOOT_X_MEMORY
2323      000717                              EQU     @LCV(L)
2324   
2325                                ;  ****************  Setup memory tables in X: space ********************
2326   
2327                                ; Define the address in P: space where the table of constants begins
2328   
2329                                          IF      @SCP("HOST","HOST")
2330      X:000036 X:000036                   ORG     X:END_COMMAND_TABLE,X:END_COMMAND_TABLE
2331                                          ENDIF
2332   
2333                                          IF      @SCP("HOST","ROM")
2335                                          ENDIF
2336   
2337                                ; Application commands
2338      X:000036 X:000036                   DC      'PON',POWER_ON
2339      X:000038 X:000038                   DC      'POF',POWER_OFF
2340      X:00003A X:00003A                   DC      'SBV',SET_BIAS_VOLTAGES
2341      X:00003C X:00003C                   DC      'IDL',START_IDLE_CLOCKING
2342      X:00003E X:00003E                   DC      'OSH',OPEN_SHUTTER
2343      X:000040 X:000040                   DC      'CSH',CLOSE_SHUTTER
2344      X:000042 X:000042                   DC      'RDC',RDCCD                       ; Begin CCD readout
2345      X:000044 X:000044                   DC      'CLR',CLEAR                       ; Fast clear the CCD
2346   
2347                                ; Exposure and readout control routines
2348      X:000046 X:000046                   DC      'SET',SET_EXPOSURE_TIME
2349      X:000048 X:000048                   DC      'RET',READ_EXPOSURE_TIME
2350      X:00004A X:00004A                   DC      'SEX',START_EXPOSURE
2351      X:00004C X:00004C                   DC      'PEX',PAUSE_EXPOSURE
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 45



2352      X:00004E X:00004E                   DC      'REX',RESUME_EXPOSURE
2353      X:000050 X:000050                   DC      'AEX',ABORT_EXPOSURE
2354      X:000052 X:000052                   DC      'ABR',ABR_RDC
2355      X:000054 X:000054                   DC      'CRD',CONTINUE_READ
2356      X:000056 X:000056                   DC      'WSI',SYNTHETIC_IMAGE
2357      X:000058 X:000058                   DC      'STR',STR_RDC
2358   
2359                                ; Support routines
2360      X:00005A X:00005A                   DC      'SGN',ST_GAIN
2361      X:00005C X:00005C                   DC      'SBN',SET_BIAS_NUMBER
2362      X:00005E X:00005E                   DC      'SMX',SET_MUX
2363      X:000060 X:000060                   DC      'CSW',CLR_SWS
2364      X:000062 X:000062                   DC      'SOS',SELECT_OUTPUT_SOURCE
2365      X:000064 X:000064                   DC      'ROI',SET_ROI
2366      X:000066 X:000066                   DC      'GEO',SET_GEOMETRY
2367      X:000068 X:000068                   DC      'RCC',READ_CONTROLLER_CONFIGURATION
2368      X:00006A X:00006A                   DC      'RAW',RAW_COMMAND                 ; So I can set voltages as I please
2369      X:00006C X:00006C                   DC      'ERS',ERASE                       ;Persistent image erase
2370      X:00006E X:00006E                   DC      'EPG',EPURGE                      ; E-Purge procedure
2371      X:000070 X:000070                   DC      'TAD',TEST_AD
2372      X:000072 X:000072                   DC      'CID',CHANGE_IDLE_DIRECTION
2373      X:000074 X:000074                   DC      'CIR',CHANGE_IDLE_READ_DIRECTION
2374      X:000076 X:000076                   DC      'CPC',CHANGE_NUMBER_PARALLEL_CLEARS
2375      X:000078 X:000078                   DC      'NPS',NO_POLARITY_SHIFT
2376      X:00007A X:00007A                   DC      'BIN',SBINN                       ; Set binning command
2377   
2378                                 END_APPLICATION_COMMAND_TABLE
2379      00007C                              EQU     @LCV(L)
2380   
2381                                          IF      @SCP("HOST","HOST")
2382      00002A                    NUM_COM   EQU     (@LCV(R)-COM_TBL_R)/2             ; Number of boot +
2383                                                                                    ;  application commands
2384      000367                    EXPOSING  EQU     CHK_TIM                           ; Address if exposing
2385                                 CONTINUE_READING
2386      100000                              EQU     CONT_RD                           ; Address if reading out
2387                                          ENDIF
2388   
2389                                          IF      @SCP("HOST","ROM")
2391                                          ENDIF
2392   
2393                                ; Now let's go for the timing waveform tables
2394                                          IF      @SCP("HOST","HOST")
2395      Y:000000 Y:000000                   ORG     Y:0,Y:0
2396                                          ENDIF
2397   
2398      Y:000000 Y:000000         GAIN      DC      END_APPLICATON_Y_MEMORY-@LCV(L)-1
2399   
2400      Y:000001 Y:000001         NSR       DC      4128                              ; 1: Number Serial Read, prescan + image + b
ias
2401      Y:000002 Y:000002         NPR       DC      2040                              ; 2: Number Parallel Read
2402      Y:000003 Y:000003         NSCLR     DC      NS_CLR                            ; 3: To clear the serial register
2403      Y:000004 Y:000004         NPCLR     DC      NP_CLR                            ; 4: To clear the parallel register
2404      Y:000005 Y:000005         NSBIN     DC      1                                 ; 5: Serial binning parameter
2405      Y:000006 Y:000006         NPBIN     DC      1                                 ; 6: Parallel binning parameter
2406      Y:000007 Y:000007         TST_DAT   DC      0                                 ; 7: Temporary definition for test images
2407      Y:000008 Y:000008         SHDEL     DC      SH_DEL                            ; 8: Delay in milliseconds between shutter c
losing
2408                                                                                    ; and image readout
2409      Y:000009 Y:000009         CONFIG    DC      CC                                ; 9: Controller configuration
2410                                 NSERIALS_READ
2411      Y:00000A Y:00000A                   DC      4128                              ; 10: Number of serials to read
2412   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 46



2413                                ; Waveform parameters
2414                                ;OS             DC      'ALL'   ; 11: Output source
2415      Y:00000B Y:00000B         OS        DC      '_U1'                             ; 11: Output source
2416                                 FIRST_CLOCKS
2417      Y:00000C Y:00000C                   DC      0                                 ; 12: Address of first clocks waveforms
2418      Y:00000D Y:00000D         CLOCK_LINE DC     0                                 ; 13: Clock one complete line of charge
2419   
2420                                ; ROI parameters
2421      Y:00000E Y:00000E         NP_SKIP   DC      0                                 ; 14: Number of rows to skip
2422      Y:00000F Y:00000F         NS_SKP1   DC      0                                 ; 15: Number of serials to clear before read
2423      Y:000010 Y:000010         NS_SKP2   DC      0                                 ; 16: Number of serials to clear after read
2424      Y:000011 Y:000011         NS_PRE    DC      0                                 ; 17: number of prescan pixels to read
2425      Y:000012 Y:000012         NS_OVR    DC      0                                 ; 18: number of overscan pixels to read
2426      Y:000013 Y:000013         NS_RD     DC      0                                 ; 19: number of data pixels to read
2427   
2428                                ; Time parameters for Erase and E-Purge
2429      Y:000014 Y:000014         TIME1     DC      0                                 ; 20
2430      Y:000015 Y:000015         TIME2     DC      0                                 ; 21
2431   
2432                                ; Readout peculiarity parameters. Default values are selected here.
2433                                 SERIAL_READ
2434      Y:000016 Y:000016                   DC      SERIAL_READ_SPLIT                 ; 22  ; Serial readout waveforms
2435                                 SERIAL_SKIP
2436      Y:000017 Y:000017                   DC      SERIAL_SKIP_SPLIT                 ; 23  ; Serial skipping waveforms
2437      Y:000018 Y:000018         PARALLEL  DC      PARALLEL_2                        ; 24  ; Parallel shifting waveforms
2438                                 PARALLEL_CLEAR
2439      Y:000019 Y:000019                   DC      PARALLEL_CLEAR_2                  ; 25
2440      Y:00001A Y:00001A         PC_1      DC      PARALLEL_CLEAR_1                  ; 26
2441      Y:00001B Y:00001B         PC_2      DC      PARALLEL_CLEAR_2                  ; 27
2442                                 SERIAL_IDLE
2443      Y:00001C Y:00001C                   DC      SERIAL_IDLE_SPLIT                 ; 28
2444      Y:00001D Y:00001D         SI_L      DC      SERIAL_IDLE_LEFT                  ; 29
2445      Y:00001E Y:00001E         SI_R      DC      SERIAL_IDLE_RIGHT                 ; 30
2446      Y:00001F Y:00001F         BYTE_1    DC      0                                 ; 31
2447      Y:000020 Y:000020         BYTE_2    DC      0                                 ; 32
2448      Y:000021 Y:000021         SR_L      DC      SERIAL_READ_LEFT                  ; 33
2449      Y:000022 Y:000022         SR_R      DC      SERIAL_READ_RIGHT                 ; 34
2450      Y:000023 Y:000023         P_1       DC      PARALLEL_1                        ; 35
2451      Y:000024 Y:000024         P_2       DC      PARALLEL_2                        ; 36
2452                                 N_PARALLEL_CLEARS
2453      Y:000025 Y:000025                   DC      1                                 ; 37
2454      Y:000026 Y:000026         SR_S      DC      SERIAL_READ_SPLIT                 ; 38
2455      Y:000027 Y:000027         TMP_1     DC      TMP_PXL_TBL1                      ; 39 stuff where I can load a idle without r
esetting.
2456      Y:000028 Y:000028         TMP_2     DC      TMP_PXL_TBL2                      ; 40
2457      Y:000029 Y:000029         TMP_3     DC      TMP_PXL_TBL3                      ; 41
2458      Y:00002A Y:00002A         PC_S      DC      PARALLEL_CLEAR_SPLIT
2459      Y:00002B Y:00002B         NSRI      DC      4128                              ; 43 Total size of serial register
2460      Y:00002C Y:00002C         NSR_ACTIVE DC     4114                              ; 44 Size of active pixels
2461   
2462   
2463                                ; Include the waveform table for the designated type of CCD
2464                                          INCLUDE "LBNL_2Kx4K.waveforms"            ; Readout and clocking waveform file
2465                                ; vim: syntax=asm
2466   
2467                                ; Waveform tables and definitions for the LBNL CCD
2468                                ; This is for a slow, low noise readout
2469                                ; In this version the reset encloses the three serial clocks
2470                                ;**********************************************************************
2471   
2472                                ; U2 and L2 working.
2473                                ; Parallel clocking also needs fixing as shifting to register 1 has
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 47



2474                                ; charge under different phases during integration and clocking
2475   
2476                                ; Change history:
2477                                ; 2011-07-25 (GR): Original code from SWIFT.
2478                                ;                  VIDEO bits definitions were for ARC-47 (4-ch video board), with single polari
ty bit (SS3) and no DC Restore
2479                                ;                  VIDEO calls change for bits in ARC-45: SS2 (POL-) and SS3 (POL+), DC Restore 
bit is SS1.
2480                                ;                  Due to p-channel CCD, Inv and NonInv are reversed.
2481   
2482                                ; Miscellaneous definitions
2483   
2484      000000                    VIDEO     EQU     $000000                           ; Video processor board select for switching
2485      000000                    VID0      EQU     $000000                           ; Video processor board select for DAC and A
/D - Board 0
2486      100000                    VID1      EQU     $100000                           ; Video processor board select for DAC and A
/D - Board 1
2487      002000                    BIAS      EQU     $002000                           ; Bias Generator board select = 3
2488      003000                    HVBIAS    EQU     $003000                           ; Bias Generator board select = 3
2489      002000                    CLK2      EQU     $002000                           ; Clock driver board select = 2
2490      003000                    CLK3      EQU     $003000                           ; Clock driver board select = 3
2491                                ;CLK4      EQU $004000 ; Clock driver board select = 4
2492                                ;CLK5      EQU $005000 ; Clock driver board select = 5
2493   
2494                                 VIDEO_CONFIG
2495      0C000C                              EQU     $0C000C                           ; WARP = DAC_OUT = ON; H16B, Reset FIFOs
2496      0E0000                    DAC_ADDR  EQU     $0E0000                           ; DAC Channel Address
2497      0F4000                    DAC_RegM  EQU     $0F4000                           ; DAC m Register
2498      0F8000                    DAC_RegC  EQU     $0F8000                           ; DAC c Register
2499      0FC000                    DAC_RegD  EQU     $0FC000                           ; DAC X1 Register
2500   
2501      -5.000000E+000            VABG      EQU     -5.0                              ; Anti-blooming gate
2502      8.700000E+000             VRSV_MAX  EQU     8.70
2503      000D9A                    DAC_VRSV  EQU     @CVI(((VABG+VRSV_MAX)/VRSV_MAX)*8192-1) ; Bipolar
2504   
2505      000834                    NP_CLR    EQU     2100
2506                                ;NS_CLR  EQU     4200
2507      000400                    NS_CLR    EQU     1024                              ; To speed up readout
2508   
2509                                ; GenIII: if bit #23=1; 22-16 = # of 320 nanos cycles that bits #15-0 held at
2510                                ;         if bit #23=0; 22-16 = # of  40 nanos cycles that bits #15-0 held at
2511   
2512                                ;I_DELAY         EQU     $980000        ; 24*320 + 40 = 7720 ns
2513                                ;I_DELAY         EQU     $920000        ; 18*320 + 40 = 5800 ns
2514      4B0000                    I_DELAY   EQU     $4B0000                           ; 3 usec
2515                                ;I_DELAY         EQU     $190000        ; 1 usec
2516   
2517                                ; Delay numbers for clocking
2518      980000                    P_DELAY   EQU     $980000                           ; 24*320 = 7680 ns
2519                                ;P_DELAY         EQU     $180000                ; 1000 ns
2520                                ;30 KHz waveforms
2521                                ;S_DELAY         EQU     $310000 ; Serial register transfer delay (2000 ns)
2522      080000                    S_DELAY   EQU     $080000                           ; Serial register transfer delay (360 ns)
2523                                ;S_DELAY         EQU     $180000 ; Serial register transfer delay (1000 ns)
2524      180000                    SW_DELAY  EQU     $180000                           ; Sum_well  clock delay = 24*40+40 = 1000 ns
2525                                ;PRE_SET_DLY     EQU     $310000 ; 49*40 + 40 = 2000 ns
2526                                ;POST_SET_DLY    EQU     $310000 ; 49*40 + 40 = 2000 ns
2527                                 PRE_SET_DLY
2528      0C0000                              EQU     $C0000                            ; 12*40 + 40 = 520 ns
2529                                 POST_SET_DLY
2530      0C0000                              EQU     $C0000                            ; 12*40 + 40 = 520 ns
2531   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 48



2532                                 DCRST_DELAY
2533      210000                              EQU     $210000                           ; 33*40 + 40 = 1360 ns
2534   
2535                                 S_DELAY_SKIP
2536      030000                              EQU     $030000                           ; 160ns
2537                                 SW_DELAY_SKIP
2538      030000                              EQU     $030000                           ; 160ns
2539   
2540                                 I_DELAY_FAST
2541      060000                              EQU     $60000
2542                                 S_DELAY_FAST
2543      070000                              EQU     $70000
2544                                 SW_DELAY_FAST
2545      0C0000                              EQU     $c0000
2546                                 PRE_SET_DLY_FAST
2547      070000                              EQU     $70000
2548                                 POST_SET_DLY_FAST
2549      070000                              EQU     $70000
2550                                 DCRST_DELAY_FAST
2551      070000                              EQU     $70000
2552   
2553   
2554                                ; TODO: Is this a waste of clearing in split readout mode?  Calculate.
2555                                ;NP_CLR  EQU     2048  ; 2040 parallel direction
2556                                ;This doesn't do anything
2557                                ;NP_CLR  EQU     2100  ; 2040 parallel direction
2558                                ;NS_CLR  EQU     4200    ; 4128 in serial direction
2559                                ;NS_CLR  EQU     4128    ; 4128 in serial direction
2560      000064                    SH_DEL    EQU     100
2561   
2562                                ; CHANGE ABOVE TO GENIII TIMING BOARD DELAYS
2563   
2564                                ; Macros to help getting from volts to bits.
2565                                ; The \ in front of NAME substitutes the value of NAME into the variable. (tx)
2566   
2567                                VDEF      MACRO   NAME,BRDTYP,BRDNUM,DAC,ALO,AHI
2568 m 
2569 m                              LO_\NAME  EQU     ALO
2570 m                              HI_\NAME  EQU     AHI
2571 m                              DAC_\NAME EQU     DAC
2572 m                               BRDNUM_\NAME
2573 m                                        EQU     BRDNUM
2574 m                                        IF      @SCP("BRDTYP",'VID')
2575 m                               BRDTYP_\NAME
2576 m                                        EQU     3
2577 m                                        ELSE
2578 m                               BRDTYP_\NAME
2579 m                                        EQU     0
2580 m                                        ENDIF
2581 m 
2582 m                              ;        MSG     'Defining voltage ',"NAME",' type ',"BRDTYP",' board ',"BRDNUM",' dac ',"DAC",'
 with limits ',"ALO",' ',"AHI"
2583 m                                        ENDM
2584   
2585                                VOLTS     MACRO   NAME,F
2586 m 
2587 m                              DUMMY     SET     @CVI(@MIN(4095,@MAX(0,(F-LO_\NAME)/(HI_\NAME-LO_\NAME)*4096.)))
2588 m                              DUMMY2    SET     @CVI((BRDNUM_\NAME<<20)|(BRDTYP_\NAME<<18)|(DAC_\NAME<<14)|DUMMY)
2589 m                                        DC      DUMMY2
2590 m                                        MSG     'Setting voltage ',"NAME ","F",'V ',DUMMY,DUMMY2
2591 m                                        ENDM
2592   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 49



2593                                ;*********************************************************************
2594                                ;
2595                                ; ; define bias board voltage symbols
2596                                          VDEF    VRL1,VID,2,0,0.0,-25.0
2609                                          VDEF    VRU1,VID,2,1,0.0,-25.0
2622                                          VDEF    VRL2,VID,2,2,0.0,-25.0
2635                                          VDEF    VRU2,VID,2,3,0.0,-25.0
2648                                          VDEF    VDDL1,VID,2,4,0.0,-25.0
2661                                          VDEF    VDDU1,VID,2,5,0.0,-25.0
2674                                          VDEF    VDDL2,VID,2,6,0.0,-25.0
2687                                          VDEF    VDDU2,VID,2,7,0.0,-25.0
2700                                          VDEF    VOGL1,VID,2,8,0.0,5
2713                                          VDEF    VOGU1,VID,2,9,0.0,5
2726                                          VDEF    VOGL2,VID,2,10,0.0,5
2739                                          VDEF    VOGU2,VID,2,11,0.0,5
2752                                          VDEF    VSUB,VID,2,12,0.0,80.0
2765                                          VDEF    RAMP,VID,2,13,0.0,10.0            ;  for ramping p.s.
2778                                ;
2779                                ; ; define clock board symbols bank0
2780                                ;
2781   
2782   
2783                                ; Output video offset parameters
2784                                ; Voltage range is -10V to +10V
2785                                ; DAC range is $000 to $FFF
2786   
2787                                ; Setting of $500 produces bias level of ~7,500 ADUs in U1 (CCD @101K)
2788                                ; Setting of $600 produces bias level of ~5,150 ADUs in U1 (CCD @101K)
2789                                ; Setting of $800 produces bias level of ~250 ADUs in U1 (CCD @116K)
2790                                ; Setting of $780 produces bias level of ~1,460 ADUs in U1 (CCD @116K)
2791                                ; Setting of $7E5 produces bias level of ~505 ADUs in U1 (CCD @116K)
2792                                ; Setting of $7B0 produces bias level of ~1,000 ADUs in U1 (CCD @116K)
2793                                ; Gain is -9.45 signal counts per DAC counts
2794      0007B0                    OFFSET0   EQU     $7B0
2795      0007B0                    OFFSET1   EQU     $7B0
2796      0007B0                    OFFSET2   EQU     $7B0
2797      0007B0                    OFFSET3   EQU     $7B0
2798   
2799                                ; CCD clock voltage
2800      0.000000E+000             ZERO      EQU     0.0                               ; Unused pins
2801      1.300000E+001             Vmax      EQU     +13.0                             ; Clock driver board rails
2802   
2803                                ;LBNL DEFINITIONS
2804      5.000000E+000             V1_HI     EQU     5.0                               ; Vertical High
2805      -3.000000E+000            V1_LO     EQU     -3.0                              ; Vertical Low
2806      5.000000E+000             V2_HI     EQU     5.0                               ; Vertical High
2807      -3.000000E+000            V2_LO     EQU     -3.0                              ; Vertical Low
2808      5.000000E+000             V3_HI     EQU     5.0                               ; Vertical High
2809      -3.000000E+000            V3_LO     EQU     -3.0                              ; Vertical Low
2810   
2811      5.000000E+000             FS1_HI    EQU     5.0                               ; Vertical High
2812      -3.000000E+000            FS1_LO    EQU     -3.0                              ; Vertical Low
2813      5.000000E+000             FS2_HI    EQU     5.0                               ; Vertical High
2814      -3.000000E+000            FS2_LO    EQU     -3.0                              ; Vertical Low
2815      5.000000E+000             FS3_HI    EQU     5.0                               ; Vertical High
2816      -3.000000E+000            FS3_LO    EQU     -3.0                              ; Vertical Low
2817   
2818      5.000000E+000             T2_HI     EQU     5.0                               ; Transfer gate High
2819      -3.000000E+000            T2_LO     EQU     -3.0                              ; Transfer gate Low
2820   
2821      5.000000E+000             T1_HI     EQU     5.0                               ; Transfer gate High
2822      -3.000000E+000            T1_LO     EQU     -3.0                              ; Transfer gate Low
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 50



2823   
2824      6.000000E+000             H1U1_L2_HI EQU    +6.0                              ; Horizontal High
2825      -3.900000E+000            H1U1_L2_LO EQU    -3.9                              ; Horizontal Low
2826      6.000000E+000             H2U1_L2_HI EQU    +6.0                              ; Horizontal High
2827      -3.900000E+000            H2U1_L2_LO EQU    -3.9                              ; Horizontal Low
2828      6.000000E+000             H3U1_L2_HI EQU    +6.0                              ; Horizontal High
2829      -3.900000E+000            H3U1_L2_LO EQU    -3.9                              ; Horizontal Low
2830      6.000000E+000             H1U2_L1_HI EQU    +6.0                              ; Horizontal High
2831      -3.900000E+000            H1U2_L1_LO EQU    -3.9                              ; Horizontal Low
2832      6.000000E+000             H2U2_L1_HI EQU    +6.0                              ; Horizontal High
2833      -3.900000E+000            H2U2_L1_LO EQU    -3.9                              ; Horizontal Low
2834      6.000000E+000             H3U2_L1_HI EQU    +6.0                              ; Horizontal High
2835      -3.900000E+000            H3U2_L1_LO EQU    -3.9                              ; Horizontal Low
2836   
2837                                ;H1U1_L2_HI  EQU +0.0 ; Horizontal High
2838                                ;H1U1_L2_LO  EQU -0.0 ; Horizontal Low
2839                                ;H2U1_L2_HI  EQU +0.0 ; HoVR2rizontal High
2840                                ;H2U1_L2_LO  EQU -0.0 ; Horizontal Low
2841                                ;H3U1_L2_HI  EQU +0.0 ; Horizontal High
2842                                ;H3U1_L2_LO  EQU -0.0 ; Horizontal Low
2843                                ;H1U2_L1_HI  EQU +0.0 ; Horizontal High
2844                                ;H1U2_L1_LO  EQU -0.0 ; Horizontal Low
2845                                ;H2U2_L1_HI  EQU +0.0 ; Horizontal High
2846                                ;H2U2_L1_LO  EQU -0.0 ; Horizontal Low
2847                                ;H3U2_L1_HI  EQU +0.0 ; Horizontal High
2848                                ;H3U2_L1_LO  EQU -0.0 ; Horizontal Low
2849                                ;
2850                                ;Put summing wells low for conduction channel
2851      5.000000E+000             SWU_HI    EQU     +5.0                              ; Summing Well High
2852      -5.000000E+000            SWU_LO    EQU     -5.0                              ; Summing Well Low
2853      5.000000E+000             SWL_HI    EQU     +5.0                              ; Summing Well High
2854      -5.000000E+000            SWL_LO    EQU     -5.0                              ; Summing Well Low
2855   
2856      -6.000000E+000            RU_HI     EQU     -6.0                              ; Reset ACTIVE wrong polarity....
2857      -1.000000E-001            RU_LO     EQU     -0.1                              ; Reset INACTIVE
2858      -6.000000E+000            RL_HI     EQU     -6.0                              ; Reset ACTIVE wrong polarity....
2859      -1.000000E-001            RL_LO     EQU     -0.1                              ; Reset INACTIVE
2860   
2861   
2862                                ; Bit defintions for bottom half of clock driver board, CLK2
2863                                ; Clock FS ando vertical regions together
2864                                ; 1,2,4,8,10,20,40,80,100,200,400,800
2865   
2866      000009                    V1_1H     EQU     1|8                               ; V1: Pin 1 -- FS1: pin 4
2867      000000                    V1_1L     EQU     0
2868   
2869      000012                    V2_1H     EQU     2|$10                             ; V2: Pin 2 -- FS2: pin 5
2870      000000                    V2_1L     EQU     0
2871   
2872      000024                    V3_1H     EQU     4|$20                             ; V3: pin 3 -- FS3: pin 6
2873      000000                    V3_1L     EQU     0
2874   
2875      000240                    V1_2H     EQU     $40|$200                          ; V1: pin 7 -- FS1: pin 10
2876      000000                    V1_2L     EQU     0
2877   
2878      000480                    V2_2H     EQU     $80|$400                          ; V2: pin 8 -- FS2: pin 11
2879      000000                    V2_2L     EQU     0
2880   
2881      000900                    V3_2H     EQU     $100|$800                         ; V3: pin 9 -- FS3: pin 12
2882      000000                    V3_2L     EQU     0
2883   
2884   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 51



2885                                ; Top bank
2886   
2887      000001                    H1_1H     EQU     1                                 ; Horizontal 1 Upper Pin 13
2888      000000                    H1_1L     EQU     0
2889   
2890      000002                    H1_2H     EQU     2                                 ; Horizontal 2 Upper, Pin 14
2891      000000                    H1_2L     EQU     0
2892   
2893      000004                    H1_3H     EQU     4                                 ; Horizontal 3 Upper, Pin 15
2894      000000                    H1_3L     EQU     0
2895   
2896      000008                    H2_1H     EQU     8                                 ; Horizontal 1 Lower, Pin 16
2897      000000                    H2_1L     EQU     0
2898   
2899      000010                    H2_2H     EQU     $10                               ; Horizontal 2 Lower, Pin 17
2900      000000                    H2_2L     EQU     0
2901   
2902      000020                    H2_3H     EQU     $20                               ; Horizontal 3 Lower, Pin 18
2903      000000                    H2_3L     EQU     0
2904   
2905      000040                    SWLH      EQU     $40                               ; Summing Well Upper, Pin 19
2906      000000                    SWLL      EQU     0
2907   
2908      000080                    SWUH      EQU     $80                               ; Summing Well Lower, Pin 33
2909      000000                    SWUL      EQU     0
2910   
2911      000100                    RLH       EQU     $100                              ; Reset Gate Upper, Pin 34
2912      000000                    RLL       EQU     0
2913   
2914      000200                    RUH       EQU     $200                              ; Reset Gate Lower, Pin 35
2915      000000                    RUL       EQU     0
2916   
2917      000400                    T1        EQU     $400                              ; Transfer Gate Upper, Pin 36
2918      000800                    T2        EQU     $800                              ; Transfer Gate Lower, Pin 37
2919   
2920                                ;Both summing wells;
2921                                ;SWUH+SWLH = $80+$100
2922                                ;This was wrong -- should be $40+$80.
2923      000000                    WL        EQU     $00
2924      0000C0                    WH        EQU     $c0
2925   
2926                                ; Both Reset Gates together
2927      000000                    RL        EQU     $000
2928      000300                    RH        EQU     $300
2929   
2930                                ;both transfer gates together
2931      000000                    TL        EQU     $000
2932      000C00                    TH        EQU     $c00
2933   
2934                                ; frame store definitions for clearing out FS region separately.
2935      000008                    FS1_1H    EQU     8
2936      000000                    FS1_1L    EQU     0
2937      000010                    FS2_1H    EQU     $10
2938      000000                    FS2_1L    EQU     0
2939      000020                    FS3_1H    EQU     $20
2940      000000                    FS3_1L    EQU     0
2941      000200                    FS1_2H    EQU     $200
2942      000000                    FS1_2L    EQU     0
2943      000400                    FS2_2H    EQU     $400
2944      000000                    FS2_2L    EQU     0
2945      000800                    FS3_2H    EQU     $800
2946      000000                    FS3_2L    EQU     0
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 52



2947                                ; LBNL waveforms
2948   
2949                                ;TODO:  Need to remove references to frame store clocks -- I've bussed
2950                                ;       those together with the verticals in the 1&2 halves.  Fortunately
2951                                ;       the switch state bits select the right thing at the moment...
2952                                ;TODO:  Check first CLK3 line for parallel split is correct.
2953   
2954      Y:00002D Y:00002D         CLEAR_FS  DC      END_CLEAR_FS-1
2955      Y:00002E Y:00002E                   DC      CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
2956      Y:00002F Y:00002F                   DC      CLK2|P_DELAY|FS1_1H|FS2_1L|FS3_1H|FS1_2H|FS2_2L|FS3_2H
2957      Y:000030 Y:000030                   DC      CLK2|P_DELAY|FS1_1L|FS2_1L|FS3_1H|FS1_2H|FS2_2L|FS3_2L
2958      Y:000031 Y:000031                   DC      CLK2|P_DELAY|FS1_1L|FS2_1H|FS3_1H|FS1_2H|FS2_2H|FS3_2L
2959      Y:000032 Y:000032                   DC      CLK2|P_DELAY|FS1_1L|FS2_1H|FS3_1L|FS1_2L|FS2_2H|FS3_2L
2960      Y:000033 Y:000033                   DC      CLK2|P_DELAY|FS1_1H|FS2_1H|FS3_1L|FS1_2L|FS2_2H|FS3_2H
2961      Y:000034 Y:000034                   DC      CLK2|$000000|FS1_1H|FS2_1L|FS3_1L|FS1_2L|FS2_2L|FS3_2H
2962   
2963      Y:000035 Y:000035                   DC      CLK3|P_DELAY|RL|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH
2964                                                                                    ; wfk -  Next line: Add DCRestore,StrtRstInt
 to machine state at end of CLEAR to stabilize baseline (04/04/07)
2965      Y:000036 Y:000036                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
2966                                END_CLEAR_FS
2967   
2968   
2969                                 PARALLEL_SPLIT
2970      Y:000037 Y:000037                   DC      END_PARALLEL_SPLIT-PARALLEL_SPLIT-1
2971      Y:000038 Y:000038                   DC      CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
2972      Y:000039 Y:000039                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
2973      Y:00003A Y:00003A                   DC      CLK2|P_DELAY|V1_1L|V2_1L|V3_1H|V1_2H|V2_2L|V3_2L
2974      Y:00003B Y:00003B                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2H|V2_2H|V3_2L
2975      Y:00003C Y:00003C                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
2976      Y:00003D Y:00003D                   DC      CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2L|V2_2H|V3_2H
2977      Y:00003E Y:00003E                   DC      CLK2|$000000|V1_1H|V2_1L|V3_1L|V1_2L|V2_2L|V3_2H
2978      Y:00003F Y:00003F                   DC      CLK3|P_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Shut the transfer g
ate
2979      Y:000040 Y:000040                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
2980   
2981                                END_PARALLEL_SPLIT
2982   
2983   
2984                                ;Shift towards register 2
2985      Y:000041 Y:000041         PARALLEL_2 DC     END_PARALLEL_2-PARALLEL_2-1
2986      Y:000042 Y:000042                   DC      CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
2987      Y:000043 Y:000043                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
2988      Y:000044 Y:000044                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1L|V1_2H|V2_2L|V3_2L
2989      Y:000045 Y:000045                   DC      CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2H|V2_2H|V3_2L
2990      Y:000046 Y:000046                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
2991      Y:000047 Y:000047                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2L|V2_2H|V3_2H
2992      Y:000048 Y:000048                   DC      CLK2|$000000|V1_1L|V2_1L|V3_1H|V1_2L|V2_2L|V3_2H ; shut TG
2993      Y:000049 Y:000049                   DC      CLK3|P_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Shut the transfer g
ate
2994      Y:00004A Y:00004A                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
2995                                END_PARALLEL_2
2996   
2997                                ;Shift towards register 1
2998                                ;charge stored under 2&3.  Issue with switching between which register to go to.
2999      Y:00004B Y:00004B         PARALLEL_1 DC     END_PARALLEL_1-PARALLEL_1-1
3000                                ;  DC  CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3001      Y:00004C Y:00004C                   DC      CLK3|$000000|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3002      Y:00004D Y:00004D                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 53



3003      Y:00004E Y:00004E                   DC      CLK2|P_DELAY|V1_1L|V2_1L|V3_1H|V1_2L|V2_2L|V3_2H
3004      Y:00004F Y:00004F                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2L|V2_2H|V3_2H
3005      Y:000050 Y:000050                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
3006      Y:000051 Y:000051                   DC      CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2H|V2_2H|V3_2L
3007      Y:000052 Y:000052                   DC      CLK2|$000000|V1_1H|V2_1L|V3_1L|V1_2H|V2_2L|V3_2L
3008      Y:000053 Y:000053                   DC      CLK3|P_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Shut the transfer g
ate
3009      Y:000054 Y:000054                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
3010                                END_PARALLEL_1
3011   
3012   
3013                                PARALLEL_CLEAR_1
3014      Y:000055 Y:000055                   DC      END_PARALLEL_CLEAR_1-PARALLEL_CLEAR_1-1
3015      Y:000056 Y:000056                   DC      CLK3|$000000|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3016   
3017      Y:000057 Y:000057                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
3018      Y:000058 Y:000058                   DC      CLK2|P_DELAY|V1_1L|V2_1L|V3_1H|V1_2L|V2_2L|V3_2H
3019      Y:000059 Y:000059                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2L|V2_2H|V3_2H
3020      Y:00005A Y:00005A                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
3021      Y:00005B Y:00005B                   DC      CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2H|V2_2H|V3_2L
3022      Y:00005C Y:00005C                   DC      CLK2|$000000|V1_1H|V2_1L|V3_1L|V1_2H|V2_2L|V3_2L
3023   
3024      Y:00005D Y:00005D                   DC      CLK3|P_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Shut the transfer g
ate
3025                                                                                    ; wfk -  Next line: Add DCRestore,StrtRstInt
 to machine state at end of CLEAR to stabilize baseline (04/04/07)
3026      Y:00005E Y:00005E                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
3027                                END_PARALLEL_CLEAR_1
3028   
3029                                PARALLEL_CLEAR_2
3030      Y:00005F Y:00005F                   DC      END_PARALLEL_CLEAR_2-PARALLEL_CLEAR_2-1
3031      Y:000060 Y:000060                   DC      CLK3|$000000|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3032   
3033      Y:000061 Y:000061                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
3034      Y:000062 Y:000062                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1L|V1_2H|V2_2L|V3_2L
3035      Y:000063 Y:000063                   DC      CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2H|V2_2H|V3_2L
3036      Y:000064 Y:000064                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
3037      Y:000065 Y:000065                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2L|V2_2H|V3_2H
3038      Y:000066 Y:000066                   DC      CLK2|$000000|V1_1L|V2_1L|V3_1H|V1_2L|V2_2L|V3_2H
3039      Y:000067 Y:000067                   DC      CLK3|P_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Shut the transfer g
ate
3040   
3041   
3042                                                                                    ; wfk -  Next line: Add DCRestore,StrtRstInt
 to machine state at end of CLEAR to stabilize baseline (04/04/07)
3043      Y:000068 Y:000068                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
3044                                END_PARALLEL_CLEAR_2
3045   
3046                                ; this parallel split mixes two central rows on the CCD.
3047                                PARALLEL_CLEAR_SPLIT
3048      Y:000069 Y:000069                   DC      END_PARALLEL_CLEAR_SPLIT-PARALLEL_CLEAR_SPLIT-1
3049                                ;  DC  CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3050      Y:00006A Y:00006A                   DC      CLK3|$000000|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3051   
3052      Y:00006B Y:00006B                   DC      CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
3053      Y:00006C Y:00006C                   DC      CLK2|P_DELAY|V1_1L|V2_1L|V3_1H|V1_2H|V2_2L|V3_2L
3054      Y:00006D Y:00006D                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2H|V2_2H|V3_2L
3055      Y:00006E Y:00006E                   DC      CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
3056      Y:00006F Y:00006F                   DC      CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2L|V2_2H|V3_2H
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 54



3057      Y:000070 Y:000070                   DC      CLK2|$000000|V1_1H|V2_1L|V3_1L|V1_2L|V2_2L|V3_2H
3058   
3059      Y:000071 Y:000071                   DC      CLK3|P_DELAY|RL|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH
3060                                                                                    ; wfk -  Next line: Add DCRestore,StrtRstInt
 to machine state at end of CLEAR to stabilize baseline (04/04/07)
3061      Y:000072 Y:000072                   DC      VIDEO+$000000+%0011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wf
k add DCRestore,StrtRstInt
3062                                END_PARALLEL_CLEAR_SPLIT
3063   
3064   
3065   
3067                                ;PARALLEL_CLEAR_SPLIT
3068                                ;  DC  END_PARALLEL_CLEAR_SPLIT-PARALLEL_CLEAR_SPLIT-1
3069                                ;  DC  CLK3|$000000|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TL ;SW->lo
3070                                ;
3071                                ;  DC  CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2H|V2_2L|V3_2H
3072                                ;  DC  CLK2|P_DELAY|V1_1L|V2_1L|V3_1H|V1_2H|V2_2L|V3_2L
3073                                ;  DC  CLK2|P_DELAY|V1_1L|V2_1H|V3_1H|V1_2H|V2_2H|V3_2L
3074                                ;  DC  CLK2|P_DELAY|V1_1L|V2_1H|V3_1L|V1_2L|V2_2H|V3_2L
3075                                ;  DC  CLK2|P_DELAY|V1_1H|V2_1H|V3_1L|V1_2L|V2_2H|V3_2H
3076                                ;  DC  CLK2|P_DELAY|V1_1H|V2_1L|V3_1L|V1_2L|V2_2L|V3_2H
3077                                ;  DC  CLK2|P_DELAY|V1_1H|V2_1L|V3_1H|V1_2L|V2_2L|V3_2H
3078                                ;  DC  CLK2|$000000|V1_1L|V2_1L|V3_1H|V1_2L|V2_2L|V3_2H
3079                                ;
3080                                ;  DC  CLK3|P_DELAY|RL|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH
3081                                ;  ; wfk -  Next line: Add DCRestore,StrtRstInt to machine state at end of CLEAR to stabilize ba
seline (04/04/07)
3082                                ;  DC  VIDEO+$000000+%1110000  ; ADCLatch,NonInv,DCRestore,StrtRstInt. - wfk add DCRestore,StrtR
stInt
3083                                ;END_PARALLEL_CLEAR_SPLIT
3084   
3085                                ; ARC47:  |xfer|A/D|integ|polarity|not used|not used|rst| (1 => switch open)
3086                                SERIAL_IDLE_LEFT                                    ; Clock serial charge from both L and R ends
3087      Y:000073 Y:000073                   DC      END_SERIAL_IDLE_LEFT-SERIAL_IDLE_LEFT-1
3088      Y:000074 Y:000074                   DC      VIDEO+$000000+%1111000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3089                                ; L2 idle version
3090                                ; 2->3->1->2->3
3091      Y:000075 Y:000075                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3092      Y:000076 Y:000076                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3093      Y:000077 Y:000077                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3094      Y:000078 Y:000078                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3095      Y:000079 Y:000079                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3096      Y:00007A Y:00007A                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3097      Y:00007B Y:00007B                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3098   
3099   
3100      Y:00007C Y:00007C                   DC      CLK3|$0000000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;dummy for trans
mit delay
3101      Y:00007D Y:00007D                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3102      Y:00007E Y:00007E                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3103      Y:00007F Y:00007F                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3104      Y:000080 Y:000080                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3105      Y:000081 Y:000081                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3106      Y:000082 Y:000082                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3107      Y:000083 Y:000083                   DC      VIDEO+DCRST_DELAY+%0010111        ;  Now sit around for at least 520ns while t
he conversion happens
3108                                END_SERIAL_IDLE_LEFT
3109   
3110   
3111                                ; ARC47:  |xfer|A/D|integ|polarity|not used|not used|rst| (1 => switch open)
3112                                SERIAL_IDLE_LEFT_NO_POL                             ; Clock serial charge from both L and R ends
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 55



3113      Y:000084 Y:000084                   DC      END_SERIAL_IDLE_LEFT_NO_POL-SERIAL_IDLE_LEFT_NO_POL-1
3114      Y:000085 Y:000085                   DC      VIDEO+$000000+%1111000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3115   
3116                                ; L2 idle version
3117                                ; 2->3->1->2->3
3118      Y:000086 Y:000086                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3119      Y:000087 Y:000087                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2L|H1_3L|H2_1H|H2_2L|H2_3L|WL|TH ;h2->hi
3120      Y:000088 Y:000088                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2H|H1_3L|H2_1H|H2_2H|H2_3L|WL|TH ;h1->lo
3121      Y:000089 Y:000089                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3L|H2_1L|H2_2H|H2_3L|WL|TH ;h3->hi
3122      Y:00008A Y:00008A                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3H|H2_1L|H2_2H|H2_3H|WL|TH ;h2->lo
3123      Y:00008B Y:00008B                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2L|H1_3H|H2_1L|H2_2L|H2_3H|WL|TH ;h1->hi
3124   
3125      Y:00008C Y:00008C                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3126   
3127   
3128      Y:00008D Y:00008D                   DC      CLK3|$0000000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;dummy for trans
mit delay
3129      Y:00008E Y:00008E                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3130      Y:00008F Y:00008F                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3131      Y:000090 Y:000090                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3132      Y:000091 Y:000091                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi -- charge d
ump
3133   
3134                                                                                    ;SW going low here suggests that no charge w
ill leak over OG barrier onto sense node.
3135      Y:000092 Y:000092                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3136      Y:000093 Y:000093                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3137      Y:000094 Y:000094                   DC      VIDEO+DCRST_DELAY+%0011011        ; ,NonInv  ;mF to do ADC sampling before res
etting
3138   
3139                                ;  DC  CLK3|POST_SET_DLY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL ;SW->lo
3140                                END_SERIAL_IDLE_LEFT_NO_POL
3141   
3142                                SERIAL_IDLE_RIGHT                                   ; Clock serial charge from both L and R ends
3143      Y:000095 Y:000095                   DC      END_SERIAL_IDLE_RIGHT-SERIAL_IDLE_RIGHT-1
3144      Y:000096 Y:000096                   DC      VIDEO+$000000+%1011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3145   
3146                                ; L2 read version
3147                                ; 2->3->1->2->3
3148      Y:000097 Y:000097                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h2->lo,SW->lo,Reset
_On
3149      Y:000098 Y:000098                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3150      Y:000099 Y:000099                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->hi
3151      Y:00009A Y:00009A                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->l
3152      Y:00009B Y:00009B                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->hi
3153      Y:00009C Y:00009C                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->lo
3154      Y:00009D Y:00009D                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->hi, Reset_O
ff|Delay
3155   
3156   
3157      Y:00009E Y:00009E                   DC      CLK3|$0000000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;dummy for trans
mit delay
3158      Y:00009F Y:00009F                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3159      Y:0000A0 Y:0000A0                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3160      Y:0000A1 Y:0000A1                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3161      Y:0000A2 Y:0000A2                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi -- charge d
ump
3162   
3163                                                                                    ;SW going low here suggests that no charge w
ill leak over OG barrier onto sense node.
3164      Y:0000A3 Y:0000A3                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 56



3165      Y:0000A4 Y:0000A4                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3166      Y:0000A5 Y:0000A5                   DC      VIDEO+$000000+%0110111            ; StopIntegrator
3167      Y:0000A6 Y:0000A6                   DC      VIDEO+DCRST_DELAY+%0110111        ; ADCLatch,NonInv  ;mF to do ADC sampling be
fore resetting
3168   
3169                                ;  DC  CLK3|POST_SET_DLY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL ;SW->lo
3170   
3171                                END_SERIAL_IDLE_RIGHT
3172   
3173   
3174                                SERIAL_IDLE_RIGHT_NO_POL                            ; Clock serial charge from both L and R ends
3175      Y:0000A7 Y:0000A7                   DC      END_SERIAL_IDLE_RIGHT_NO_POL-SERIAL_IDLE_RIGHT_NO_POL-1
3176      Y:0000A8 Y:0000A8                   DC      VIDEO+$000000+%1011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3177   
3178                                ; L2 read version
3179                                ; 2->3->1->2->3
3180      Y:0000A9 Y:0000A9                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h2->lo,SW->lo,Reset
_On
3181      Y:0000AA Y:0000AA                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3182      Y:0000AB Y:0000AB                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->hi
3183      Y:0000AC Y:0000AC                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->l
3184      Y:0000AD Y:0000AD                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->hi
3185      Y:0000AE Y:0000AE                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->lo
3186      Y:0000AF Y:0000AF                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->hi, Reset_O
ff|Delay
3187   
3188   
3189      Y:0000B0 Y:0000B0                   DC      CLK3|$0000000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;dummy for trans
mit delay
3190      Y:0000B1 Y:0000B1                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3191      Y:0000B2 Y:0000B2                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3192      Y:0000B3 Y:0000B3                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3193      Y:0000B4 Y:0000B4                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi -- charge d
ump
3194   
3195                                                                                    ;SW going low here suggests that no charge w
ill leak over OG barrier onto sense node.
3196      Y:0000B5 Y:0000B5                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3197      Y:0000B6 Y:0000B6                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3198      Y:0000B7 Y:0000B7                   DC      VIDEO+$000000+%0010111            ; StopIntegrator
3199      Y:0000B8 Y:0000B8                   DC      VIDEO+DCRST_DELAY+%0111011        ; ADCLatch,NonInv  ;mF to do ADC sampling be
fore resetting
3200   
3201                                ;  DC  CLK3|POST_SET_DLY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL ;SW->lo
3202   
3203                                END_SERIAL_IDLE_RIGHT_NO_POL
3204   
3205                                SERIAL_IDLE_SPLIT
3206      Y:0000B9 Y:0000B9                   DC      END_SERIAL_IDLE_SPLIT-SERIAL_IDLE_SPLIT-1
3207      Y:0000BA Y:0000BA                   DC      VIDEO+$000000+%1011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3208   
3209                                ; split read version:
3210      Y:0000BB Y:0000BB                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3211   
3212                                ;  DC  CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3213                                ;  DC  CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3214                                ;  DC  CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3215                                ;  DC  CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3216                                ;  DC  CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3217   
3218      Y:0000BC Y:0000BC                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2L|H1_3H|H2_1H|H2_2L|H2_3L|WL|TH ;h2->hi
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 57



3219      Y:0000BD Y:0000BD                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3H|H2_1H|H2_2H|H2_3L|WL|TH ;h1->lo
3220      Y:0000BE Y:0000BE                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3L|H2_1L|H2_2H|H2_3L|WL|TH ;h3->hi
3221      Y:0000BF Y:0000BF                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2H|H1_3L|H2_1L|H2_2H|H2_3H|WL|TH ;h2->lo
3222      Y:0000C0 Y:0000C0                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2L|H1_3L|H2_1L|H2_2L|H2_3H|WL|TH ;h1->hi
3223   
3224   
3225      Y:0000C1 Y:0000C1                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3226   
3227      Y:0000C2 Y:0000C2                   DC      CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;dummy for transmit 
delay
3228      Y:0000C3 Y:0000C3                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3229      Y:0000C4 Y:0000C4                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3230      Y:0000C5 Y:0000C5                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3231      Y:0000C6 Y:0000C6                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3232      Y:0000C7 Y:0000C7                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3233      Y:0000C8 Y:0000C8                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3234      Y:0000C9 Y:0000C9                   DC      VIDEO+$000000+%0010111            ; StopIntegrator
3235      Y:0000CA Y:0000CA                   DC      VIDEO+DCRST_DELAY+%0110111        ; ADCLatch,NonInv  ;mF to do ADC sampling be
fore resetting
3236                                END_SERIAL_IDLE_SPLIT
3237   
3238                                ; ARC47:  |xfer|A/D|integ|polarity|not used|not used|rst| (1 => switch open)
3239                                SERIAL_IDLE_SPLIT_NO_POL
3240      Y:0000CB Y:0000CB                   DC      END_SERIAL_IDLE_SPLIT_NO_POL-SERIAL_IDLE_SPLIT_NO_POL-1
3241      Y:0000CC Y:0000CC                   DC      VIDEO+$000000+%1011001            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3242   
3243                                ; split read version:
3244      Y:0000CD Y:0000CD                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3245      Y:0000CE Y:0000CE                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3246      Y:0000CF Y:0000CF                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3247      Y:0000D0 Y:0000D0                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3248      Y:0000D1 Y:0000D1                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3249      Y:0000D2 Y:0000D2                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3250      Y:0000D3 Y:0000D3                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3251   
3252      Y:0000D4 Y:0000D4                   DC      CLK3|$000000|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;dummy for transmit 
delay
3253      Y:0000D5 Y:0000D5                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3254      Y:0000D6 Y:0000D6                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3255      Y:0000D7 Y:0000D7                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3256      Y:0000D8 Y:0000D8                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3257      Y:0000D9 Y:0000D9                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3258      Y:0000DA Y:0000DA                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3259      Y:0000DB Y:0000DB                   DC      VIDEO+$000000+%0010111            ; StopIntegrator
3260      Y:0000DC Y:0000DC                   DC      VIDEO+DCRST_DELAY+%0111011        ; ADCLatch,NonInv  ;mF to do ADC sampling be
fore resetting
3261                                END_SERIAL_IDLE_SPLIT_NO_POL
3262   
3263   
3264                                ;start binning waveforms
3265                                CCD_RESET                                           ;Used for binning only
3266      Y:0000DD Y:0000DD                   DC      END_CCD_RESET-CCD_RESET-1
3267      Y:0000DE Y:0000DE                   DC      VIDEO+$000000+%1011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3268                                END_CCD_RESET
3269   
3270                                SERIAL_CLOCK_L                                      ;"NORMAL" clocking
3271      Y:0000DF Y:0000DF                   DC      END_SERIAL_CLOCK_L-SERIAL_CLOCK_L-1
3272      Y:0000E0 Y:0000E0                   DC      VIDEO+$000000+%1011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3273      Y:0000E1 Y:0000E1                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3274      Y:0000E2 Y:0000E2                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 58



3275      Y:0000E3 Y:0000E3                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3276      Y:0000E4 Y:0000E4                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3277      Y:0000E5 Y:0000E5                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3278      Y:0000E6 Y:0000E6                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3279      Y:0000E7 Y:0000E7                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3280                                END_SERIAL_CLOCK_L
3281   
3282                                SERIAL_CLOCK_R                                      ;"REVERSE" clocking
3283      Y:0000E8 Y:0000E8                   DC      END_SERIAL_CLOCK_R-SERIAL_CLOCK_R-1
3284      Y:0000E9 Y:0000E9                   DC      VIDEO+$000000+%1011000            ; NonInv,DCRestore,StrtRstInt.
3285      Y:0000EA Y:0000EA                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3286      Y:0000EB Y:0000EB                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h2->hi
3287      Y:0000EC Y:0000EC                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h1->lo
3288      Y:0000ED Y:0000ED                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3289      Y:0000EE Y:0000EE                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h2->lo
3290      Y:0000EF Y:0000EF                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h1->hi
3291      Y:0000F0 Y:0000F0                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3292                                END_SERIAL_CLOCK_R
3293   
3294                                SERIAL_CLOCK_SPLIT                                  ;"SPLIT" clocking
3295      Y:0000F1 Y:0000F1                   DC      END_SERIAL_CLOCK_SPLIT-SERIAL_CLOCK_SPLIT-1
3296      Y:0000F2 Y:0000F2                   DC      VIDEO+$000000+%1011000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3297      Y:0000F3 Y:0000F3                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3298      Y:0000F4 Y:0000F4                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3299      Y:0000F5 Y:0000F5                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3300      Y:0000F6 Y:0000F6                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3301      Y:0000F7 Y:0000F7                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3302      Y:0000F8 Y:0000F8                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3303      Y:0000F9 Y:0000F9                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3304                                END_SERIAL_CLOCK_SPLIT
3305   
3306   
3307                                VIDEO_PROCESS
3308      Y:0000FA Y:0000FA                   DC      END_VIDEO_PROCESS-VIDEO_PROCESS-1
3309                                ;SXMIT  DC  $00F000     ; Transmit A/D data to host
3310      Y:0000FB Y:0000FB                   DC      VIDEO+$000000+%1011011            ; StopDCRestore and StopResetIntegrator
3311      Y:0000FC Y:0000FC                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3312      Y:0000FD Y:0000FD                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3313      Y:0000FE Y:0000FE                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3314      Y:0000FF Y:0000FF                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3315      Y:000100 Y:000100                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3316      Y:000101 Y:000101                   DC      VIDEO+$000000+%0010111            ; StopResetIntegrator
3317      Y:000102 Y:000102                   DC      VIDEO+DCRST_DELAY+%0110111        ; ADCLatch,NonInv  ;mF to do ADC sampeling b
evore resetting
3318                                END_VIDEO_PROCESS
3319                                ;end binning waveforms
3320   
3321   
3322                                ; Video processor bit definition
3323                                ;      xfer, A/D, integ, Pol+, Pol-, DCrestore, rst   (1 => switch open)
3324   
3325                                ; These are the three reading tables. Make sure they're all the same length
3326                                ; 2->3->1->2
3327                                SERIAL_READ_LEFT
3328      Y:000103 Y:000103                   DC      END_SERIAL_READ_LEFT-SERIAL_READ_LEFT-1
3329      Y:000104 Y:000104                   DC      VIDEO+$000000+%1111000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3330      Y:000105 Y:000105                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3331      Y:000106 Y:000106                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3332      Y:000107 Y:000107                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 59



3333      Y:000108 Y:000108                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3334      Y:000109 Y:000109                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3335      Y:00010A Y:00010A                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3336      Y:00010B Y:00010B                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3337      Y:00010C Y:00010C         SXL       DC      $00F000                           ;Transmit a/d data to host
3338      Y:00010D Y:00010D                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3339      Y:00010E Y:00010E                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3340      Y:00010F Y:00010F                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3341      Y:000110 Y:000110                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3342      Y:000111 Y:000111                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3343      Y:000112 Y:000112                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3344      Y:000113 Y:000113                   DC      VIDEO+DCRST_DELAY+%0010111        ;  Now sit around for at least 520ns while t
he conversion happens
3345      Y:000114 Y:000114                   DC      VIDEO+$40000+%0010111             ;  Now sit around for at least 520ns while t
he conversion happens
3346                                END_SERIAL_READ_LEFT
3347   
3348                                SERIAL_READ_SPLIT
3349      Y:000115 Y:000115                   DC      END_SERIAL_READ_SPLIT-SERIAL_READ_SPLIT-1
3350      Y:000116 Y:000116                   DC      VIDEO+$000000+%1111000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3351      Y:000117 Y:000117                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3352   
3353      Y:000118 Y:000118                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2L|H1_3H|H2_1H|H2_2L|H2_3L|WL|TH ;h2->hi
3354      Y:000119 Y:000119                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3H|H2_1H|H2_2H|H2_3L|WL|TH ;h1->lo
3355      Y:00011A Y:00011A                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3L|H2_1L|H2_2H|H2_3L|WL|TH ;h3->hi
3356      Y:00011B Y:00011B                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2H|H1_3L|H2_1L|H2_2H|H2_3H|WL|TH ;h2->lo
3357      Y:00011C Y:00011C                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2L|H1_3L|H2_1L|H2_2L|H2_3H|WL|TH ;h1->hi
3358   
3359                                ;  DC  CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3360                                ;  DC  CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3361                                ;  DC  CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3362                                ;  DC  CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3363                                ;  DC  CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3364   
3365   
3366      Y:00011D Y:00011D                   DC      CLK3|PRE_SET_DLY|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;Reset_Off|Delay
3367      Y:00011E Y:00011E         SXRL      DC      $00F0C2                           ;Transmit a/d data to host
3368      Y:00011F Y:00011F                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3369      Y:000120 Y:000120                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3370      Y:000121 Y:000121                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3371      Y:000122 Y:000122                   DC      CLK3|SW_DELAY|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WH|TH ;SW->hi
3372      Y:000123 Y:000123                   DC      CLK3|POST_SET_DLY|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;SW->lo
3373      Y:000124 Y:000124                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3374      Y:000125 Y:000125                   DC      VIDEO+DCRST_DELAY+%0010111        ; Sit around whilst sampling.
3375      Y:000126 Y:000126                   DC      VIDEO+$50000+%0010111             ;  Now sit around for at least 520ns while t
he conversion happens
3376                                END_SERIAL_READ_SPLIT
3377   
3378   
3379                                SERIAL_READ_SPLIT_SPECIAL
3380      Y:000127 Y:000127                   DC      END_SERIAL_READ_SPLIT_SPECIAL-SERIAL_READ_SPLIT_SPECIAL-1
3381      Y:000128 Y:000128                   DC      VIDEO+$000000+%1111000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3382      Y:000129 Y:000129                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3383   
3384      Y:00012A Y:00012A                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2L|H1_3H|H2_1H|H2_2L|H2_3L|WL|TH ;h2->hi
3385      Y:00012B Y:00012B                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3H|H2_1H|H2_2H|H2_3L|WL|TH ;h1->lo
3386      Y:00012C Y:00012C                   DC      CLK3|S_DELAY|RH|H1_1L|H1_2H|H1_3L|H2_1L|H2_2H|H2_3L|WL|TH ;h3->hi
3387      Y:00012D Y:00012D                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2H|H1_3L|H2_1L|H2_2H|H2_3H|WL|TH ;h2->lo
3388      Y:00012E Y:00012E                   DC      CLK3|S_DELAY|RH|H1_1H|H1_2L|H1_3L|H2_1L|H2_2L|H2_3H|WL|TH ;h1->hi
3389   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 60



3390      Y:00012F Y:00012F                   DC      CLK3|PRE_SET_DLY|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;Reset_Off|Delay
3391      Y:000130 Y:000130                   DC      $00F041                           ;Transmit a/d data to host
3392      Y:000131 Y:000131                   DC      CLK3|$20000|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;Reset_Off|Delay
3393      Y:000132 Y:000132                   DC      $00F082                           ; get the other ADC
3394      Y:000133 Y:000133                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3395      Y:000134 Y:000134                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3396      Y:000135 Y:000135                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3397      Y:000136 Y:000136                   DC      CLK3|SW_DELAY|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WH|TH ;SW->hi
3398      Y:000137 Y:000137                   DC      CLK3|POST_SET_DLY|RL|H1_1H|H1_2L|H1_3H|H2_1H|H2_2L|H2_3H|WL|TH ;SW->lo
3399      Y:000138 Y:000138                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3400      Y:000139 Y:000139                   DC      VIDEO+DCRST_DELAY+%0010111        ; Sit around whilst sampling.
3401      Y:00013A Y:00013A                   DC      VIDEO+$50000+%0010111             ;  Now sit around for at least 520ns while t
he conversion happens
3402                                END_SERIAL_READ_SPLIT_SPECIAL
3403   
3404   
3405                                ; 2->1->3->2
3406                                SERIAL_READ_RIGHT
3407      Y:00013B Y:00013B                   DC      END_SERIAL_READ_RIGHT-SERIAL_READ_RIGHT-1
3408      Y:00013C Y:00013C                   DC      VIDEO+$000000+%1111000            ; NonInv,DCRestore,StrtRstInt.
3409      Y:00013D Y:00013D                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3410      Y:00013E Y:00013E                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h2->hi
3411      Y:00013F Y:00013F                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h1->lo
3412      Y:000140 Y:000140                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3413      Y:000141 Y:000141                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h2->lo
3414      Y:000142 Y:000142                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h1->hi
3415      Y:000143 Y:000143                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3416      Y:000144 Y:000144         SXR       DC      $00F041                           ;Transmit a/d data to host
3417      Y:000145 Y:000145                   DC      VIDEO+$000000+%0011011            ; StopDCRestore and StopResetIntegrator
3418      Y:000146 Y:000146                   DC      VIDEO+I_DELAY+%0001011            ; Integrate for I_DELAY microsec
3419      Y:000147 Y:000147                   DC      VIDEO+$000000+%0010111            ; Stop Integrate and sel inverting int.
3420      Y:000148 Y:000148                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3421      Y:000149 Y:000149                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3422      Y:00014A Y:00014A                   DC      VIDEO+I_DELAY+%0000111            ; Integrate for I_DELAY microsec
3423      Y:00014B Y:00014B                   DC      VIDEO+DCRST_DELAY+%0010111        ; Wait for sampling
3424      Y:00014C Y:00014C                   DC      VIDEO+$30000+%0010111             ;
3425                                END_SERIAL_READ_RIGHT
3426   
3427                                ; This waveforms for test only. Video is left under permamnet reset
3428                                SERIAL_READ_LEFT_NULL
3429      Y:00014D Y:00014D                   DC      END_SERIAL_READ_LEFT_NULL-SERIAL_READ_LEFT_NULL-1
3430      Y:00014E Y:00014E                   DC      VIDEO+$000000+%1111000            ; ADCLatch,NonInv,DCRestore,StrtRstInt.
3431      Y:00014F Y:00014F                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;h3->lo,SW->lo,Reset
_On
3432      Y:000150 Y:000150                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3433      Y:000151 Y:000151                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3434      Y:000152 Y:000152                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3435      Y:000153 Y:000153                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3436      Y:000154 Y:000154                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3437      Y:000155 Y:000155                   DC      CLK3|PRE_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3438      Y:000156 Y:000156                   DC      $00F000                           ;Transmit a/d data to host
3439      Y:000157 Y:000157                   DC      VIDEO+$000000+%0011010            ; StopDCRestore and StopResetIntegrator
3440      Y:000158 Y:000158                   DC      VIDEO+I_DELAY+%0001010            ; Integrate for I_DELAY microsec
3441      Y:000159 Y:000159                   DC      VIDEO+$000000+%0010110            ; Stop Integrate and sel inverting int.
3442      Y:00015A Y:00015A                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3443      Y:00015B Y:00015B                   DC      CLK3|POST_SET_DLY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;SW->lo
3444      Y:00015C Y:00015C                   DC      VIDEO+I_DELAY+%0000110            ; Integrate for I_DELAY microsec
3445      Y:00015D Y:00015D                   DC      VIDEO+DCRST_DELAY+%0010110        ;  Now sit around for at least 520ns while t
he conversion happens
3446      Y:00015E Y:00015E                   DC      VIDEO+$40000+%0010110             ;  Now sit around for at least 520ns while t
he conversion happens
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 61



3447                                END_SERIAL_READ_LEFT_NULL
3448   
3449   
3450                                ; These are the three skipping tables. Make sure they're all the same length
3451                                SERIAL_SKIP_LEFT                                    ; Serial clocking waveform for skipping left
3452      Y:00015F Y:00015F                   DC      END_SERIAL_SKIP_LEFT-SERIAL_SKIP_LEFT-1
3453      Y:000160 Y:000160                   DC      VIDEO+$000000+%1011000            ; Change nearly everything
3454      Y:000161 Y:000161                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h2->hi
3455      Y:000162 Y:000162                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h1->lo
3456      Y:000163 Y:000163                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3457      Y:000164 Y:000164                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h2->lo
3458      Y:000165 Y:000165                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h1->hi
3459      Y:000166 Y:000166                   DC      CLK3|S_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3460      Y:000167 Y:000167                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3461                                END_SERIAL_SKIP_LEFT
3462   
3463                                SERIAL_SKIP_RIGHT                                   ; Serial clocking waveform for skipping righ
t
3464      Y:000168 Y:000168                   DC      END_SERIAL_SKIP_RIGHT-SERIAL_SKIP_RIGHT-1
3465      Y:000169 Y:000169                   DC      VIDEO+$000000+%1011000            ; Change nearly everything
3466      Y:00016A Y:00016A                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2L|H2_3L|H1_1H|H1_2L|H1_3L|WL|TH ;h2->hi
3467      Y:00016B Y:00016B                   DC      CLK3|S_DELAY|RH|H2_1H|H2_2H|H2_3L|H1_1H|H1_2H|H1_3L|WL|TH ;h1->lo
3468      Y:00016C Y:00016C                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3469      Y:00016D Y:00016D                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2H|H2_3H|H1_1L|H1_2H|H1_3H|WL|TH ;h2->lo
3470      Y:00016E Y:00016E                   DC      CLK3|S_DELAY|RH|H2_1L|H2_2L|H2_3H|H1_1L|H1_2L|H1_3H|WL|TH ;h1->hi
3471      Y:00016F Y:00016F                   DC      CLK3|S_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Delay
3472      Y:000170 Y:000170                   DC      CLK3|SW_DELAY|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3473                                END_SERIAL_SKIP_RIGHT
3474   
3475                                SERIAL_SKIP_SPLIT                                   ; Serial clocking waveform for skipping both
 ends
3476      Y:000171 Y:000171                   DC      END_SERIAL_SKIP_SPLIT-SERIAL_SKIP_SPLIT-1
3477      Y:000172 Y:000172                   DC      VIDEO+$000000+%1011000            ; Change nearly everything
3478      Y:000173 Y:000173                   DC      CLK3|S_DELAY_SKIP|RH|H2_1H|H2_2L|H2_3L|H1_1L|H1_2L|H1_3H|WL|TH ;h2->hi
3479      Y:000174 Y:000174                   DC      CLK3|S_DELAY_SKIP|RH|H2_1H|H2_2H|H2_3L|H1_1L|H1_2H|H1_3H|WL|TH ;h1->lo
3480      Y:000175 Y:000175                   DC      CLK3|S_DELAY_SKIP|RH|H2_1L|H2_2H|H2_3L|H1_1L|H1_2H|H1_3L|WL|TH ;h3->hi
3481      Y:000176 Y:000176                   DC      CLK3|S_DELAY_SKIP|RH|H2_1L|H2_2H|H2_3H|H1_1H|H1_2H|H1_3L|WL|TH ;h2->lo
3482      Y:000177 Y:000177                   DC      CLK3|S_DELAY_SKIP|RH|H2_1L|H2_2L|H2_3H|H1_1H|H1_2L|H1_3L|WL|TH ;h1->hi
3483      Y:000178 Y:000178                   DC      CLK3|S_DELAY_SKIP|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WL|TH ;Reset_Off|Dela
y
3484      Y:000179 Y:000179                   DC      CLK3|SW_DELAY_SKIP|RL|H2_1H|H2_2L|H2_3H|H1_1H|H1_2L|H1_3H|WH|TH ;SW->hi
3485                                END_SERIAL_SKIP_SPLIT
3486   
3488                                ; ORG Y:$1C0,Y:$1C0   ; Download address
3490                                VSUBN
3491                                          VOLTS   VSUB,60.0                         ; Vsub  0.0 140 V, pin #
**** 3496 [LBNL_2Kx4K.waveforms 843]: Setting voltage VSUB 60.0V 30723083264
3497      Y:00017B Y:00017B         ERHI      DC      ERHI_END-ERHI-1
3498                                          VOLTS   VSUB,0                            ; Vsub  0.0 140 V, pin #
**** 3503 [LBNL_2Kx4K.waveforms 845]: Setting voltage VSUB 0V 03080192
3504                                ; VOLTS V1_HI,9   ; Vertical High
3505                                ; VOLTS V1_LO,9   ; Vertical Low
3506                                ; VOLTS V2_HI,9   ; Vertical High
3507                                ; VOLTS V2_LO,9   ; Vertical Low
3508                                ; VOLTS V3_HI,9   ; Vertical High
3509                                ; VOLTS V3_LO,9   ; Vertical Low
3510                                ; VOLTS FS1_HI,9    ; Vertical High
3511                                ; VOLTS FS1_LO,9    ; Vertical Low
3512                                ; VOLTS FS2_HI,9    ; Vertical High
3513                                ; VOLTS FS2_LO,9    ; Vertical Low
3514                                ; VOLTS FS3_HI,9    ; Vertical High
3515                                ; VOLTS FS3_LO,9    ; Vertical Low
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 62



3516      Y:00017D Y:00017D                   DC      $200100+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #1, Vertical Clock 1
3517      Y:00017E Y:00017E                   DC      $200200+@CVI((9+Vmax)/(2*Vmax)*255)
3518      Y:00017F Y:00017F                   DC      $200400+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #2, Vertical Clock 2
3519      Y:000180 Y:000180                   DC      $200800+@CVI((9+Vmax)/(2*Vmax)*255)
3520      Y:000181 Y:000181                   DC      $202000+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #3, Vertical Clock 3
3521      Y:000182 Y:000182                   DC      $204000+@CVI((9+Vmax)/(2*Vmax)*255)
3522      Y:000183 Y:000183                   DC      $208000+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #4, Frame Store 1
3523      Y:000184 Y:000184                   DC      $210000+@CVI((9+Vmax)/(2*Vmax)*255)
3524      Y:000185 Y:000185                   DC      $220100+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #5, Frame Store 2
3525      Y:000186 Y:000186                   DC      $220200+@CVI((9+Vmax)/(2*Vmax)*255)
3526      Y:000187 Y:000187                   DC      $220400+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #6, Frame Store 3
3527      Y:000188 Y:000188                   DC      $220800+@CVI((9+Vmax)/(2*Vmax)*255)
3528      Y:000189 Y:000189                   DC      $222000+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #7, Transfer Gate 2
3529      Y:00018A Y:00018A                   DC      $224000+@CVI((9+Vmax)/(2*Vmax)*255)
3530      Y:00018B Y:00018B                   DC      $228000+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #8, Transger Gate 1
3531      Y:00018C Y:00018C                   DC      $230000+@CVI((9+Vmax)/(2*Vmax)*255)
3532      Y:00018D Y:00018D                   DC      $240100+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #9, Unused
3533      Y:00018E Y:00018E                   DC      $240200+@CVI((9+Vmax)/(2*Vmax)*255)
3534      Y:00018F Y:00018F                   DC      $240400+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #10, Unused
3535      Y:000190 Y:000190                   DC      $240800+@CVI((9+Vmax)/(2*Vmax)*255)
3536      Y:000191 Y:000191                   DC      $242000+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #11, Unused
3537      Y:000192 Y:000192                   DC      $244000+@CVI((9+Vmax)/(2*Vmax)*255)
3538      Y:000193 Y:000193                   DC      $248000+@CVI((9+Vmax)/(2*Vmax)*255) ; Pin #12, Unused
3539      Y:000194 Y:000194                   DC      $250000+@CVI((9+Vmax)/(2*Vmax)*255)
3540      Y:000195 Y:000195         ERHI_END  DC      EPUR-ERHI_END-1
3541                                ; VOLTS V1_HI,5.0 ; Vertical High
3542                                ; VOLTS V1_LO,-3.0  ; Vertical Low
3543                                ; VOLTS V2_HI,5.0 ; Vertical High
3544                                ; VOLTS V2_LO,-3.0  ; Vertical Low
3545                                ; VOLTS V3_HI,5.0 ; Vertical High
3546                                ; VOLTS V3_LO,-3.0  ; Vertical Low
3547                                ; VOLTS FS1_HI,5.0  ; Vertical High
3548                                ; VOLTS FS1_LO,-3.0 ; Vertical Low
3549                                ; VOLTS FS2_HI,5.0  ; Vertical High
3550                                ; VOLTS FS2_LO,-3.0 ; Vertical Low
3551                                ; VOLTS FS3_HI,5.0  ; Vertical High
3552                                ; VOLTS FS3_LO,-3.0 ; Vertical Low
3553                                ;Return to normal voltages
3554      Y:000196 Y:000196                   DC      $200100+@CVI((V1_HI+Vmax)/(2*Vmax)*255) ; Pin #1, Vertical Clock 1
3555      Y:000197 Y:000197                   DC      $200200+@CVI((V1_LO+Vmax)/(2*Vmax)*255)
3556      Y:000198 Y:000198                   DC      $200400+@CVI((V2_HI+Vmax)/(2*Vmax)*255) ; Pin #2, Vertical Clock 2
3557      Y:000199 Y:000199                   DC      $200800+@CVI((V2_LO+Vmax)/(2*Vmax)*255)
3558      Y:00019A Y:00019A                   DC      $202000+@CVI((V3_HI+Vmax)/(2*Vmax)*255) ; Pin #3, Vertical Clock 3
3559      Y:00019B Y:00019B                   DC      $204000+@CVI((V3_LO+Vmax)/(2*Vmax)*255)
3560      Y:00019C Y:00019C                   DC      $208000+@CVI((V1_HI+Vmax)/(2*Vmax)*255) ; Pin #4, Frame Store 1
3561      Y:00019D Y:00019D                   DC      $210000+@CVI((V1_LO+Vmax)/(2*Vmax)*255)
3562      Y:00019E Y:00019E                   DC      $220100+@CVI((V2_HI+Vmax)/(2*Vmax)*255) ; Pin #5, Frame Store 2
3563      Y:00019F Y:00019F                   DC      $220200+@CVI((V2_LO+Vmax)/(2*Vmax)*255)
3564      Y:0001A0 Y:0001A0                   DC      $220400+@CVI((V3_HI+Vmax)/(2*Vmax)*255) ; Pin #6, Frame Store 3
3565      Y:0001A1 Y:0001A1                   DC      $220800+@CVI((V3_LO+Vmax)/(2*Vmax)*255)
3566      Y:0001A2 Y:0001A2                   DC      $222000+@CVI((V1_HI+Vmax)/(2*Vmax)*255) ; Pin #7, Transfer Gate 2
3567      Y:0001A3 Y:0001A3                   DC      $224000+@CVI((V1_LO+Vmax)/(2*Vmax)*255)
3568      Y:0001A4 Y:0001A4                   DC      $228000+@CVI((V2_HI+Vmax)/(2*Vmax)*255) ; Pin #8, Transger Gate 1
3569      Y:0001A5 Y:0001A5                   DC      $230000+@CVI((V2_LO+Vmax)/(2*Vmax)*255)
3570      Y:0001A6 Y:0001A6                   DC      $240100+@CVI((V3_HI+Vmax)/(2*Vmax)*255) ; Pin #9, Unused
3571      Y:0001A7 Y:0001A7                   DC      $240200+@CVI((V3_LO+Vmax)/(2*Vmax)*255)
3572      Y:0001A8 Y:0001A8                   DC      $240400+@CVI((FS1_HI+Vmax)/(2*Vmax)*255) ; Pin #10, Unused
3573      Y:0001A9 Y:0001A9                   DC      $240800+@CVI((FS1_LO+Vmax)/(2*Vmax)*255)
3574      Y:0001AA Y:0001AA                   DC      $242000+@CVI((FS2_HI+Vmax)/(2*Vmax)*255) ; Pin #11, Unused
3575      Y:0001AB Y:0001AB                   DC      $244000+@CVI((FS2_LO+Vmax)/(2*Vmax)*255)
3576      Y:0001AC Y:0001AC                   DC      $248000+@CVI((FS3_HI+Vmax)/(2*Vmax)*255) ; Pin #12, Unused
3577      Y:0001AD Y:0001AD                   DC      $250000+@CVI((FS3_LO+Vmax)/(2*Vmax)*255)
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 63



3578   
3579      Y:0001AE Y:0001AE                   DC      $2A0100+@CVI((RL_HI+Vmax)/(2*Vmax)*255) ; Pin #34, Reset Gate Upper
3580      Y:0001AF Y:0001AF                   DC      $2A0200+@CVI((RL_LO+Vmax)/(2*Vmax)*255)
3581      Y:0001B0 Y:0001B0                   DC      $2A0400+@CVI((RU_HI+Vmax)/(2*Vmax)*255) ; Pin #35, Reset Gate Lower
3582      Y:0001B1 Y:0001B1                   DC      $2A0800+@CVI((RU_LO+Vmax)/(2*Vmax)*255)
3583   
3584      Y:0001B2 Y:0001B2         EPUR      DC      EPUR_END-EPUR-1
3585      Y:0001B3 Y:0001B3                   DC      $200100+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #1, Vertical Clock 1
3586      Y:0001B4 Y:0001B4                   DC      $200200+@CVI((-9+Vmax)/(2*Vmax)*255)
3587      Y:0001B5 Y:0001B5                   DC      $200400+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #2, Vertical Clock 2
3588      Y:0001B6 Y:0001B6                   DC      $200800+@CVI((-9+Vmax)/(2*Vmax)*255)
3589      Y:0001B7 Y:0001B7                   DC      $202000+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #3, Vertical Clock 3
3590      Y:0001B8 Y:0001B8                   DC      $204000+@CVI((-9+Vmax)/(2*Vmax)*255)
3591      Y:0001B9 Y:0001B9                   DC      $208000+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #4, Frame Store 1
3592      Y:0001BA Y:0001BA                   DC      $210000+@CVI((-9+Vmax)/(2*Vmax)*255)
3593      Y:0001BB Y:0001BB                   DC      $220100+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #5, Frame Store 2
3594      Y:0001BC Y:0001BC                   DC      $220200+@CVI((-9+Vmax)/(2*Vmax)*255)
3595      Y:0001BD Y:0001BD                   DC      $220400+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #6, Frame Store 3
3596      Y:0001BE Y:0001BE                   DC      $220800+@CVI((-9+Vmax)/(2*Vmax)*255)
3597      Y:0001BF Y:0001BF                   DC      $222000+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #7, Transfer Gate 2
3598      Y:0001C0 Y:0001C0                   DC      $224000+@CVI((-9+Vmax)/(2*Vmax)*255)
3599      Y:0001C1 Y:0001C1                   DC      $228000+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #8, Transger Gate 1
3600      Y:0001C2 Y:0001C2                   DC      $230000+@CVI((-9+Vmax)/(2*Vmax)*255)
3601      Y:0001C3 Y:0001C3                   DC      $240100+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #9, Unused
3602      Y:0001C4 Y:0001C4                   DC      $240200+@CVI((-9+Vmax)/(2*Vmax)*255)
3603      Y:0001C5 Y:0001C5                   DC      $240400+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #10, Unused
3604      Y:0001C6 Y:0001C6                   DC      $240800+@CVI((-9+Vmax)/(2*Vmax)*255)
3605      Y:0001C7 Y:0001C7                   DC      $242000+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #11, Unused
3606      Y:0001C8 Y:0001C8                   DC      $244000+@CVI((-9+Vmax)/(2*Vmax)*255)
3607      Y:0001C9 Y:0001C9                   DC      $248000+@CVI((-9+Vmax)/(2*Vmax)*255) ; Pin #12, Unused
3608      Y:0001CA Y:0001CA                   DC      $250000+@CVI((-9+Vmax)/(2*Vmax)*255)
3609   
3610      Y:0001CB Y:0001CB                   DC      $2A0100+@CVI((-6+Vmax)/(2*Vmax)*255) ; Pin #34, Reset Gate Upper
3611      Y:0001CC Y:0001CC                   DC      $2A0200+@CVI((-6+Vmax)/(2*Vmax)*255)
3612      Y:0001CD Y:0001CD                   DC      $2A0400+@CVI((-6+Vmax)/(2*Vmax)*255) ; Pin #35, Reset Gate Lower
3613      Y:0001CE Y:0001CE                   DC      $2A0800+@CVI((-6+Vmax)/(2*Vmax)*255)
3614   
3615                                EPUR_END
3616   
3617                                ; Code for ARC32 = universal clock driver board
3618      Y:0001CF Y:0001CF         DACS      DC      END_DACS-DACS-1
3619      Y:0001D0 Y:0001D0                   DC      $2A0080                           ; DAC = unbuffered mode
3620   
3621      Y:0001D1 Y:0001D1                   DC      $200100+@CVI((V1_HI+Vmax)/(2*Vmax)*255) ; Pin #1, Vertical Clock 1
3622      Y:0001D2 Y:0001D2                   DC      $200200+@CVI((V1_LO+Vmax)/(2*Vmax)*255)
3623      Y:0001D3 Y:0001D3                   DC      $200400+@CVI((V2_HI+Vmax)/(2*Vmax)*255) ; Pin #2, Vertical Clock 2
3624      Y:0001D4 Y:0001D4                   DC      $200800+@CVI((V2_LO+Vmax)/(2*Vmax)*255)
3625      Y:0001D5 Y:0001D5                   DC      $202000+@CVI((V3_HI+Vmax)/(2*Vmax)*255) ; Pin #3, Vertical Clock 3
3626      Y:0001D6 Y:0001D6                   DC      $204000+@CVI((V3_LO+Vmax)/(2*Vmax)*255)
3627      Y:0001D7 Y:0001D7                   DC      $208000+@CVI((V1_HI+Vmax)/(2*Vmax)*255) ; Pin #4, Frame Store 1
3628      Y:0001D8 Y:0001D8                   DC      $210000+@CVI((V1_LO+Vmax)/(2*Vmax)*255)
3629      Y:0001D9 Y:0001D9                   DC      $220100+@CVI((V2_HI+Vmax)/(2*Vmax)*255) ; Pin #5, Frame Store 2
3630      Y:0001DA Y:0001DA                   DC      $220200+@CVI((V2_LO+Vmax)/(2*Vmax)*255)
3631      Y:0001DB Y:0001DB                   DC      $220400+@CVI((V3_HI+Vmax)/(2*Vmax)*255) ; Pin #6, Frame Store 3
3632      Y:0001DC Y:0001DC                   DC      $220800+@CVI((V3_LO+Vmax)/(2*Vmax)*255)
3633      Y:0001DD Y:0001DD                   DC      $222000+@CVI((V1_HI+Vmax)/(2*Vmax)*255) ; Pin #7, Vertical Clock 1
3634      Y:0001DE Y:0001DE                   DC      $224000+@CVI((V1_LO+Vmax)/(2*Vmax)*255)
3635      Y:0001DF Y:0001DF                   DC      $228000+@CVI((V2_HI+Vmax)/(2*Vmax)*255) ; Pin #8, Vertical Clock 2
3636      Y:0001E0 Y:0001E0                   DC      $230000+@CVI((V2_LO+Vmax)/(2*Vmax)*255)
3637   
3638      Y:0001E1 Y:0001E1                   DC      $240100+@CVI((V3_HI+Vmax)/(2*Vmax)*255) ; Pin #9, Vertical Clock 3
3639      Y:0001E2 Y:0001E2                   DC      $240200+@CVI((V3_LO+Vmax)/(2*Vmax)*255)
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 64



3640      Y:0001E3 Y:0001E3                   DC      $240400+@CVI((FS1_HI+Vmax)/(2*Vmax)*255) ; Pin #10, Frame Store 1
3641      Y:0001E4 Y:0001E4                   DC      $240800+@CVI((FS1_LO+Vmax)/(2*Vmax)*255)
3642      Y:0001E5 Y:0001E5                   DC      $242000+@CVI((FS2_HI+Vmax)/(2*Vmax)*255) ; Pin #11, Frame Store 2
3643      Y:0001E6 Y:0001E6                   DC      $244000+@CVI((FS2_LO+Vmax)/(2*Vmax)*255)
3644      Y:0001E7 Y:0001E7                   DC      $248000+@CVI((FS3_HI+Vmax)/(2*Vmax)*255) ; Pin #12, Frame Store 3
3645      Y:0001E8 Y:0001E8                   DC      $250000+@CVI((FS3_LO+Vmax)/(2*Vmax)*255)
3646   
3647      Y:0001E9 Y:0001E9                   DC      $260100+@CVI((H1U2_L1_HI+Vmax)/(2*Vmax)*255) ; Pin #13, Horizontal 1 Upper
3648      Y:0001EA Y:0001EA                   DC      $260200+@CVI((H1U2_L1_LO+Vmax)/(2*Vmax)*255)
3649      Y:0001EB Y:0001EB                   DC      $260400+@CVI((H2U2_L1_HI+Vmax)/(2*Vmax)*255) ; Pin #14, Horizontal 2 Upper
3650      Y:0001EC Y:0001EC                   DC      $260800+@CVI((H2U2_L1_LO+Vmax)/(2*Vmax)*255)
3651      Y:0001ED Y:0001ED                   DC      $262000+@CVI((H3U2_L1_HI+Vmax)/(2*Vmax)*255) ; Pin #15, Horizontal 3 Upper
3652      Y:0001EE Y:0001EE                   DC      $264000+@CVI((H3U2_L1_LO+Vmax)/(2*Vmax)*255)
3653      Y:0001EF Y:0001EF                   DC      $268000+@CVI((H1U1_L2_HI+Vmax)/(2*Vmax)*255) ; Pin #16, Horizontal 1 Lower
3654      Y:0001F0 Y:0001F0                   DC      $270000+@CVI((H1U1_L2_LO+Vmax)/(2*Vmax)*255)
3655      Y:0001F1 Y:0001F1                   DC      $280100+@CVI((H2U1_L2_HI+Vmax)/(2*Vmax)*255) ; Pin #17, Horizontal 2 Lower
3656      Y:0001F2 Y:0001F2                   DC      $280200+@CVI((H2U1_L2_LO+Vmax)/(2*Vmax)*255)
3657      Y:0001F3 Y:0001F3                   DC      $280400+@CVI((H3U1_L2_HI+Vmax)/(2*Vmax)*255) ; Pin #18, Horizontal 3 Lower
3658      Y:0001F4 Y:0001F4                   DC      $280800+@CVI((H3U1_L2_LO+Vmax)/(2*Vmax)*255)
3659      Y:0001F5 Y:0001F5                   DC      $282000+@CVI((SWL_HI+Vmax)/(2*Vmax)*255) ; Pin #19, Summing Well Upper
3660      Y:0001F6 Y:0001F6                   DC      $284000+@CVI((SWL_LO+Vmax)/(2*Vmax)*255)
3661      Y:0001F7 Y:0001F7                   DC      $288000+@CVI((SWU_HI+Vmax)/(2*Vmax)*255) ; Pin #33, Summing Well Lower
3662      Y:0001F8 Y:0001F8                   DC      $290000+@CVI((SWU_LO+Vmax)/(2*Vmax)*255)
3663      Y:0001F9 Y:0001F9                   DC      $2A0100+@CVI((RL_HI+Vmax)/(2*Vmax)*255) ; Pin #34, Reset Gate Upper
3664      Y:0001FA Y:0001FA                   DC      $2A0200+@CVI((RL_LO+Vmax)/(2*Vmax)*255)
3665      Y:0001FB Y:0001FB                   DC      $2A0400+@CVI((RU_HI+Vmax)/(2*Vmax)*255) ; Pin #35, Reset Gate Lower
3666      Y:0001FC Y:0001FC                   DC      $2A0800+@CVI((RU_LO+Vmax)/(2*Vmax)*255)
3667      Y:0001FD Y:0001FD                   DC      $2A2000+@CVI((T1_HI+Vmax)/(2*Vmax)*255) ; Pin #36, Transfer Gate 1
3668      Y:0001FE Y:0001FE                   DC      $2A4000+@CVI((T1_LO+Vmax)/(2*Vmax)*255)
3669      Y:0001FF Y:0001FF                   DC      $2A8000+@CVI((T2_HI+Vmax)/(2*Vmax)*255) ; Pin #37, Transfer Gate 2
3670      Y:000200 Y:000200                   DC      $2B0000+@CVI((T2_LO+Vmax)/(2*Vmax)*255)
3671   
3672   
3673                                ; DC bias voltages for the LBL CCD chip
3674                                          VOLTS   VSUB,60.0                         ; Vsub  0.0 140 V
**** 3679 [LBNL_2Kx4K.waveforms 1016]: Setting voltage VSUB 60.0V 30723083264
3680                                          VOLTS   RAMP,5.0                          ; Vsub  AVG RAMP RATE
**** 3685 [LBNL_2Kx4K.waveforms 1017]: Setting voltage RAMP 5.0V 20483098624
3686                                          VOLTS   VDDL2,-22.0                       ; Vdd  -5.1 -25V
**** 3691 [LBNL_2Kx4K.waveforms 1018]: Setting voltage VDDL2 -22.0V 36042985492
3692                                          VOLTS   VDDU2,-22.0                       ; Vdd  -5.1 -25V
**** 3697 [LBNL_2Kx4K.waveforms 1019]: Setting voltage VDDU2 -22.0V 36043001876
3698                                          VOLTS   VDDL1,-22.0                       ; Vdd  -5.1 -25V
**** 3703 [LBNL_2Kx4K.waveforms 1020]: Setting voltage VDDL1 -22.0V 36042952724
3704                                          VOLTS   VDDU1,-22.0                       ; Vdd  -5.1 -25V
**** 3709 [LBNL_2Kx4K.waveforms 1021]: Setting voltage VDDU1 -22.0V 36042969108
3710                                          VOLTS   VRL2,-12.5                        ; Vr   -5.1 -25V
**** 3715 [LBNL_2Kx4K.waveforms 1022]: Setting voltage VRL2 -12.5V 20482918400
3716                                          VOLTS   VRU2,-12.5                        ; Vr   -5.1 -25V
**** 3721 [LBNL_2Kx4K.waveforms 1023]: Setting voltage VRU2 -12.5V 20482934784
3722                                          VOLTS   VRL1,-12.5                        ; Vr   -5.1 -25V
**** 3727 [LBNL_2Kx4K.waveforms 1024]: Setting voltage VRL1 -12.5V 20482885632
3728                                          VOLTS   VRU1,-12.5                        ; Vr   -5.1 -25V
**** 3733 [LBNL_2Kx4K.waveforms 1025]: Setting voltage VRU1 -12.5V 20482902016
3734                                          VOLTS   VOGL2,2.50                        ; Vopg  -10  10 V
**** 3739 [LBNL_2Kx4K.waveforms 1026]: Setting voltage VOGL2 2.50V 20483049472
3740                                          VOLTS   VOGU2,2.50                        ; Vopg  -10  10 V
**** 3745 [LBNL_2Kx4K.waveforms 1027]: Setting voltage VOGU2 2.50V 20483065856
3746                                          VOLTS   VOGL1,2.50                        ; Vopg  -10  10 V
**** 3751 [LBNL_2Kx4K.waveforms 1028]: Setting voltage VOGL1 2.50V 20483016704
3752                                          VOLTS   VOGU1,3.50                        ; Vopg  -10  10 V
**** 3757 [LBNL_2Kx4K.waveforms 1029]: Setting voltage VOGU1 3.50V 28673033907
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  LBNL_2Kx4K.waveforms  Page 65



3758   
3759   
3760                                ; Set gain and integrator speed. (77, bb, dd, ee; low gain to high)
3761   
3762                                ; Board #0
3763                                ;  DC      $0c3c77                 ; Gain x1, slow integ. speed, board #0
3764                                ;  DC      $0c3f77                 ; Gain x1, fast integrate speed
3765      Y:00020F Y:00020F                   DC      $0c3cbb                           ; Gain x2
3766                                ;  DC      $0c3cdd                 ; Gain x4.75, slow
3767                                ;  DC      $0c3fee                 ; Gain x9.50
3768                                ;  DC      $0c3cee                 ; Gain x9.50, slow
3769   
3770                                ; Board #1
3771                                ;  DC      $1c3c77                 ; Gain x1, slow integ. speed, board #1
3772                                ;  DC      $1c3f77                 ; Gain x1, fast integrate speed
3773      Y:000210 Y:000210                   DC      $1c3cbb                           ; Gain x2, slow
3774                                ;  DC      $1c3cdd                 ; Gain x4.75, slow
3775                                ;  DC      $1c3cee                 ; Gain x9.50, slow
3776   
3777                                 GAIN_SETTING
3778      Y:000211 Y:000211                   DC      VID0+$0D000E
3779   
3780                                ; Set offset voltages in Video boards
3781   
3782                                ; Board #0
3783      Y:000212 Y:000212                   DC      $0C8000+OFFSET0
3784      Y:000213 Y:000213                   DC      $0CC000+OFFSET1
3785   
3786                                ; Board #1
3787      Y:000214 Y:000214                   DC      $1C8000+OFFSET2
3788      Y:000215 Y:000215                   DC      $1CC000+OFFSET3
3789   
3790                                ; Bias voltages in the Video Boards used only for diagnostics (not connected to CCD)
3791                                ; Bias range = -10V to +10V
3792      000700                    BIAS_TST  EQU     $700                              ; -1.25V
3793   
3794                                ; Board #0
3795      Y:000216 Y:000216                   DC      $0E0000+BIAS_TST                  ; DB-25 Pin 9
3796      Y:000217 Y:000217                   DC      $0E4000+BIAS_TST                  ; DB-25 Pin 10
3797   
3798                                ; Board #1
3799      Y:000218 Y:000218                   DC      $1E0000+BIAS_TST                  ; DB-25 Pin 9
3800      Y:000219 Y:000219                   DC      $1E4000+BIAS_TST                  ; DB-25 Pin 10
3801   
3802                                END_DACS
3803   
3804   
3805                                ; Pixel table generated in "timCCD.asm"
3806      Y:00021A Y:00021A         PXL_TBL   DC      0
3807   
3808   
3809   
3810      Y:00024D Y:00024D                   ORG     Y:@LCV(L)+50,Y:@LCV(L)+50
3811   
3812                                 TMP_PXL_TBL1
3813      Y:00024D Y:00024D                   DC      0
3814   
3815      Y:000280 Y:000280                   ORG     Y:@LCV(L)+50,Y:@LCV(L)+50
3816   
3817                                 TMP_PXL_TBL2
3818      Y:000280 Y:000280                   DC      0
3819   
Motorola DSP56300 Assembler  Version 6.3.4   21-12-08  11:13:23  tim.asm  Page 66



3820      Y:0002B3 Y:0002B3                   ORG     Y:@LCV(L)+50,Y:@LCV(L)+50
3821   
3822                                 TMP_PXL_TBL3
3823      Y:0002B3 Y:0002B3                   DC      0
3824   
3825                                 END_APPLICATON_Y_MEMORY
3826      0002B4                              EQU     @LCV(L)
3827   
3828   
3829                                ; End of program
3830                                          END

0    Errors
0    Warnings


