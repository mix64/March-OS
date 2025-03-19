OBJS = init.o stdio.o 

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

devfs.o: $(OBJS)
	$(LD) -r -o ../$@ $^