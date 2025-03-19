OBJS = init.o pm.o slab.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I ../include $<

mm.o: $(OBJS)
	$(LD) -r -o ../$@ $^