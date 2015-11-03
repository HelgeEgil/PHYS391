# Makefile for Beginning Linux Programming 3rd Edition
# Chapter 11

TIMER=ARMtimer timer

CFLAGS=-g
LDFLAGS=-g

ALL= $(TIMER)

all: $(ALL)

clean:
	@rm -f $(ALL) *~ *.o
