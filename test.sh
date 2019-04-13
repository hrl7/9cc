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

show_title char
try 3 "int main() { char a; a = 3; return a;}";
try 120 "int main() { char a; a = 'x'; return a; }"
try 3 "int main() {char x[3];x[0] = -1;x[1] = 2;int y;y = 4;x[0] + y;}"
try 97 "int main() {char d; char c; int a; d = 'a'; c = 123; a = 20; return d;}"

show_title array
try 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
try 3 "int main() { int a[2]; a[0] = 1; a[1] = 2; int *p; p = a; return p[0] + p[1]; }"

show_title function_call
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

echo OK
