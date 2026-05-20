CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra -pedantic
LDFLAGS := -lm

TARGET := homework07
OBJS := homework07.o rng.o

.PHONY: all run plots clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

homework07.o: homework07.c rng.h
	$(CC) $(CFLAGS) -c homework07.c

rng.o: rng.c rng.h
	$(CC) $(CFLAGS) -c rng.c

run: $(TARGET)
	./$(TARGET)

plots: run
	MPLCONFIGDIR=/tmp/mpl-hw07 python3 plot_results.py

clean:
	rm -f $(TARGET) $(OBJS)
	rm -rf output figures
