C_FILES = $(wildcard *.c)

boot.efi: $(C_FILES)
	x86_64-w64-mingw32-gcc -Wall -Wextra -e efi_main -nostdinc -nostdlib \
		-fno-builtin -I ../include -Wl,--subsystem,10 -o $@ $^