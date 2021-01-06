ODIR=./build/objs
BDIR=./build/bin

IDIR=./src
SDIR=./src

CC=clang
LINK=clang
DFLAGS=-g -O0 -fsanitize=address
RFLAGS=-O3 -g -march=native
CFLAGS=-I$(IDIR) -I$(IDIR)/regex/src -Wall $(RFLAGS) -flto=thin
LIBS=-lpng -lz -lm

_SRC=$(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/*/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(_SRC))

DEPS=$(wildcard $(IDIR)/*.h)

_BIN=raycast
BIN=$(patsubst %,$(BDIR)/%,$(_BIN))

.PHONY: all
all: $(BIN)

$(BDIR)/$(_BIN): $(OBJ)
	mkdir -p `dirname $@`
	$(LINK) $(CFLAGS) -o $@ $^ $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: new
new: clean all
	
.PHONY: clean
clean:
	rm -fr $(ODIR)/*

.PHONY: cleanall
cleanall:
	rm -fr $(ODIR)/* $(BDIR)/*

.PHONY: check
check: all
	./build/bin/test
