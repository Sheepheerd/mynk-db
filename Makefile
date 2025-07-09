CC = gcc
CFLAGS = -Wall
LDFLAGS = -lsqlite3

SRC = src/*.c
OBJ = $(SRC:.c=.o)
OUT = build/my-c-program

.PHONY: all install clean

all: $(OUT)

# Ensure the build directory exists before linking
$(OUT): $(OBJ)
	@mkdir -p build
	$(CC) $(OBJ) -o $(OUT) $(CFLAGS) $(LDFLAGS)

# Compile source files to object files
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

install: $(OUT)
	@mkdir -p build
	$(OUT)

clean:
	rm -f build/*.o $(OUT)
