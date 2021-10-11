#include <stdio.h>

class A {
public:
  int a;
  int b;
  int c[100];
};

int main(int argc, char **argv) {
  int size = 100;
  A a[size];

  for (int i = 0; i < 99; i += 2) {
    a[i].a = i;
    a[i].b = i;
  }

  int b[100];
  A d;
  for (int i = 0; i < 100; i++) {
    b[i] = i;
    d.c[i] = i;
  }
  return 0;
}
