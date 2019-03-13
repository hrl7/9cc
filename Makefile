CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -g -O0 -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h call-printf.o malloc.o

call-printf.o:
	$(CC) -o call-printf.o -c ./tests/call-printf.c

malloc.o:
	$(CC) -o malloc.o -c ./tests/malloc.c

test: 9cc
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o tmp* core
