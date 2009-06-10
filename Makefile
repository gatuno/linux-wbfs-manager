
CC = gcc
CFLAGS = -O2 -Wall -DLARGE_FILES -D_FILE_OFFSET_BITS=64 -Ilibwbfs -I. `pkg-config --cflags gmodule-export-2.0 libglade-2.0`
LDFLAGS = -s

OBJS = wbfs_gtk.o libwbfs_os.o wbfs_ops.o message.o app_state.o progress.o list_dir.o $(foreach f,$(LIBWBFS_OBJS),libwbfs/$(f))
LIBWBFS_OBJS = libwbfs.o libwbfs_unix.o wiidisc.o rijndael.o
LIBS = `pkg-config --libs gmodule-export-2.0 libglade-2.0`

all: wbfs_gtk

clean:
	rm -f *~ libwbfs/*~ $(OBJS) wbfs_gtk

wbfs_gtk: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
