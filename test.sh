#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 3 "1+2;"
assert 21 "5+20-4;"
assert 21 "5 +  20 -4;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 0 '-1 + 1;'
assert 10 '2*-1 + 12;'
assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 1 'a = 1;'
assert 100 'a = 100;'
assert 3 "a = 1; b = 2; a + b;"
assert 1 'avar = 1;'
assert 3 "ab = 1; ac = 2; ab + ac;"
assert 2 "i = 1; i = i + 1; i;"
assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert 0 "0; if (0) return 2; "
assert 2 "if (1) return 2; "
assert 0 "if (0) return 1; else return 0;"
assert 3 "if (0) return 1; else if (0) return 2; else if (1) return 3;"

echo OK
