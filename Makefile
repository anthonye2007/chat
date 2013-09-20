PROGRAMS = chat
CFLAGS = -Wall -Wextra -std=gnu99
CC = gcc

all: $(PROGRAMS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

% : %.o
	$(CC) -o $@ $^

clean:
	rm -f *.o $(PROGRAMS) tmp

