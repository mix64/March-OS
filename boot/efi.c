#include <boot/efi.h>
#include <boot/info.h>

EFI_GUID SFSP_GUID = {0x0964e5b22,
                      0x6459,
                      0x11d2,
                      {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

EFI_GUID GOP_GUID = {0x9042a9de,
                     0x23dc,
                     0x4a38,
                     {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

// 5.2.5.2. Finding the RSDP on UEFI Enabled Systems
EFI_GUID ACPIv1_GUID = {0xeb9d2d30,
                        0x2d88,
                        0x11d3,
                        {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}};

EFI_GUID ACPIv2_GUID = {0x8868e871,
                        0xe4f1,
                        0x11d3,
                        {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

EFI_SYSTEM_TABLE *ST;
EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;

#define MMAP_SIZE 0x30000
uint8 mmap_buf[MMAP_SIZE];

void efi_init(EFI_SYSTEM_TABLE *SystemTable) {
    ST = SystemTable;
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->BootServices->LocateProtocol(&GOP_GUID, NULL, (void **)&GOP);
    puts(L"UEFI Boot!\r\n");
}

EFI_FILE_PROTOCOL *search_volume_contains_file(uint16 *filename) {
    void **sfs_handles;
    uint64 sfs_handle_count;
    uint64 status;
    status = ST->BootServices->LocateHandleBuffer(ByProtocol, &SFSP_GUID, NULL,
                                                  &sfs_handle_count,
                                                  (void ***)&sfs_handles);
    assert(status, L"LocateHandleBuffer");
    put_param(L"[uefi] Number of volumes", sfs_handle_count);

    EFI_FILE_PROTOCOL *root;
    for (uint64 i = 0; i < sfs_handle_count; i++) {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp;
        status = ST->BootServices->HandleProtocol(sfs_handles[i], &SFSP_GUID,
                                                  (void **)&sfsp);
        if (status) {
            warn(status, L"HandleProtocol@sfsp");
            continue;
        }

        status = sfsp->OpenVolume(sfsp, &root);
        if (status) {
            warn(status, L"OpenVolume@sfsp");
            continue;
        }

        EFI_FILE_PROTOCOL *_file;
        status = root->Open(root, &_file, filename, EFI_FILE_MODE_READ, 0);
        if (!status) {
            put_param(L"[uefi] Find Kernel in Volume", i);
            return root;
        }
    }
    return NULL;
}

uint64 get_total_memory_size() {
    uint64 mmap_size = MMAP_SIZE;
    uint64 mmap_key, desc_size;
    uint32 desc_version;
    uint64 status = ST->BootServices->GetMemoryMap(
        &mmap_size, (EFI_MEMORY_DESCRIPTOR *)mmap_buf, &mmap_key, &desc_size,
        &desc_version);
    assert(status, L"GetMemoryMap@get_total_memory_size");

    uint64 total_memory_size = 0;
    uint64 entry_count = mmap_size / desc_size;
    for (uint64 i = 0; i < entry_count; i++) {
        EFI_MEMORY_DESCRIPTOR *e =
            (EFI_MEMORY_DESCRIPTOR *)(mmap_buf + (i * desc_size));
        uint64 paddr = e->PhysicalStart + e->NumberOfPages * 0x1000;  // 4KiB
        if (paddr > total_memory_size) {
            total_memory_size = paddr;
        }
    }
    return total_memory_size;
}

uintptr search_xsdp() {
    EFI_CONFIGURATION_TABLE *table = ST->ConfigurationTable;
    for (uint64 i = 0; i < ST->NumberOfTableEntries; i++) {
        if (test_guid(&table[i].VendorGuid, &ACPIv2_GUID) == 0) {
            return (uintptr)table[i].VendorTable;
        }
    }
    return 0;
}

void exit_boot_services(void *ImageHandle) {
    uint64 mmap_size = MMAP_SIZE;
    uint64 map_key, desc_size;
    uint32 desc_version;
    uint64 status = ST->BootServices->GetMemoryMap(
        &mmap_size, (EFI_MEMORY_DESCRIPTOR *)mmap_buf, &map_key, &desc_size,
        &desc_version);
    assert(status, L"GetMemoryMap@exit_boot_services");
    status = ST->BootServices->ExitBootServices(ImageHandle, map_key);
    assert(status, L"ExitBootServices");
}

int test_guid(EFI_GUID *guid1, EFI_GUID *guid2) {
    if (guid1->Data1 != guid2->Data1) {
        return 1;
    }
    if (guid1->Data2 != guid2->Data2) {
        return 1;
    }
    if (guid1->Data3 != guid2->Data3) {
        return 1;
    }
    for (uint8 i = 0; i < 8; i++) {
        if (guid1->Data4[i] != guid2->Data4[i]) {
            return 1;
        }
    }
    return 0;  // GUIDs are equal
}