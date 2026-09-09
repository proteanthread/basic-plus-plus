// FILENAME: uefi_main.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: engine/src/bootstrap/uefi/uefi_main.c
// NEEDS: <stdint.h>, <stddef.h>, <stdbool.h>
// UEFI Firmware Application Architecture and Protocols for Bare-Metal Boot.
//

#ifndef UEFI_MAIN_H
#define UEFI_MAIN_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(_MSC_VER)
    #define EFIAPI __cdecl
#elif defined(__GNUC__) || defined(__clang__)
    #define EFIAPI __attribute__((ms_abi))
#else
    #define EFIAPI
#endif

typedef void *EFI_HANDLE;
typedef void *EFI_EVENT;
typedef uint64_t EFI_STATUS;
typedef uint16_t CHAR16;
typedef size_t UINTN;

#define EFI_SUCCESS 0ULL
#define EFI_NOT_READY 0x8000000000000006ULL

typedef struct {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    uint16_t ScanCode;
    CHAR16   UnicodeChar;
} EFI_INPUT_KEY;

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET)(
    struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    bool ExtendedVerification
);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(
    struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    EFI_INPUT_KEY *Key
);

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET    Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    EFI_EVENT          WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    bool ExtendedVerification
);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    const CHAR16 *String
);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
);

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET        Reset;
    EFI_TEXT_STRING       OutputString;
    void                 *TestString;
    void                 *QueryMode;
    void                 *SetMode;
    void                 *SetAttribute;
    EFI_TEXT_CLEAR_SCREEN ClearScreen;
    void                 *SetCursorPosition;
    void                 *EnableCursor;
    void                 *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef void (EFIAPI *EFI_RESET_SYSTEM)(
    EFI_RESET_TYPE ResetType,
    EFI_STATUS ResetStatus,
    UINTN DataSize,
    void *ResetData
);

typedef struct {
    EFI_TABLE_HEADER Hdr;
    void            *GetTime;
    void            *SetTime;
    void            *GetWakeupTime;
    void            *SetWakeupTime;
    void            *SetVirtualAddressMap;
    void            *ConvertPointer;
    void            *GetVariable;
    void            *GetNextVariableName;
    void            *SetVariable;
    void            *GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM ResetSystem;
} EFI_RUNTIME_SERVICES;

typedef EFI_STATUS (EFIAPI *EFI_EXIT)(
    EFI_HANDLE ImageHandle,
    EFI_STATUS ExitStatus,
    UINTN ExitDataSize,
    CHAR16 *ExitData
);

typedef struct {
    EFI_TABLE_HEADER Hdr;
    void            *RaiseTPL;
    void            *RestoreTPL;
    void            *AllocatePages;
    void            *FreePages;
    void            *GetMemoryMap;
    void            *AllocatePool;
    void            *FreePool;
    void            *CreateEvent;
    void            *SetTimer;
    void            *WaitForEvent;
    void            *SignalEvent;
    void            *CloseEvent;
    void            *CheckEvent;
    void            *InstallProtocolInterface;
    void            *ReinstallProtocolInterface;
    void            *UninstallProtocolInterface;
    void            *HandleProtocol;
    void            *VoidReserved;
    void            *RegisterProtocolNotify;
    void            *LocateHandle;
    void            *LocateDevicePath;
    void            *InstallConfigurationTable;
    void            *LoadImage;
    void            *StartImage;
    EFI_EXIT         Exit;
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER                 Hdr;
    CHAR16                          *FirmwareVendor;
    uint32_t                         FirmwareRevision;
    EFI_HANDLE                       ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *ConIn;
    EFI_HANDLE                       ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                       StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;
    UINTN                            NumberOfTableEntries;
    void                            *ConfigurationTable;
} EFI_SYSTEM_TABLE;

// Bare-metal UEFI ROM BASIC entry point
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

#endif // UEFI_MAIN_H
