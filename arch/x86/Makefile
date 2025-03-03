OBJS = apic.o cpu.o entry.o ide.o pm.o serial.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

%.o: %.S
	$(CC) $(CFLAGS) -Wa,--noexecstack -c $<

arch.o: $(OBJS)
	$(LD) -r -o $(BUILDROOT)/$@ $^