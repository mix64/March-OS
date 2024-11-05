#include "efi.h"
#include "elf.h"

uint64 load_kernel(EFI_FILE_PROTOCOL *root, uint16 *filename) {
    EFI_FILE_PROTOCOL *file;
    uint64 status = root->Open(root, &file, filename, EFI_FILE_MODE_READ, 0);
    assert(status, L"root->Open(kernel)");

    Elf64_Ehdr ehdr;
    uint64 ehdr_size = sizeof(Elf64_Ehdr);
    status = file->Read(file, &ehdr_size, (void *)&ehdr);
    assert(status, L"kernel->Read(Ehdr)");
    if (ehdr.magic != ELF_MAGIC) {
        panic(L"Invalid ELF Magic Number.");
    }

    Elf64_Phdr phdr;
    uint64 phdr_size = sizeof(Elf64_Phdr);
    for (uint8 i = 0; i < ehdr.e_phnum; i++) {
        status = file->SetPosition(file, ehdr.e_phoff + phdr_size * i);
        assert(status, L"kernel->SetPosition(e_phoff)");
        status = file->Read(file, &phdr_size, (void *)&phdr);
        assert(status, L"kernel->Read(Phdr)");
        if (phdr.p_type != ELF_PROG_LOAD) {
            continue;
        }
        status = file->SetPosition(file, phdr.p_offset);
        assert(status, L"kernel->SetPosition(p_offset)");
        put_param(L"Load Segment", i);
        put_param(L"p_paddr", phdr.p_paddr);
        put_param(L"p_filesz", phdr.p_filesz);
        put_param(L"p_memsz", phdr.p_memsz);
        status = file->Read(file, &phdr.p_filesz, (void *)phdr.p_paddr);
        assert(status, L"kernel->Read(Segment)");
        if (phdr.p_filesz < phdr.p_memsz) {
            ST->BootServices->SetMem((void *)(phdr.p_paddr + phdr.p_filesz),
                                     phdr.p_memsz - phdr.p_filesz, 0);
        }
    }
    file->Close(file);
    root->Close(root);

    return ehdr.e_entry;
}

#define MMAP_SIZE 0x100000
uint8 mmap_buf[MMAP_SIZE];

void exit_boot_services(void *ImageHandle) {
    uint64 mmap_size = MMAP_SIZE;
    uint64 map_key, desc_size;
    uint32 desc_version;
    uint64 status = ST->BootServices->GetMemoryMap(
        &mmap_size, (EFI_MEMORY_DESCRIPTOR *)mmap_buf, &map_key, &desc_size,
        &desc_version);
    assert(status, L"GetMemoryMap");
    status = ST->BootServices->ExitBootServices(ImageHandle, map_key);
    assert(status, L"ExitBootServices");
}

void efi_main(void *ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    efi_init(SystemTable);
    EFI_FILE_PROTOCOL *root = search_volume_contains_file(L"kernel.bin");
    if (root == NULL) {
        panic(L"No volume contains kernel.bin.");
    }
    uint64 kernel_entry_addr = load_kernel(root, L"kernel.bin");
    put_param(L"Kernel Entry", kernel_entry_addr);
    // ST->ConOut->ClearScreen(ST->ConOut);
    exit_boot_services(ImageHandle);
    void (*entry)(void) = (void (*)(void))(kernel_entry_addr);
    entry();
    panic(L"Failed to Start kernel.");
}