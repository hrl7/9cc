int main() {
  int a[2];
  a[0] = 1;
  a[1] = 2;
  int *p;
  p = a;
  /*
  printf("*a: %d, a[0]: %d, p: %d\n", *a, a[0], *p);
  /*printf("*(a + 1): %d, a[1]: %d, p[1]: %d\n", *(a + 1), a[1], p[1]);
  printf("0: %d, 1: %d\n", *a , *(a + 1));
  printf("num %d\n", *p);
  /*printf("num %d\n", *(p + 1));*/
  printf("num %d\n", *p + *(p + 1));
  return 0;
}
