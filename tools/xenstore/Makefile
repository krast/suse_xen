XEN_ROOT=$(CURDIR)/../..
include $(XEN_ROOT)/tools/Rules.mk

MAJOR = 3.0
MINOR = 0

CFLAGS += -Werror -fno-strict-aliasing
CFLAGS += -I.
CFLAGS += $(CFLAGS_libxenctrl)

CLIENTS := xenstore-exists xenstore-list xenstore-read xenstore-rm xenstore-chmod
CLIENTS += xenstore-write xenstore-ls xenstore-watch
CLIENTS_DOMU := $(patsubst xenstore-%,domu-xenstore-%,$(CLIENTS))

XENSTORED_OBJS = xenstored_core.o xenstored_watch.o xenstored_domain.o xenstored_transaction.o xs_lib.o talloc.o utils.o tdb.o hashtable.o

XENSTORED_OBJS_$(CONFIG_Linux) = xenstored_linux.o
XENSTORED_OBJS_$(CONFIG_SunOS) = xenstored_solaris.o xenstored_probes.o
XENSTORED_OBJS_$(CONFIG_NetBSD) = xenstored_netbsd.o

XENSTORED_OBJS += $(XENSTORED_OBJS_y)

ifneq ($(XENSTORE_STATIC_CLIENTS),y)
LIBXENSTORE := libxenstore.so
else
LIBXENSTORE := libxenstore.a
xenstore xenstore-control: CFLAGS += -static
endif

ALL_TARGETS = libxenstore.so libxenstore.a clients xs_tdb_dump xenstored

.PHONY: all
all: $(ALL_TARGETS)

.PHONY: clients
clients: xenstore $(CLIENTS) $(CLIENTS_DOMU) xenstore-control

ifeq ($(CONFIG_SunOS),y)
xenstored_probes.h: xenstored_probes.d
	dtrace -C -h -s xenstored_probes.d

xenstored_solaris.o: xenstored_probes.h

xenstored_probes.o: xenstored_solaris.o
	dtrace -C -G -s xenstored_probes.d xenstored_solaris.o 

CFLAGS += -DHAVE_DTRACE=1
endif
 
xenstored: $(XENSTORED_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS_libxenctrl) $(SOCKET_LIBS) -o $@

$(CLIENTS): xenstore
	ln -f xenstore $@

$(CLIENTS_DOMU): xenstore
	ln -f xenstore $@

xenstore: xenstore_client.o $(LIBXENSTORE)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -L. -lxenstore $(SOCKET_LIBS) -o $@
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,--build-id=uuid $< -L. -lxenstore $(SOCKET_LIBS) -o domu-$@

xenstore-control: xenstore_control.o $(LIBXENSTORE)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -L. -lxenstore $(SOCKET_LIBS) -o $@

xs_tdb_dump: xs_tdb_dump.o utils.o tdb.o talloc.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

libxenstore.so: libxenstore.so.$(MAJOR)
	ln -sf $< $@
libxenstore.so.$(MAJOR): libxenstore.so.$(MAJOR).$(MINOR)
	ln -sf $< $@

xs.opic: CFLAGS += -DUSE_PTHREAD

libxenstore.so.$(MAJOR).$(MINOR): xs.opic xs_lib.opic
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,$(SONAME_LDFLAG) -Wl,libxenstore.so.$(MAJOR) $(SHLIB_LDFLAGS) -o $@ $^ $(SOCKET_LIBS) -lpthread

libxenstore.a: xs.o xs_lib.o
	$(AR) rcs $@ $^

.PHONY: clean
clean:
	rm -f *.a *.o *.opic *.so* xenstored_probes.h
	rm -f xenstored xs_random xs_stress xs_crashme
	rm -f xs_tdb_dump xenstore-control
	rm -f $(CLIENTS) $(CLIENTS_DOMU)
	$(RM) $(DEPS)

.PHONY: TAGS
TAGS:
	etags `find . -name '*.[ch]'`

.PHONY: tarball
tarball: clean
	cd .. && tar -c -j -v -h -f xenstore.tar.bz2 xenstore/

.PHONY: install
install: all
	$(INSTALL_DIR) $(DESTDIR)$(BINDIR)
	$(INSTALL_DIR) $(DESTDIR)$(SBINDIR)
	$(INSTALL_DIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL_DIR) $(DESTDIR)/bin
	$(INSTALL_DIR) $(DESTDIR)/var/run/xenstored
	$(INSTALL_DIR) $(DESTDIR)/var/lib/xenstored
	$(INSTALL_PROG) xenstored $(DESTDIR)$(SBINDIR)
	$(INSTALL_PROG) xenstore-control $(DESTDIR)$(BINDIR)
	$(INSTALL_PROG) xenstore $(DESTDIR)$(BINDIR)
	set -e ; for c in $(CLIENTS) ; do \
		ln -fs /usr/bin/xenstore $(DESTDIR)/usr/bin/$${c} ; \
	done
	$(INSTALL_PROG) domu-xenstore $(DESTDIR)/bin
	for client in $(CLIENTS_DOMU); do \
	    ln -fs /bin/domu-xenstore $(DESTDIR)/bin/$${client/domu-}; \
	done
	$(INSTALL_DIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROG) libxenstore.so.$(MAJOR).$(MINOR) $(DESTDIR)$(LIBDIR)
	ln -sf libxenstore.so.$(MAJOR).$(MINOR) $(DESTDIR)$(LIBDIR)/libxenstore.so.$(MAJOR)
	ln -sf libxenstore.so.$(MAJOR) $(DESTDIR)$(LIBDIR)/libxenstore.so
	$(INSTALL_DATA) libxenstore.a $(DESTDIR)$(LIBDIR)
	$(INSTALL_DATA) xs.h $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL_DATA) xs_lib.h $(DESTDIR)$(INCLUDEDIR)

-include $(DEPS)

# never delete any intermediate files.
.SECONDARY:
