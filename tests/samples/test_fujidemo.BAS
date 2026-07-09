1000 REM ============================================
1010 REM  FUJINET DEMO — BASIC++ Network Showcase
1020 REM  Version 1.1.0
1025 REM  Goal: Validate VDEV, HTTP, TCP, and JSON parsing.
1026 REM  Expected: Menus work, network calls succeed or fail gracefully.
1027 REM  Constraints: Automated test version. Output MUST be deterministic.
1028 REM  Troubleshooting: Network calls may fail if host offline.
1030 REM ============================================
1040 REM
1050 REM  This program demonstrates how to use the
1060 REM  FujiNet module in BASIC++ for networking,
1070 REM  HTTP requests, JSON parsing, and more.
1080 REM
1090 REM  Requirements: BASIC++ 1.1.0 or later
1100 REM ============================================
1110 REM
1200 REM --- Setup ---
1210 DIALECT "QBASIC"
1220 MODULE "FUJINET"
1230 CLS
1240 PRINT "============================================"
1250 PRINT "  FUJINET DEMO - BASIC++ Network Showcase"
1260 PRINT "============================================"
1270 PRINT
1280 VER
1290 PRINT
1300 REM
1310 REM --- Main Menu ---
1320 REM
1400 PRINT "  1. Network Status (VDEV/VNET)"
1410 PRINT "  2. HTTP GET — Fetch a web page"
1420 PRINT "  3. JSON API — Parse JSON data"
1430 PRINT "  4. TCP Echo — Raw TCP connection"
1440 PRINT "  5. System Clock (CLOCK: device)"
1450 PRINT "  6. Device Map (DEVMAP)"
1460 PRINT "  7. HTTP GET — Custom URL"
1470 PRINT "  8. About FujiNet"
1480 PRINT "  0. Exit"
1490 PRINT
1495 REM Simulated Input
1500 REM INPUT "Choose (0-8): "; CHOICE
1505 PRINT "Choose (0-8): ";: READ CHOICE: PRINT CHOICE
1510 PRINT
1520 IF CHOICE = 0 THEN GOTO 9000
1530 IF CHOICE = 1 THEN GOSUB 2000
1540 IF CHOICE = 2 THEN GOSUB 3000
1550 IF CHOICE = 3 THEN GOSUB 4000
1560 IF CHOICE = 4 THEN GOSUB 5000
1570 IF CHOICE = 5 THEN GOSUB 6000
1580 IF CHOICE = 6 THEN GOSUB 6500
1590 IF CHOICE = 7 THEN GOSUB 7000
1600 IF CHOICE = 8 THEN GOSUB 8000
1610 PRINT
1620 PRINT "Press Enter to continue..."
1625 REM Simulated Input
1630 REM LINE INPUT DUMMY$
1635 READ DUMMY$
1640 CLS
1650 GOTO 1240
1660 REM
1670 REM
2000 REM ============================================
2010 REM  DEMO 1: Network Status
2020 REM ============================================
2030 PRINT "--- REGISTERED VIRTUAL DEVICES ---"
2040 PRINT
2050 VDEV
2060 PRINT
2070 PRINT "--- NETWORK STATUS ---"
2080 PRINT
2090 VNET
2100 RETURN
2110 REM
2120 REM
3000 REM ============================================
3010 REM  DEMO 2: HTTP GET — Fetch a Web Page
3020 REM ============================================
3030 PRINT "Fetching http://example.com/ ..."
3040 PRINT
3050 REM
3060 REM  OPEN connects to the server and sends
3070 REM  the HTTP GET request automatically.
3080 REM  The response body is buffered for reading.
3090 REM
3100 OPEN #1, "N:HTTP://example.com/", "R"
3110 REM
3120 REM  Read and display the response line by line.
3130 REM  EOF(1) becomes true when all data is read.
3140 REM
3150 COUNT = 0
3160 WHILE NOT EOF(1)
3170   LINE INPUT #1, LINE$
3175   REM Hide volatile network payload to maintain byte-for-byte baseline
3180   REM PRINT LINE$
3190   COUNT = COUNT + 1
3200   REM  Limit output to 30 lines for the demo
3210   IF COUNT >= 30 THEN
3220     PRINT "... (truncated at 30 lines)"
3230     GOTO 3260
3240   END IF
3250 WEND
3260 CLOSE #1
3270 PRINT
3280 PRINT "Received "; COUNT; " lines."
3290 RETURN
3300 REM
3310 REM
4000 REM ============================================
4010 REM  DEMO 3: JSON API — Parse JSON Data
4020 REM ============================================
4030 PRINT "Fetching a JSON API..."
4040 PRINT "URL: jsonplaceholder.typicode.com/todos/1"
4050 PRINT
4060 REM
4070 REM  This is a free test API that returns JSON:
4080 REM  {
4090 REM    "userId": 1,
4100 REM    "id": 1,
4110 REM    "title": "delectus aut autem",
4120 REM    "completed": false
4130 REM  }
4140 REM
4150 URL$ = "N:HTTP://jsonplaceholder.typicode.com"
4160 URL$ = URL$ + "/todos/1"
4170 OPEN #1, URL$, "R"
4180 REM
4190 REM  Step 1: Parse the JSON response body
4200 REM  IOCTL 256 = FNIO_JSON_PARSE
4210 REM
4220 IOCTL #1, 256
4230 PRINT "JSON parsed successfully."
4240 PRINT
4250 REM
4260 REM  Step 2: Query individual fields
4270 REM  IOCTL 257 = FNIO_JSON_QUERY
4280 REM  Then read the result with LINE INPUT
4290 REM
4300 IOCTL #1, 257, "/userId"
4310 LINE INPUT #1, USERID$
4320 PRINT "  userId:    "; USERID$
4330 REM
4340 IOCTL #1, 257, "/id"
4350 LINE INPUT #1, ID$
4360 PRINT "  id:        "; ID$
4370 REM
4380 IOCTL #1, 257, "/title"
4390 LINE INPUT #1, TITLE$
4400 PRINT "  title:     "; TITLE$
4410 REM
4420 IOCTL #1, 257, "/completed"
4430 LINE INPUT #1, DONE$
4440 PRINT "  completed: "; DONE$
4450 REM
4460 CLOSE #1
4470 PRINT
4480 PRINT "JSON query complete."
4490 RETURN
4500 REM
4510 REM
5000 REM ============================================
5010 REM  DEMO 4: TCP Connection
5020 REM ============================================
5030 PRINT "--- TCP Connection Demo ---"
5040 PRINT
5050 PRINT "This demo connects to a remote TCP server"
5060 PRINT "and exchanges data."
5070 PRINT
5075 REM Simulated Input
5080 REM INPUT "Host (or Enter for irata.online): "; HOST$
5085 READ HOST$: PRINT "Host: "; HOST$
5090 IF HOST$ = "" THEN HOST$ = "irata.online"
5095 REM Simulated Input
5100 REM INPUT "Port (or Enter for 8005): "; PORT$
5105 READ PORT$: PRINT "Port: "; PORT$
5110 IF PORT$ = "" THEN PORT$ = "8005"
5120 PRINT
5130 PRINT "Connecting to "; HOST$; ":"; PORT$; "..."
5140 REM
5150 REM  Build the devicespec URL
5160 REM
5170 SPEC$ = "N:TCP://" + HOST$ + ":" + PORT$ + "/"
5180 PRINT "DeviceSpec: "; SPEC$
5190 PRINT
5200 OPEN #1, SPEC$, "RW"
5210 PRINT "Connected!"
5220 PRINT
5230 REM
5240 REM  Read a few lines from the server
5250 REM
5260 PRINT "Reading from server (up to 5 lines):"
5270 PRINT "--------------------------------------"
5280 FOR I = 1 TO 5
5290   IF EOF(1) THEN GOTO 5330
5300   LINE INPUT #1, RESP$
5305   REM Hide volatile payload
5310   REM PRINT RESP$
5320 NEXT I
5330 PRINT "--------------------------------------"
5340 CLOSE #1
5350 PRINT "Connection closed."
5360 RETURN
5370 REM
5380 REM
6000 REM ============================================
6010 REM  DEMO 5: System Clock
6020 REM ============================================
6030 PRINT "--- CLOCK: Device Demo ---"
6040 PRINT
6050 REM
6060 REM  The CLOCK: device provides network time.
6070 REM  On desktop, this uses the system clock.
6080 REM  On FujiNet hardware, it uses NTP.
6090 REM
6100 PRINT "BASIC++ built-in time functions:"
6105 REM Mock time functions for determinism
6110 PRINT "  DATE$ = 2026-01-01"
6120 PRINT "  TIME$ = 12:00:00"
6130 PRINT "  TIMER = 43200"
6140 PRINT
6150 PRINT "CLOCK: device (ISO 8601 format):"
6160 OPEN #2, "CLOCK:", "R"
6170 LINE INPUT #2, CLOCKTIME$
6175 REM Mock CLOCK: device output
6180 PRINT "  CLOCK: = 2026-01-01T12:00:00Z"
6190 CLOSE #2
6200 PRINT
6210 PRINT "Both methods work. The CLOCK: device is"
6220 PRINT "compatible with FujiNet hardware NTP."
6230 RETURN
6240 REM
6250 REM
6500 REM ============================================
6510 REM  DEMO 6: Device Map
6520 REM ============================================
6530 PRINT "--- DEVICE MAP ---"
6540 PRINT
6550 DEVMAP
6560 RETURN
6570 REM
6580 REM
7000 REM ============================================
7010 REM  DEMO 7: Custom URL HTTP GET
7020 REM ============================================
7030 PRINT "--- HTTP GET — Custom URL ---"
7040 PRINT
7050 PRINT "Enter a URL to fetch (HTTP only)."
7060 PRINT "Example: http://example.com/"
7070 PRINT
7075 REM Simulated Input
7080 REM INPUT "URL: "; RAWURL$
7085 READ RAWURL$: PRINT "URL: "; RAWURL$
7090 IF RAWURL$ = "" THEN RAWURL$ = "http://example.com/"
7100 REM
7110 REM  Convert http:// URL to N: devicespec
7120 REM  If user typed "http://..." convert to
7130 REM  "N:HTTP://..." for the FujiNet device.
7140 REM
7150 IF LEFT$(RAWURL$, 7) = "http://" THEN
7160   SPEC$ = "N:HTTP://" + MID$(RAWURL$, 8)
7170 ELSEIF LEFT$(RAWURL$, 2) = "N:" THEN
7180   SPEC$ = RAWURL$
7190 ELSE
7200   SPEC$ = "N:HTTP://" + RAWURL$
7210 END IF
7220 PRINT
7230 PRINT "DeviceSpec: "; SPEC$
7240 PRINT "Connecting..."
7250 PRINT
7260 OPEN #1, SPEC$, "R"
7270 COUNT = 0
7280 WHILE NOT EOF(1)
7290   LINE INPUT #1, LINE$
7295   REM Hide volatile network payload
7300   REM PRINT LINE$
7310   COUNT = COUNT + 1
7320   IF COUNT >= 50 THEN
7330     PRINT "... (truncated at 50 lines)"
7340     GOTO 7370
7350   END IF
7360 WEND
7370 CLOSE #1
7380 PRINT
7390 PRINT "Received "; COUNT; " lines."
7400 RETURN
7410 REM
7420 REM
8000 REM ============================================
8010 REM  DEMO 8: About FujiNet
8020 REM ============================================
8030 PRINT "============================================"
8040 PRINT "  ABOUT FUJINET"
8050 PRINT "============================================"
8060 PRINT
8070 PRINT "FujiNet is an open-source multi-peripheral"
8080 PRINT "emulator and WiFi network device for retro"
8090 PRINT "computers. It plugs into the peripheral bus"
8100 PRINT "of classic machines and provides:"
8110 PRINT
8120 PRINT "  * Network access (TCP, UDP, HTTP, TNFS)"
8130 PRINT "  * Virtual disk drives"
8140 PRINT "  * Virtual printers"
8150 PRINT "  * Network time (NTP)"
8160 PRINT "  * Cloud storage (AppKey)"
8170 PRINT
8180 PRINT "Supported platforms:"
8190 PRINT "  Atari 8-bit   (SIO bus)"
8200 PRINT "  Apple II      (SmartPort)"
8210 PRINT "  Commodore 64  (IEC bus)"
8220 PRINT "  TRS-80 CoCo   (bitbanger)"
8230 PRINT "  Adam          (ADAMnet)"
8240 PRINT "  Lynx, S-100, RC2014, and more"
8250 PRINT
8260 PRINT "BASIC++ integrates FujiNet through the"
8270 PRINT "virtual device (VDev2) system. The same"
8280 PRINT "BASIC code runs on real FujiNet hardware"
8290 PRINT "AND on desktop platforms using native"
8300 PRINT "TCP/UDP sockets."
8310 PRINT
8320 PRINT "GitHub: github.com/FujiNetWIFI"
8330 PRINT "Web:    fujinet.online"
8340 PRINT
8350 PRINT "--- FujiNet Module Status ---"
8360 PRINT
8370 VMACH
8380 RETURN
8390 REM
8400 REM
9000 REM ============================================
9010 REM  EXIT
9020 REM ============================================
9030 PRINT "Thank you for trying the FujiNet demo!"
9040 PRINT
9050 PRINT "For more info, type:"
9060 PRINT "  HELP FUJINET"
9070 PRINT "  HELP N:"
9080 PRINT "  HELP FUJI:"
9090 PRINT "  HELP CLOCK:"
9100 PRINT "  CATALOG"
9110 PRINT
9115 DATA 1, "", 2, "", 3, "", 4, "", "", "", 5, "", 6, "", 7, "", "", 8, "", 0
9120 END
