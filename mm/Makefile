OBJS = slab.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I ../include $<

mm.o: $(OBJS)
	$(LD) -r -o ../$@ $^