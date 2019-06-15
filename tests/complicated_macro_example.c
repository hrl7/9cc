#define X 3+4
#define Y 123
#define Z 456
#define A (4+6)
#define B C
#define C B
#define D hogehoge
#define E test

#define F f+G
#define G g+H
#define H h+F
F

#define J(x) (x+2)

//J(H)
int main() {
  printf("X: %d\n", X);
  printf("Y: %d\n", Y);

#ifdef X
  printf("x is defined\n");
#endif

  return 0;
}

