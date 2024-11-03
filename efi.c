#include "efi.h"

EFI_GUID SFSP_GUID = {0x0964e5b22,
                      0x6459,
                      0x11d2,
                      {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

EFI_SYSTEM_TABLE *ST;

void efi_init(EFI_SYSTEM_TABLE *SystemTable) {
    ST = SystemTable;
    ST->ConOut->ClearScreen(ST->ConOut);
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
    put_param(L"Number of volumes", sfs_handle_count);

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
            put_param(L"Find Kernel in Volume", i);
            return root;
        }
    }
    return NULL;
}