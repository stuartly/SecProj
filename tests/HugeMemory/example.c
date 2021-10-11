/*
 * example of checker "2.15     空间问题发现"
 * Author: Jiexuan He
 * Description:
 ********************************************
 *   1. huge global variable                *
 *   2. huge local variable                 *
 *   3. unreasonable .data segmemt          *
 *   4. redundancy member in a structure    *
 ********************************************/

/*
 * In ubuntu 18.04, gcc 7.5.0
 * we compile this file into ELF, and use `nm` to see all symbol:
 * b fixarr
 * d hugeArr1
 * B hugeArr2
 * B hugeArr3
 * B shareNodes
 * d staticlocalnodes
 * D undsVar
 * B/b means .bss
 * D/d means .data
 * hugeExt is Extern and we do not specified locaiton for ld,
 * so not in ELF symbol, and it likes B in this file(TU).
 * Other vars are in function local, store in stack in runtime
 * */

#include <stdio.h>

#define HUGEDADASECTION256 256
#define HUGENUM512 512
#define HUGENUM1024 1024

static int hugeArr1[HUGENUM512] = {0, 1, 2, 3};          // 1 and 3
int hugeArr2[HUGENUM1024];                               // 1
int hugeArr3[HUGENUM1024] = {0, 0, 0, 1 - 1, 2 - 1 - 1}; // 1, but *not* 3

extern long hugeExt[HUGENUM1024]; // 1, redundant
struct MyNode {                   // 4
  int a;
  short b;
  char str[HUGEDADASECTION256];
  struct MyNode *next, *prev;
} shareNodes[2] = {0}; // 1, but *not* 3, because no sence init

struct MyNode undsVar = {1, 2, {'h', 'w'}, NULL, NULL}; // 3

int bubblesort() {
  // 2, but *not* 3, *not* 1, it is static *local*
  static int fixarr[HUGENUM1024];
  int i, j;
  for (i = 0; i < HUGENUM1024; i++) {
    fixarr[i] = (i * i + HUGENUM1024) % HUGENUM512 + 76;
  }
  for (i = 0; i < HUGENUM1024; i++) {
    for (j = 0; j < HUGENUM1024 - i; j++) {
      int tmp = fixarr[i];
      fixarr[i] = fixarr[j];
      fixarr[j] = tmp;
    }
  }
  return 0;
}

int problem2() {
  int localHugeArr[HUGENUM1024]; // 2
  // 2, but *not* 3, because is store in stack, not .data section
  struct MyNode localnodes[2] = {{3, 4, {'p', '4', '0'}, NULL, NULL}};
  // 2 and 3
  static struct MyNode staticlocalnodes[2] = {
      {5, 6, {'m', 'a', 't', 'e'}, NULL, NULL},
      {7, 8, {'n', 'o', 'v', 'a'}, NULL, NULL}};
  return 0;
}

/********************************************
 *   4. redundancy member in a structure    *
 ********************************************/

struct Record1 { // all fields in Record1 use
  char a;
  short b;
  int c;
  unsigned long d;
};

struct Embedrec { // 4
  struct Record1 e;
  struct Record1 *next; // unused *.next field in Embedrec
};

typedef struct Embedrec HType;

int problem4() {
  HType ec1 = {.e = {0, 1, 2, 3}};
  return 0;
}

int main() {
  printf("Hello world\n");
  return 0;
}
