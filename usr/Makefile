BINS = shell

%: %.c
	$(CC) $(CFLAGS) -c -I ./include $<
	$(LD) -T user.ls -o $(BUILDROOT)/root/bin/$@ $@.o

usr: $(BINS)
