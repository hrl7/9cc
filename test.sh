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

try 21 '5+20-4;'
try 21 '5 + 20 - 4;'
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 55 "1+2+3+4+5+6+7+8+9+10;"
try 4 "hoge = 4;hoge;"
try 8 "a=4;a + 4;"
try 3 "a = 4; b = 5; c = 6; a + b - c;"
try 3 "fuga= 4; hoge = 5; piyo  = 6; hoge+fuga-piyo;"
try 1 "10 == 10;"
try 0 "10 == 9;"
try 0 "10 != 10;"
try 1 "10 != 9;"
try 2 "a = 0; b = 0; c = a == b; c + 1; "
try 0 "foo();"
try 3 "bar(3,4);"
echo OK
