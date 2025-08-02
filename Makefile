CC = gcc
CFLAGS = -Wall
LDFLAGS = -lmicrohttpd -lcollectc -lcjson -lsqlite3

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
OUT = build/my-c-program

.PHONY: all clean run

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build

run: $(OUT)
	./$(OUT)
