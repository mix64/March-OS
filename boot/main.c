#include <boot/efi.h>
#include <boot/info.h>
#include <elf.h>

BootInfo boot_info;

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

void efi_main(void *ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    efi_init(SystemTable);
    EFI_FILE_PROTOCOL *root = search_volume_contains_file(L"kernel.bin");
    if (root == NULL) {
        panic(L"No volume contains kernel.bin.");
    }
    uint64 kernel_entry_addr = load_kernel(root, L"kernel.bin");
    put_param(L"Kernel Entry", kernel_entry_addr);

    // Setup FrameBuffer
    boot_info.screen.base = (void *)GOP->Mode->FrameBufferBase;
    boot_info.screen.size = GOP->Mode->FrameBufferSize;
    boot_info.screen.hr = GOP->Mode->Info->HorizontalResolution;
    boot_info.screen.vr = GOP->Mode->Info->VerticalResolution;
    put_param(L"Screen Base", (uint64)boot_info.screen.base);
    put_param(L"Screen Size", boot_info.screen.size);
    put_param(L"Screen HR", boot_info.screen.hr);
    put_param(L"Screen VR", boot_info.screen.vr);

    boot_info.memtotal = get_total_memory_size();
    put_param(L"Total Memory Size", boot_info.memtotal);

    exit_boot_services(ImageHandle);

    void (*entry)(BootInfo *) = (void (*)(BootInfo *))(kernel_entry_addr);
    entry(&boot_info);
    panic(L"Failed to Start kernel.");
}