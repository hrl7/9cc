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

c-test: 9cc
	./9cc tests/test.c > tmp.s
	$(CC) -no-pie -o tmp tmp.s call-printf.o malloc.o
	@echo "-----run-----"
	./tmp

clean:
	rm -f 9cc *.o tmp* core
