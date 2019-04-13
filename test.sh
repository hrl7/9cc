#!/bin/bash

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -no-pie -o tmp tmp.s util.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "***** failed ******"
    echo "$input => $actual"
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

show_title() {
echo
echo "*****************"
echo $1
echo "*****************"
}

show_title string_literal
try 97 "int main() { char *msg = \"abcd\"; return msg[0];}"
try 0 "int main() { char *msg = \"piyopiyo\"; return 0;}"
try 0 "int main() { char *msg = \"abcd\"; return msg[4];}"
try 0 "int main() { printf(\"hello world\"); return 0;}"


show_title char
try 3 "int main() { char a; return 3;}";
try 3 "int main() { char a; a = 3; return 3;}";
try 3 "int main() { char a; a = 3; return a;}";
try 120 "int main() { char a; a = 'x'; return a; }"
try 3 "int main() {char x[3];x[0] = -1;x[1] = 2;int y;y = 4;x[0] + y;}"
try 97 "int main() {char d; char c; int a; d = 'a'; c = 123; a = 20; return d;}"

show_title global_variables
try 7 "int g; int main(){g = 8; return 7;} "
try 8 "int g; int h; int main(){g = 8; return g;} "
try 7 "int *g; int main(){int a[10]; a[5] = 7; g = a + 5; return *g;} "

show_title array
try 7 "int main(){int a[10]; return 7;} "
try 2 "int main(){int a[2]; *a = 2; return *a;} "
try 3 "int main(){int a[2]; *a = 2; a = a + 1; *a = 3; return *a;} "
try 5 "int main(){int *b; int a[2]; b = 0; *a = 2; b = a + 1; *b = 3; return *a + *b;} "
try 5 "int main(){int *b; int a[20]; b = 0; *a = 2; b = a + 19; *b = 3; return *a + *b;} "
try 2 "int main(){int a[3]; *(a + 2) = 2; return *(a + 2);} "
try 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
try 2 "int main(){int a[3]; a[2] = 2; return a[2];} "
try 3 "int main() { int a[2]; a[0] = 1; a[1] = 2; int *p; p = a; return p[0] + p[1]; }"

show_title arithmetic_for_pointer
try 7 "int main(){int *a; a = alloc4(7, 2, 3, 4); int *b; b = a; return *b;}"
try 2 "int main(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = a + 1; return *b;}"
try 3 "int main(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = 2 + a; return *b;}"
try 4 "int main(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = a + 3; return *b;}"
try 4 "int main(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = a + 2 + 1; return *b;}"
try 4 "int main(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = 1 + a + 2; return *b;}"
try 4 "int main(){int *a; a = alloc4(1, 2, 3, 4); int *b; b = 1 + a + 1 * 2; return *b;}"

show_title pointer
try 7 "int main(){int a = 7; int *b = &a; return *b;}"
try 7 "int main(){int a = 5; int *b = &a; return *b + 2;}"
try 7 "int main(){int *a = allocate(4); *a = 7; return *a;}"
# this should be segv
# try 4 "int main(){int *a; *a = 4; return *a;}"
try 7 "int main(){int *x; x = allocate(4); int *y; y = allocate(4); *x = 7; y = x; return *y;}"
try 11 "int main(){int *a; a = allocate(4); *a = 4; int *b; b = allocate(7); *b = 7; return *a + *b;}"

show_title assignment
try 4 "int main(){int hoge; hoge = 4; return hoge;}"
try 8 "int main(){int a; a=4; return a + 4;}"
try 9 "int main(){int a; int b; a = 4; b = 5; return a + b;}"
try 3 "int main(){int a; int b; int c; a = 4; b = 5; c = 6; return a + b - c;}"
try 3 "int main(){int fuga;fuga = 4; int hoge = 5; int piyo  = 6; return hoge+fuga-piyo;}"
try 2 "int main(){int a; a = 0; int b; b = 0; int c; c = a == b; return c + 1; }"

show_title function_call
try 0 "int main(){ return foo();}"
try 7 "int main(){ return bar(3,4);}"
try 10 "int add(int a,int b) {return  a + b; } int main() {return add(8, 2);}"
try 6 "int sub(int a,int  b) {return a - b; } int main() {return sub(8, 2);}"
try 10 "int sub(int a, int b) {return  a - b; } int main(){int x = sub(8, 2);return x + 4;}"
try 0 "\
int sub(int a, int b) { \
  return a - b; \
} \
int add(int a, int b) { \
  return a + b; \
} \
int main() {\
  int x; \
  x = add(8, 2); \
  return sub(x, 10); \
}"

show_title if-else
try 1 "int main() {int b=1;if (1 == 0) b = 0 ; return b; }"
try 0 "int main() {int b=1;if (0 == 0) b = 0 ; return b; }"

try 0 "int main() {int b=1;if (1 != 0) b = 0 ; else b = 2; return b; }"
try 2 "int main() {int b=1;if (0 != 0) b = 0 ; else b = 2; return b; }"

try 1 "int main() {int b=1;if (1 == 0) {b = 0;} return b; }"
try 0 "int main() {int b=1;if (0 == 0) {b = 0;} return b; }"
try 2 "int main() {int b=1;if (0 == 0) {b = 0; if (b == 0) {b = 2;}} return b; }"

try 0 "int main() {int b=1;if (1 != 0) b = 0 ; else { b = 2; } return b; }"
try 2 "int main() {int b=1;if (0 != 0) b = 0 ; else { b = 2; } return b; }"

try 0 "int main() { if(1 == 1) return 0; return 1; }"

show_title while-loop
try 10 "int main() {int i; i = 0; while(i != 10) { i = i + 1; } return i; }"
try 66 "int main() {int sum; int i; sum = 0; i = 0; while(i <= 10) { i = i + 1; sum = sum + i;  } return sum; }"

show_title for-loop
try 55 "int main() {int i; int sum; i = 0; sum = 0; for(i=0; i <=10; i = i + 1) { sum = sum + i; } return sum; }"
try 55 "int main() {int i = 0 ; int sum = 0; for(i=10;i >= 0; i = i - 1) { sum = sum + i; } return sum; }"

show_title recursion
try 120 "int fact(int n){if(n>0){return n*fact(n-1);}else{return 1;}} int main(){return fact(5);}"
try 6 "int fact(int n){if(n>0){return n*fact(n-1);}else{return 1;}} int main(){return fact(3);}"


echo OK
