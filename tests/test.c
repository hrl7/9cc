int expect(int line, int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  printf("%s %d: %d expected, but got %d\n",__FILE__, line, expected, actual);
  exit_with_error();
}

int init1() { int a = 9; return a;}
int init2() { int a = 3 + 6; return a;}
int init3_x = 13;
int init3() { return init3_x;}

int char1() {char a; return 3;}
int char2() {char a; a = 3; return 3;}
int char3() {char a; a = 3; return a;}
int char4() {char a; a = 'x'; return a; }
int char5() {char x[3];x[0] = 5;x[1] = 2;int y;y = 4;x[0] + y;}
int char6() {char d; char c; int a; d = 'a'; c = 123; a = 20; return d;}

int string1() { char *msg = "abcd"; return msg[0];}
int string2() { char *msg = "piyopiyo"; return 0;}
int string3() { char *msg = "abcd"; return msg[4];}
int string4() { printf("hello world"); return 0;}

int g1;
int g2;
int h;
int *g3;
int global_var1(){g1 = 8; return 7;}
int global_var2(){g2 = 8; return g2;}
int global_var3(){int a[10]; a[5] = 7; g3 = a + 5; return *g3;}

int array1(){int a[10]; return 7;}
int array2(){int a[2]; *a = 2; return *a;}
int array3(){int a[2]; *a = 2; a = a + 1; *a = 3; return *a;}
int array4(){int *b; int a[2]; b = 0; *a = 2; b = a + 1; *b = 3; return *a + *b;}
int array5(){int *b; int a[20]; b = 0; *a = 2; b = a + 19; *b = 3; return *a + *b;}
int array6(){int a[3]; *(a + 2) = 2; return *(a + 2);}
int array7() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }
int array8(){int a[3]; a[2] = 2; return a[2];}
int array9() { int a[2]; a[0] = 1; a[1] = 2; int *p; p = a; return p[0] + p[1]; }

int ptr_op1(){int *a; a = alloc4(7, 2, 3, 4); int *b; b = a; return *b;}
int ptr_op2(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = a + 1; return *b;}
int ptr_op3(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = 2 + a; return *b;}
int ptr_op4(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = a + 3; return *b;}
int ptr_op5(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = a + 2 + 1; return *b;}
int ptr_op6(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = 1 + a + 2; return *b;}
int ptr_op7(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = 1 + a + 1 * 2; return *b;}

int ptr1(){int a = 7; int *b = &a; return *b;}
int ptr2(){int a = 5; int *b = &a; return *b + 2;}
int ptr3(){int *a = allocate(4); *a = 7; return *a;}
int ptr4(){int *x; x = allocate(4); int *y; y = allocate(4); *x = 7; y = x; return *y;}
int ptr5(){int *a; a = allocate(4); *a = 4; int *b; b = allocate(7); *b = 7; return *a + *b;}

int assign1(){int hoge; hoge = 4; return hoge;}
int assign2(){int a; a=4; return a + 4;}
int assign3(){int a; int b; a = 4; b = 5; return a + b;}
int assign4(){int a; int b; int c; a = 4; b = 5; c = 6; return a + b - c;}
int assign5(){int fuga;fuga = 4; int hoge = 5; int piyo  = 6; return hoge+fuga-piyo;}
int assign6(){int a; a = 0; int b; b = 0; int c; c = a == b; return c + 1; }

int fn_call1(){ return foo();}
int fn_call2(){ return bar(3,4);}
int fn_call3_add(int a,int b) {return  a + b; }
int fn_call3() {return fn_call3_add(8, 2);}
int fn_call4_sub(int a,int  b) {return a - b; }
int fn_call4() {return fn_call4_sub(8, 2);}
int fn_call5_sub(int a, int b) {return  a - b; }
int fn_call5(){int x = fn_call5_sub(8, 2);return x + 4;}
int fn_call6_sub(int a, int b) {return  a - b; }
int fn_call6_add(int a,int b) {return  a + b; }
int fn_call6() {int x;   x = fn_call6_add(8, 2);   return fn_call6_sub(x, 10);   }

int if_else1() {int b=1;if (1 == 0) b = 0 ; return b; }
int if_else2() {int b=1;if (0 == 0) b = 0 ; return b; }
int if_else3() {int b=1;if (1 != 0) b = 0 ; else b = 2; return b; }
int if_else4() {int b=1;if (0 != 0) b = 0 ; else b = 2; return b; }
int if_else5() {int b=1;if (1 == 0) {b = 0;} return b; }
int if_else6() {int b=1;if (0 == 0) {b = 0;} return b; }
int if_else7() {int b=1;if (0 == 0) {b = 0; if (b == 0) {b = 2;}} return b; }
int if_else8() {int b=1;if (1 != 0) b = 0 ; else { b = 2; } return b; }
int if_else9() {int b=1;if (0 != 0) b = 0 ; else { b = 2; } return b; }
int if_else10() { if(1 == 1) return 0; return 1; }

int while_loop1() {int i; i = 0; while(i != 10) { i = i + 1; } return i; }
int while_loop2() {int sum; int i; sum = 0; i = 0; while(i <= 10) { i = i + 1; sum = sum + i;  } return sum; }

int for_loop1() {int i; int sum; i = 0; sum = 0; for(i=0; i <=10; i = i + 1) { sum = sum + i; } return sum; }
int for_loop2() {int i = 0 ; int sum = 0; for(i=10;i >= 0; i = i - 1) { sum = sum + i; } return sum; }

