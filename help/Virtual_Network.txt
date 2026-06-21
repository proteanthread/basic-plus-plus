THE BASIC++ VIRTUAL NETWORK
=============================
Version 4.0.1

This manual explains how BASIC++ programs access networks —
local area networks, the Internet, and every protocol from
raw TCP sockets through high-level services like HTTP, FTP,
SSH, Telnet, IRC, Usenet, and e-mail.

Unlike a quick reference, this manual gives you deep
understanding.  By the time you finish reading it, you will
know:

  - How a BASIC "NET OPEN" reaches the wire
  - What every layer does and why it's there
  - How security sandboxing protects network access
  - How to open TCP and UDP connections
  - How to use every supported application protocol
  - How to talk to Ethernet, Wi-Fi, and virtual adapters
  - How to connect to legacy services like PLATO/IRATA
  - How to write safe, robust networked BASIC programs
  - How to debug network problems when something goes wrong


TABLE OF CONTENTS
=================

  Part I:   How It Works (Architecture)
  1.  The Journey of a Packet
  2.  Layer 1: The BASIC Statement
  3.  Layer 2: The VM Dispatcher
  4.  Layer 3: The Security Gate
  5.  Layer 4: The Network Channel Table
  6.  Layer 5: The Socket Abstraction
  7.  Layer 6: The Operating System Network Stack
  8.  Why These Layers Exist

  Part II:  Network Adapters
  9.  Adapter Discovery and Selection
      9.1  NET ADAPTER LIST
      9.2  Ethernet Adapters
      9.3  Wi-Fi Adapters
      9.4  Virtual and Loopback Adapters
      9.5  Adapter Properties
      9.6  Adapter Events
  10. Wi-Fi Operations
      10.1 NET WIFI SCAN
      10.2 NET WIFI JOIN
      10.3 NET WIFI STATUS
      10.4 NET WIFI LEAVE
      10.5 Signal Strength and Channel Info

  Part III: Core Networking
  11. TCP Connections
      11.1 NET OPEN (TCP Client)
      11.2 NET LISTEN (TCP Server)
      11.3 NET ACCEPT
      11.4 NET SEND / NET RECV
      11.5 NET CLOSE
      11.6 Connection States
      11.7 Timeouts and Keep-Alives
  12. UDP Datagrams
      12.1 NET OPEN (UDP Mode)
      12.2 NET SENDTO / NET RECVFROM
      12.3 Broadcast and Multicast
  13. DNS Resolution
      13.1 NET RESOLVE
      13.2 Reverse Lookup
      13.3 DNS Caching
  14. TLS / SSL Encryption
      14.1 NET OPEN ... SECURE
      14.2 Certificate Verification
      14.3 TLS Versions
      14.4 Client Certificates

  Part IV:  Application Protocols — Telnet, SSH, FTP
  15. Telnet
      15.1 NET TELNET OPEN
      15.2 Telnet Negotiation
      15.3 Raw Telnet (Manual IAC Handling)
      15.4 Building a Telnet Server
  16. SSH (Secure Shell)
      16.1 NET SSH OPEN
      16.2 SSH Host Key Verification
      16.3 SSH Shell (Interactive)
      16.4 SSH EXEC (Run Remote Commands)
      16.5 SSH Port Forwarding (Tunneling)
      16.6 SCP / SFTP (Secure File Transfer)
  17. FTP (File Transfer Protocol)
      17.1 NET FTP OPEN
      17.2 FTP Navigation
      17.3 FTP File Transfer
      17.4 FTP File Management
      17.5 FTP Transfer Progress
      17.6 FTP Close

  Part V:   Communication Protocols — IRC, NNTP, E-Mail
  18. IRC (Internet Relay Chat)
      18.1 NET IRC OPEN
      18.2 IRC Channel Operations
      18.3 Receiving IRC Messages
      18.4 IRC Bot Example
      18.5 Raw IRC Protocol
  19. NNTP (Usenet / Network News)
      19.1 NET NNTP OPEN
      19.2 Browsing Newsgroups
      19.3 Reading Articles
      19.4 Posting Articles
      19.5 NNTP Close
      19.6 Usenet Newsreader Example
  20. E-Mail (SMTP, POP3, IMAP)
      20.1 SMTP (Sending E-Mail)
           20.1.1 NET SMTP OPEN
           20.1.2 Sending a Simple Message
           20.1.3 Multiple Recipients and CC/BCC
           20.1.4 Attachments
      20.2 POP3 (Receiving E-Mail)
           20.2.1 NET POP3 OPEN
           20.2.2 Reading Messages
           20.2.3 Deleting Messages
      20.3 IMAP (Advanced E-Mail Access)
           20.3.1 NET IMAP OPEN
           20.3.2 Folder Operations
           20.3.3 Searching and Reading
           20.3.4 Message Management

  Part VI:  HTTP / HTTPS and the World Wide Web
  21. HTTP Requests
      21.1 NET HTTP GET
      21.2 NET HTTP POST
      21.3 NET HTTP PUT / DELETE / PATCH
      21.4 HTTP Headers
      21.5 Cookies
      21.6 File Uploads (Multipart)
      21.7 REST API Client Example
  22. WebSockets
      22.1 NET WS OPEN
      22.2 Sending and Receiving Messages
      22.3 WebSocket Events
      22.4 WebSocket Close

  Part VII: Connecting to IRATA.ONLINE
  23. The PLATO Protocol
      23.1 Protocol Basics
      23.2 PLATO Screen Coordinates
      23.3 Connecting to IRATA.ONLINE
      23.4 Processing PLATO Bytes
      23.5 Keyboard Mapping
      23.6 Screen Rendering
      23.7 Character Drawing
      23.8 IRATA.ONLINE Features

  Part VIII: Network Security
  24. The Security Gate
      24.1 CAP_NETWORK Capability
      24.2 The Security Gate Checks
      24.3 Network Firewall Rules
      24.4 Sandboxing
      24.5 Rate Limiting
      24.6 Input Validation
      24.7 Credential Management

  Part IX:  Error Reference and Complete Example
  25. Network Error Codes
  26. Network Utility Functions
      26.1 String Utilities
      26.2 Address Utilities
      26.3 Network Diagnostics
  27. Complete Example: Multi-Protocol Client
  28. Keyword Quick Reference
  29. Summary


======================================================================
Part I:  HOW IT WORKS
======================================================================


1.  THE JOURNEY OF A PACKET (Architecture)
-------------------------------------------

When your BASIC++ program executes a network statement,
the data passes through six layers before reaching the
wire.  This architecture mirrors the virtual filesystem
(see W_Virtual_Filesystem.txt) but is tuned for the
unique requirements of network I/O: asynchronous delivery,
protocol negotiation, and security enforcement.

  Your BASIC Program
       |
       v
  [Layer 1]  BASIC Statement Parser
       |           Parses NET OPEN, NET SEND, etc.
       v
  [Layer 2]  VM Dispatcher
       |           Routes to the correct handler
       v
  [Layer 3]  Security Gate
       |           Checks CAP_NETWORK, host whitelist
       v
  [Layer 4]  Network Channel Table
       |           Manages open connections (channels)
       v
  [Layer 5]  Socket Abstraction
       |           Cross-platform BSD socket wrapper
       v
  [Layer 6]  Operating System
                   TCP/IP stack, NIC drivers, wire

Every byte your program sends travels down through all
six layers.  Every byte received travels back up.  Each
layer has a single responsibility and can be tested,
replaced, or extended independently.

This is the same "onion" architecture used by the virtual
filesystem and the virtual device layer.  If you have read
W_Virtual_Filesystem.txt or R_Virtual_Devices.txt, this
pattern will be familiar.


2.  LAYER 1: THE BASIC STATEMENT
---------------------------------

All network operations begin with a BASIC statement.
BASIC++ extends the classic BASIC vocabulary with the NET
keyword family:

  NET OPEN     - open a connection or listener
  NET CLOSE    - close a connection
  NET SEND     - send data on a connection
  NET RECV     - receive data from a connection
  NET STATUS   - query connection state
  NET RESOLVE  - DNS name resolution

Protocol-specific commands are prefixed with their
protocol name:

  TELNET, SSH, FTP, HTTP, IRC, NNTP, SMTP, POP3

These are not separate modules — they are built-in
keywords that the parser recognizes.  When the parser
encounters:

  100 NET OPEN "irata.online", 8005 AS #1

it produces a parse node of type PN_NET_OPEN with:

  .host     = "irata.online"
  .port     = 8005
  .channel  = 1
  .protocol = PROTO_RAW  (default)

The parse node is then passed to the VM dispatcher.


How Statement Parsing Works
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The tokenizer recognizes NET as a compound keyword prefix.
The second token determines which sub-handler runs:

  Token Stream            Parse Function
  ──────────────────────  ──────────────────────
  NET OPEN ...            parse_net_open()
  NET CLOSE ...           parse_net_close()
  NET SEND ...            parse_net_send()
  NET RECV ...            parse_net_recv()
  NET LISTEN ...          parse_net_listen()
  NET ACCEPT ...          parse_net_accept()
  NET ADAPTER ...         parse_net_adapter()
  NET WIFI ...            parse_net_wifi()
  NET RESOLVE ...         parse_net_resolve()
  NET GET ...             parse_net_http_get()
  NET POST ...            parse_net_http_post()
  NET REQUEST ...         parse_net_http_request()
  NET STATUS ...          parse_net_status()
  NET TRACE ...           parse_net_trace()

Protocol-specific keywords follow the same pattern:

  TELNET SEND ...         parse_telnet_send()
  SSH EXEC ...            parse_ssh_exec()
  FTP GET ...             parse_ftp_get()
  IRC JOIN ...            parse_irc_join()
  NNTP GROUP ...          parse_nntp_group()
  SMTP DATA ...           parse_smtp_data()
  POP3 RETR ...           parse_pop3_retr()


3.  LAYER 2: THE VM DISPATCHER
--------------------------------

The VM dispatcher receives the parse node and routes it
to the appropriate handler function.  Network handlers
are registered in the dispatch table alongside all other
VM operations:

  Opcode             Handler Function
  ─────────────────  ────────────────────────
  OP_NET_OPEN        vm_net_open()
  OP_NET_CLOSE       vm_net_close()
  OP_NET_SEND        vm_net_send()
  OP_NET_RECV        vm_net_recv()
  OP_NET_LISTEN      vm_net_listen()
  OP_NET_ACCEPT      vm_net_accept()
  OP_NET_STATUS      vm_net_status()
  OP_NET_RESOLVE     vm_net_resolve()
  ...

Each handler function:

  1. Validates arguments (host, port, channel, etc.)
  2. Calls the Security Gate (Layer 3)
  3. If permitted, calls the Network Channel Table (Layer 4)
  4. Returns a result code to the VM

The dispatcher also handles error mapping.  If any lower
layer returns an error code, the dispatcher translates it
into a BASIC++ error number that your ON ERROR GOTO
handler can catch:

  Error Code          BASIC++ ERR Number    Meaning
  ──────────────────  ──────────────────    ─────────────────────
  ENET_OK             (no error)            Success
  ENET_REFUSED        220                   Connection refused
  ENET_TIMEOUT        221                   Connection timed out
  ENET_HOSTNOTFOUND   222                   Host not found
  ENET_CONNRESET      223                   Connection reset
  ENET_ADDRINUSE      224                   Address already in use
  ENET_DENIED         225                   Permission denied
  ENET_NETDOWN        226                   Network is down
  ENET_TLSFAIL        227                   TLS handshake failed
  ENET_PROTOERR       228                   Protocol error
  ENET_BUFOVERFLOW    229                   Buffer overflow
  ENET_NOCHANNEL      230                   No free channels
  ENET_BADCHANNEL     231                   Invalid channel number


4.  LAYER 3: THE SECURITY GATE
--------------------------------

Before any network operation reaches the socket layer, it
must pass through the Security Gate.  This is the same
gate architecture used by the virtual filesystem (see
W_Virtual_Filesystem.txt, Section 4) but with network-
specific rules.

The Security Gate checks:

  1. CAP_NETWORK capability
     The running module must have CAP_NETWORK (0x0080u)
     in its capability flags.  If not, the operation is
     denied immediately with ENET_DENIED (ERR 225).

  2. Host whitelist
     If the interpreter was started with --net-allow,
     only connections to listed hosts are permitted.

       basicpp --net-allow "irata.online,example.com"

     If no --net-allow is specified, all hosts are
     allowed by default (but see blocked ports below).

  3. Port restrictions
     Certain ports are blocked by default to prevent
     accidental damage:

       Port 25   (SMTP)    - blocked unless --net-smtp
       Port 137-139        - blocked (NetBIOS)
       Port 445            - blocked (SMB)

     These can be overridden with --net-allow-port.

  4. Rate limiting
     By default, a program may open at most 8 simultaneous
     connections and send at most 64 KB/sec aggregate.
     These can be adjusted:

       basicpp --net-max-conn 16 --net-rate 256K

  5. Data size limits
     A single NET RECV buffer is limited to 64 KB by
     default.  Large transfers must use chunked reading.

The Security Gate logs every decision to the network audit
log if --net-log is enabled:

  basicpp --net-log network.log myprog.bas

The log records:

  [2026-06-08 12:00:01] ALLOW NET OPEN irata.online:8005 ch#1
  [2026-06-08 12:00:02] ALLOW NET SEND ch#1 45 bytes
  [2026-06-08 12:00:15] DENY  NET OPEN evil.example:445 (blocked port)


5.  LAYER 4: THE NETWORK CHANNEL TABLE
----------------------------------------

