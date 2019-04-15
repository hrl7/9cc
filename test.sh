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

show_title char
try 3 "int main() { char a; a = 3; return a;}";
try 120 "int main() { char a; a = 'x'; return a; }"
try 3 "int main() {char x[3];x[0] = -1;x[1] = 2;int y;y = 4;x[0] + y;}"
try 97 "int main() {char d; char c; int a; d = 'a'; c = 123; a = 20; return d;}"

echo OK
