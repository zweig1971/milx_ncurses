# PREFIX  controls where programs and libraries get installed
# Example usage:
#   make PREFIX=/usr all

PWD     := $(shell pwd)
HOME    = $(PWD)
ODIR    = $(HOME)
IDIR    = $(HOME)
PREFIX  ?= /usr/local
LIBA	= -lscumil 
LIBB	= -lpanel
LIBC	= -lncurses
LIBS    = -letherbone
FLAG    = DONTSIMULATE

all:
	g++ -g -Wall -D$(FLAG) $(INCLUDE) -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/example $(IDIR)/example.cpp $(LIBA) $(LIBS) $(LIBB) $(LIBC) 

clean:
	@echo cleaning stuff
	rm -f $(ODIR)/example

