OBJS = cpu.o ide.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

arch.o: $(OBJS)
	$(LD) -r -o $(BUILDROOT)/$@ $^