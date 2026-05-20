CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic -Iinclude
LDFLAGS = -lm
PYTHON = python3

SRC = src/main.c src/legendre_hw.c
OBJ = $(SRC:.c=.o)
BIN = legendre_hw05

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c include/legendre_hw.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

high_precision: run
	$(PYTHON) scripts/add_mpmath_reference_and_errors.py

plots: high_precision
	$(PYTHON) scripts/plot_legendre_errors.py

full: plots

clean:
	rm -f src/*.o $(BIN)
	rm -rf results/*

.PHONY: all run high_precision plots full clean
