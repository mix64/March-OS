OBJS = init.o dump.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

fat16.o: $(OBJS)
	$(LD) -r -o ../$@ $^