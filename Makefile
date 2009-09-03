
CC ?= gcc
BUILD_CC ?= $(CC)
CFLAGS ?= -O2
CFLAGS += -Wall
CPPFLAGS += -DLARGE_FILES -D_FILE_OFFSET_BITS=64
CPPFLAGS += -Ilibwbfs -I.
CPPFLAGS := $(CPPFLAGS) $(shell pkg-config --cflags gmodule-export-2.0 libglade-2.0)
LDFLAGS ?= -s

OBJS = wbfs_gtk.o libwbfs_os.o wbfs_ops.o message.o app_state.o devices.o progress.o list_dir.o $(foreach f,$(LIBWBFS_OBJS),libwbfs/$(f))
LIBWBFS_OBJS = libwbfs.o libwbfs_unix.o wiidisc.o rijndael.o
LDLIBS := $(shell pkg-config --libs gmodule-export-2.0 libglade-2.0)

.PHONY: all clean dist

all: wbfs_gtk

clean:
	rm -f *~ libwbfs/*~ $(OBJS) file2h.o wbfs_gui_glade.h file2h wbfs_gtk

dist: clean
	cd .. && tar cvzf linux-wbfs-manager-$(VERSION).tar.gz --exclude=.svn linux-wbfs-manager

wbfs_gui_glade.h: wbfs_gui.glade file2h
	./file2h $@ $<

file2h: file2h.o
	$(BUILD_CC) -o $@ $<

wbfs_gtk: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

wbfs_gtk.o: wbfs_gui_glade.h
