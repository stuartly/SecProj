#include <stdio.h>

static int gx = 0;
struct testStru {
    int t1;
    char t2;
    int t3[10];
    long t4;
};

typedef struct testStru2 {
    char t22;
    char t23;
    //struct testStru tmp;
    int t3[10];
    char t24;
} ts2;


struct __attribute__((aligned(64))) testStru3 {
    int t1;
    char t2;
    int t3[20];
    long t4;
};

class testClass {
    int c1;
    char c2;
    int c3[10];
public:
    long c4;
    void test(){
        c1 = 1;
        c2 = '1';
        c3[0] = c2;
        c4 = c1;
    }
};
struct __attribute__((aligned(64))) t_struct_inner {
    int a;
    int c3[2];
};


void test1()
{
    struct testStru test1;
    test1.t1 = 1;
    test1.t1 = 2;
    test1.t1 = 3;
    testClass test2;
    test2.c4 = 1;
    test2.c4 = 2;
    test2.c4 = 3;
    t_struct_inner test4;
    test2.c4 = test1.t4;
    ts2 test3;
    test3.t22 = '1';
}

void test2() {
    struct testStru3 test;
    for (int i = 0; i < 3000; i++) {
        test.t1 = 1;
        test.t4 = 1;
    }
}

int main() {
    test1();
    test2();
    return 0;
}
