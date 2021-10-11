#include <stdio.h>


int main(int argc, char** argv)
{
    unsigned a;
    unsigned n, x, t;
    scanf("%d %d %d",&a,&n,&x);
    if (a>9) {
        x = 4;
        t = x;
        a = a%t;
    }
    else {
        t = 3;
    }
    a = a%t;
    a = (a+1)%4;
    a = a%3;
    a = a%(-2);
    a %= 4;
    a /= 8;
    a *= 2;
    a *= 3;
    int y = x/(-2);
    y = x/2;
    printf("%d",x*5);
    printf("%d",x*(-4));
    return 0;
}
