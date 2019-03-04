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

try 21 "main(){5+20-4;}"
try 21 "main(){5 + 20 - 4;}"
try 41 "main(){ 12 + 34 - 5 ;}"
try 47 "main(){5+6*7;}"
try 15 "main(){5*(9-6);}"
try 4 "main(){(3+5)/2;}"
try 55 "main(){1+2+3+4+5+6+7+8+9+10;}"
try 4 "main(){hoge = 4;hoge;}"
try 8 "main(){a=4;a + 4;}"
try 3 "main(){a = 4; b = 5; c = 6; a + b - c;}"
try 3 "main(){fuga= 4; hoge = 5; piyo  = 6; hoge+fuga-piyo;}"
try 1 "main(){10 == 10;}"
try 0 "main(){10 == 9;}"
try 0 "main(){10 != 10;}"
try 1 "main(){10 != 9;}"
try 2 "main(){a = 0; b = 0; c = a == b; c + 1; }"
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

try 1 "main() { b=1;if (1 == 0) b = 0 ; b; }"
try 0 "main() { b=1;if (0 == 0) b = 0 ; b; }"
try 0 "main() { b=1;if (1 != 0) b = 0 ; b; }"
try 1 "main() { b=1;if (0 != 0) b = 0 ; b; }"

try 1 "main() { b=1;if (1 == 0) {b = 0;} b; }"
try 0 "main() { b=1;if (0 == 0) {b = 0;} b; }"

try 1 "main() { b=1;if (1 == 0) {b = 0;} b; }"
try 0 "main() { b=1;if (0 == 0) {b = 0;} b; }"

echo OK
