CC=gcc
CFLAGS= -Wall -ansi -pedantic -g

ait_shell: $(patsubst %.c, %.o, $(wildcard *.c))
	$(CC) $(CFLAGS) -o $@ $^


%.o : %.c %.h
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f *.o ait_shell
