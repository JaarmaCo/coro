
CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -Werror -std=c11 -Isrc
OUTDIR := build/linux-x86_64

OBJECTS := $(OUTDIR)/coro.o \
		   $(OUTDIR)/coro_exchange_.o

$(OUTDIR)/coro.o: src/coro.c
	+@[ -f $(OUTDIR) ] || mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTDIR)/coro_exchange_.o: src/linux-x86_64/coro_exchange_.S
	+@[ -f $(OUTDIR) ] || mkdir -p $(OUTDIR)
	$(CC) -c -o $@ $<

$(OUTDIR)/coro-unit: test/coro-unit.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJECTS)

build: $(OUTDIR)/coro-unit

all: clean $(OUTDIR)/coro-unit

.PHONY: clean
clean:
	rm -rf $(OUTDIR)

