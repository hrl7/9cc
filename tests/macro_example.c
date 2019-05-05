#define X 3+4
int x = 123;
#include<libc-header-start.h>
#define Y 123
#define Z 456

int main() {
  printf("X: %d\n", X);
  printf("Y: %d\n", Y);

#ifdef Z
  printf("Z is defined %d\n", Z);
#endif

#ifdef NONE
  printf("NONE is defined %d\n", Z);
#endif

  return 0;
}

