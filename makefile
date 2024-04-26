# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Werror
SRC = src/*.c
OUTFILE = FJoys_driver

# Default target
all: $(OUTFILE)

$(OUTFILE):
	$(CC) $(CFLAGS) $(SRC) -o $(OUTFILE)

# Cleaning option
clean:
	rm -f $(OUTFILE)

