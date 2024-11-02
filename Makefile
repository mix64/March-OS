run: build
	mkdir -p root/EFI/BOOT/
	cp main.efi root/EFI/BOOT/BOOTX64.EFI
	echo "Hello Kernel" > root/kernel.bin
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -hda fat:rw:root

build:
	x86_64-w64-mingw32-gcc -Wall -Wextra -e efi_main -nostdinc -nostdlib \
		-fno-builtin -Wl,--subsystem,10 -o main.efi main.c efi.c common.c

clean:
	-rm -f main.efi root/EFI/BOOT/BOOTX64.EFI root/kernel.bin

env:
	sudo apt install gcc-mingw-w64-x86-64 qemu-system-x86 ovmf