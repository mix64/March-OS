#include "efi.h"
#include "elf.h"

void load_kernel(EFI_FILE_PROTOCOL *root, uint16 *filename) {
    EFI_FILE_PROTOCOL *file;
    uint64 status = root->Open(root, &file, filename, EFI_FILE_MODE_READ, 0);
    assert(status, L"root->Open(kernel)");

    Elf64_Ehdr ehdr;
    uint64 ehdr_size = sizeof(Elf64_Ehdr);
    status = file->Read(file, &ehdr_size, (void *)&ehdr);
    assert(status, L"kernel->Read(Ehdr)");

    Elf64_Phdr phdr;
    uint64 phdr_size = sizeof(Elf64_Phdr);
    status = file->SetPosition(file, ehdr.e_phoff);
    assert(status, L"kernel->SetPosition(phoff)");

    for (uint8 i = 0; i < ehdr.e_phnum; i++) {
        status = file->Read(file, &phdr_size, (void *)&phdr);
        assert(status, L"kernel->Read(Phdr)");
        if (phdr.p_type != ELF_PROG_LOAD) {
            continue;
        }
        put_param(L"Load Segment", i);
        put_param(L"p_paddr", phdr.p_paddr);
        put_param(L"p_filesz", phdr.p_filesz);
        put_param(L"p_memsz", phdr.p_memsz);
        file->Read(file, &phdr.p_filesz, (void *)phdr.p_paddr);
        if (phdr.p_filesz < phdr.p_memsz) {
            uint64 diff = phdr.p_memsz - phdr.p_filesz;
            for (uint64 i = 0; i < diff; i++) {
                *(uint8 *)(phdr.p_paddr + phdr.p_filesz + i) = 0;
            }
        }
    }
    file->Close(file);
}

void efi_main(void *ImageHandle __attribute__((unused)),
              EFI_SYSTEM_TABLE *SystemTable) {
    efi_init(SystemTable);
    EFI_FILE_PROTOCOL *root = search_volume_contains_file(L"kernel.bin");
    if (root == NULL) {
        panic(L"No volume contains kernel.bin.");
    }
    load_kernel(root, L"kernel.bin");

    while (1);
}