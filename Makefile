#*******************************************************************************
# ©overcq                on ‟Gentoo Linux 23.0” “x86_64”              2025‒9‒8 V
#*******************************************************************************
.PHONY: all build install uninstall clean
.DEFAULT: all
all: build
#===============================================================================
build: a.out
a.out: Makefile 0.c
	$(CC) -Wall -Wextra -Wno-unused-parameter -Os -s $$(pkg-config --cflags gtk4) -o $@ $(filter %.c,$^) $$(pkg-config --libs gtk4)
install:
	install -C -m 755 a.out /usr/bin/gui-srv-gtk
uninstall:
	rm /usr/bin/gui-srv-gtk
clean:
	rm a.out
#*******************************************************************************
