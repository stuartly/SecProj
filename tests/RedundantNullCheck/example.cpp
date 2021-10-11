#include <stdio.h>
int E(int *a)
{
   if(a != NULL)
      printf("The content of pointer a is %d in function E\n",a);
      return 10;
   return 20;

}
void D(int *a)
{
    if(a != NULL)
        printf("The content of Pointer a is %d in function D\n", a);
        int (*f)(int *) = &E;
        int b = f(a);
    return;
}

void C(int *p)
{
    int *n = p;
    if(n != NULL)
        printf("The content of Pointer n is %d in function C\n", n);
        D(n);
    int (*f)(int *) = &E;
    f(n);
    return;
}

void B(int *p)
{
    if(p != NULL)
        printf("Pointer p is not  Null in function B\n");
        C(p);
    return;
}

void A(int *p)
{
    if(p != NULL)
        printf("Pointer p is not null in function A\n");
        B(p);
    return;
}

int main()
{
    int a = 3;
    int *p = &a;

    A(p);

    return 0;
}
