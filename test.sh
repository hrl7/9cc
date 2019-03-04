#!/bin/bash

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s call-printf.o
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

try 21 "main(){5+20-4;}"
try 21 "main(){5 + 20 - 4;}"
try 41 "main(){ 12 + 34 - 5 ;}"
try 47 "main(){5+6*7;}"
try 15 "main(){5*(9-6);}"
try 4 "main(){(3+5)/2;}"
try 55 "main(){1+2+3+4+5+6+7+8+9+10;}"

show_title assignment
try 4 "main(){hoge = 4;hoge;}"
try 8 "main(){a=4;a + 4;}"
try 3 "main(){a = 4; b = 5; c = 6; a + b - c;}"
try 3 "main(){fuga= 4; hoge = 5; piyo  = 6; hoge+fuga-piyo;}"
try 2 "main(){a = 0; b = 0; c = a == b; c + 1; }"

show_title binary operator
try 1 "main(){7 <= 8;}"
try 1 "main(){7 <= 7;}"
try 0 "main(){7 <= 6;}"
try 1 "main(){8 >= 7;}"
try 1 "main(){7 >= 7;}"
try 0 "main(){6 >= 7;}"
try 1 "main(){7 < 8;}"
try 0 "main(){7 < 7;}"
try 0 "main(){7 < 6;}"
try 1 "main(){8 > 7;}"
try 0 "main(){7 > 7;}"
try 0 "main(){6 > 7;}"
try 1 "main(){10 == 10;}"
try 0 "main(){10 == 9;}"
try 0 "main(){10 != 10;}"
try 1 "main(){10 != 9;}"

show_title function_call
try 0 "main(){foo();}"
try 7 "main(){bar(3,4);}"
try 10 "add(a, b) { a + b; } main() {add(8, 2);}"
try 6 "sub(a, b) { a - b; } main() {sub(8, 2);}"
try 10 "sub(a, b) { a - b; } main(){x = sub(8, 2);x + 4;}"
try 0 "\
sub(a, b) { \
  a - b; \
} \
add(a, b) { \
  a + b; \
} \
main() {\
  x = add(8, 2); \
  sub(x, 10); \
}"

show_title if-else
try 1 "main() { b=1;if (1 == 0) b = 0 ; return b; }"
try 0 "main() { b=1;if (0 == 0) b = 0 ; return b; }"

try 0 "main() { b=1;if (1 != 0) b = 0 ; else b = 2; return b; }"
try 2 "main() { b=1;if (0 != 0) b = 0 ; else b = 2; return b; }"

try 1 "main() { b=1;if (1 == 0) {b = 0;} return b; }"
try 0 "main() { b=1;if (0 == 0) {b = 0;} return b; }"
try 2 "main() { b=1;if (0 == 0) {b = 0; if (b == 0) {b = 2;}} return b; }"

try 0 "main() { b=1;if (1 != 0) b = 0 ; else { b = 2; } return b; }"
try 2 "main() { b=1;if (0 != 0) b = 0 ; else { b = 2; } return b; }"

show_title while-loop
try 10 "main() { i = 0; while(i != 10) { i = i + 1; } return i; }"
try 66 "main() { sum = 0; i = 0; while(i <= 10) { i = i + 1; sum = sum + i;  } return sum; }"

show_title for-loop
try 55 "main() { i = 0; sum = 0; for(i=0; i <=10; i = i + 1) { sum = sum + i; } return sum; }"
try 55 "main() { i = 0; sum = 0; for(i=10;i >= 0; i = i - 1) { sum = sum + i; } return sum; }"

show_title recursion
try 6 "fact(n){if(n>0){return n*fact(n-1);}else{return 1;}} main(){return fact(3);}"
try 120 "fact(n){if(n>0){return n*fact(n-1);}else{return 1;}} main(){return fact(5);}"

echo OK
