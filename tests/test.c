int expect(int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  printf("%d expected, but got %d\n", expected, actual);
  exit_with_error();
}

int init1() { int a = 9; return a;}
int init2() { int a = 3 + 6; return a;}
int init3_x = 13;
int init3() { return init3_x;}

int char1() { char a; return 3;}
int char2() { char a; a = 3; return 3;}
int char3() { char a; a = 3; return a;}
int char4() { char a; a = 'x'; return a; }
int char5() {char x[3];x[0] = -1;x[1] = 2;int y;y = 4;x[0] + y;}
int char6() {char d; char c; int a; d = 'a'; c = 123; a = 20; return d;}

int main() {
  char *msg1 = "abcd";
  printf("binary operators\n");
  expect(1, 7 <= 8);
  expect(1, 7 <= 7);
  expect(0, 7 <= 6);
  expect(1, 8 >= 7);
  expect(1, 7 >= 7);
  expect(0, 6 >= 7);
  expect(1, 7 < 8);
  expect(0, 7 < 7);
  expect(0, 7 < 6);
  expect(1, 8 > 7);
  expect(0, 7 > 7);
  expect(0, 6 > 7);
  expect(1, 10 == 10);
  expect(0, 10 == 9);
  expect(0, 10 != 10);
  expect(1, 10 != 9);
  printf("OK\n");

  printf("arithmetic\n");
  expect(21, 5 + 20 - 4);
  expect(41, 12 + 34 -5);
  expect(47, 5 + 6 * 7);
  expect(15, 5 * (9 - 6));
  expect(4, (3+5)/2);
  expect(55, 1+2+3+4+5+6+7+8+9+10);
  printf("OK\n");

  printf("char\n");
  expect(3, char1());
  expect(3, char2());
  printf("OK\n");

  printf("initialization\n");
  expect(9, init1());
  expect(9, init2());
  expect(13, init3());
  printf("OK\n");

  return 0;
}

