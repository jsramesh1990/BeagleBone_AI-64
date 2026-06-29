# Makefile for Sensor Reader Example

# ============================================================================
# Variables
# ============================================================================
CC = arm-poky-linux-gnueabi-gcc
CFLAGS = -O2 -Wall -g -I/usr/include
LDFLAGS = -L/usr/lib
TARGET = sensor-reader
SOURCES = sensor-reader.c

# ============================================================================
# Targets
# ============================================================================
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

install:
	cp $(TARGET) /usr/bin/

.PHONY: all clean install

# ============================================================================
# End of Makefile
# ============================================================================
