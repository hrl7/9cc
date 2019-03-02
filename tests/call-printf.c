int foo() {
  printf("OK\n");
  return 0;
}

int bar(int a, int b) {
  printf("arg1: %d, arg2: %d, => %d\n", a, b, a + b);
  return a + b;
};
