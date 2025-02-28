OBJS = 

%.o: %.c
	$(CC) $(CFLAGS) -c -I ../include $<

drivers.o: $(OBJS)
	$(LD) -r -o ../$@ $^