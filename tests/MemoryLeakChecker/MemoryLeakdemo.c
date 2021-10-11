#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

//别名
void alias_case1() {
    char* p1;
    p1 = (char *)malloc(15);
    char* p2 = p1;

    free(p1);

    p1 = (char *)malloc(15);
    p2 = p1;
}

//覆盖
void while_case1(){
    char* p;
    int x = 5;
    while(x < 10){
        p = (char*)malloc(15);
        x++;
    }
    free(p);
}

//多分支泄露和重复释放
void branch_case1() {
    char* p;
    int x = rand()%11;
    if(x == 1){
        p = (char*)malloc(15);
    } else {
        p = (char*)malloc(10);
        free(p);
    }

    int y = rand()%11;
    if(y == 1){
        free(p);
    }
}

void branch_case2()
{
    char *p;
    p = (char *)malloc(100);
    if(p == NULL) return;
    free(p);
}

//多分支嵌套泄漏
void branch_case3(){
    char* p = (char *)malloc(15);
    int x = rand()%10;
    int y = rand()%10;
    int z = rand()%10;
    if(x > 5){
        if(y > 7){
            free(p);
        } else if(y < 3){
            if(z == 7){
                free(p);
            }
        } else {
            free(p);
        }
    } else if(x < 3){
        free(p);
    } else {
        free(p);
    }
}

typedef void(*MyHook)(char*p);
MyHook g_GlobalCallBack = NULL;
void MyFree(char* p)
{
    if(p != NULL)free(p);
}

//过程间
void interprocedural_case1(){
    char *p;

    p = (char *)malloc(100);
    if(p == NULL) return;
    MyFree(p);

    p = (char *)malloc(100);
    if(p == NULL) return;
    g_GlobalCallBack(p);

    p = (char *)malloc(100);
    if(p == NULL) return;
}

void main(){
    g_GlobalCallBack = MyFree;
    interprocedural_case1();
}