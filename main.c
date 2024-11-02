#include "efi.h"

void efi_main(void *ImageHandle __attribute__ ((unused)),
        struct EFI_SYSTEM_TABLE *SystemTable)
{
    efi_init(SystemTable);
    struct EFI_FILE_PROTOCOL *root = search_volume_contains_file(L"kernel.bin");
    if (root == NULL) {
		assert(1, L"No volume contains kernel.bin.");
	}

    while (1);
}