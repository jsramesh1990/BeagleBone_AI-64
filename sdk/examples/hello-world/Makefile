# Simple Makefile for Hello World example

# ============================================================================
# Variables
# ============================================================================
CC = arm-poky-linux-gnueabi-gcc
CFLAGS = -O2 -Wall -g
LDFLAGS = 
TARGET = hello
SOURCES = hello.c

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
