# Compilation:
#
# Using default GCC:
# $ make
#
# Using default "native" compiler:
# $ make native
#
# To override compiler, flags, or librarires, specify CC, CFLAGS, LDFLAGS.

NATIVE_CC ?= gcc
NATIVE_CFLAGS ?= -std=c99
NATIVE_LDFLAGS ?=

CC = $(NATIVE_CC)
CFLAGS = $(NATIVE_CFLAGS)
LDFLAGS = $(NATIVE_LDFLAGS)

default: cc

cc:
	$(CC) $(CFLAGS) -o zid zid.c $(LDFLAGS)

c11: NATIVE_CC=gcc
c11: NATIVE_CFLAGS=-std=c11
c11: cc

native.SunOS: NATIVE_CC=c99
native.SunOS: NATIVE_CFLAGS=
native.SunOS: NATIVE_LDFLAGS=-lsocket -lnsl
native.SunOS: cc

native.AIX: NATIVE_CC=c99_r
native.AIX: NATIVE_CFLAGS=
native.AIX: NATIVE_LDFLAGS=-lisode
native.AIX: cc

native.Linux: cc
native.Darwin: cc
native.FreeBSD: cc
native.OpenBSD: cc

UNAME_S := $(shell uname -s)

native: native.$(UNAME_S)

zid.8.txt: zid.8
	groff -man -Tascii $< > $@

#
# `zid` by Andrew Paprocki
#
# To the extent possible under law, the person who associated CC0 with
# `zid` has waived all copyright and related or neighboring rights to
# `zid`.
#
# You should have received a copy of the CC0 legalcode along with this
# work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0>.
#
