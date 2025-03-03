OBJS = apic.o cpu.o entry.o ide.o idt.o msr.o pm.o proc.o serial.o swtch.o sys.o syscall.o trap.o vectors.o vm.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

%.o: %.S
	$(CC) $(CFLAGS) -Wa,--noexecstack -c $<

arch.o: $(OBJS)
	$(LD) -r -o $(BUILDROOT)/$@ $^

initcode: initcode.S
	$(CC) $(CFLAGS) -nostdinc -c -I $(BUILDROOT)/include initcode.S
	$(LD) -N -e start -Ttext 0x10000000000 -o initcode.out initcode.o
	objdump -S initcode.out > initcode.asm
	rm initcode.o initcode.out