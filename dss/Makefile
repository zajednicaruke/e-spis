CC = cc
CFLAGS = -std=c2x -O3 -Wall -Wextra -Wformat=2 -D_GNU_SOURCE -Iinclude

SRC = src/main.c src/sha256.c src/qes.c src/pdf.c src/report.c src/ident.c src/scan.c src/util.c src/der.c src/akd.c src/font_mod.c
OBJ = $(SRC:.c=.o)
TARGET = DSSvalidator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
