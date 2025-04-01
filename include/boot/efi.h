#pragma once
#include <types.h>

typedef struct {
    uint32 Data1;
    uint16 Data2;
    uint16 Data3;
    uint8 Data4[8];
} EFI_GUID;

// 7.3.6 EFI_BOOT_SERVICES.LocateHandle()
typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

// 4.2.1 EFI_TABLE_HEADER
typedef struct {
    uint64 Signature;
    uint32 Revision;
    uint32 HeaderSize;
    uint32 CRC32;
    uint32 Reserved;
} EFI_TABLE_HEADER;

// 7.2.1 EFI_BOOT_SERVICES.AllocatePages()
typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

// 7.2.3 EFI_BOOT_SERVICES.GetMemoryMap()
typedef struct {
    uint32 Type;
    uint64 PhysicalStart;
    uint64 VirtualStart;
    uint64 NumberOfPages;
    uint64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

// 4.4.1 EFI_BOOT_SERVICES
typedef struct {
    EFI_TABLE_HEADER Hdr;
    // Task Priority Services
    void *RaiseTPL;
    void *RestoreTPL;
    // Memory Services
    void *AllocatePages;
    void *FreePages;
    uint64 (*GetMemoryMap)(  // 7.2.3 EFI_BOOT_SERVICES.GetMemoryMap()
        uint64 *MemoryMapSize, EFI_MEMORY_DESCRIPTOR *MemoryMap, uint64 *MapKey,
        uint64 *DescriptorSize, uint32 *DescriptorVersion);
    void *AllocatePool;
    void *FreePool;
    // Event & Timer Services
    void *CreateEvent;
    void *SetTimer;
    void *WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;
    // Protocol Handler Services
    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    uint64 (*HandleProtocol)(  // 7.3.7 EFI_BOOT_SERVICES.HandleProtocol()
        void *Handle, EFI_GUID *Protocol, void **Interface);
    void *Reserved;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;
    // Image Services
    void *LoadImage;
    void *StartImage;
    void *Exit;
    void *UnloadImage;
    uint64 (*ExitBootServices)(void *ImageHandle, unsigned long long MapKey);
    // Miscellaneous Services
    void *GetNextMonotonicCount;
    void *Stall;
    void *SetWatchdogTimer;
    // DriverSupport Services
    void *ConnectController;
    void *DisconnectController;
    // Open and Close Protocol Services
    void *OpenProtocol;
    void *CloseProtocol;
    void *OpenProtocolInformation;
    // Library Services
    void *ProtocolsPerHandle;
    uint64 (*LocateHandleBuffer)(
        // 7.3.15 EFI_BOOT_SERVICES.LocateHandleBuffer()
        EFI_LOCATE_SEARCH_TYPE SearchType, EFI_GUID *Protocol, void *SearchKey,
        uint64 *NoHandles, void ***Buffer);
    uint64 (*LocateProtocol)(EFI_GUID *Protocol, void *Registration,
                             void **Interface);
    void *InstallMultipleProtocolInterfaces;
    void *UninstallMultipleProtocolInterfaces;
    // 32-bit CRC Services
    void *CalculateCrc32;
    // Miscellaneous Services
    void *CopyMem;
    void (*SetMem)(void *Buffer, uint64 Size, uint8 Value);
    void *CreateEventEx;
} EFI_BOOT_SERVICES;

// 12.9.2 EFI_GRAPHICS_OUTPUT_PROTOCOL
typedef struct {
    uint64 QueryMode;
    uint64 SetMode;
    uint64 Blt;
    struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
        uint32 MaxMode;
        uint32 Mode;
        struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
            uint32 Version;
            uint32 HorizontalResolution;
            uint32 VerticalResolution;
            enum EFI_GRAPHICS_PIXEL_FORMAT {
                PixelRedGreenBlueReserved8BitPerColor,
                PixelBlueGreenRedReserved8BitPerColor,
                PixelBitMask,
                PixelBltOnly,
                PixelFormatMax
            } PixelFormat;
        } *Info;
        uint64 SizeOfInfo;
        uint64 FrameBufferBase;
        uint64 FrameBufferSize;
    } *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

// 13.5.1 EFI_FILE_PROTOCOL
typedef struct EFI_FILE_PROTOCOL {
    uint64 Revision;
    uint64 (*Open)(struct EFI_FILE_PROTOCOL *This,
                   struct EFI_FILE_PROTOCOL **NewHandle, uint16 *FileName,
                   uint64 OpenMode, uint64 Attributes);
    uint64 (*Close)(struct EFI_FILE_PROTOCOL *This);
    void *Delete;
    uint64 (*Read)(struct EFI_FILE_PROTOCOL *This, uint64 *BufferSize,
                   void *Buffer);
    uint64 (*Write)(struct EFI_FILE_PROTOCOL *This, uint64 *BufferSize,
                    void *Buffer);
    void *GetPosition;
    uint64 (*SetPosition)(struct EFI_FILE_PROTOCOL *This, uint64 Position);
    void *GetInfo;
    void *SetInfo;
    uint64 (*Flush)(struct EFI_FILE_PROTOCOL *This);
    void *OpenEx;
} EFI_FILE_PROTOCOL;

// 13.5.2 EFI_FILE_PROTOCOL.Open()
#define EFI_FILE_MODE_READ 0x0000000000000001
#define EFI_FILE_MODE_WRITE 0x0000000000000002
#define EFI_FILE_MODE_CREATE 0x8000000000000000
#define EFI_FILE_READ_ONLY 0x0000000000000001
#define EFI_FILE_HIDDEN 0x0000000000000002
#define EFI_FILE_SYSTEM 0x0000000000000004
#define EFI_FILE_RESERVED 0x0000000000000008
#define EFI_FILE_DIRECTORY 0x0000000000000010
#define EFI_FILE_ARCHIVE 0x0000000000000020
#define EFI_FILE_VALID_ATTR 0x0000000000000037

// 13.4.1 EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    uint64 Revision;
    uint64 (*OpenVolume)(struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
                         EFI_FILE_PROTOCOL **Root);
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

// 4.6.1 EFI_CONFIGURATION_TABLE
typedef struct {
    EFI_GUID VendorGuid;
    void *VendorTable;
} EFI_CONFIGURATION_TABLE;

// 4.3.1 EFI_SYSTEM_TABLE
typedef struct {
    EFI_TABLE_HEADER Hdr;
    uint16 *FirmwareVendor;
    uint32 FirmwareRevision;
    uint64 ConsoleInHandle;
    void *ConIn;
    uint64 ConsoleOutHandle;
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
        void *reset;
        uint64 (*OutputString)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
                               unsigned short *String);
        void *TestString;
        void *QueryMode;
        void *SetMode;
        void *SetAttribute;
        uint64 (*ClearScreen)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
    } *ConOut;
    uint64 StandardErrorHandle;
    void *StdErr;
    void *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    uint64 NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE *ConfigurationTable;
} EFI_SYSTEM_TABLE;

// common.c
void puts(uint16 *s);
void put_hex(uint64 n);
void put_param(uint16 *s, uint64 n);
void warn(uint64 status, uint16 *message);
void assert(uint64 status, uint16 *message);
void panic(uint16 *message);

// efi.c
void efi_init(EFI_SYSTEM_TABLE *SystemTable);
struct EFI_FILE_PROTOCOL *search_volume_contains_file(uint16 *filename);
uint64 get_total_memory_size();
uintptr search_rsdp2();
void exit_boot_services(void *ImageHandle);
int test_guid(EFI_GUID *guid1, EFI_GUID *guid2);
extern EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSP;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;
extern EFI_SYSTEM_TABLE *ST;
