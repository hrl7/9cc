#include <stdio.h>
#define A_VALUE (5+6)
#define F_FUNC (x ,y) (x+y)
#define X 2
#define Y (X+5)
#define A (B+b)
#define B (A+a)
#define test(x) a x
#define str_2(x) #x

A_VALUE
F_FUNC(5,6)
Y
A
test(test(x))

str_2(hogepiyo)

#define D_IF(b) D_IF_##b
//#define D_IF_2(b) D_IF_##b
#define D_IF_0(x,y) y
#define D_IF_1(x,y) x

D_IF(1)(a,b)
D_IF(0)(a,b)

#define D_Band(a,b) D_IF(a)(b,0)

D_Band(0,1)

#if __INCLUDE_LEVEL__ < 10
//  #include __FILE__
  count:__INCLUDE_LEVEL__
#endif
#define x 0
#define y 0
#if x < 1
  x < 1
#else
  #if y < 1
    not X < 1, y < 1
  #else
    not X < 1 not y < 1
  #endif
#endif

#define D A
D
#define A 0
#undef A

#define F(x,y) (x+y)
F(1,2)

#define printf(x) X x
printf("hoge")

__LINE__

#define G(x) +x
#define G g

G(3)