BASIC++ manages network connections through channel
numbers, just as it manages files.  Network channels and
file channels share the same numbering space (#1 through
#255), which means you cannot have a file and a network
connection on the same channel simultaneously.

The Network Channel Table is an array of NetChannel
structures:

  typedef struct {
      int           in_use;       /* 0 = free, 1 = active  */
      int           channel_num;  /* BASIC channel number   */
      int           socket_fd;    /* OS socket descriptor   */
      int           protocol;     /* PROTO_RAW, PROTO_TCP.. */
      int           app_proto;    /* APP_TELNET, APP_FTP..  */
      int           state;        /* connection state       */
      int           tls_active;   /* 1 = TLS enabled        */
      void         *tls_ctx;      /* TLS context pointer    */
      char          host[256];    /* remote host name       */
      int           port;         /* remote port number     */
      int           local_port;   /* local port (servers)   */
      unsigned long bytes_sent;   /* total bytes sent       */
      unsigned long bytes_recv;   /* total bytes received   */
      int           timeout_ms;   /* I/O timeout in ms      */
      int           blocking;     /* 1=blocking, 0=async    */
      char          recv_buf[65536]; /* receive buffer      */
      int           recv_len;     /* bytes in recv buffer   */
      int           last_error;   /* last error code        */
  } NetChannel;

When you write:

  100 NET OPEN "irata.online", 8005 AS #1

The system:

  1. Finds channel #1 in the table
  2. Verifies it's not already in use
  3. Creates a TCP socket
  4. Connects to irata.online:8005
  5. Fills in the NetChannel structure
  6. Sets state = NSTATE_CONNECTED

Channel States
~~~~~~~~~~~~~~~

  State               Meaning
  ──────────────────  ─────────────────────────────
  NSTATE_CLOSED       Channel is free
  NSTATE_RESOLVING    DNS lookup in progress
  NSTATE_CONNECTING   TCP handshake in progress
  NSTATE_CONNECTED    Connection established
  NSTATE_TLS_HAND     TLS handshake in progress
  NSTATE_READY        Connection ready (TLS done)
  NSTATE_LISTENING    Server socket listening
  NSTATE_CLOSING      Graceful shutdown in progress
  NSTATE_ERROR        Error occurred


6.  LAYER 5: THE SOCKET ABSTRACTION
--------------------------------------

The socket abstraction provides a uniform API across
operating systems.  BASIC++ uses BSD sockets on Unix/Linux
and WinSock2 on Windows, wrapped in a thin compatibility
layer:

  Function          Unix/Linux       Windows
  ────────────────  ──────────────   ──────────────
  sock_create()     socket()         socket()
  sock_connect()    connect()        connect()
  sock_bind()       bind()           bind()
  sock_listen()     listen()         listen()
  sock_accept()     accept()         accept()
  sock_send()       send()           send()
  sock_recv()       recv()           recv()
  sock_close()      close()          closesocket()
  sock_select()     select()         select()
  sock_error()      errno            WSAGetLastError()
  sock_init()       (no-op)          WSAStartup()
  sock_cleanup()    (no-op)          WSACleanup()

The abstraction handles:

  - Byte order conversion (htons, ntohs, htonl, ntohl)
  - Address resolution (getaddrinfo / freeaddrinfo)
  - Non-blocking mode (fcntl / ioctlsocket)
  - Socket options (setsockopt for timeouts, keep-alive)
  - Error code translation to ENET_* constants

TLS support is provided through an optional backend.
When BASIC++ is compiled with TLS support:

  - OpenSSL (libssl/libcrypto)  on Linux/macOS
  - SChannel                    on Windows
  - mbedTLS                     on embedded/Raspberry Pi

The TLS layer wraps sock_send/sock_recv transparently.
Once a connection is upgraded to TLS, all data passes
through the TLS engine automatically.


7.  LAYER 6: THE OPERATING SYSTEM NETWORK STACK
-------------------------------------------------

Below the socket abstraction is the operating system's
TCP/IP stack.  BASIC++ does not interact with this layer
directly — it relies on the OS to handle:

  - IP routing and forwarding
  - TCP flow control and retransmission
  - UDP datagram delivery
  - ARP / NDP neighbor discovery
  - DHCP address configuration
  - Firewall rules (iptables, Windows Firewall)

The OS also manages the physical and wireless network
adapters, which BASIC++ can query through the adapter
discovery API (see Section 9).


8.  WHY THESE LAYERS EXIST
-----------------------------

  Layer   Purpose               Without It
  ──────  ────────────────────  ────────────────────────────
  1       User syntax           No readable network code
  2       Dispatch + error map  No structured error handling
  3       Security enforcement  Programs can attack network
  4       Channel management    No way to track connections
  5       Portability           Code breaks across OS's
  6       Actual networking     No packets move

Each layer isolates one concern.  You can test the parser
without a network.  You can test security rules without
opening sockets.  You can swap TLS backends without
changing any BASIC code.


======================================================================
Part II:  NETWORK ADAPTERS
======================================================================


9.  ADAPTER DISCOVERY AND SELECTION
-------------------------------------

Before making connections, you may want to know what
network interfaces are available on the machine.  This is
especially important on systems with multiple adapters
(Ethernet + Wi-Fi), virtual machines, or embedded boards
with specific interface names.


9.1  NET ADAPTER LIST
~~~~~~~~~~~~~~~~~~~~~~

Lists all available network adapters:

  100 DIM A$(16)
  110 DIM IP$(16)
  120 COUNT% = 0
  130 NET ADAPTER LIST A$(), IP$(), COUNT%
  140 FOR I% = 1 TO COUNT%
  150   PRINT I%; ": "; A$(I%); " - "; IP$(I%)
  160 NEXT I%

Output on a typical laptop:

  1: eth0 - 192.168.1.100
  2: wlan0 - 192.168.1.101
  3: lo - 127.0.0.1

On Windows:

  1: Ethernet - 192.168.1.100
  2: Wi-Fi - 192.168.1.101
  3: Loopback - 127.0.0.1


9.2  Ethernet Adapters
~~~~~~~~~~~~~~~~~~~~~~~~

Ethernet adapters are wired connections.  They are always
available on desktops, most laptops, Raspberry Pi boards,
and servers.  Ethernet is preferred for reliable, high-
throughput connections.

To select a specific adapter for outgoing connections:

  100 NET ADAPTER USE "eth0"
  110 NET OPEN "irata.online", 8005 AS #1

This binds the socket to the specified interface.  If the
adapter is not available, ENET_NETDOWN (ERR 226) is
raised.

On systems with only one adapter, NET ADAPTER USE is
optional — the OS will select the default route.


9.3  Wi-Fi Adapters
~~~~~~~~~~~~~~~~~~~~~

Wi-Fi adapters are wireless interfaces.  BASIC++ provides
commands to scan for networks, join them, and monitor
signal strength.

Wi-Fi operations require CAP_NETWORK capability and may
additionally require OS-level permissions (e.g., root on
Linux for scanning).

  100 NET ADAPTER USE "wlan0"
  110 NET WIFI SCAN RESULTS$(), COUNT%
  120 FOR I% = 1 TO COUNT%
  130   PRINT RESULTS$(I%)
  140 NEXT I%

See Section 10 for full Wi-Fi operations.


9.4  Virtual and Loopback Adapters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The loopback adapter (127.0.0.1 / ::1) is always available
and is useful for testing networked programs locally
without a real network connection.

Virtual adapters (VPN tunnels, Docker bridges, etc.) appear
in the adapter list like any other interface.  BASIC++
makes no distinction — if the OS exposes it, you can use
it.

  100 REM Test a server locally
  110 NET LISTEN 8080 AS #1           : REM server
  120 NET OPEN "127.0.0.1", 8080 AS #2 : REM client
  130 NET SEND #2, "Hello, self!"
  140 NET RECV #1, BUF$
  150 PRINT BUF$                       : REM Hello, self!


9.5  Adapter Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

Query detailed information about an adapter:

  100 NET ADAPTER INFO "eth0", INFO$()
  110 PRINT "MAC:    "; INFO$(1)
  120 PRINT "IPv4:   "; INFO$(2)
  130 PRINT "IPv6:   "; INFO$(3)
  140 PRINT "Mask:   "; INFO$(4)
  150 PRINT "GW:     "; INFO$(5)
  160 PRINT "MTU:    "; INFO$(6)
  170 PRINT "Speed:  "; INFO$(7)
  180 PRINT "Status: "; INFO$(8)


9.6  Adapter Events
~~~~~~~~~~~~~~~~~~~~~

You can be notified when adapter state changes:

  100 ON NET ADAPTER EVENT GOSUB 500
  110 REM ... main program ...
  500 REM -- Adapter event handler --
  510 A$ = NET ADAPTER EVENT$
  520 PRINT "Adapter event: "; A$
  530 RETURN

Events include:

  "LINK_UP"      - cable plugged in / Wi-Fi connected
  "LINK_DOWN"    - cable removed / Wi-Fi disconnected
  "ADDR_CHANGE"  - IP address changed (DHCP renewal)


10.  WI-FI OPERATIONS
-----------------------

BASIC++ provides a full Wi-Fi management API for systems
where the interpreter has appropriate OS permissions.


10.1  NET WIFI SCAN
~~~~~~~~~~~~~~~~~~~~~

Scan for available wireless networks:

  100 DIM SSID$(32), SIGNAL%(32), SEC$(32)
  110 COUNT% = 0
  120 NET WIFI SCAN SSID$(), SIGNAL%(), SEC$(), COUNT%
  130 PRINT "Available Networks:"
  140 PRINT "──────────────────────────────────────────"
  150 FOR I% = 1 TO COUNT%
  160   PRINT USING "  \              \  ###%  \       \"; _
              SSID$(I%); SIGNAL%(I%); SEC$(I%)
  170 NEXT I%

Output:

  Available Networks:
  ──────────────────────────────────────────
    HomeNetwork        87%  WPA2
    CoffeeShop         45%  Open
    OfficeWiFi5G       92%  WPA3


10.2  NET WIFI JOIN
~~~~~~~~~~~~~~~~~~~~~

Connect to a wireless network:

  100 NET WIFI JOIN "HomeNetwork", "MyPassword123"
  110 IF NET WIFI STATUS = 1 THEN
  120   PRINT "Connected to Wi-Fi!"
  130 ELSE
  140   PRINT "Connection failed: "; NET ERROR$
  150 END IF

For open networks (no password):

  100 NET WIFI JOIN "CoffeeShop"


10.3  NET WIFI STATUS
~~~~~~~~~~~~~~~~~~~~~~~

Check current Wi-Fi connection status:

  100 S% = NET WIFI STATUS
  110 IF S% = 0 THEN PRINT "Disconnected"
  120 IF S% = 1 THEN PRINT "Connected"
  130 IF S% = 2 THEN PRINT "Connecting..."
  140 IF S% = 3 THEN PRINT "Authentication failed"

  200 PRINT "SSID:     "; NET WIFI SSID$
  210 PRINT "Signal:   "; NET WIFI SIGNAL%; "%"
  220 PRINT "Channel:  "; NET WIFI CHANNEL%
  230 PRINT "Speed:    "; NET WIFI SPEED$


10.4  NET WIFI LEAVE
~~~~~~~~~~~~~~~~~~~~~~

Disconnect from the current wireless network:

  100 NET WIFI LEAVE
  110 PRINT "Disconnected from Wi-Fi."


10.5  Signal Strength and Channel Info
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For applications that need to monitor wireless quality:

  100 REM -- Wi-Fi signal monitor --
  110 CLS
  120 DO
  130   LOCATE 1, 1
  140   S% = NET WIFI SIGNAL%
  150   PRINT "Signal: ";
  160   FOR I% = 1 TO S% \ 10
  170     PRINT CHR$(219);   : REM block character
  180   NEXT I%
  190   FOR I% = S% \ 10 + 1 TO 10
  200     PRINT ".";
  210   NEXT I%
  220   PRINT " "; S%; "%  "
  230   SLEEP 1
  240 LOOP UNTIL INKEY$ <> ""


======================================================================
Part III:  CORE NETWORKING
======================================================================


11.  TCP CONNECTIONS
---------------------

TCP (Transmission Control Protocol) provides reliable,
ordered, error-checked delivery of a stream of bytes.
Most application protocols (HTTP, FTP, SSH, Telnet, IRC,
NNTP, SMTP, POP3) run on top of TCP.


11.1  NET OPEN (TCP Client)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open a TCP connection to a remote host:

  Syntax:
    NET OPEN host$, port% AS #channel%
    NET OPEN host$, port% AS #channel% TIMEOUT ms%
    NET OPEN host$, port% AS #channel% SECURE

  Parameters:
    host$       Host name or IP address
    port%       TCP port number (1-65535)
    channel%    Channel number (1-255)
    ms%         Connection timeout in milliseconds
    SECURE      Enable TLS encryption

  Examples:

  100 REM -- Simple TCP connection --
  110 NET OPEN "irata.online", 8005 AS #1
  120 PRINT "Connected to IRATA!"

  200 REM -- With timeout --
  210 NET OPEN "example.com", 80 AS #2 TIMEOUT 5000

  300 REM -- With TLS --
  310 NET OPEN "example.com", 443 AS #3 SECURE

  400 REM -- Error handling --
  410 ON ERROR GOTO 900
  420 NET OPEN "example.com", 80 AS #1
  430 PRINT "Connected!"
  440 GOTO 999
  900 IF ERR = 220 THEN PRINT "Connection refused"
  910 IF ERR = 221 THEN PRINT "Connection timed out"
  920 IF ERR = 222 THEN PRINT "Host not found"
  999 END


11.2  NET LISTEN (TCP Server)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Listen for incoming TCP connections:

  Syntax:
    NET LISTEN port% AS #channel%
    NET LISTEN port% AS #channel% BACKLOG n%

  Parameters:
    port%       Local port to listen on
    channel%    Channel for the listening socket
    n%          Maximum pending connections (default 5)

  Example:

  100 REM -- Simple echo server --
  110 NET LISTEN 7777 AS #1
  120 PRINT "Listening on port 7777..."
  130 NET ACCEPT #1 AS #2
  140 PRINT "Client connected!"
  150 DO
  160   NET RECV #2, BUF$
  170   IF LEN(BUF$) > 0 THEN
  180     PRINT "Received: "; BUF$
  190     NET SEND #2, BUF$
  200   END IF
  210   SLEEP 100
  220 LOOP UNTIL NET STATUS(#2) <> 3
  230 NET CLOSE #2
  240 PRINT "Client disconnected."
  250 GOTO 130


11.3  NET ACCEPT
~~~~~~~~~~~~~~~~~

Accept an incoming connection on a listening socket:

  Syntax:
    NET ACCEPT #listen_ch% AS #client_ch%
    NET ACCEPT #listen_ch% AS #client_ch% TIMEOUT ms%

  The listening channel remains open.  The new client
  connection is placed on client_ch%.

  To get information about the connecting client:

  100 NET ACCEPT #1 AS #2
  110 PRINT "Client IP: "; NET REMOTE$(#2)
  120 PRINT "Client port: "; NET REMOTEPORT%(#2)


11.4  NET SEND / NET RECV
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Send and receive data on an open TCP channel:

  Syntax:
    NET SEND #channel%, data$
    NET SEND #channel%, data$, bytes_sent%
    NET RECV #channel%, buffer$
    NET RECV #channel%, buffer$, max_bytes%
    NET RECV #channel%, buffer$, max_bytes%, actual%

  NET SEND transmits the contents of data$ to the remote
  end.  The optional bytes_sent% variable receives the
  number of bytes actually sent (may be less than
  LEN(data$) if the send buffer is full).

  NET RECV reads available data into buffer$.  If no data
  is available and the connection is blocking (default),
  it waits until data arrives or the timeout expires.

  Examples:

  100 REM -- Send a string --
  110 NET SEND #1, "Hello, world!" + CHR$(13) + CHR$(10)

  200 REM -- Receive up to 1024 bytes --
  210 NET RECV #1, BUF$, 1024, ACTUAL%
  220 PRINT "Received "; ACTUAL%; " bytes"
  230 PRINT BUF$

  300 REM -- Receive loop (read until done) --
  310 FULL$ = ""
  320 DO
  330   NET RECV #1, CHUNK$, 4096, N%
  340   FULL$ = FULL$ + CHUNK$
  350 LOOP UNTIL N% = 0
  360 PRINT "Total received: "; LEN(FULL$); " bytes"


11.5  NET CLOSE
~~~~~~~~~~~~~~~~~

Close a network connection:

  Syntax:
    NET CLOSE #channel%
    NET CLOSE ALL

  NET CLOSE performs a graceful TCP shutdown (FIN
  handshake).  Any unsent data in the buffer is flushed
  before closing.

  NET CLOSE ALL closes all open network channels.  This
  is automatically called when the program ends.

  Example:

  100 NET OPEN "example.com", 80 AS #1
  110 NET SEND #1, "GET / HTTP/1.0" + CHR$(13) + CHR$(10)
  120 NET SEND #1, CHR$(13) + CHR$(10)
  130 NET RECV #1, PAGE$
  140 PRINT PAGE$
  150 NET CLOSE #1


11.6  Connection States
~~~~~~~~~~~~~~~~~~~~~~~~~

Query the state of a connection:

  100 S% = NET STATUS(#1)

  Value   Constant            Meaning
  ──────  ──────────────────  ──────────────────────────
  0       NSTATE_CLOSED       Not connected
  1       NSTATE_RESOLVING    DNS lookup in progress
  2       NSTATE_CONNECTING   TCP handshake in progress
  3       NSTATE_CONNECTED    Connection established
  4       NSTATE_TLS_HAND     TLS handshake in progress
  5       NSTATE_READY        Fully ready (TLS complete)
  6       NSTATE_LISTENING    Server socket listening
  7       NSTATE_CLOSING      Shutdown in progress
  8       NSTATE_ERROR        Error state

  Example:

  100 NET OPEN "example.com", 80 AS #1
  110 DO
  120   S% = NET STATUS(#1)
  130   IF S% = 1 THEN PRINT "Resolving..."
  140   IF S% = 2 THEN PRINT "Connecting..."
  150   SLEEP 100
  160 LOOP UNTIL S% >= 3
  170 IF S% = 8 THEN
  180   PRINT "Error: "; NET ERROR$(#1)
  190 ELSE
  200   PRINT "Connected!"
  210 END IF


11.7  Timeouts and Keep-Alives
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configure connection behavior:

  100 REM -- Set recv timeout to 10 seconds --
  110 NET TIMEOUT #1, 10000

  200 REM -- Enable TCP keep-alive --
  210 NET KEEPALIVE #1, ON

  300 REM -- Set send buffer size --
  310 NET OPTION #1, "SNDBUF", 32768

  400 REM -- Set receive buffer size --
  410 NET OPTION #1, "RCVBUF", 65536

  500 REM -- Disable Nagle algorithm (low latency) --
  510 NET OPTION #1, "NODELAY", 1

If a NET RECV times out, ENET_TIMEOUT (ERR 221) is
raised.  You should always set a reasonable timeout to
prevent your program from hanging:

  100 ON ERROR GOTO 900
  110 NET TIMEOUT #1, 5000
  120 NET RECV #1, BUF$
  130 PRINT BUF$
  140 GOTO 999
  900 IF ERR = 221 THEN
  910   PRINT "Timed out waiting for data"
  920 END IF
  999 END


12.  UDP DATAGRAMS
--------------------

UDP (User Datagram Protocol) provides connectionless,
unreliable delivery of individual datagrams.  It is
faster than TCP but does not guarantee delivery or
ordering.  UDP is used for DNS queries, game networking,
streaming, and other latency-sensitive applications.


12.1  NET OPEN (UDP Mode)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open a UDP socket:

  Syntax:
    NET OPEN UDP AS #channel%
    NET OPEN UDP port% AS #channel%

  Without a port, the OS assigns a random local port
  (for sending only).  With a port, the socket is bound
  to that port for receiving.

  Examples:

  100 REM -- UDP sender --
  110 NET OPEN UDP AS #1
  120 NET SENDTO #1, "Hello!", "192.168.1.50", 5000

  200 REM -- UDP receiver --
  210 NET OPEN UDP 5000 AS #2
  220 NET RECVFROM #2, BUF$, FROM$, FROMPORT%
  230 PRINT "From "; FROM$; ":"; FROMPORT%; " - "; BUF$


12.2  NET SENDTO / NET RECVFROM
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Send and receive UDP datagrams:

  Syntax:
    NET SENDTO #ch%, data$, host$, port%
    NET RECVFROM #ch%, buffer$, from_host$, from_port%
    NET RECVFROM #ch%, buffer$, from_host$, from_port%, max%, actual%

  Each NET SENDTO sends a single datagram.  Each
  NET RECVFROM receives a single datagram.

  Example: UDP ping-pong

  100 REM -- UDP Ping --
  110 NET OPEN UDP AS #1
  120 NET SENDTO #1, "PING", "127.0.0.1", 6000
  130 NET RECVFROM #1, REPLY$, FROM$, FP%
  140 PRINT "Got: "; REPLY$

  200 REM -- UDP Pong (separate program) --
  210 NET OPEN UDP 6000 AS #1
  220 NET RECVFROM #1, MSG$, FROM$, FP%
  230 PRINT "Ping from "; FROM$
  240 NET SENDTO #1, "PONG", FROM$, FP%


12.3  Broadcast and Multicast
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Send to all devices on the local network:

  100 REM -- UDP broadcast --
  110 NET OPEN UDP AS #1
  120 NET OPTION #1, "BROADCAST", 1
  130 NET SENDTO #1, "Anyone there?", "255.255.255.255", 9999

Join a multicast group:

  100 REM -- Multicast receiver --
  110 NET OPEN UDP 5000 AS #1
  120 NET OPTION #1, "JOIN_MULTICAST", "239.1.2.3"
  130 NET RECVFROM #1, BUF$, FROM$, FP%
  140 PRINT "Multicast: "; BUF$


13.  DNS RESOLUTION
---------------------

DNS (Domain Name System) converts human-readable host
names into IP addresses.


13.1  NET RESOLVE
~~~~~~~~~~~~~~~~~~

Resolve a hostname to an IP address:

  Syntax:
    NET RESOLVE hostname$, ip$
    NET RESOLVE hostname$, ip$(), count%

  Single result:

  100 NET RESOLVE "irata.online", IP$
  110 PRINT "IP: "; IP$

  Multiple results (round-robin DNS):

  100 DIM IPS$(8)
  110 NET RESOLVE "example.com", IPS$(), COUNT%
  120 FOR I% = 1 TO COUNT%
  130   PRINT IPS$(I%)
  140 NEXT I%


13.2  Reverse Lookup
~~~~~~~~~~~~~~~~~~~~~~

Resolve an IP address back to a hostname:

  100 NET RESOLVE "93.184.216.34", HOST$, REVERSE
  110 PRINT "Hostname: "; HOST$


13.3  DNS Caching
~~~~~~~~~~~~~~~~~~~

BASIC++ caches DNS results for the duration of the
program to avoid redundant lookups.  You can control
the cache:

  100 NET DNS CACHE CLEAR          : REM flush cache
  110 NET DNS CACHE SIZE 64        : REM max entries
  120 NET DNS CACHE TTL 300        : REM seconds


14.  TLS / SSL ENCRYPTION
----------------------------

TLS (Transport Layer Security) encrypts network
connections to prevent eavesdropping and tampering.


14.1  NET OPEN ... SECURE
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The SECURE keyword enables TLS:

  100 NET OPEN "example.com", 443 AS #1 SECURE
  110 PRINT "Encrypted connection established"

This performs a full TLS handshake after the TCP
connection is established.  The connection is then
encrypted for all subsequent NET SEND / NET RECV calls.

You can also upgrade an existing connection to TLS:

  100 NET OPEN "smtp.example.com", 587 AS #1
  110 REM ... SMTP EHLO and STARTTLS command ...
  120 NET STARTTLS #1
  130 PRINT "Connection upgraded to TLS"


14.2  Certificate Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, BASIC++ verifies the server's TLS
certificate against the system's trusted CA store.

If verification fails, ENET_TLSFAIL (ERR 227) is raised.

For development and testing, you can disable verification
(NOT RECOMMENDED for production):

  100 NET OPTION #1, "TLS_VERIFY", 0
  110 NET OPEN "self-signed.example", 443 AS #1 SECURE

To specify a custom CA certificate:

  100 NET OPTION #0, "TLS_CA_FILE", "/path/to/ca.pem"
  110 NET OPEN "internal.example", 443 AS #1 SECURE


14.3  TLS Versions
~~~~~~~~~~~~~~~~~~~~

BASIC++ supports TLS 1.2 and TLS 1.3.  Older versions
(SSL 2.0, SSL 3.0, TLS 1.0, TLS 1.1) are disabled by
default for security.

To query the negotiated TLS version:

  100 NET OPEN "example.com", 443 AS #1 SECURE
  110 PRINT "TLS version: "; NET TLSVERSION$(#1)
  120 PRINT "Cipher:      "; NET TLSCIPHER$(#1)


14.4  Client Certificates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some servers require client certificates for mutual TLS:

  100 NET OPTION #0, "TLS_CERT_FILE", "/path/to/client.pem"
  110 NET OPTION #0, "TLS_KEY_FILE", "/path/to/client.key"
  120 NET OPEN "secure.example", 443 AS #1 SECURE


======================================================================
Part IV:  APPLICATION PROTOCOLS — TELNET, SSH, FTP
======================================================================

BASIC++ provides built-in support for classic remote-
access and file transfer protocols.  These are implemented
as high-level wrappers over the TCP/TLS primitives from
Part III, so you can always fall back to raw NET SEND /
NET RECV if you need lower-level control.


15.  TELNET
------------

Telnet is a simple, unencrypted protocol for interactive
text communication.  It is used for connecting to legacy
systems, BBSes, MUDs, and services like IRATA.ONLINE.

The Telnet protocol (RFC 854) adds a negotiation layer
on top of TCP: special byte sequences starting with IAC
(0xFF) are used to negotiate terminal options such as
echo mode, line mode, window size, and terminal type.


15.1  NET TELNET OPEN
~~~~~~~~~~~~~~~~~~~~~~~

Open a Telnet connection with automatic IAC negotiation:

  Syntax:
    NET TELNET OPEN host$, port% AS #channel%
    NET TELNET OPEN host$, port% AS #channel% TERM type$

  Parameters:
    host$       Remote hostname or IP address
    port%       Port number (standard Telnet is 23)
    channel%    Channel number
    type$       Terminal type to advertise (default "VT100")

  When you use NET TELNET OPEN instead of plain NET OPEN,
  BASIC++ automatically:

    1. Establishes the TCP connection
    2. Responds to IAC WILL/WONT/DO/DONT negotiations
    3. Advertises the specified terminal type
    4. Reports the terminal window size (NAWS)
    5. Enables local line editing if requested by server

  Example: Connect to IRATA.ONLINE

  100 REM ======================================
  110 REM  IRATA.ONLINE Telnet Client
  120 REM ======================================
  130 ON ERROR GOTO 900
  140 NET TELNET OPEN "irata.online", 8005 AS #1
  150 PRINT "Connected to IRATA.ONLINE!"
  160 PRINT "Press Ctrl+] to disconnect."
  170 PRINT "────────────────────────────────────"
  180 REM -- Main terminal loop --
  190 DO
  200   REM Check for incoming data
  210   NET RECV #1, BUF$, 1024, N%
  220   IF N% > 0 THEN
  230     REM Filter out IAC sequences (handled internally)
  240     PRINT NET TELNET STRIP$(BUF$);
  250   END IF
  260   REM Check for keyboard input
  270   K$ = INKEY$
  280   IF K$ = CHR$(29) THEN GOTO 800  : REM Ctrl+]
  290   IF LEN(K$) > 0 THEN
  300     NET SEND #1, K$
  310   END IF
  320   SLEEP 10
  330 LOOP
  800 REM -- Disconnect --
  810 NET CLOSE #1
  820 PRINT
  830 PRINT "Disconnected."
  840 END
  900 REM -- Error handler --
  910 PRINT "Error: "; ERR; " - "; ERROR$(ERR)
  920 IF ERR = 220 THEN PRINT "Connection refused."
  930 IF ERR = 222 THEN PRINT "Host not found."
  940 END


15.2  Telnet Negotiation
~~~~~~~~~~~~~~~~~~~~~~~~~~

BASIC++ handles Telnet option negotiation automatically.
The following options are supported:

  Option               Code   Behavior
  ───────────────────  ─────  ────────────────────────────
  ECHO                 1      Supports server-side echo
  SUPPRESS-GO-AHEAD    3      Always accepted
  STATUS               5      Reports active options
  TERMINAL-TYPE        24     Sends configured TERM type
  NAWS (Window Size)   31     Sends cols x rows
  TERMINAL-SPEED       32     Reports "38400,38400"
  NEW-ENVIRON          39     Sends environment vars
  CHARSET              42     Supports UTF-8 if available

You can override default behavior:

  100 REM Refuse echo (force local echo)
  110 NET TELNET OPTION #1, "ECHO", OFF

  200 REM Set custom terminal type
  210 NET TELNET OPTION #1, "TERM", "ANSI"

  300 REM Set window size
  310 NET TELNET OPTION #1, "NAWS", 80, 24


15.3  Raw Telnet (Manual IAC Handling)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you need full control over IAC negotiation (for
writing a Telnet server or a specialized client), use
raw mode:

  100 NET OPEN "example.com", 23 AS #1
  110 REM Now IAC sequences arrive as raw bytes
  120 NET RECV #1, BUF$, 1024, N%
  130 REM Parse IAC manually
  140 FOR I% = 1 TO N%
  150   B% = ASC(MID$(BUF$, I%, 1))
  160   IF B% = 255 THEN
  170     REM IAC byte - next byte is command
  180     I% = I% + 1
  190     CMD% = ASC(MID$(BUF$, I%, 1))
  200     I% = I% + 1
  210     OPT% = ASC(MID$(BUF$, I%, 1))
  220     PRINT "IAC "; CMD%; " "; OPT%
  230   ELSE
  240     PRINT CHR$(B%);
  250   END IF
  260 NEXT I%

  IAC command codes:
    240  SE    (Sub-negotiation End)
    241  NOP   (No Operation)
    242  DM    (Data Mark)
    243  BRK   (Break)
    244  IP    (Interrupt Process)
    245  AO    (Abort Output)
    246  AYT   (Are You There)
    247  EC    (Erase Character)
    248  EL    (Erase Line)
    249  GA    (Go Ahead)
    250  SB    (Sub-negotiation Begin)
    251  WILL
    252  WONT
    253  DO
    254  DONT
    255  IAC   (literal 0xFF)


15.4  Building a Telnet Server
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can create a Telnet server in BASIC++:

  100 REM ======================================
  110 REM  Simple Telnet BBS Server
  120 REM ======================================
  130 NET LISTEN 2323 AS #1
  140 PRINT "BBS listening on port 2323..."
  150 REM -- Accept a client --
  160 NET ACCEPT #1 AS #2
  170 PRINT "Client connected from "; NET REMOTE$(#2)
  180 REM -- Send welcome banner --
  190 CRLF$ = CHR$(13) + CHR$(10)
  200 NET SEND #2, "╔══════════════════════════════╗" + CRLF$
  210 NET SEND #2, "║   Welcome to BASIC++ BBS!    ║" + CRLF$
  220 NET SEND #2, "╚══════════════════════════════╝" + CRLF$
  230 NET SEND #2, CRLF$
  240 NET SEND #2, "Enter your name: "
  250 REM -- Read username --
  260 NAME$ = ""
  270 DO
  280   NET RECV #2, C$, 1, N%
  290   IF N% > 0 AND ASC(C$) <> 13 AND ASC(C$) <> 10 THEN
  300     NAME$ = NAME$ + C$
  310     NET SEND #2, C$  : REM echo back
  320   END IF
  330 LOOP UNTIL ASC(C$) = 13
  340 NET SEND #2, CRLF$ + "Hello, " + NAME$ + "!" + CRLF$
  350 REM -- Simple chat loop --
  360 DO
  370   NET SEND #2, "> "
  380   LINE$ = ""
  390   DO
  400     NET RECV #2, C$, 1, N%
  410     IF N% > 0 AND ASC(C$) <> 13 THEN
  420       LINE$ = LINE$ + C$
  430       NET SEND #2, C$
  440     END IF
  450   LOOP UNTIL ASC(C$) = 13 OR N% = 0
  460   NET SEND #2, CRLF$
  470   IF UCASE$(LINE$) = "QUIT" THEN GOTO 500
  480   NET SEND #2, "You said: " + LINE$ + CRLF$
  490 LOOP UNTIL NET STATUS(#2) <> 3
  500 NET SEND #2, "Goodbye, " + NAME$ + "!" + CRLF$
  510 NET CLOSE #2
  520 PRINT "Client disconnected."
  530 GOTO 160


16.  SSH (SECURE SHELL)
-------------------------

SSH provides encrypted remote shell access and secure
file transfer.  It replaces Telnet for any connection
where security matters.

BASIC++ implements SSH v2 (RFC 4253) using an embedded
library (libssh2 on desktop, mbedTLS on embedded).


16.1  NET SSH OPEN
~~~~~~~~~~~~~~~~~~~~

Open an SSH connection:

  Syntax:
    NET SSH OPEN host$, port% AS #channel%
    NET SSH OPEN host$, port% AS #channel% USER user$
    NET SSH OPEN host$, port% AS #channel% USER user$ PASS pass$
    NET SSH OPEN host$, port% AS #channel% USER user$ KEY keyfile$

  Parameters:
    host$       Remote hostname or IP
    port%       Port number (standard SSH is 22)
    channel%    Channel number
    user$       Username for authentication
    pass$       Password (password auth)
    keyfile$    Path to private key file (pubkey auth)

  Examples:

  100 REM -- SSH with password authentication --
  110 NET SSH OPEN "myserver.com", 22 AS #1 _
        USER "admin" PASS "s3cret"
  120 PRINT "SSH connected!"

  200 REM -- SSH with key file authentication --
  210 NET SSH OPEN "myserver.com", 22 AS #2 _
        USER "admin" KEY "/home/user/.ssh/id_rsa"

  300 REM -- SSH with passphrase-protected key --
  310 NET SSH OPEN "myserver.com", 22 AS #3 _
        USER "admin" KEY "/home/user/.ssh/id_ed25519"
  320 REM If key has passphrase, BASIC++ prompts for it


16.2  SSH Host Key Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On first connection, BASIC++ displays the server's
host key fingerprint and asks the user to verify it:

  SSH Host Key Fingerprint:
    SHA256:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  Accept this key? (Y/N)

Accepted keys are stored in ~/.basicpp/known_hosts.
Subsequent connections verify the stored key.

To skip verification (NOT RECOMMENDED):

  100 NET SSH OPTION "HOSTKEY_CHECK", OFF
  110 NET SSH OPEN "testserver", 22 AS #1 USER "test"


16.3  SSH Shell (Interactive)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After connecting, request a shell session:

  100 NET SSH OPEN "server.com", 22 AS #1 _
        USER "admin" PASS "secret"
  110 NET SSH SHELL #1
  120 REM -- Now channel #1 acts like a terminal --
  130 DO
  140   NET RECV #1, BUF$, 4096, N%
  150   IF N% > 0 THEN PRINT BUF$;
  160   K$ = INKEY$
  170   IF LEN(K$) > 0 THEN NET SEND #1, K$
  180   SLEEP 10
  190 LOOP UNTIL NET STATUS(#1) <> 3


16.4  SSH EXEC (Run Remote Commands)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Execute a single command on the remote server:

  Syntax:
    NET SSH EXEC #channel%, command$, output$
    NET SSH EXEC #channel%, command$, output$, exitcode%

  Examples:

  100 NET SSH OPEN "server.com", 22 AS #1 _
        USER "admin" PASS "secret"
  110 NET SSH EXEC #1, "ls -la /tmp", OUT$, RC%
  120 PRINT OUT$
  130 PRINT "Exit code: "; RC%

  200 REM -- Run multiple commands --
  210 NET SSH EXEC #1, "uname -a", OUT$
  220 PRINT "System: "; OUT$
  230 NET SSH EXEC #1, "df -h", OUT$
  240 PRINT "Disk: "; OUT$
  250 NET SSH EXEC #1, "free -m", OUT$
  260 PRINT "Memory: "; OUT$


16.5  SSH Port Forwarding (Tunneling)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create an SSH tunnel for secure access to remote services:

  Syntax:
    NET SSH TUNNEL #ch%, LOCAL lport%, remote$, rport%
    NET SSH TUNNEL #ch%, REMOTE rport%, local$, lport%

  Example: Forward local port 3306 to remote MySQL:

  100 NET SSH OPEN "jumphost.com", 22 AS #1 _
        USER "admin" KEY "~/.ssh/id_rsa"
  110 NET SSH TUNNEL #1, LOCAL 3306, "dbserver", 3306
  120 PRINT "Tunnel active: localhost:3306 -> dbserver:3306"
  130 REM Now connect to localhost:3306 for MySQL
  140 NET OPEN "127.0.0.1", 3306 AS #2
  150 REM ... MySQL protocol ...


16.6  SCP / SFTP (Secure File Transfer)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Transfer files over SSH:

  Syntax:
    NET SSH PUT #ch%, local_file$, remote_path$
    NET SSH GET #ch%, remote_path$, local_file$
    NET SSH DIR #ch%, remote_path$, listing$()

  Examples:

  100 REM -- Upload a file --
  110 NET SSH OPEN "server.com", 22 AS #1 USER "admin" PASS "s"
  120 NET SSH PUT #1, "C:\data\report.txt", "/tmp/report.txt"
  130 PRINT "Upload complete!"

  200 REM -- Download a file --
  210 NET SSH GET #1, "/var/log/syslog", "C:\data\syslog.txt"
  220 PRINT "Download complete!"

  300 REM -- List remote directory --
  310 DIM FILES$(256)
  320 NET SSH DIR #1, "/tmp", FILES$(), COUNT%
  330 FOR I% = 1 TO COUNT%
  340   PRINT FILES$(I%)
  350 NEXT I%


17.  FTP (FILE TRANSFER PROTOCOL)
------------------------------------

FTP is a classic protocol for transferring files between
systems.  BASIC++ supports both active and passive mode
FTP, and FTPS (FTP over TLS).


17.1  NET FTP OPEN
~~~~~~~~~~~~~~~~~~~~

Open an FTP connection:

  Syntax:
    NET FTP OPEN host$ AS #channel%
    NET FTP OPEN host$ AS #channel% USER user$ PASS pass$
    NET FTP OPEN host$ AS #channel% SECURE

  Without USER/PASS, anonymous login is attempted.
  SECURE enables FTPS (explicit TLS, AUTH TLS).

  Examples:

  100 REM -- Anonymous FTP --
  110 NET FTP OPEN "ftp.example.com" AS #1
  120 PRINT "FTP connected (anonymous)"

  200 REM -- Authenticated FTP --
  210 NET FTP OPEN "ftp.example.com" AS #2 _
        USER "myuser" PASS "mypass"

  300 REM -- FTPS (encrypted) --
  310 NET FTP OPEN "ftp.example.com" AS #3 SECURE _
        USER "myuser" PASS "mypass"


17.2  FTP Navigation
~~~~~~~~~~~~~~~~~~~~~~

Navigate the remote filesystem:

  100 REM -- Print working directory --
  110 NET FTP PWD #1, DIR$
  120 PRINT "Current dir: "; DIR$

  200 REM -- Change directory --
  210 NET FTP CD #1, "/pub/files"

  300 REM -- List files --
  310 DIM FILES$(256)
  320 NET FTP DIR #1, FILES$(), COUNT%
  330 FOR I% = 1 TO COUNT%
  340   PRINT FILES$(I%)
  350 NEXT I%

  400 REM -- Detailed listing (like ls -l) --
  410 DIM DETAIL$(256)
  420 NET FTP LIST #1, DETAIL$(), COUNT%
  430 FOR I% = 1 TO COUNT%
  440   PRINT DETAIL$(I%)
  450 NEXT I%


17.3  FTP File Transfer
~~~~~~~~~~~~~~~~~~~~~~~~~

Download and upload files:

  Syntax:
    NET FTP GET #ch%, remote_file$, local_file$
    NET FTP PUT #ch%, local_file$, remote_file$
    NET FTP GET #ch%, remote_file$, local_file$, BINARY
    NET FTP GET #ch%, remote_file$, local_file$, ASCII

  BINARY mode (default) transfers files byte-for-byte.
  ASCII mode converts line endings (CRLF <-> LF).

  Examples:

  100 REM -- Download a file --
  110 NET FTP OPEN "ftp.example.com" AS #1
  120 NET FTP GET #1, "/pub/readme.txt", "C:\data\readme.txt"
  130 PRINT "Downloaded!"

  200 REM -- Upload a file --
  210 NET FTP PUT #1, "C:\data\myfile.zip", "/uploads/myfile.zip"
  220 PRINT "Uploaded!"


17.4  FTP File Management
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Manage files on the remote server:

  100 REM -- Delete a file --
  110 NET FTP DELETE #1, "/tmp/old.txt"

  200 REM -- Rename a file --
  210 NET FTP RENAME #1, "/tmp/old.txt", "/tmp/new.txt"

  300 REM -- Create a directory --
  310 NET FTP MKDIR #1, "/tmp/newdir"

  400 REM -- Remove a directory --
  410 NET FTP RMDIR #1, "/tmp/olddir"

  500 REM -- Get file size --
  510 NET FTP SIZE #1, "/pub/archive.tar.gz", SZ&
  520 PRINT "File size: "; SZ&; " bytes"


17.5  FTP Transfer Progress
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Monitor transfer progress for large files:

  100 REM -- Download with progress callback --
  110 NET FTP OPEN "ftp.example.com" AS #1
  120 NET FTP GET #1, "/pub/bigfile.iso", "C:\data\big.iso"
  130 REM -- Poll progress --
  140 DO
  150   PCT% = NET FTP PROGRESS(#1)
  160   BYTES& = NET FTP TRANSFERRED(#1)
  170   LOCATE 1, 1
  180   PRINT USING "Progress: ###%  (###,###,### bytes)"; _
              PCT%; BYTES&
  190   SLEEP 500
  200 LOOP UNTIL PCT% >= 100
  210 PRINT
  220 PRINT "Transfer complete!"


17.6  FTP Close
~~~~~~~~~~~~~~~~~

Close the FTP session:

  100 NET FTP CLOSE #1
  110 PRINT "FTP session ended."


======================================================================
Part V:  COMMUNICATION PROTOCOLS — IRC, NNTP, E-MAIL
======================================================================

These protocols form the backbone of text-based
communication on the Internet.  BASIC++ provides
high-level wrappers for each, while still allowing
raw protocol access when needed.


18.  IRC (INTERNET RELAY CHAT)
--------------------------------

IRC (RFC 2812) is a real-time text messaging protocol
used for group chat (channels) and private messages.
It is still widely used in open-source communities,
retro-computing groups, and hobbyist networks.


18.1  NET IRC OPEN
~~~~~~~~~~~~~~~~~~~~

Connect to an IRC server:

  Syntax:
    NET IRC OPEN host$, port% AS #channel%
    NET IRC OPEN host$, port% AS #channel% SECURE
    NET IRC OPEN host$, port% AS #channel% NICK nick$
    NET IRC OPEN host$, port% AS #channel% NICK nick$ _
        USER user$ REAL realname$

  Parameters:
    host$       IRC server hostname
    port%       Port (6667 for plain, 6697 for TLS)
    channel%    BASIC++ channel number
    nick$       Desired nickname
    user$       Username (ident)
    realname$   Real name / description

  Example:

  100 REM ======================================
  110 REM  Connect to Libera.Chat IRC
  120 REM ======================================
  130 NET IRC OPEN "irc.libera.chat", 6697 AS #1 SECURE _
        NICK "BasicBot" USER "basicpp" REAL "BASIC++ User"
  140 PRINT "Connected to IRC!"


18.2  IRC Channel Operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Join and interact with IRC channels:

  100 REM -- Join a channel --
  110 NET IRC JOIN #1, "#retro-computing"

  200 REM -- Send a message to the channel --
  210 NET IRC MSG #1, "#retro-computing", "Hello from BASIC++!"

  300 REM -- Send a private message --
  310 NET IRC MSG #1, "someuser", "Hi there!"

  400 REM -- Leave a channel --
  410 NET IRC PART #1, "#retro-computing"
  420 NET IRC PART #1, "#retro-computing", "Goodbye!"

  500 REM -- Change nickname --
  510 NET IRC NICK #1, "NewNick"

  600 REM -- Set away status --
  610 NET IRC AWAY #1, "Be right back"
  620 NET IRC AWAY #1, ""  : REM clear away


18.3  Receiving IRC Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Process incoming IRC messages in a loop:

  100 REM ======================================
  110 REM  IRC Chat Client
  120 REM ======================================
  130 NET IRC OPEN "irc.libera.chat", 6697 AS #1 SECURE _
        NICK "BPPUser" USER "bpp" REAL "BASIC++ IRC"
  140 NET IRC JOIN #1, "#basicpp"
  150 PRINT "Joined #basicpp"
  160 PRINT "Type messages and press Enter."
  170 PRINT "Type /quit to exit."
  180 PRINT "────────────────────────────────────"
  190 REM -- Main loop --
  200 DO
  210   REM Check for incoming messages
  220   NET IRC POLL #1, TYPE%, FROM$, TARGET$, MSG$
  230   IF TYPE% > 0 THEN
  240     SELECT CASE TYPE%
  250       CASE 1  : REM PRIVMSG (channel or private)
  260         PRINT "<"; FROM$; "> "; MSG$
  270       CASE 2  : REM NOTICE
  280         PRINT "[Notice] "; FROM$; ": "; MSG$
  290       CASE 3  : REM JOIN
  300         PRINT "*** "; FROM$; " joined "; TARGET$
  310       CASE 4  : REM PART
  320         PRINT "*** "; FROM$; " left "; TARGET$
  330       CASE 5  : REM QUIT
  340         PRINT "*** "; FROM$; " quit ("; MSG$; ")"
  350       CASE 6  : REM NICK change
  360         PRINT "*** "; FROM$; " is now "; MSG$
  370       CASE 7  : REM KICK
  380         PRINT "*** "; FROM$; " kicked from "; TARGET$
  390       CASE 8  : REM MODE change
  400         PRINT "*** Mode "; TARGET$; " "; MSG$
  410       CASE 9  : REM TOPIC change
  420         PRINT "*** Topic: "; MSG$
  430     END SELECT
  440   END IF
  450   REM Check for user input
  460   K$ = INKEY$
  470   IF LEN(K$) > 0 THEN
  480     IF K$ = CHR$(13) THEN
  490       IF LEFT$(LINE$, 5) = "/quit" THEN GOTO 800
  500       IF LEFT$(LINE$, 5) = "/join" THEN
  510         NET IRC JOIN #1, MID$(LINE$, 7)
  520       ELSEIF LEFT$(LINE$, 5) = "/part" THEN
  530         NET IRC PART #1, MID$(LINE$, 7)
  540       ELSE
  550         NET IRC MSG #1, "#basicpp", LINE$
  560         PRINT "<BPPUser> "; LINE$
  570       END IF
  580       LINE$ = ""
  590     ELSE
  600       LINE$ = LINE$ + K$
  610     END IF
  620   END IF
  630   SLEEP 50
  640 LOOP
  800 NET IRC QUIT #1, "Goodbye from BASIC++!"
  810 NET CLOSE #1
  820 PRINT "Disconnected from IRC."
  830 END


18.4  IRC Bot Example
~~~~~~~~~~~~~~~~~~~~~~~~

A simple auto-response IRC bot:

  100 REM ======================================
  110 REM  Simple IRC Bot
  120 REM ======================================
  130 NET IRC OPEN "irc.libera.chat", 6697 AS #1 SECURE _
        NICK "BPPBot" USER "bot" REAL "BASIC++ Bot"
  140 NET IRC JOIN #1, "#test-channel"
  150 PRINT "Bot running..."
  160 DO
  170   NET IRC POLL #1, TYPE%, FROM$, TARGET$, MSG$
  180   IF TYPE% = 1 THEN
  190     REM Respond to commands
  200     IF MSG$ = "!hello" THEN
  210       NET IRC MSG #1, TARGET$, "Hello, " + FROM$ + "!"
  220     ELSEIF MSG$ = "!time" THEN
  230       NET IRC MSG #1, TARGET$, "Time: " + TIME$
  240     ELSEIF MSG$ = "!version" THEN
  250       NET IRC MSG #1, TARGET$, "BASIC++ v2.0"
  260     ELSEIF LEFT$(MSG$, 5) = "!calc" THEN
  270       REM Simple calculator
  280       EXPR$ = MID$(MSG$, 7)
  290       ON ERROR GOTO 350
  300       RESULT# = VAL(EXPR$)  : REM simplified
  310       NET IRC MSG #1, TARGET$, EXPR$ + " = " + STR$(RESULT#)
  320       GOTO 360
  330     END IF
  340   END IF
  350   IF ERR <> 0 THEN NET IRC MSG #1, TARGET$, "Error in expression"
  360   SLEEP 100
  370 LOOP


18.5  Raw IRC Protocol
~~~~~~~~~~~~~~~~~~~~~~~~

For full protocol control, send raw IRC commands:

  100 NET OPEN "irc.libera.chat", 6667 AS #1
  110 CRLF$ = CHR$(13) + CHR$(10)
  120 NET SEND #1, "NICK BasicUser" + CRLF$
  130 NET SEND #1, "USER bpp 0 * :BASIC++ User" + CRLF$
  140 REM -- Wait for welcome (001) --
  150 DO
  160   NET RECV #1, BUF$, 1024, N%
  170   IF N% > 0 THEN PRINT BUF$;
  180   IF INSTR(BUF$, "001") > 0 THEN EXIT DO
  190   REM Handle PING during registration
  200   IF LEFT$(BUF$, 4) = "PING" THEN
  210     NET SEND #1, "PONG" + MID$(BUF$, 5) + CRLF$
  220   END IF
  230 LOOP
  240 NET SEND #1, "JOIN #test" + CRLF$
  250 NET SEND #1, "PRIVMSG #test :Hello from raw IRC!" + CRLF$


19.  NNTP (USENET / NETWORK NEWS)
------------------------------------

NNTP (Network News Transfer Protocol, RFC 3977) provides
access to Usenet newsgroups — a worldwide distributed
discussion system.  Despite being one of the oldest
Internet services, Usenet remains active for technical
discussion, binary distribution, and archival.


19.1  NET NNTP OPEN
~~~~~~~~~~~~~~~~~~~~~

Connect to an NNTP server:

  Syntax:
    NET NNTP OPEN host$, port% AS #channel%
    NET NNTP OPEN host$, port% AS #channel% SECURE
    NET NNTP OPEN host$, port% AS #channel% _
        USER user$ PASS pass$

  Parameters:
    host$       NNTP server hostname
    port%       Port (119 plain, 563 TLS)
    channel%    BASIC++ channel number
    user$       Username (if required)
    pass$       Password (if required)

  Examples:

  100 REM -- Public NNTP server --
  110 NET NNTP OPEN "news.eternal-september.org", 119 AS #1 _
        USER "myuser" PASS "mypass"
  120 PRINT "Connected to Usenet!"

  200 REM -- Encrypted connection --
  210 NET NNTP OPEN "news.example.com", 563 AS #2 SECURE


19.2  Browsing Newsgroups
~~~~~~~~~~~~~~~~~~~~~~~~~~~

List and select newsgroups:

  100 REM -- List newsgroups matching a pattern --
  110 DIM GROUPS$(256)
  120 NET NNTP LIST #1, "comp.sys.*", GROUPS$(), COUNT%
  130 PRINT "Found "; COUNT%; " groups:"
  140 FOR I% = 1 TO COUNT%
  150   PRINT "  "; GROUPS$(I%)
  160 NEXT I%

  200 REM -- Select a newsgroup --
  210 NET NNTP GROUP #1, "comp.sys.cbm", _
        FIRST&, LAST&, TOTAL&
  220 PRINT "Group: comp.sys.cbm"
  230 PRINT "Articles: "; TOTAL&
  240 PRINT "Range: "; FIRST&; " - "; LAST&

  300 REM -- List all subscribed groups --
  310 DIM SUBS$(64)
  320 NET NNTP SUBSCRIPTIONS #1, SUBS$(), NSUBS%
  330 FOR I% = 1 TO NSUBS%
  340   PRINT SUBS$(I%)
  350 NEXT I%


19.3  Reading Articles
~~~~~~~~~~~~~~~~~~~~~~~~

Read Usenet articles:

  100 REM -- Read article headers --
  110 NET NNTP GROUP #1, "comp.sys.cbm", F&, L&, T&
  120 REM Read last 20 article headers
  130 DIM SUBJ$(20), AUTH$(20), DATE$(20), MSGID$(20)
  140 START& = L& - 19
  150 IF START& < F& THEN START& = F&
  160 NET NNTP HEADERS #1, START&, L&, _
        SUBJ$(), AUTH$(), DATE$(), MSGID$(), COUNT%
  170 PRINT "Recent articles in comp.sys.cbm:"
  180 PRINT "──────────────────────────────────────────"
  190 FOR I% = 1 TO COUNT%
  200   PRINT I%; ": "; SUBJ$(I%)
  210   PRINT "   From: "; AUTH$(I%)
  220   PRINT "   Date: "; DATE$(I%)
  230   PRINT
  240 NEXT I%

  300 REM -- Read a full article --
  310 INPUT "Read article #: ", N%
  320 NET NNTP ARTICLE #1, MSGID$(N%), BODY$
  330 PRINT "────────────────────────────────────"
  340 PRINT "Subject: "; SUBJ$(N%)
  350 PRINT "From:    "; AUTH$(N%)
  360 PRINT "Date:    "; DATE$(N%)
  370 PRINT "────────────────────────────────────"
  380 PRINT BODY$


19.4  Posting Articles
~~~~~~~~~~~~~~~~~~~~~~~~

Post a new article to a newsgroup:

  100 REM -- Post to comp.sys.cbm --
  110 NET NNTP GROUP #1, "comp.sys.cbm", F&, L&, T&
  120 SUBJECT$ = "Hello from BASIC++!"
  130 BODY$ = "This message was posted using BASIC++ " + _
             "networking." + CHR$(10) + CHR$(10) + _
             "BASIC++ supports NNTP natively." + CHR$(10) + _
             "-- " + CHR$(10) + _
             "Sent from BASIC++"
  140 NET NNTP POST #1, "comp.sys.cbm", SUBJECT$, BODY$
  150 PRINT "Article posted!"

  200 REM -- Reply to an article --
  210 NET NNTP REPLY #1, MSGID$(3), _
        "Re: " + SUBJ$(3), _
        "Great point!  I agree." + CHR$(10) + _
        "-- " + CHR$(10) + "Sent from BASIC++"
  220 PRINT "Reply posted!"


19.5  NNTP Close
~~~~~~~~~~~~~~~~~~

  100 NET NNTP CLOSE #1


19.6  Usenet Newsreader Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A simple text-mode newsreader:

  100 REM ======================================
  110 REM  BASIC++ Usenet Newsreader
  120 REM ======================================
  130 NET NNTP OPEN "news.eternal-september.org", 119 _
        AS #1 USER "user" PASS "pass"
  140 PRINT "╔══════════════════════════════════╗"
  150 PRINT "║   BASIC++ Usenet Newsreader      ║"
  160 PRINT "╚══════════════════════════════════╝"
  170 REM -- Group selection --
  180 DIM MYGROUPS$(5)
  190 MYGROUPS$(1) = "comp.sys.cbm"
  200 MYGROUPS$(2) = "comp.lang.basic"
  210 MYGROUPS$(3) = "alt.bbs"
  220 MYGROUPS$(4) = "comp.os.cpm"
  230 MYGROUPS$(5) = "rec.games.roguelike"
  240 PRINT
  250 PRINT "Newsgroups:"
  260 FOR I% = 1 TO 5
  270   NET NNTP GROUP #1, MYGROUPS$(I%), F&, L&, T&
  280   PRINT USING "  #. \                  \ (#### articles)"; _
              I%; MYGROUPS$(I%); T&
  290 NEXT I%
  300 PRINT
  310 INPUT "Select group (1-5): ", G%
  320 IF G% < 1 OR G% > 5 THEN END
  330 REM -- Show articles --
  340 NET NNTP GROUP #1, MYGROUPS$(G%), F&, L&, T&
  350 DIM SUBJ$(20), AUTH$(20), DT$(20), MID$(20)
  360 START& = L& - 19
  370 IF START& < F& THEN START& = F&
  380 NET NNTP HEADERS #1, START&, L&, _
        SUBJ$(), AUTH$(), DT$(), MID$(), NC%
  390 PRINT
  400 PRINT "Articles in "; MYGROUPS$(G%); ":"
  410 PRINT "────────────────────────────────────"
  420 FOR I% = 1 TO NC%
  430   PRINT USING "  ##. \                              \"; _
              I%; SUBJ$(I%)
  440 NEXT I%
  450 INPUT "Read article (0=quit): ", A%
  460 IF A% < 1 OR A% > NC% THEN GOTO 300
  470 NET NNTP ARTICLE #1, MID$(A%), BODY$
  480 CLS
  490 PRINT "Subject: "; SUBJ$(A%)
  500 PRINT "From:    "; AUTH$(A%)
  510 PRINT "Date:    "; DT$(A%)
  520 PRINT "────────────────────────────────────"
  530 PRINT BODY$
  540 PRINT "────────────────────────────────────"
  550 INPUT "Press Enter to continue...", DUMMY$
  560 GOTO 390


20.  E-MAIL (SMTP, POP3, IMAP)
---------------------------------

BASIC++ provides built-in support for the three core
e-mail protocols: SMTP for sending, POP3 and IMAP for
receiving.


20.1  SMTP (Sending E-Mail)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SMTP (Simple Mail Transfer Protocol) is used to send
e-mail messages.


20.1.1  NET SMTP OPEN

  Syntax:
    NET SMTP OPEN host$, port% AS #channel%
    NET SMTP OPEN host$, port% AS #channel% SECURE
    NET SMTP OPEN host$, port% AS #channel% _
        USER user$ PASS pass$

  Common port numbers:
    25     Unencrypted SMTP (often blocked by ISPs)
    465    SMTP over TLS (implicit)
    587    SMTP with STARTTLS (recommended)

  Example:

  100 NET SMTP OPEN "smtp.gmail.com", 587 AS #1 _
        USER "user@gmail.com" PASS "app-password"
  110 PRINT "SMTP connected!"

BASIC++ automatically handles:
  - EHLO greeting
  - STARTTLS upgrade (port 587)
  - AUTH LOGIN or AUTH PLAIN authentication
  - Proper QUIT on close


20.1.2  Sending a Simple Message

  100 REM ======================================
  110 REM  Send an e-mail
  120 REM ======================================
  130 NET SMTP OPEN "smtp.gmail.com", 587 AS #1 _
        USER "me@gmail.com" PASS "app-password"
  140 NET SMTP FROM #1, "me@gmail.com"
  150 NET SMTP TO #1, "friend@example.com"
  160 NET SMTP SUBJECT #1, "Hello from BASIC++!"
  170 NET SMTP BODY #1, _
        "Hi there!" + CHR$(10) + CHR$(10) + _
        "This e-mail was sent using BASIC++ networking." + _
        CHR$(10) + CHR$(10) + _
        "Cheers," + CHR$(10) + "BASIC++ User"
  180 NET SMTP SEND #1
  190 PRINT "E-mail sent!"
  200 NET SMTP CLOSE #1


20.1.3  Multiple Recipients and CC/BCC

  100 NET SMTP OPEN "smtp.example.com", 587 AS #1 _
        USER "me@example.com" PASS "pass"
  110 NET SMTP FROM #1, "me@example.com"
  120 NET SMTP TO #1, "alice@example.com"
  130 NET SMTP TO #1, "bob@example.com"
  140 NET SMTP CC #1, "charlie@example.com"
  150 NET SMTP BCC #1, "secret@example.com"
  160 NET SMTP SUBJECT #1, "Team Update"
  170 NET SMTP BODY #1, "See attached report."
  180 NET SMTP SEND #1


20.1.4  Attachments

  100 NET SMTP OPEN "smtp.example.com", 587 AS #1 _
        USER "me@example.com" PASS "pass"
  110 NET SMTP FROM #1, "me@example.com"
  120 NET SMTP TO #1, "boss@example.com"
  130 NET SMTP SUBJECT #1, "Monthly Report"
  140 NET SMTP BODY #1, "Please find the report attached."
  150 NET SMTP ATTACH #1, "C:\data\report.pdf"
  160 NET SMTP ATTACH #1, "C:\data\spreadsheet.xlsx"
  170 NET SMTP SEND #1
  180 PRINT "Sent with attachments!"


20.2  POP3 (Receiving E-Mail)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

POP3 (Post Office Protocol v3) downloads e-mail from
a server.  Messages are typically deleted from the server
after download.


20.2.1  NET POP3 OPEN

  Syntax:
    NET POP3 OPEN host$, port% AS #channel%
    NET POP3 OPEN host$, port% AS #channel% SECURE
    NET POP3 OPEN host$, port% AS #channel% _
        USER user$ PASS pass$

  Common ports:
    110    POP3 unencrypted
    995    POP3 over TLS


20.2.2  Reading Messages

  100 REM ======================================
  110 REM  Check e-mail via POP3
  120 REM ======================================
  130 NET POP3 OPEN "pop.gmail.com", 995 AS #1 SECURE _
        USER "me@gmail.com" PASS "app-password"
  140 NET POP3 STAT #1, MSGCOUNT%, TOTALSIZE&
  150 PRINT "You have "; MSGCOUNT%; " messages"
  160 PRINT "Total size: "; TOTALSIZE&; " bytes"
  170 PRINT "────────────────────────────────────"
  180 FOR I% = 1 TO MSGCOUNT%
  190   NET POP3 HEADER #1, I%, FROM$, SUBJ$, DATE$
  200   PRINT USING "  ##. \                              \"; _
              I%; SUBJ$
  210   PRINT "      From: "; FROM$
  220   PRINT "      Date: "; DATE$
  230 NEXT I%
  240 INPUT "Read message #: ", M%
  250 IF M% < 1 OR M% > MSGCOUNT% THEN END
  260 NET POP3 RETR #1, M%, BODY$
  270 PRINT "────────────────────────────────────"
  280 PRINT BODY$
  290 NET POP3 CLOSE #1


20.2.3  Deleting Messages

  100 REM Delete message #3 from server
  110 NET POP3 DELE #1, 3
  120 PRINT "Message marked for deletion"
  130 REM Deletions happen when you close:
  140 NET POP3 CLOSE #1


20.3  IMAP (Advanced E-Mail Access)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IMAP (Internet Message Access Protocol) provides richer
e-mail access than POP3: messages stay on the server,
folders are supported, and you can search/filter messages.


20.3.1  NET IMAP OPEN

  Syntax:
    NET IMAP OPEN host$, port% AS #channel% SECURE _
        USER user$ PASS pass$

  Common ports:
    143    IMAP unencrypted (with STARTTLS)
    993    IMAP over TLS


20.3.2  Folder Operations

  100 NET IMAP OPEN "imap.gmail.com", 993 AS #1 SECURE _
        USER "me@gmail.com" PASS "app-password"

  200 REM -- List folders --
  210 DIM FOLDERS$(64)
  220 NET IMAP LIST #1, FOLDERS$(), NFOLDERS%
  230 FOR I% = 1 TO NFOLDERS%
  240   PRINT FOLDERS$(I%)
  250 NEXT I%

  300 REM -- Select a folder --
  310 NET IMAP SELECT #1, "INBOX", TOTAL%, UNSEEN%
  320 PRINT "INBOX: "; TOTAL%; " messages, "; _
        UNSEEN%; " unread"


20.3.3  Searching and Reading

  100 REM -- Search for unread messages --
  110 DIM IDS&(256)
  120 NET IMAP SEARCH #1, "UNSEEN", IDS&(), COUNT%
  130 PRINT COUNT%; " unread messages"

  200 REM -- Fetch message headers --
  210 FOR I% = 1 TO COUNT%
  220   NET IMAP FETCH #1, IDS&(I%), "HEADER", HDR$
  230   PRINT "Message "; IDS&(I%); ":"
  240   PRINT HDR$
  250   PRINT "────────────────────────────────────"
  260 NEXT I%

  300 REM -- Fetch full message body --
  310 NET IMAP FETCH #1, IDS&(1), "BODY", BODY$
  320 PRINT BODY$

  400 REM -- Search by criteria --
  410 NET IMAP SEARCH #1, "FROM ""alice@example.com""", _
        IDS&(), COUNT%
  420 NET IMAP SEARCH #1, "SUBJECT ""BASIC++""", _
        IDS&(), COUNT%
  430 NET IMAP SEARCH #1, "SINCE 01-Jan-2025", _
        IDS&(), COUNT%


20.3.4  Message Management

  100 REM -- Mark message as read --
  110 NET IMAP FLAG #1, IDS&(1), "+", "\Seen"

  200 REM -- Star a message --
  210 NET IMAP FLAG #1, IDS&(1), "+", "\Flagged"

  300 REM -- Move to folder --
  310 NET IMAP MOVE #1, IDS&(1), "Archive"

  400 REM -- Delete message --
  410 NET IMAP FLAG #1, IDS&(1), "+", "\Deleted"
  420 NET IMAP EXPUNGE #1

  500 REM -- Close connection --
  510 NET IMAP CLOSE #1


======================================================================
Part VI:  HTTP / HTTPS AND THE WORLD WIDE WEB
======================================================================

HTTP (Hypertext Transfer Protocol) is the foundation of
the World Wide Web.  BASIC++ provides both low-level HTTP
primitives and a high-level HTTP client for common web
operations.


21.  HTTP REQUESTS
--------------------

21.1  NET HTTP GET
~~~~~~~~~~~~~~~~~~~~

Fetch a web page or API resource:

  Syntax:
    NET HTTP GET url$ AS #channel%
    NET HTTP GET url$, response$
    NET HTTP GET url$, response$, status%

  The first form opens a streaming channel.  The second
  and third forms perform a complete request and return
  the response body as a string.

  Examples:

  100 REM -- Simple GET request --
  110 NET HTTP GET "http://example.com", PAGE$
  120 PRINT PAGE$

  200 REM -- HTTPS (automatic TLS) --
  210 NET HTTP GET "https://api.example.com/data", JSON$, STATUS%
  220 IF STATUS% = 200 THEN
  230   PRINT "Response: "; JSON$
  240 ELSE
  250   PRINT "HTTP Error: "; STATUS%
  260 END IF

  300 REM -- Streaming response (large files) --
  310 NET HTTP GET "https://example.com/big.dat" AS #1
  320 OPEN "C:\data\big.dat" FOR OUTPUT AS #2
  330 DO
  340   NET RECV #1, CHUNK$, 8192, N%
  350   IF N% > 0 THEN PRINT #2, CHUNK$;
  360 LOOP UNTIL N% = 0
  370 CLOSE #2
  380 NET CLOSE #1
  390 PRINT "Download complete!"


21.2  NET HTTP POST
~~~~~~~~~~~~~~~~~~~~~

Send data to a web server:

  Syntax:
    NET HTTP POST url$, body$, response$
    NET HTTP POST url$, body$, response$, status%

  Examples:

  100 REM -- POST form data --
  110 BODY$ = "username=admin&password=secret"
  120 NET HTTP HEADER "Content-Type", _
        "application/x-www-form-urlencoded"
  130 NET HTTP POST "https://example.com/login", _
        BODY$, RESP$, STATUS%
  140 PRINT "Status: "; STATUS%
  150 PRINT RESP$

  200 REM -- POST JSON data --
  210 JSON$ = "{""name"":""BASIC++"",""version"":2}"
  220 NET HTTP HEADER "Content-Type", "application/json"
  230 NET HTTP POST "https://api.example.com/data", _
        JSON$, RESP$, STATUS%
  240 PRINT RESP$


21.3  NET HTTP PUT / DELETE / PATCH
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RESTful API operations:

  100 REM -- PUT (update resource) --
  110 JSON$ = "{""name"":""Updated""}"
  120 NET HTTP HEADER "Content-Type", "application/json"
  130 NET HTTP PUT "https://api.example.com/items/42", _
        JSON$, RESP$, STATUS%

  200 REM -- DELETE (remove resource) --
  210 NET HTTP DELETE "https://api.example.com/items/42", _
        RESP$, STATUS%

  300 REM -- PATCH (partial update) --
  310 JSON$ = "{""status"":""active""}"
  320 NET HTTP HEADER "Content-Type", "application/json"
  330 NET HTTP PATCH "https://api.example.com/items/42", _
        JSON$, RESP$, STATUS%


21.4  HTTP Headers
~~~~~~~~~~~~~~~~~~~~

Set and read HTTP headers:

  100 REM -- Set request headers --
  110 NET HTTP HEADER "Authorization", "Bearer mytoken123"
  120 NET HTTP HEADER "Accept", "application/json"
  130 NET HTTP HEADER "User-Agent", "BASIC++/2.0"
  140 NET HTTP GET "https://api.example.com/me", RESP$

  200 REM -- Read response headers --
  210 NET HTTP GET "https://example.com" AS #1
  220 H$ = NET HTTP RESPONSE_HEADER$(#1, "Content-Type")
  230 PRINT "Content-Type: "; H$
  240 L& = VAL(NET HTTP RESPONSE_HEADER$(#1, "Content-Length"))
  250 PRINT "Content-Length: "; L&
  260 NET CLOSE #1


21.5  Cookies
~~~~~~~~~~~~~~~

HTTP cookie management:

  100 REM -- Automatic cookie handling --
  110 NET HTTP COOKIE JAR ON
  120 NET HTTP GET "https://example.com/login", RESP$
  130 REM Cookies from Set-Cookie headers are stored
  140 NET HTTP GET "https://example.com/dashboard", RESP$
  150 REM Cookies are sent automatically

  200 REM -- Manual cookie management --
  210 NET HTTP HEADER "Cookie", "session=abc123; theme=dark"
  220 NET HTTP GET "https://example.com/page", RESP$

  300 REM -- Save/load cookies to file --
  310 NET HTTP COOKIE SAVE "C:\data\cookies.txt"
  320 NET HTTP COOKIE LOAD "C:\data\cookies.txt"


21.6  File Uploads (Multipart)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Upload files via HTTP:

  100 REM -- Upload a file --
  110 NET HTTP MULTIPART START
  120 NET HTTP MULTIPART FIELD "username", "admin"
  130 NET HTTP MULTIPART FILE "avatar", _
        "C:\data\photo.jpg", "image/jpeg"
  140 NET HTTP MULTIPART POST _
        "https://example.com/upload", RESP$, STATUS%
  150 PRINT "Upload status: "; STATUS%


21.7  REST API Client Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A complete REST API client:

  100 REM ======================================
  110 REM  REST API Client Example
  120 REM ======================================
  130 API$ = "https://jsonplaceholder.typicode.com"
  140 PRINT "REST API Demo"
  150 PRINT "────────────────────────────────────"
  160 REM -- GET all posts --
  170 NET HTTP GET API$ + "/posts", RESP$, STATUS%
  180 PRINT "GET /posts: Status "; STATUS%
  190 REM Parse first title (simplified)
  200 P% = INSTR(RESP$, """title""")
  210 IF P% > 0 THEN
  220   T% = INSTR(P% + 9, RESP$, """")
  230   PRINT "First title: "; MID$(RESP$, P% + 9, T% - P% - 9)
  240 END IF
  250 PRINT
  260 REM -- POST a new resource --
  270 JSON$ = "{""title"":""BASIC++ Post""," + _
             """body"":""Hello from BASIC++""," + _
             """userId"":1}"
  280 NET HTTP HEADER "Content-Type", "application/json"
  290 NET HTTP POST API$ + "/posts", JSON$, RESP$, STATUS%
  300 PRINT "POST /posts: Status "; STATUS%
  310 PRINT "Response: "; LEFT$(RESP$, 80)
  320 PRINT
  330 REM -- GET single resource --
  340 NET HTTP GET API$ + "/posts/1", RESP$, STATUS%
  350 PRINT "GET /posts/1: Status "; STATUS%
  360 PRINT "Response: "; LEFT$(RESP$, 80)


22.  WEBSOCKETS
-----------------

WebSockets provide full-duplex communication channels
over a single TCP connection.  They are used for real-time
applications such as chat, live data feeds, and gaming.


22.1  NET WS OPEN
~~~~~~~~~~~~~~~~~~~

Open a WebSocket connection:

  Syntax:
    NET WS OPEN url$ AS #channel%
    NET WS OPEN url$ AS #channel% PROTOCOL proto$

  The URL should use ws:// or wss:// (secure):

  100 NET WS OPEN "ws://echo.websocket.org" AS #1
  110 PRINT "WebSocket connected!"

  200 NET WS OPEN "wss://stream.example.com/live" AS #2
  210 PRINT "Secure WebSocket connected!"


22.2  Sending and Receiving Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Syntax:
    NET WS SEND #channel%, message$
    NET WS SEND #channel%, data$, BINARY
    NET WS RECV #channel%, message$
    NET WS RECV #channel%, message$, type%

  Message types:
    1 = Text message
    2 = Binary message
    8 = Close frame
    9 = Ping
    10 = Pong

  Example: WebSocket echo client

  100 NET WS OPEN "ws://echo.websocket.org" AS #1
  110 PRINT "Connected to WebSocket echo server"
  120 FOR I% = 1 TO 5
  130   MSG$ = "Message #" + STR$(I%)
  140   NET WS SEND #1, MSG$
  150   NET WS RECV #1, REPLY$
  160   PRINT "Sent: "; MSG$; "  Got: "; REPLY$
  170 NEXT I%
  180 NET WS CLOSE #1


22.3  WebSocket Events
~~~~~~~~~~~~~~~~~~~~~~~~

Event-driven WebSocket handling:

  100 NET WS OPEN "wss://stream.example.com" AS #1
  110 ON NET WS MESSAGE #1 GOSUB 500
  120 ON NET WS CLOSE #1 GOSUB 600
  130 ON NET WS ERROR #1 GOSUB 700
  140 REM -- Main program continues --
  150 DO
  160   SLEEP 100
  170 LOOP
  500 REM -- Message handler --
  510 NET WS RECV #1, MSG$
  520 PRINT "Received: "; MSG$
  530 RETURN
  600 REM -- Close handler --
  610 PRINT "WebSocket closed"
  620 RETURN
  700 REM -- Error handler --
  710 PRINT "WebSocket error: "; NET ERROR$(#1)
  720 RETURN


22.4  WebSocket Close
~~~~~~~~~~~~~~~~~~~~~~~

  Syntax:
    NET WS CLOSE #channel%
    NET WS CLOSE #channel%, code%, reason$

  100 NET WS CLOSE #1               : REM normal close
  110 NET WS CLOSE #1, 1000, "Done" : REM with code/reason


======================================================================
Part VII:  CONNECTING TO IRATA.ONLINE
======================================================================

IRATA.ONLINE is a modern PLATO (Programmed Logic for
Automatic Teaching Operations) system accessible over
the Internet.  Originally developed on the CDC mainframe
at the University of Illinois in the 1960s, PLATO was
the world's first online community — with chat rooms,
message boards, multi-player games, and e-mail, decades
before the World Wide Web.

IRATA.ONLINE (https://irata.online) is a modern
reimplementation that runs the classic PLATO software
on contemporary hardware, accessible via TCP on port
8005.

This section demonstrates how to build a full PLATO
terminal client in BASIC++.


23.  THE PLATO PROTOCOL
--------------------------

PLATO uses a custom binary protocol over TCP.  The
terminal communicates using a series of command bytes
that control text display, graphics rendering, touch
panel input, and terminal modes.


23.1  Protocol Basics
~~~~~~~~~~~~~~~~~~~~~~~

PLATO commands are byte sequences where:

  - Data bytes have bit 7 clear (0x00-0x7F)
  - Command bytes have bit 7 set (0x80-0xFF)

  Command    Hex   Meaning
  ─────────  ────  ──────────────────────────────────
  MODE       80    Set terminal mode
  ADDR       81    Set cursor address (X, Y)
  CHAR       82    Draw character at cursor
  SSF        84    Select Super/Sub Font
  EXT        88    Extended command
  LOADMEM    90    Load memory (font data)
  FG_COLOR   A0    Set foreground color
  BG_COLOR   A1    Set background color
  PAINT      A2    Paint/fill region
  LINE       A4    Draw line
  BLOCK      A8    Draw filled rectangle

Terminal modes:
  Mode 0 = Normal text (write mode)
  Mode 1 = Inverse (XOR mode)
  Mode 2 = Rewrite mode (overwrite)
  Mode 3 = Erase mode (clear)


23.2  PLATO Screen Coordinates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The PLATO screen is 512 x 512 pixels, with:
  - X: 0 (left) to 511 (right)
  - Y: 0 (bottom) to 511 (top)

Note: Y=0 is at the BOTTOM, unlike most modern systems
where Y=0 is at the top.

Character cells are 8 x 16 pixels, giving a text grid
of 64 columns x 32 rows.


23.3  Connecting to IRATA.ONLINE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  100 REM ======================================
  110 REM  IRATA.ONLINE PLATO Terminal
  120 REM ======================================
  130 REM
  140 REM This program implements a basic PLATO
  150 REM terminal emulator that connects to
  160 REM irata.online on port 8005.
  170 REM
  180 REM ======================================
  190 REM
  200 REM -- Constants --
  210 CONST PLATO_HOST$ = "irata.online"
  220 CONST PLATO_PORT% = 8005
  230 CONST SCREEN_W% = 512
  240 CONST SCREEN_H% = 512
  250 CONST CHAR_W% = 8
  260 CONST CHAR_H% = 16
  270 CONST COLS% = 64
  280 CONST ROWS% = 32
  290 REM
  300 REM -- Terminal state --
  310 DIM SCREEN%(SCREEN_W%, SCREEN_H%)   : REM pixel buffer
  320 DIM CHARSET%(128, 16)               : REM font data
  330 CURX% = 0 : CURY% = 511             : REM cursor pos
  340 MODE% = 0                            : REM write mode
  350 FG% = 7   : BG% = 0                 : REM colors
  360 FLOW% = 0                            : REM flow control
  370 REM
  380 REM -- Load default font --
  390 GOSUB 5000
  400 REM
  410 REM -- Connect --
  420 ON ERROR GOTO 9000
  430 PRINT "Connecting to "; PLATO_HOST$; ":"; PLATO_PORT%; "..."
  440 NET OPEN PLATO_HOST$, PLATO_PORT% AS #1 TIMEOUT 10000
  450 PRINT "Connected!"
  460 NET TIMEOUT #1, 100   : REM short timeout for polling
  470 REM
  480 REM ======================================
  490 REM  MAIN TERMINAL LOOP
  500 REM ======================================
  510 DO
  520   REM -- Receive data from PLATO --
  530   ON ERROR GOTO 600
  540   NET RECV #1, BUF$, 4096, N%
  550   IF N% > 0 THEN
  560     FOR I% = 1 TO N%
  570       B% = ASC(MID$(BUF$, I%, 1))
  580       GOSUB 2000   : REM process byte
  590     NEXT I%
  600   END IF
  610   ON ERROR GOTO 9000
  620   REM
  630   REM -- Check for keyboard input --
  640   K$ = INKEY$
  650   IF LEN(K$) > 0 THEN
  660     REM Map keyboard to PLATO keys
  670     GOSUB 3000
  680   END IF
  690   REM
  700   REM -- Redraw screen if needed --
  710   IF DIRTY% THEN
  720     GOSUB 4000   : REM render
  730     DIRTY% = 0
  740   END IF
  750   REM
  760   SLEEP 10
  770 LOOP UNTIL QUIT%
  780 REM
  790 NET CLOSE #1
  800 PRINT "Disconnected from IRATA.ONLINE."
  810 END


23.4  Processing PLATO Bytes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  2000 REM ======================================
  2010 REM  PROCESS INCOMING BYTE
  2020 REM  B% = byte value (0-255)
  2030 REM ======================================
  2040 IF B% AND 128 THEN
  2050   REM -- Command byte --
  2060   CMD% = B%
  2070   SELECT CASE CMD% AND &HF0
  2080     CASE &H80  : REM MODE command
  2090       MODE% = CMD% AND &H0F
  2100     CASE &H90  : REM LOADMEM (font data)
  2110       LOADSTATE% = 1
  2120       LOADADDR% = 0
  2130     CASE &HA0  : REM Color/Paint
  2140       IF CMD% = &HA0 THEN COLCMD% = 1  : REM FG next
  2150       IF CMD% = &HA1 THEN COLCMD% = 2  : REM BG next
  2160     CASE &HA4  : REM LINE
  2170       LINECMD% = 1
  2180       LINESTATE% = 0
  2190     CASE &HA8  : REM BLOCK
  2200       BLOCKCMD% = 1
  2210       BLOCKSTATE% = 0
  2220   END SELECT
  2230 ELSE
  2240   REM -- Data byte --
  2250   IF COLCMD% = 1 THEN
  2260     FG% = B% AND &H07
  2270     COLCMD% = 0
  2280   ELSEIF COLCMD% = 2 THEN
  2290     BG% = B% AND &H07
  2300     COLCMD% = 0
  2310   ELSEIF LOADSTATE% = 1 THEN
  2320     REM Store font data byte
  2330     GOSUB 2500
  2340   ELSEIF LINECMD% = 1 THEN
  2350     REM Accumulate line coordinates
  2360     GOSUB 2600
  2370   ELSEIF BLOCKCMD% = 1 THEN
  2380     REM Accumulate block coordinates
  2390     GOSUB 2700
  2400   ELSE
  2410     REM -- Display character --
  2420     CH% = B%
  2430     GOSUB 2800
  2440   END IF
  2450 END IF
  2460 RETURN


23.5  Keyboard Mapping
~~~~~~~~~~~~~~~~~~~~~~~~

  3000 REM ======================================
  3010 REM  MAP KEYBOARD INPUT TO PLATO KEYS
  3020 REM  K$ = key pressed
  3030 REM ======================================
  3040 KB% = ASC(K$)
  3050 SELECT CASE KB%
  3060   CASE 27    : REM ESC
  3070     QUIT% = 1
  3080     RETURN
  3090   CASE 13    : REM Enter -> PLATO NEXT
  3100     NET SEND #1, CHR$(13)
  3110   CASE 8     : REM Backspace -> PLATO ERASE
  3120     NET SEND #1, CHR$(8)
  3130   CASE 9     : REM Tab -> PLATO TAB
  3140     NET SEND #1, CHR$(9)
  3150   CASE 1     : REM Ctrl+A -> PLATO STOP
  3160     NET SEND #1, CHR$(1)
  3170   CASE ELSE
  3180     IF KB% >= 32 AND KB% <= 126 THEN
  3190       REM Printable ASCII -> send directly
  3200       NET SEND #1, K$
  3210     END IF
  3220 END SELECT
  3230 RETURN


23.6  Screen Rendering
~~~~~~~~~~~~~~~~~~~~~~~~

  4000 REM ======================================
  4010 REM  RENDER PLATO SCREEN
  4020 REM ======================================
  4030 REM This renders the 512x512 pixel buffer
  4040 REM to the BASIC++ graphics screen.
  4050 REM
  4060 REM On modern displays, we scale 1:1 or 2:1
  4070 SCREEN 12  : REM 640x480, 16 colors
  4080 REM (In a real implementation, you would blit
  4090 REM  the SCREEN% array to the display buffer
  4100 REM  using PUT or a custom VDev render call)
  4110 FOR Y% = 0 TO SCREEN_H% - 1
  4120   FOR X% = 0 TO SCREEN_W% - 1
  4130     IF SCREEN%(X%, Y%) <> 0 THEN
  4140       PSET (X%, SCREEN_H% - 1 - Y%), FG%
  4150     END IF
  4160   NEXT X%
  4170 NEXT Y%
  4180 RETURN


23.7  Character Drawing
~~~~~~~~~~~~~~~~~~~~~~~~~

  2800 REM ======================================
  2810 REM  DRAW CHARACTER AT CURSOR
  2820 REM  CH% = character code (0-127)
  2830 REM ======================================
  2840 CX% = CURX%
  2850 CY% = CURY%
  2860 FOR ROW% = 0 TO CHAR_H% - 1
  2870   BITS% = CHARSET%(CH%, ROW%)
  2880   FOR COL% = 0 TO CHAR_W% - 1
  2890     PX% = CX% + COL%
  2900     PY% = CY% - ROW%
  2910     IF PX% >= 0 AND PX% < SCREEN_W% AND _
             PY% >= 0 AND PY% < SCREEN_H% THEN
  2920       SELECT CASE MODE%
  2930         CASE 0  : REM Write
  2940           IF BITS% AND (128 >> COL%) THEN
  2950             SCREEN%(PX%, PY%) = FG%
  2960           END IF
  2970         CASE 1  : REM XOR
  2980           IF BITS% AND (128 >> COL%) THEN
  2990             SCREEN%(PX%, PY%) = SCREEN%(PX%, PY%) XOR FG%
  3000           END IF
  3010         CASE 2  : REM Rewrite
  3020           IF BITS% AND (128 >> COL%) THEN
  3030             SCREEN%(PX%, PY%) = FG%
  3040           ELSE
  3050             SCREEN%(PX%, PY%) = BG%
  3060           END IF
  3070         CASE 3  : REM Erase
  3080           IF BITS% AND (128 >> COL%) THEN
  3090             SCREEN%(PX%, PY%) = BG%
  3100           END IF
  3110       END SELECT
  3120     END IF
  3130   NEXT COL%
  3140 NEXT ROW%
  3150 REM -- Advance cursor --
  3160 CURX% = CURX% + CHAR_W%
  3170 IF CURX% >= SCREEN_W% THEN
  3180   CURX% = 0
  3190   CURY% = CURY% - CHAR_H%
  3200   IF CURY% < 0 THEN CURY% = SCREEN_H% - CHAR_H%
  3210 END IF
  3220 DIRTY% = 1
  3230 RETURN


23.8  IRATA.ONLINE Features Accessible via BASIC++
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once connected, IRATA.ONLINE provides:

  Feature              Description
  ───────────────────  ──────────────────────────────────
  PLATO Lessons        Interactive educational courseware
  Talkomatic           The original online chat rooms
  Personal Notes       The original e-mail system
  Group Notes          Threaded message boards / forums
  Term-Talk            Private messaging
  Empire               Classic space strategy game
  Avatar               Multiplayer dungeon crawler
  Airfight             Aerial combat game
  Moria                Roguelike dungeon game
  PLATO Editor         On-system program editor
  TUTOR Language       PLATO's native programming lang

These features are accessed through the PLATO terminal
interface after logging in.  No additional protocol
support is needed — the PLATO terminal emulator handles
all interactions.

To access IRATA.ONLINE from BASIC++:

  100 REM -- Quick connect to IRATA.ONLINE --
  110 NET TELNET OPEN "irata.online", 8005 AS #1
  120 REM -- OR use raw TCP: --
  130 REM NET OPEN "irata.online", 8005 AS #1
  140 REM
  150 REM For the full PLATO experience, use the
  160 REM terminal emulator from Section 23.3.
  170 REM
  180 REM For basic text interaction (lobby/chat),
  190 REM the Telnet client from Section 15.1 works.


======================================================================
Part VIII:  NETWORK SECURITY
======================================================================

Network programming introduces significant security
risks.  BASIC++ provides a layered security model to
protect users and systems from network-based attacks.


24.  THE SECURITY GATE
-------------------------

All network operations pass through the Security Gate
(introduced in Section 3 of Part I).  This section
provides comprehensive coverage.


24.1  CAP_NETWORK Capability
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The CAP_NETWORK bit (0x0080) in the module capability
flags controls whether a module can access the network:

  Flag        Bit      Hex     Meaning
  ──────────  ───────  ──────  ────────────────────────
  CAP_FILEIO  bit 0    0x0001  File I/O access
  CAP_SCREEN  bit 1    0x0002  Screen/graphics access
  CAP_AUDIO   bit 4    0x0010  Audio device access
  CAP_USB     bit 6    0x0040  USB device access
  CAP_NETWORK bit 7    0x0080  Network access
  CAP_SYSTEM  bit 15   0x8000  Full system access

To enable networking in a module:

  Module header:

  MODULE "MyNetApp"
    CAPABILITY CAP_NETWORK
    CAPABILITY CAP_SCREEN
  END MODULE

Without CAP_NETWORK, any NET statement raises
ESECURITY (ERR 200): "Network access denied."


24.2  The Security Gate Checks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a NET statement is executed, the Security Gate
performs the following checks in order:

  1. CAP_NETWORK bit set?
     NO  -> ESECURITY (ERR 200)

  2. Operation allowed by policy?
     (e.g., listening on privileged ports < 1024
      requires CAP_SYSTEM)
     NO  -> ESECURITY (ERR 201)

  3. Destination allowed by firewall rules?
     (see Section 24.3)
     NO  -> ENET_BLOCKED (ERR 228)

  4. Rate limit exceeded?
     (see Section 24.5)
     YES -> ENET_RATELIMIT (ERR 229)

  5. Connection count limit exceeded?
     YES -> ENET_MAXCONN (ERR 230)

Only after passing all checks does the operation proceed
to the socket layer.


24.3  Network Firewall Rules
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BASIC++ includes a built-in firewall that controls which
hosts and ports programs can connect to:

  Configuration file: ~/.basicpp/net_policy.conf

  # Allow all HTTPS connections
  ALLOW TCP * 443

  # Allow IRATA.ONLINE
  ALLOW TCP irata.online 8005

  # Allow IRC (Libera.Chat)
  ALLOW TCP irc.libera.chat 6667
  ALLOW TCP irc.libera.chat 6697

  # Allow SMTP (sending email)
  ALLOW TCP smtp.gmail.com 587

  # Allow local network
  ALLOW TCP 192.168.*.* *
  ALLOW UDP 192.168.*.* *

  # Block everything else (implicit)
  DENY TCP * *
  DENY UDP * *

Programmatic firewall control:

  100 REM -- Check if destination is allowed --
  110 IF NET FIREWALL CHECK("irata.online", 8005) THEN
  120   PRINT "Connection allowed"
  130 ELSE
  140   PRINT "Connection blocked by firewall"
  150 END IF


24.4  Sandboxing
~~~~~~~~~~~~~~~~~~

When running untrusted BASIC++ programs, use the sandbox:

  basicpp --sandbox=network:restricted program.bas

Sandbox levels:

  Level         Allowed Operations
  ────────────  ──────────────────────────────────────
  none          No network access at all
  local         Loopback only (127.0.0.1 / ::1)
  restricted    Outbound only, no servers, no raw
  standard      Full client, no servers (default)
  full          Everything including servers
  custom        Uses net_policy.conf rules

Example sandbox configurations:

  REM -- Run a game with no network access --
  basicpp --sandbox=network:none game.bas

  REM -- Run a chat client (outbound only) --
  basicpp --sandbox=network:restricted irc.bas

  REM -- Run a web server (full access) --
  basicpp --sandbox=network:full server.bas


24.5  Rate Limiting
~~~~~~~~~~~~~~~~~~~~~

BASIC++ enforces rate limits to prevent abuse:

  Default Limits:
  ──────────────────────────────────────────────────
  Connections per second:     10
  DNS lookups per second:     5
  Data send rate:             1 MB/s (per channel)
  Data recv rate:             10 MB/s (per channel)
  Total open channels:        64
  Connections per host:       8

Adjust limits programmatically:

  100 NET RATELIMIT "CONN_PER_SEC", 20
  110 NET RATELIMIT "MAX_CHANNELS", 128

Or in the configuration file:

  # ~/.basicpp/net_policy.conf
  RATELIMIT CONN_PER_SEC 20
  RATELIMIT MAX_CHANNELS 128


24.6  Input Validation
~~~~~~~~~~~~~~~~~~~~~~~~

Always validate data received from the network:

  100 REM -- DANGEROUS: Never do this! --
  110 NET RECV #1, CODE$
  120 REM EXEC CODE$   : REM !! NEVER execute network data !!

  200 REM -- SAFE: Validate input --
  210 NET RECV #1, DATA$, 1024, N%
  220 REM Check length
  230 IF N% > 256 THEN
  240   PRINT "Data too large, rejected"
  250   GOTO 300
  260 END IF
  270 REM Check for valid characters only
  280 FOR I% = 1 TO N%
  290   C% = ASC(MID$(DATA$, I%, 1))
  300   IF C% < 32 OR C% > 126 THEN
  310     PRINT "Invalid character in data"
  320     GOTO 400
  330   END IF
  340 NEXT I%
  350 REM Data is safe to use
  360 PRINT "Received: "; DATA$
  400 REM Continue


24.7  Credential Management
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Never hard-code passwords in your programs:

  100 REM -- BAD: Hard-coded password --
  110 REM NET SSH OPEN "server", 22 AS #1 _
  120 REM   USER "admin" PASS "password123"

  200 REM -- GOOD: Read from secure storage --
  210 OPEN ENVIRON$("HOME") + "/.basicpp/credentials" _
        FOR INPUT AS #9
  220 INPUT #9, SSH_USER$
  230 INPUT #9, SSH_PASS$
  240 CLOSE #9
  250 NET SSH OPEN "server", 22 AS #1 _
        USER SSH_USER$ PASS SSH_PASS$

  300 REM -- BETTER: Use key-based auth --
  310 NET SSH OPEN "server", 22 AS #1 _
        USER "admin" KEY "~/.ssh/id_ed25519"

  400 REM -- BEST: Prompt the user --
  410 INPUT "Username: ", U$
  420 LINE INPUT "Password: ", P$    : REM echo disabled
  430 NET SSH OPEN "server", 22 AS #1 USER U$ PASS P$


======================================================================
Part IX:  ERROR REFERENCE AND COMPLETE EXAMPLE
======================================================================


25.  NETWORK ERROR CODES
---------------------------

Complete list of network-related error codes:

  Code  Constant             Description
  ────  ───────────────────  ──────────────────────────────
  200   ESECURITY            Network access denied (no cap)
  201   ESECURITY_POLICY     Operation blocked by policy
  220   ENET_CONNREFUSED     Connection refused by remote
  221   ENET_TIMEOUT         Connection or I/O timed out
  222   ENET_HOSTNOTFOUND    DNS lookup failed
  223   ENET_CONNRESET       Connection reset by remote
  224   ENET_CONNABORTED     Connection aborted locally
  225   ENET_NOTCONNECTED    Channel is not connected
  226   ENET_NETDOWN         Network interface is down
  227   ENET_TLSFAIL         TLS handshake failed
  228   ENET_BLOCKED         Blocked by firewall rules
  229   ENET_RATELIMIT       Rate limit exceeded
  230   ENET_MAXCONN         Max connections exceeded
  231   ENET_ADDRINUSE       Port already in use
  232   ENET_ADDRNOTAVAIL    Address not available
  233   ENET_PROTOCOL        Protocol error
  234   ENET_MSGSIZE         Message too large (UDP)
  235   ENET_BUFOVERFLOW     Receive buffer overflow
  236   ENET_AUTHFAIL        Authentication failed
  237   ENET_SSHFAIL         SSH protocol error
  238   ENET_FTPFAIL         FTP protocol error
  239   ENET_HTTPFAIL        HTTP protocol error
  240   ENET_IRCFAIL         IRC protocol error
  241   ENET_NNTPFAIL        NNTP protocol error
  242   ENET_SMTPFAIL        SMTP protocol error
  243   ENET_IMAPFAIL        IMAP protocol error
  244   ENET_POP3FAIL        POP3 protocol error
  245   ENET_WSFAIL          WebSocket protocol error

To get a human-readable error message:

  100 ON ERROR GOTO 900
  110 NET OPEN "example.com", 80 AS #1
  120 GOTO 999
  900 PRINT "Error "; ERR; ": "; ERROR$(ERR)
  999 END


26.  NETWORK UTILITY FUNCTIONS
---------------------------------

BASIC++ provides several utility functions for network
programming:


26.1  String Utilities
~~~~~~~~~~~~~~~~~~~~~~~~

  Function                  Description
  ────────────────────────  ────────────────────────────
  NET URLENCODE$(s$)        URL-encode a string
  NET URLDECODE$(s$)        URL-decode a string
  NET BASE64ENCODE$(s$)     Base64-encode a string
  NET BASE64DECODE$(s$)     Base64-decode a string
  NET HTMLENCODE$(s$)       HTML-encode (escape <>&")
  NET HTMLDECODE$(s$)       HTML-decode entities
  NET MD5$(s$)              MD5 hash (hex string)
  NET SHA1$(s$)             SHA-1 hash (hex string)
  NET SHA256$(s$)           SHA-256 hash (hex string)

  Examples:

  100 PRINT NET URLENCODE$("hello world & foo")
  110 REM Output: hello%20world%20%26%20foo

  200 PRINT NET BASE64ENCODE$("BASIC++")
  210 REM Output: QkFTSUMrKw==

  300 PRINT NET SHA256$("password")
  310 REM Output: 5e884898da2813...


26.2  Address Utilities
~~~~~~~~~~~~~~~~~~~~~~~~~

  Function                  Description
  ────────────────────────  ────────────────────────────
  NET ISIPV4$(a$)           Returns 1 if valid IPv4
  NET ISIPV6$(a$)           Returns 1 if valid IPv6
  NET LOCALIP$              Returns local IP address
  NET HOSTNAME$             Returns local hostname
  NET MACADDR$              Returns MAC address

  Examples:

  100 PRINT "Local IP:   "; NET LOCALIP$
  110 PRINT "Hostname:   "; NET HOSTNAME$
  120 PRINT "MAC:        "; NET MACADDR$
  130 IF NET ISIPV4$("192.168.1.1") THEN PRINT "Valid IPv4"


26.3  Network Diagnostics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  100 REM -- Ping a host --
  110 NET PING "irata.online", LATENCY%, RESULT%
  120 IF RESULT% = 0 THEN
  130   PRINT "Ping: "; LATENCY%; "ms"
  140 ELSE
  150   PRINT "Ping failed"
  160 END IF

  200 REM -- Traceroute --
  210 DIM HOPS$(30), TIMES%(30)
  220 NET TRACEROUTE "irata.online", HOPS$(), TIMES%(), NHOPS%
  230 FOR I% = 1 TO NHOPS%
  240   PRINT USING "  ##. \                   \ ###ms"; _
              I%; HOPS$(I%); TIMES%(I%)
  250 NEXT I%

  300 REM -- Check if port is open --
  310 NET PORTSCAN "irata.online", 8005, OPEN%
  320 IF OPEN% THEN
  330   PRINT "Port 8005 is open!"
  340 ELSE
  350   PRINT "Port 8005 is closed."
  360 END IF


27.  COMPLETE EXAMPLE: MULTI-PROTOCOL CLIENT
------------------------------------------------

This final example brings together multiple protocols
into a single program — a "Swiss Army knife" network
tool:

  100 REM ======================================
  110 REM  BASIC++ Network Swiss Army Knife
  120 REM ======================================
  130 REM
  140 REM  A multi-protocol client demonstrating
  150 REM  all networking capabilities.
  160 REM
  170 REM ======================================
  180 MODULE "NetToolkit"
  190   CAPABILITY CAP_NETWORK
  200   CAPABILITY CAP_SCREEN
  210   CAPABILITY CAP_FILEIO
  220 END MODULE
  230 REM
  240 DO
  250   CLS
  260   PRINT "╔══════════════════════════════════════╗"
  270   PRINT "║   BASIC++ Network Toolkit v2.0       ║"
  280   PRINT "╠══════════════════════════════════════╣"
  290   PRINT "║                                      ║"
  300   PRINT "║   1. Telnet Client                   ║"
  310   PRINT "║   2. SSH Client                      ║"
  320   PRINT "║   3. FTP Client                      ║"
  330   PRINT "║   4. IRC Chat                        ║"
  340   PRINT "║   5. Usenet Reader                   ║"
  350   PRINT "║   6. E-Mail (Check/Send)             ║"
  360   PRINT "║   7. HTTP Request                    ║"
  370   PRINT "║   8. IRATA.ONLINE (PLATO)            ║"
  380   PRINT "║   9. Network Diagnostics             ║"
  390   PRINT "║   0. Exit                            ║"
  400   PRINT "║                                      ║"
  410   PRINT "╚══════════════════════════════════════╝"
  420   PRINT
  430   INPUT "Select option: ", OPT%
  440   SELECT CASE OPT%
  450     CASE 1 : GOSUB 10000  : REM Telnet
  460     CASE 2 : GOSUB 11000  : REM SSH
  470     CASE 3 : GOSUB 12000  : REM FTP
  480     CASE 4 : GOSUB 13000  : REM IRC
  490     CASE 5 : GOSUB 14000  : REM NNTP
  500     CASE 6 : GOSUB 15000  : REM E-Mail
  510     CASE 7 : GOSUB 16000  : REM HTTP
  520     CASE 8 : GOSUB 17000  : REM IRATA
  530     CASE 9 : GOSUB 18000  : REM Diagnostics
  540     CASE 0 : EXIT DO
  550   END SELECT
  560 LOOP
  570 PRINT "Goodbye!"
  580 END
  590 REM
  10000 REM ======================================
  10010 REM  TELNET CLIENT
  10020 REM ======================================
  10030 INPUT "Host: ", HOST$
  10040 INPUT "Port (23): ", PORT%
  10050 IF PORT% = 0 THEN PORT% = 23
  10060 ON ERROR GOTO 10200
  10070 NET TELNET OPEN HOST$, PORT% AS #1
  10080 PRINT "Connected! (Ctrl+] to disconnect)"
  10090 DO
  10100   NET RECV #1, BUF$, 1024, N%
  10110   IF N% > 0 THEN PRINT NET TELNET STRIP$(BUF$);
  10120   K$ = INKEY$
  10130   IF K$ = CHR$(29) THEN EXIT DO
  10140   IF LEN(K$) > 0 THEN NET SEND #1, K$
  10150   SLEEP 10
  10160 LOOP
  10170 NET CLOSE #1
  10180 PRINT : PRINT "Disconnected."
  10190 RETURN
  10200 PRINT "Error: "; ERROR$(ERR) : RETURN
  10210 REM
  17000 REM ======================================
  17010 REM  IRATA.ONLINE (PLATO)
  17020 REM ======================================
  17030 PRINT "Connecting to irata.online:8005..."
  17040 ON ERROR GOTO 17200
  17050 NET TELNET OPEN "irata.online", 8005 AS #1
  17060 PRINT "Connected to IRATA.ONLINE!"
  17070 PRINT "(Ctrl+] to disconnect)"
  17080 PRINT "────────────────────────────────────"
  17090 DO
  17100   NET RECV #1, BUF$, 4096, N%
  17110   IF N% > 0 THEN PRINT NET TELNET STRIP$(BUF$);
  17120   K$ = INKEY$
  17130   IF K$ = CHR$(29) THEN EXIT DO
  17140   IF LEN(K$) > 0 THEN NET SEND #1, K$
  17150   SLEEP 10
  17160 LOOP
  17170 NET CLOSE #1
  17180 PRINT : PRINT "Disconnected from IRATA.ONLINE."
  17190 RETURN
  17200 PRINT "Error: "; ERROR$(ERR) : RETURN
  17210 REM
  18000 REM ======================================
  18010 REM  NETWORK DIAGNOSTICS
  18020 REM ======================================
  18030 PRINT "Network Diagnostics"
  18040 PRINT "────────────────────────────────────"
  18050 PRINT "Local IP:    "; NET LOCALIP$
  18060 PRINT "Hostname:    "; NET HOSTNAME$
  18070 PRINT "MAC Address: "; NET MACADDR$
  18080 PRINT
  18090 REM -- Adapter list --
  18100 DIM A$(16), IP$(16)
  18110 NET ADAPTER LIST A$(), IP$(), AC%
  18120 PRINT "Adapters:"
  18130 FOR I% = 1 TO AC%
  18140   PRINT "  "; A$(I%); " - "; IP$(I%)
  18150 NEXT I%
  18160 PRINT
  18170 REM -- Ping test --
  18180 INPUT "Ping host (Enter for irata.online): ", PH$
  18190 IF PH$ = "" THEN PH$ = "irata.online"
  18200 NET RESOLVE PH$, RESOLVED$
  18210 PRINT "Resolved: "; RESOLVED$
  18220 NET PING PH$, LAT%, PRES%
  18230 IF PRES% = 0 THEN
  18240   PRINT "Ping: "; LAT%; "ms"
  18250 ELSE
  18260   PRINT "Ping failed."
  18270 END IF
  18280 PRINT
  18290 INPUT "Press Enter to continue...", DUMMY$
  18300 RETURN


28.  KEYWORD QUICK REFERENCE
-------------------------------

All NET keywords at a glance:

  Keyword                     Section   Description
  ──────────────────────────  ────────  ──────────────────────
  NET OPEN                    11.1      TCP client connect
  NET LISTEN                  11.2      TCP server listen
  NET ACCEPT                  11.3      Accept connection
  NET SEND                    11.4      Send data (TCP)
  NET RECV                    11.4      Receive data (TCP)
  NET CLOSE                   11.5      Close connection
  NET STATUS()                11.6      Connection state
  NET TIMEOUT                 11.7      Set I/O timeout
  NET KEEPALIVE               11.7      TCP keep-alive
  NET OPTION                  11.7      Socket options
  NET SENDTO                  12.2      Send UDP datagram
  NET RECVFROM                12.2      Receive UDP datagram
  NET RESOLVE                 13.1      DNS lookup
  NET DNS CACHE               13.3      DNS cache control
  NET STARTTLS                14.1      Upgrade to TLS
  NET TELNET OPEN             15.1      Telnet connect
  NET TELNET OPTION           15.2      Telnet negotiation
  NET TELNET STRIP$()         15.1      Strip IAC sequences
  NET SSH OPEN                16.1      SSH connect
  NET SSH SHELL               16.3      SSH shell session
  NET SSH EXEC                16.4      SSH remote command
  NET SSH TUNNEL              16.5      SSH port forward
  NET SSH PUT / GET           16.6      SCP/SFTP transfer
  NET SSH DIR                 16.6      SFTP directory list
  NET FTP OPEN                17.1      FTP connect
  NET FTP CD / PWD            17.2      FTP navigation
  NET FTP DIR / LIST          17.2      FTP directory list
  NET FTP GET / PUT           17.3      FTP transfer
  NET FTP DELETE / RENAME     17.4      FTP file management
  NET FTP MKDIR / RMDIR       17.4      FTP directories
  NET FTP PROGRESS()          17.5      Transfer progress
  NET FTP CLOSE               17.6      FTP disconnect
  NET IRC OPEN                18.1      IRC connect
  NET IRC JOIN / PART         18.2      IRC channels
  NET IRC MSG                 18.2      IRC message
  NET IRC NICK                18.2      IRC nickname
  NET IRC POLL                18.3      IRC message poll
  NET IRC QUIT                18.3      IRC disconnect
  NET NNTP OPEN               19.1      NNTP connect
  NET NNTP GROUP              19.2      Select newsgroup
  NET NNTP LIST               19.2      List newsgroups
  NET NNTP HEADERS            19.3      Read headers
  NET NNTP ARTICLE            19.3      Read article
  NET NNTP POST               19.4      Post article
  NET NNTP REPLY              19.4      Reply to article
  NET NNTP CLOSE              19.5      NNTP disconnect
  NET SMTP OPEN               20.1.1    SMTP connect
  NET SMTP FROM / TO / CC     20.1.2    Mail addresses
  NET SMTP SUBJECT / BODY     20.1.2    Mail content
  NET SMTP ATTACH             20.1.4    Mail attachment
  NET SMTP SEND               20.1.2    Send e-mail
  NET SMTP CLOSE              20.1.2    SMTP disconnect
  NET POP3 OPEN               20.2.1    POP3 connect
  NET POP3 STAT               20.2.2    Mailbox status
  NET POP3 HEADER             20.2.2    Message header
  NET POP3 RETR               20.2.2    Retrieve message
  NET POP3 DELE               20.2.3    Delete message
  NET POP3 CLOSE              20.2.2    POP3 disconnect
  NET IMAP OPEN               20.3.1    IMAP connect
  NET IMAP LIST               20.3.2    List folders
  NET IMAP SELECT             20.3.2    Select folder
  NET IMAP SEARCH             20.3.3    Search messages
  NET IMAP FETCH              20.3.3    Fetch message
  NET IMAP FLAG               20.3.4    Set/clear flags
  NET IMAP MOVE               20.3.4    Move message
  NET IMAP EXPUNGE            20.3.4    Expunge deleted
  NET IMAP CLOSE              20.3.4    IMAP disconnect
  NET HTTP GET                21.1      HTTP GET request
  NET HTTP POST               21.2      HTTP POST request
  NET HTTP PUT                21.3      HTTP PUT request
  NET HTTP DELETE             21.3      HTTP DELETE request
  NET HTTP PATCH              21.3      HTTP PATCH request
  NET HTTP HEADER             21.4      Set/get headers
  NET HTTP COOKIE             21.5      Cookie management
  NET HTTP MULTIPART          21.6      Multipart upload
  NET WS OPEN                 22.1      WebSocket connect
  NET WS SEND / RECV          22.2      WebSocket messages
  NET WS CLOSE                22.4      WebSocket close
  NET ADAPTER LIST            9.1       List adapters
  NET ADAPTER USE             9.2       Select adapter
  NET ADAPTER INFO            9.5       Adapter details
  NET WIFI SCAN               10.1      Scan Wi-Fi
  NET WIFI JOIN / LEAVE       10.2-4    Wi-Fi connect
  NET WIFI STATUS             10.3      Wi-Fi status
  NET PING                    26.3      Ping host
  NET TRACEROUTE              26.3      Trace route
  NET PORTSCAN                26.3      Port scan
  NET FIREWALL CHECK          24.3      Firewall check
  NET RATELIMIT               24.5      Rate limiting
  NET URLENCODE$ / DECODE$    26.1      URL encoding
  NET BASE64ENCODE$ / DECODE$ 26.1      Base64 encoding
  NET SHA256$                 26.1      Hash functions
  NET LOCALIP$                26.2      Local IP address
  NET HOSTNAME$               26.2      Local hostname


29.  SUMMARY
--------------

This tutorial has covered the complete BASIC++ network
programming system, from low-level adapter discovery to
high-level application protocols:

  Part I     Architecture — The 6-layer network stack
  Part II    Adapters — Ethernet, Wi-Fi, loopback
  Part III   Core — TCP, UDP, DNS, TLS
  Part IV    Protocols — Telnet, SSH, FTP
  Part V     Communication — IRC, NNTP, E-Mail
  Part VI    Web — HTTP/HTTPS, WebSockets
  Part VII   IRATA.ONLINE — The PLATO terminal
  Part VIII  Security — Capabilities, firewall, sandbox
  Part IX    Reference — Errors, utilities, examples

Key takeaways:

  1. All network access requires CAP_NETWORK capability
  2. The Security Gate enforces policy before any socket
     operation reaches the network
  3. High-level protocol wrappers (NET TELNET, NET SSH,
     NET FTP, NET IRC, NET NNTP, NET SMTP, NET HTTP)
     handle protocol details automatically
  4. Raw TCP/UDP access is always available for custom
     protocols or when you need full control
  5. TLS encryption is available for all TCP protocols
     via the SECURE keyword or NET STARTTLS
  6. IRATA.ONLINE is accessible on irata.online:8005
     using either Telnet or the full PLATO terminal

For more information:
  - Tutorial R: Virtual Devices (VDev architecture)
  - Tutorial W: Virtual Filesystem (secure file I/O)
  - Tutorial V: Virtual Machines (VM opcode dispatch)

======================================================================
  END OF TUTORIAL X: VIRTUAL NETWORK
======================================================================
