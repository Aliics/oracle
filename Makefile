.PHONY: rebuild all clean build run debug

SRC=oracle.c log.c flags.c
LIBS=x11 xinerama
OUT=out
BIN=$(OUT)/oracle

rebuild: clean build

all: clean build run

debug: clean build run-debug

clean:
	rm -rf $(OUT)

build:
	mkdir $(OUT)
	cc $(SRC) $$(pkg-config --cflags --libs $(LIBS)) -o $(BIN) -Wall

run:
	./$(BIN)

run-debug:
	./$(BIN) --debug
