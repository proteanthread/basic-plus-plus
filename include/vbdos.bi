' =============================================================================
' VBDOS.BI - Standard Visual Basic for DOS 1.0 Include File
' Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
' =============================================================================

' --- Structure Definitions for Interrupt & Low-Level Bridge ---
TYPE RegType
    ax    AS INTEGER
    bx    AS INTEGER
    cx    AS INTEGER
    dx    AS INTEGER
    bp    AS INTEGER
    si    AS INTEGER
    di    AS INTEGER
    flags AS INTEGER
END TYPE

TYPE RegTypeX
    ax    AS INTEGER
    bx    AS INTEGER
    cx    AS INTEGER
    dx    AS INTEGER
    bp    AS INTEGER
    si    AS INTEGER
    di    AS INTEGER
    flags AS INTEGER
    ds    AS INTEGER
    es    AS INTEGER
END TYPE

' --- MsgBox Style Constants ---
CONST vbOKOnly = 0
CONST vbOKCancel = 1
CONST vbAbortRetryIgnore = 2
CONST vbYesNoCancel = 3
CONST vbYesNo = 4
CONST vbRetryCancel = 5

CONST vbCritical = 16
CONST vbQuestion = 32
CONST vbExclamation = 48
CONST vbInformation = 64

CONST vbDefaultButton1 = 0
CONST vbDefaultButton2 = 256
CONST vbDefaultButton3 = 512

' --- MsgBox Return Values ---
CONST vbOK = 1
CONST vbCancel = 2
CONST vbAbort = 3
CONST vbRetry = 4
CONST vbIgnore = 5
CONST vbYes = 6
CONST vbNo = 7

' --- File Attribute Constants ---
CONST vbNormal = 0
CONST vbReadOnly = 1
CONST vbHidden = 2
CONST vbSystem = 4
CONST vbVolume = 8
CONST vbDirectory = 16
CONST vbArchive = 32

' --- Form Display Modality ---
CONST vbModeless = 0
CONST vbModal = 1

' --- Standard Color Indices ---
CONST vbBlack = 0
CONST vbBlue = 1
CONST vbGreen = 2
CONST vbCyan = 3
CONST vbRed = 4
CONST vbMagenta = 5
CONST vbBrown = 6
CONST vbWhite = 7
CONST vbGray = 8
CONST vbBrightBlue = 9
CONST vbBrightGreen = 10
CONST vbBrightCyan = 11
CONST vbBrightRed = 12
CONST vbBrightMagenta = 13
CONST vbYellow = 14
CONST vbBrightWhite = 15
