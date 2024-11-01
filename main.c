#define uint64 unsigned long long
#define uint32 unsigned long
#define uint16 unsigned short
#define uint8 unsigned char

// 4.3.1 EFI_SYSTEM_TABLE
struct EFI_SYSTEM_TABLE {
    struct EFI_TABLE_HEADER {
        uint64 Signature;
        uint32 Revision;
        uint32 HeaderSize;
        uint32 CRC32;
        uint32 Reserved;
    } Hdr;
    uint16 *FirmwareVendor;
    uint32 FirmwareRevision;
    uint64 ConsoleInHandle;
    void *ConIn;
    uint64 ConsoleOutHandle;
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
            void *reset;
            uint64 (*OutputString)(
                    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
                    unsigned short *String);
            void *TestString;
            void *QueryMode;
            void *SetMode;
            void *SetAttribute;
            uint64 (*ClearScreen)(
                    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
    } *ConOut;
    uint64 StandardErrorHandle;
    void *StdErr;
    void *RuntimeServices;
    void *BootServices;
    uint64 NumberOfTableEntries;
    void *ConfigurationTable;
};

void efi_main(void *ImageHandle __attribute__ ((unused)),
        struct EFI_SYSTEM_TABLE *SystemTable)
{
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    SystemTable->ConOut->OutputString(SystemTable->ConOut,
                                    L"Hello UEFI!\n");
    while (1);
}