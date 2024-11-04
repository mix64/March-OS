CC = gcc
LD = ld

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -fno-asynchronous-unwind-tables -O2 -Wall -MD -ggdb -Werror -fno-omit-frame-pointer -nostdinc
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

run: build
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -hda fat:rw:root

build: boot.efi kernel.bin

boot.efi:
	mkdir -p root/EFI/BOOT/
	make -C boot
	mv boot/boot.efi root/EFI/BOOT/BOOTX64.EFI

kernel.bin:
	$(CC) $(CFLAGS) -o kernel.o -c kernel.c
	$(LD) -T kernel.ls -o root/kernel.bin kernel.o

clean:
	-rm -f root/EFI/BOOT/BOOTX64.EFI root/kernel.bin kernel.o kernel.d

env:
	sudo apt install gcc-mingw-w64-x86-64 qemu-system-x86 ovmf