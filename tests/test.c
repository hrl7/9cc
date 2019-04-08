int num_errors = 0;
int expect(int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  printf("%d expected, but got %d\n", expected, actual);
  num_errors = num_errors + 1;
}

int check() {
  if (num_errors == 0) {
    printf("OK\n");
    return 0;
  }
  return 1;
}

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
  if (check()) return 1;

  printf("arithmetic\n");
  expect(21, 5 + 20 - 4);
  expect(41, 12 + 34 -5);
  expect(47, 5 + 6 * 7);
  expect(15, 5 * (9 - 6));
  expect(4, (3+5)/2);
  expect(55, 1+2+3+4+5+6+7+8+9+10);
  if (check()) return 1;

  return 0;
}

