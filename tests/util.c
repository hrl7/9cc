#include <stdio.h>
#include <stdlib.h>

int *allocate(int size) {
  return malloc(size);
}

int *alloc4(int a, int b, int c, int d) {
  int *p = malloc(sizeof(int) * 4);
  p[0] = a;
  p[1] = b;
  p[2] = c;
  p[3] = d;
  return p;
}

int foo() {
  printf("OK\n");
  return 0;
}

int bar(int a, int b) {
  printf("arg1: %d, arg2: %d, => %d\n", a, b, a + b);
  return a + b;
};

int exit_with_error() {
  exit(1);
}
