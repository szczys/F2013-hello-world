CC=msp430-gcc
CFLAGS=-Os -Wall -g -mmcu=msp430x2013 -std=gnu99

OBJS=main.o


all: $(OBJS)
	$(CC) $(CFLAGS) -o main.elf $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -fr main.elf $(OBJS)
