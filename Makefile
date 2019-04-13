CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

build:
	docker build . -t 9cc
run:
	docker run -it --rm -v `pwd`:/root/app 9cc sh

9cc: $(OBJS)
	$(CC) -g -O0 -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h util.o

util.o:
	$(CC) -o util.o -c ./tests/util.c

test: 9cc
	make sh-test
	make c-test

sh-test: 9cc
	./9cc -test
	./test.sh

c-test: 9cc
	./9cc tests/test.c > tmp.s
	$(CC) -no-pie -o tmp tmp.s util.o
	@echo "-----run-----"
	./tmp
c-test-tmp: 9cc
	./9cc tests/tmp.c > tmp.s
	$(CC) -no-pie -o tmp tmp.s util.o
	@echo "-----run-----"
	./tmp

clean:
	rm -f 9cc *.o tmp* core
