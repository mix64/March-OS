OBJS = inode.o vfs.o
SUBDIR = fat16

all: fs.o

%.o: %.c
	$(CC) $(CFLAGS) -c -I $(BUILDROOT)/include $<

$(SUBDIR:=.o):
	make -C $(@:.o=)

fs.o: $(OBJS) $(SUBDIR:=.o)
	$(LD) -r -o ../$@ $^