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

CFLAGS += -D__DEBUG__

run: build
	qemu-system-x86_64 -d cpu_reset -D ./debug.log \
	  -bios /usr/share/ovmf/OVMF.fd -m 512M \
	  -chardev stdio,mux=on,id=com1 -serial chardev:com1 \
	  -device ich9-ahci,id=ahci \
	  -hda fat:rw:root

build: boot.efi kernel.bin

boot.efi:
	mkdir -p root/EFI/BOOT/
	make -C boot
	mv boot/boot.efi root/EFI/BOOT/BOOTX64.EFI

kernel.bin: apic.o entry.o kernel.o serial.o pm.o pci.o vectors.o trap.o vm.o x86.o slab.o
	$(LD) -T kernel.ls -o root/kernel.bin $^

%.o: %.c
	$(CC) $(CFLAGS) -c -I ./include $<

%.o: %.S
	$(CC) $(CFLAGS) -Wa,--noexecstack -c $<

clean:
	-rm -f root/EFI/BOOT/BOOTX64.EFI root/kernel.bin *.o *.d

env:
	sudo apt install gcc-mingw-w64-x86-64 qemu-system-x86 ovmf