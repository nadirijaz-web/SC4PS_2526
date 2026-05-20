CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic
HDF5_CFLAGS = -I/usr/include/hdf5/serial
HDF5_LIBS = -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5
TARGET = hw06
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(HDF5_CFLAGS) $(SRC) -o $(TARGET) $(HDF5_LIBS) -lm

run: $(TARGET)
	./$(TARGET) input.txt

clean:
	rm -f $(TARGET) *.o *.h5
