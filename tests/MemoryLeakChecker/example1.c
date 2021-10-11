#include <stdio.h>
#include <malloc.h>

typedef void(*MyHook)(char*p);
MyHook g_GlobalCallBack = NULL;
void MyFree(char* p)
{
    if(p != NULL)free(p);
}

void LeakTest1()
{
    char *p;
    g_GlobalCallBack = MyFree;

    char* p1;
    char* p2;

    p1 = (char*)malloc(100);
    if(p1 != NULL){
        p2 = p1;
        free(p2);
    }
    //free(p1);

    p = (char *)malloc(100);
    if(p == NULL) return;
    free(p);

    p = (char *)malloc(100);
    if(p == NULL) return;
    MyFree(p);

    p1 = (char*)malloc(100);
    char*alias = p1;
    if(p1 == NULL)return;
    if(alias != NULL){
        //p2 = p1;
        free(p1);
    }


    p = (char*)malloc(100);
    if(p == NULL)return;
    g_GlobalCallBack(p);
}