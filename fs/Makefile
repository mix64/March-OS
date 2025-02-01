OBJS = init.o fat16.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I ../include $<

fs.o: $(OBJS)
	$(LD) -r -o ../$@ $^