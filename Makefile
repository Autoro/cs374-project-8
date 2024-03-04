CC=gcc
CCOPTS=-Wall -Wextra -g
LIBS=

SRCS=$(wildcard *.c)
TARGETS=$(SRCS:.c=)

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

%: %.c
	$(CC) $(CCOPTS) -o $@ $< $(LIBS)
    
