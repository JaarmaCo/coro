CC := cl
AS := ml64
CFLAGS := /Wall /wd4820 /WX /std:c11 /Isrc /Zi /DEBUG /Od

OUTDIR := build/windows-x86_64

OBJECTS := $(OUTDIR)/coro.obj \
		   $(OUTDIR)/coro_exchange_.obj

$(OUTDIR)/coro.obj: src/coro.c
	+@[ -f $(OUTDIR) ] || mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) /c /Fo$@ $<

$(OUTDIR)/coro_exchange_.obj: src/windows-x86_64/coro_exchange_.masm
	+@[ -f $(OUTDIR) ] || mkdir -p $(OUTDIR)
	$(AS) /c /Zi /DEBUG /Od /Fo$@ $<

$(OUTDIR)/coro-unit.exe: test/coro-unit.c $(OBJECTS)
	$(CC) $(CFLAGS) /Fe$@ $< $(OBJECTS)

build: $(OUTDIR)/coro-unit.exe

all: clean $(OUTDIR)/coro-unit.exe

.PHONY: clean
clean:
	rm -rf $(OUTDIR)


