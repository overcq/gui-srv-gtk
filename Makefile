#*******************************************************************************
# ©overcq                on ‟Gentoo Linux 23.0” “x86_64”              2025‒9‒8 V
#*******************************************************************************
.PHONY: all run build clean
.DEFAULT: all
all: build
#===============================================================================
build: a.out
run: a.out
	./a.out
a.out: Makefile 0.c
	$(CC) -Wall -Wextra -Wno-unused-parameter -Os -s $$(pkg-config --cflags gtk4 ) -o $@ $(filter %.c,$^) $$(pkg-config --libs gtk4 )
clean:
	rm a.out
#*******************************************************************************
