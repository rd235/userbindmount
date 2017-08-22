lib_LTLIBRARIES = libuserbindmount.la
bin_PROGRAMS = userbindmount
man_MANS = userbindmount.1 libuserbindmount.3
include_HEADERS = userbindmount.h

libuserbindmount_la_SOURCES = libuserbindmount.c
libuserbindmount_la_LIBADD = -lcap

userbindmount_SOURCES = userbindmount.c
userbindmount_LDADD = -lcap -luserbindmount

