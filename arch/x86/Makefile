OBJS = cpu.o ide.o pm.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

arch.o: $(OBJS)
	$(LD) -r -o $(BUILDROOT)/$@ $^