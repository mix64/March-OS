OBJS = common.o dump.o init.o name.o read.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

fat16.o: $(OBJS)
	$(LD) -r -o ../$@ $^