int fact(int n){if(n>0){return n*fact(n-1);}else{return 1;}}

int main() {
  char *msg1 = "abcd";
  printf("binary operators\n");
  expect(__LINE__, 1, 7 <= 8);
  expect(__LINE__, 1, 7 <= 7);
  expect(__LINE__, 0, 7 <= 6);
  expect(__LINE__, 1, 8 >= 7);
  expect(__LINE__, 1, 7 >= 7);
  expect(__LINE__, 0, 6 >= 7);
  expect(__LINE__, 1, 7 < 8);
  expect(__LINE__, 0, 7 < 7);
  expect(__LINE__, 0, 7 < 6);
  expect(__LINE__, 1, 8 > 7);
  expect(__LINE__, 0, 7 > 7);
  expect(__LINE__, 0, 6 > 7);
  expect(__LINE__, 1, 10 == 10);
  expect(__LINE__, 0, 10 == 9);
  expect(__LINE__, 0, 10 != 10);
  expect(__LINE__, 1, 10 != 9);
  printf("OK\n");

  printf("arithmetic\n");
  expect(__LINE__, 21, 5 + 20 - 4);
  expect(__LINE__, 41, 12 + 34 -5);
  expect(__LINE__, 47, 5 + 6 * 7);
  expect(__LINE__, 15, 5 * (9 - 6));
  expect(__LINE__, 4, (3+5)/2);
  expect(__LINE__, 55, 1+2+3+4+5+6+7+8+9+10);
  printf("OK\n");

  printf("char\n");
  expect(__LINE__, 3, char1());
  expect(__LINE__, 3, char2());
  //expect(__LINE__, 3, char3());
  //expect(__LINE__, 120, char4());
  expect(__LINE__, 9, char5());
  //expect(__LINE__, 97, char6());
  printf("OK\n");

  printf("initialization\n");
  expect(__LINE__, 9, init1());
  expect(__LINE__, 9, init2());
  expect(__LINE__, 13, init3());
  printf("OK\n");

  printf("string literal\n");
  expect(__LINE__, 97, string1());
  expect(__LINE__, 0, string2());
  expect(__LINE__, 0, string3());
  expect(__LINE__, 0, string4());
  printf("OK\n");

  printf("global variables\n");
  expect(__LINE__, 7, global_var1());
  expect(__LINE__, 8, global_var2());
  expect(__LINE__, 7, global_var3());
  printf("OK\n");

  printf("array\n");
  expect(__LINE__, 7, array1());
  expect(__LINE__, 2, array2());
  expect(__LINE__, 3, array3());
  expect(__LINE__, 5, array4());
  expect(__LINE__, 5, array5());
  expect(__LINE__, 2, array6());
  expect(__LINE__, 3, array7());
  expect(__LINE__, 2, array8());
  expect(__LINE__, 3, array9());
  printf("OK\n");

  printf("pointer operator\n");
  expect(__LINE__, 7, ptr_op1());
  expect(__LINE__, 2, ptr_op2());
  expect(__LINE__, 3, ptr_op3());
  expect(__LINE__, 4, ptr_op4());
  expect(__LINE__, 4, ptr_op5());
  expect(__LINE__, 4, ptr_op6());
  expect(__LINE__, 4, ptr_op7());
  printf("OK\n");

  printf("pointer\n");
  expect(__LINE__, 7, ptr1());
  expect(__LINE__, 7, ptr2());
  expect(__LINE__, 7, ptr3());
  expect(__LINE__, 7, ptr4());
  expect(__LINE__, 11, ptr5());
  printf("OK\n");

  printf("assignment\n");
  expect(__LINE__, 4, assign1());
  expect(__LINE__, 8, assign2());
  expect(__LINE__, 9, assign3());
  expect(__LINE__, 3, assign4());
  expect(__LINE__, 3, assign5());
  expect(__LINE__, 2, assign6());
  printf("OK\n");


  printf("function call\n");
  expect(__LINE__, 0, fn_call1());
  expect(__LINE__, 7, fn_call2());
  expect(__LINE__, 10, fn_call3());
  expect(__LINE__, 6, fn_call4());
  expect(__LINE__, 10, fn_call5());
  expect(__LINE__, 0, fn_call6());
  printf("OK\n");

  printf("if-else statement\n");
  expect(__LINE__, 1, if_else1());
  expect(__LINE__, 0, if_else2());
  expect(__LINE__, 0, if_else3());
  expect(__LINE__, 2, if_else4());
  expect(__LINE__, 1, if_else5());
  expect(__LINE__, 0, if_else6());
  expect(__LINE__, 2, if_else7());
  expect(__LINE__, 0, if_else8());
  expect(__LINE__, 2, if_else9());
  expect(__LINE__, 0, if_else10());
  printf("OK\n");

  printf("while loop\n");
  expect(__LINE__, 10, while_loop1());
  expect(__LINE__, 66, while_loop2());
  printf("OK\n");
  printf("for loop\n");
  expect(__LINE__, 55, for_loop1());
  expect(__LINE__, 55, for_loop2());
  printf("OK\n");

  printf("recursion\n");
  expect(__LINE__, 6, fact(3));
  expect(__LINE__, 120, fact(5));
  printf("OK\n");
  return 0;
}

