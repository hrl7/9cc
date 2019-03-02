C;FLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h call-printf.o

call-printf.o:
	$(CC) -o call-printf.o -c ./tests/call-printf.c

test: 9cc
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o tmp*
