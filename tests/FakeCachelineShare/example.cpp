#include <stdio.h>

typedef struct testStru2 {
    char t22;
    char t23;
    char t24;
} ts2;

typedef struct testStru1 {
    char t22;
    char t23;
    char t24;
} ts1;

typedef struct testStru3 {
    char t22;
    char t23;
    char t24;
} ts3;


void test1() {
    ts2 test2;
    ts1 test1;
    // for testing FakeCachelineShare
    test2.t22 = '1';
    test2.t22 = '1';
    test2.t22 = '1';
    test2.t22 = '1';
    test2.t22 = '1';
    test2.t22 = '1';
    test2.t22 = '1';
    test2.t22 = test2.t23;
    test1.t22 = test1.t23;
    test1.t22 = test1.t23;
    test1.t22 = test1.t23;
    test1.t22 = test1.t23;
    test1.t22 = test1.t23;
    test1.t22 = test1.t23;
}

void test2() {
    ts3 test1;
    for(int i =0;i<1000;i++)
        test1.t22 = test1.t23;
}

void testP(ts3 *mem)
{
    mem->t22 = mem->t23;
    mem->t22 = mem->t23;
    mem->t22 = mem->t23;
    mem->t22 = mem->t23;
    mem->t22 = mem->t23;
    mem->t22 = mem->t23;
}

void testD(ts3 *mem1)
{
    ts3 mem = *mem1;
    mem.t22 = mem.t23;
    mem.t22 = mem.t23;
    mem.t22 = mem.t23;
    mem.t22 = mem.t23;
    mem.t22 = mem.t23;
    mem.t22 = mem.t23;
}

void test3() {
    ts3 test1;
    testP(&test1);
    testD(&test1);
}

int main()
{
    test1();
    test2();
    test3();
    return 0;
}
