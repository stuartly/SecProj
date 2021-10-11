#include <stdio.h>

void DD(int *a)
{
    if(a != NULL)
        printf("The content of Pointer a is %d in function D\n", a);

    return;
}

void CC(int *p)
{
    int *n = p;
    if(n != NULL)
    	printf("The content of Pointer n is %d in function C\n", n);
    DD(n);

    return;
}

void BB(int *p)
{
    if(p == NULL)
        printf("Pointer p is Null in function B");
    CC(p);
    return;
}

void AA(int *p)
{
    if(p == NULL)
        printf("Pointer p is Null in function A");
    BB(p);
    return;
}

int main()
{
    int a = 3;
    int *p = &a;

    AA(p);

    return 0;
}
