OBJS = call.o spawn.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I ../include $<

syscall.o: $(OBJS)
	$(LD) -r -o ../$@ $^