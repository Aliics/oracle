.PHONY: rebuild all clean build run debug

SRC=oracle.c log.c flags.c
OUT=out
BIN=$(OUT)/oracle

rebuild: build run

all: clean build run

debug: clean build run-debug

clean:
	rm -rf $(OUT)

build:
	mkdir $(OUT)
	cc $(SRC) $$(pkg-config --cflags --libs x11) -o $(BIN) -Wall

run:
	./$(BIN)

run-debug:
	./$(BIN) --debug
