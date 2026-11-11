# ============================================================
# DSSvalidator v3.0 — modularni build
# ============================================================
CC       ?= clang
CFLAGS   ?= -std=c2x -O3 -Wall -Wextra -Wformat=2 -D_GNU_SOURCE
CFLAGS   += -Iinclude
LDFLAGS  ?=
PREFIX   ?= $(HOME)/.local
TARGET    = DSSvalidator

SRCS = src/main.c src/sha256.c src/qes.c src/pdf.c \
       src/report.c src/ident.c src/scan.c src/util.c src/der.c src/akd.c src/font_mod.c

OBJS = $(SRCS:.c=.o)

ARCHFLAGS ?=

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(ARCHFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(ARCHFLAGS) -c $< -o $@

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	@echo "Instalirano: $(PREFIX)/bin/$(TARGET)"

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

.PHONY: all install clean rebuild
