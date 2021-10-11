#include <stdio.h>
#define Num 4096
int my_var;
struct stru{
    int a;
    char b;
    int c[10];
    long d;
};
class testClass{
    int c1;
    char c2;
    int c3[10];
public:
    long c4;
};

int func1(int a,int b,int c,int d,int e,int f){
    const int p=3000;
    int u[100];
    int i=5000;
    //variable-sized object may not be initialized
    //int v[i] = {0};
    int v[i];
    int w[p] = {0};
    int l[Num]={0};
    int arr[2500] = {0};
    int m[] = {1,2,3};
    m[0] = 38;
    arr[3] = 5;
    arr[4]++;
    if(a == 1)
        return 0;
    return 0;
}

void func2(int a,int b,int c,int d,int e,int f,int g,int h) {
    int m,r = 2;
    int i;
    i = 100;
    i++;
    {
        int i = 55;
    }
    int j = 5;
    int o;
    o = 5;
    int k = i;
    k = i + j;
    int n;
    n = 10;
    n = 100;
    for (int n = 0;n<10;n++){
        int k = n+1;
    }
    return;
}

float func3(int x,int b,int c) {
    my_var = 20;
    int t = 12;
    float p = 1.2f;
    int a[] = {1,2,3,4};
    struct stru mys[2999];
    struct stru yy;
    struct stru uu = mys[0];//definition
    uu.a = 12;
    mys[0].a = 100;//binary operator
    mys[0].c[2] = 3;
    uu.c[3] = 34;
    mys[0].a++;//unary operator
    mys[0].c[2]--;
    testClass test2;
    test2.c4 = 1;
    test2.c4 = 2;
    testClass d[100];
    {
        int a[10] = {0};
        a[0] = 100;
        a[0]++;
    }
    if(x == 1)
        return 2.1;
    return 0.0;
}

void fun4(){
    int a = 2;
    int *b = &a;
    int *c;
    a = 15;
}


int fun5(){
    my_var = 10;
    int m = 20;//should report
    int n = Num;//should report
    float f = func3(1,2,3);//shouldn't report
    //return n+20//shouldn't report
    //return Num+20//could report
    int a = 9;
    a++;
    int l = a;
    return l;//shouldn't report
}

int fun6(){
    return 3;
}

int fun7(int x){
    if ( x > 10) {
        return 1;
    }else{
        return 0;
    }
}

int test0(int a,int b,int c,int d,int e,int f){
    return a+b+c+d+e+f;
}
int test2(int a){
    return a+2;
}
int g_max=0xFFFFFFFF;
int test4(int a,int b){
    int g=g_max;
    return g+a;
}
int main()
{
    /*int m = func1(1,2,3,4,5,6);
    func2(2,3,4,5,6,7,8,9);
    float n = func3(1,2,3);
    fun4();*/
    int k = fun5();//don't report?
    int o = fun6();
    int u = fun7(6);
    return 0;
}
