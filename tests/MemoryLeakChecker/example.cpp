#include <stdio.h>
#include <iostream>
#include <malloc.h>
#include <cstdlib>

using namespace std;

// char* student;

// void freeMem(char* str) {
//     student = (char *) malloc(15);
//     free(str);
// }

// char* getMem() {
//     char* str = (char *) malloc(15);
//     return str;
// }

// void right_demo(){
//     char* str = (char *)malloc(15);
//     free(str);
// }

// //简单情况
// void mem_leak1() {
//     char* str;
//     str = (char *)malloc(15);
// }

// //别名
// void mem_leak2() {
//     char* str = (char *)malloc(15);
//     char* mem = (char *)malloc(15);
//     str = mem;
//     free(mem);
// }

// //覆盖
// void mem_leak3() {
//     char* str = (char *)malloc(15);
//     str = (char*)malloc(15);
// }

// //过程间
// void mem_leak4() {
//     char* str = (char *)malloc(15);
//     str = getMem();
// }

// //多分支泄漏
// void mem_leak5(){
//     char* strs;
//     int x = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//     } else {
//         strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else {
//         y = 1;
//     }
// }

// //别名
// void mem_leak6() {
//     char* str;
//     str = (char *)malloc(15);
//     char* mem;
//     mem = str;
//     str = (char *)malloc(10);
//     free(str);
// }

// //重复释放
// void double_free1() {
//     char* str = (char *)malloc(15);
//     free(str);
//     free(str);
// }

// //别名重复释放
// void double_free2() {
//     char* str = (char *)malloc(15);
//     char* mem = str;
//     free(str);
//     free(mem);
// }

// //过程间
// void double_free3() {
//     char* mem = (char *)malloc(15);
//     freeMem(mem);
//     free(mem);
// }

// //多分支泄漏和嵌套
// void branch_alloc1() {
//     char* strs;
//     int x = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//     } else {
//         strs = (char*)malloc(10);
//         free(strs);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else {
//         y = 1;
//     }
// }

// //多分支泄漏
// void branch_alloc2() {
//     char* strs = (char*)malloc(15);
//     int x = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//     } else {
//         strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else {
//         y = 1;
//     }
// }

// //多分支正常，覆盖泄漏
// void branch_alloc3() {
//     char* strs = (char*)malloc(15);
//     int x = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//     } else {
//         strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else {
//         free(strs);
//         y = 1;
//     }
// }

// //多分支部分未申请
// void branch_alloc4() {
//     char* strs = (char*)malloc(15);
//     int x = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//     } else {
//         //strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else {
//         free(strs);
//         y = 1;
//     }
// }

// //多分支部分未申请未释放
// void branch_alloc7() {
//     char* strs = (char*)malloc(15);
//     int x = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//     } else {
//         //strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else {
//         //free(strs);
//         y = 1;
//     }
// }

// //多分支嵌套申请+重复释放
// void branch_alloc5() {
//     char* strs;
//     int x = rand();
//     int z = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//         if(z == 1){
//             strs = (char*)malloc(15);
//             free(strs);
//         } else if(z == 2){
//             strs = (char*)malloc(15);
//         }
//         //strs = (char*)malloc(15);
//     } else {
//         strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else if(y == 2){
//         free(strs);
//         y = 1;
//     }
// }

// //多分支别名泄漏
// void branch_alloc6() {
//     char* strs;
//     char* mem;
//     int x = rand();
//     int z = rand();
//     if(x == 1){
//         strs = (char*)malloc(15);
//         mem = strs;
//         strs = (char*)malloc(15);
//         //strs = (char*)malloc(15);
//     } else {
//         strs = (char*)malloc(10);
//     }

//     int y = rand();
//     if(y == 1){
//         free(strs);
//     } else if(y == 2){
//         free(strs);
//         y = 1;
//     }
//     free(mem);
// }

// void test(char*str){
//     char *tmp = str;
// }

// //过程间
// void mem_leak7(){
//     char* str = (char*)malloc(10);
//     test(str);
//     free(str);
// }

// char* rte_malloc(int size){
//     char* str;
//     return str;
// }

// void rte_free(char* str){}

// char* rte_pktmbuf_alloc(int size){
//     char* str;
//     return str;
// }

// void rte_pktmbuf_free(char* str){}

// void afreeMem(char* str) {
//     rte_free(str);
// }

// char* agetMem() {
//     char* str = rte_malloc(15);
//     return str;
// }

// //多接口
// void amem_leak1() {
//     char* str = rte_malloc(10);
//     rte_free(str);
//     char* strs = rte_pktmbuf_alloc(10);
//     rte_pktmbuf_free(strs);
// }

// //内存泄漏
// void amem_leak2() {
//     char* str = rte_malloc(15);
//     char* mem = rte_pktmbuf_alloc(15);
// }

// //重复释放
// void adouble_free1() {
//     char* str = rte_malloc(15);
//     rte_free(str);
//     rte_free(str);
// }

// //别名
// void adouble_free2() {
//     char* str = rte_malloc(15);
//     char* mem = str;
//     rte_free(str);
//     rte_free(mem);
// }

// //多接口，过程间
// void adouble_free3() {
//     char* mem = rte_malloc(15);
//     rte_pktmbuf_free(mem);
//     afreeMem(mem);
//     rte_free(mem);
// }

// //都free正常分支
// void branch1(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     if(x == 10){
//         free(str);
//     } else if(x == 5){
//         free(str);
//     } else {
//         free(str);
//     }
// }

// //部分未free
// void branch2(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     if(x == 10){
//         free(str);
//     } else if(x == 5){
//         x = 10;
//         free(str);
//     } else {
//         x = 7;
//         //free(str);
//     }

//     char* mem = (char *)malloc(15);
//     if(x == 10){
//         free(mem);
//     } else if(x == 5){
//         x = 10;
//         free(mem);
//     } 
// }

// //double free
// void branch3(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     if(x == 10){
//         free(str);
//     } else if(x == 5){
//         //free(str);
//     } else {
//         free(str);
//     }
//     free(str);
// }

// //if嵌套正常
// void branch4(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     int y = 7;
//     if(x == 10){
//         // if(y == 7){
//         //     free(str);
//         // } else if(y == 10){
//         //     free(str);
//         // } else {
//         //     free(str);
//         // }
//         free(str);
//     } else if(x == 5){
//         if(y == 7){
//             free(str);
//         } else if(y == 10){
//             free(str);
//         } else {
//             free(str);
//         }
//         //free(str);
//     } else {
//         free(str);
//     }
// }

// //if嵌套泄漏和重复释放
// void branch5(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     int y = 7;
//     if(x == 10){
//         if(y == 7){
//             free(str);
//         } else if(y == 10){
//             y = 5;
//             //free(str);
//         } else {
//             free(str);
//         }
//         free(str);
//     } else if(x == 5){
//         if(y == 7){
//             free(str);
//         } else if(y == 10){
//             //free(str);
//         } else {
//             free(str);
//         }
//         //free(str);
//     } else {
//         free(str);
//     }
// }

// //if嵌套重复释放
// void branch6(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     int y = 7;
//     int z = 10;
//     if(x == 10){
//         if(y == 7){
//             free(str);
//         } else if(y == 10){
//             free(str);
//             if(z == 7){
//                 free(str);
//             }
//         } else {
//             free(str);
//         }
//     } else if(x == 5){
//         free(str);
//     } else {
//         free(str);
//     }
// }

// //if多重嵌套泄漏
// void branch7(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     int y = 7;
//     int z = 10;
//     if(x == 10){
//         if(y == 7){
//             free(str);
//         } else if(y == 10){
//             //free(str);
//             if(z == 7){
//                 free(str);
//             }
//         } else {
//             free(str);
//         }
//     } else if(x == 5){
//         free(str);
//     } else {
//         free(str);
//     }
// }

// //switch分支泄漏
// void switch_branch1(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         case 3:
//             x = 7;
//             break;
//         case 7:
//             str = (char *)malloc(15);
//             x = 10;
//         default:
//             x = 0;
//     }
// }

// //switch分支default泄漏
// void switch_branch2(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         case 3:
//             free(str);
//             x = 7;
//             break;
//         case 7:
//             str = (char *)malloc(15);
//             free(str);
//             x = 10;
//         default:
//             x = 0;
//     }
// }

// //switch分支正常
// void switch_branch3(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         case 3:
//             free(str);
//             x = 7;
//             break;
//         case 7:
//             str = (char *)malloc(15);
//             free(str);
//             x = 10;
//         default:
//             free(str);
//             x = 0;
//     }
// }

// //switch分支无default泄漏
// void switch_branch4(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         case 3:
//             free(str);
//             x = 7;
//             break;
//         case 7:
//             str = (char *)malloc(15);
//             free(str);
//             x = 10;
//     }
// }

// //switch分支重复释放
// void switch_branch5(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         case 3:
//             free(str);
//             x = 7;
//             break;
//         case 7:
//             free(str);
//             x = 10;
//         default:
//             x = 0;
//     }
//     free(str);
// }

// void free_active1(char* str){
//     free(str);
// }

// //分支过程间重复释放
// void branch_fun1(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     free_active1(str);
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         default:
//             x = 0;
//     }
// }

// void free_active2(char* str){
//     int x = rand();
//     if(x == 3){
//         free(str);
//     }
// }

// //分支过程间泄漏
// void branch_fun2(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     free_active2(str);
//     switch(x){
//         case 5:
//             free(str);
// 	        x = 3;
//             break;
//         default:
//             x = 0;
//     }
// }

// void free_active3(char* str){
//     int x = rand();
//     switch(x){
//         case 5:
//             free(str);
//             break;
//     }
// }

// //分支多过程间泄漏和重复释放
// void branch_fun3(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     free_active2(str);
//     free_active3(str);
// }

// void free_active4(char* str){
//     int x = rand();
//     switch(x){
//         case 5:
//             free(str);
//             break;
//         default:
//             free(str);
//     }
// }

// //分支多过程间正常(重复释放)
// void branch_fun4(){
//     char* str = (char *)malloc(15);
//     int x = 10;
//     free_active2(str);
//     free_active4(str);
// }

// //中途返回的分支
// void return_branch1(){
//     char* str = (char*)malloc(15);
//     int x;
//     if(x == 5){
//         x = 7;
//         char* t = (char*)malloc(15);
//         return;
//     } 
//     free(str);
// }

// //中途返回的分支
// void return_branch2(){
//     char* str = (char*)malloc(15);
//     char* t;
//     int x;
//     if(x == 5){
//         x = 7;
//         t = (char*)malloc(15);
//         return;
//     } 
//     free(t);
//     free(str);
// }

// void branch8(){
//     char* str = (char*)malloc(15);
//     int x = rand();
//     if(x == 5){
//         free(str);
//     } else {
//         free(str);
//         return;
//     }
// }

// //if分支中有return导致的if初始化错误
// void branch9(){
//     char* str = (char*)malloc(15);
//     int x = rand();
//     int y = rand();
//     if(x > 5){
//         x = 3;
//         free(str);
//         return;
//     } else {
//         if(y == 7){
//             free(str);
//         }
//     }
// }

// //while循环泄漏
// void while_case1(){
//     char* str;
//     int x = 5;
//     while(x < 10){
//         str = (char*)malloc(15);
//         x++;
//     }
//     free(str);
// }

// //while循环重复释放
// void while_case2(){
//     char* str = (char*)malloc(15);;
//     int x = 5;
//     while(x < 10){
//         free(str);
//         x++;
//     }
//     free(str);
// }

// //while循环+return
// void while_case3(){
//     char* str;
//     int x = 5;
//     while(x < 10){
//         str = (char*)malloc(15);
//          return;
//         x++;
//     }
//     free(str);
// }

// //do循环
// void do_case1(){
//     char* str;
//     int x = 5;
//     do{
//         str = (char*)malloc(15);
//     } while(x < 10);
//     free(str);
// }

// //for循环
// void for_case1(){
//     char* str;
//     for(int i = 1;i < 5;i++){
//         str = (char*)malloc(15);
//     }
//     free(str);
// }

// void recursion2(char* str);

// //函数间递归调用
// void recursion1(char* str){
//     int x = rand();
//     if(x > 5){
//         recursion2(str);
//     } else {
//         free(str);
//     }
// }

// void recursion2(char* str){
//     int y = rand();
//     if(y > 5){
//         recursion1(str);
//     } else {
//         free(str);
//     }
//}
typedef void(*MyHook)(char*p);
MyHook g_GlobalCallBack = NULL;
void MyFree(char* p)
{
    if(p != NULL)free(p);
}
// void MyFree2(char* p)
// {
//     if(p != NULL)free(p);
// }
// void MyAlloc()
// {
//     char *p;
//     g_GlobalCallBack = MyFree;

//     p = (char *)malloc(100);
//     if(p == NULL) return;
//     g_GlobalCallBack(p);

//     g_GlobalCallBack = MyFree2;
//     g_GlobalCallBack(p);
// }

// void ifcondne1(){
//     char* p1;
//     char* p2;

//     p1 = (char*)malloc(100);
//     if(p1 != NULL){
//         //p2 = p1;
//         //free(p1);
//     }
//     //free(p1);
// }

// void ifcondne2(){
//     char* p1;
//     char* p2;

//     p1 = (char*)malloc(100);
//     if(p1 != NULL){
//         //p2 = p1;
//         free(p1);
//     }
//     //free(p1);
// }

// void ifcondne3(){
//     char* p1;
//     char* p2;

//     p1 = (char*)malloc(100);
//     if(p1 != NULL){
//         //p2 = p1;
//         free(p1);
//     }
//     free(p1);
// }

// void ifcondne4(){
//     char* p1;
//     char* p2;

//     p1 = (char*)malloc(100);
//     if(p1 != NULL){
//         p2 = p1;
//         free(p1);
//     }
//     //free(p1);
// }

// void ifcondne5(){
//     char* p1;
//     char* p2;

//     p1 = (char*)malloc(100);
//     if(p1 != NULL){
//         p2 = p1;
//         free(p2);
//     }
//     //free(p1);
// }

// void ifcondne6(){
//     char* p1;
//     char* p2;

//     p1 = (char*)malloc(100);
//     if(p1 != NULL){
//         p2 = p1;
//         free(p1);
//     }
//     free(p1);
// }

void ifcondne7(){
    char* p1;
    char* p2;

    p1 = (char*)malloc(100);
    free(p1);
    if(p1 != NULL){
        // p2 = p1;
        // free(p1);
    }
}

void LeakTest1()
{
    // char *p;
    // g_GlobalCallBack = MyFree;

    // char* p1;
    // char* p2;

    // p1 = (char*)malloc(100);
    // if(p1 != NULL){
    //     p2 = p1;
    //     free(p2);
    // }

    // p1 = (char *)malloc(100);
    // if(p1 == NULL) return;
    // free(p1);

    // p = (char*)malloc(100);
    // if(p == NULL)return;
    // g_GlobalCallBack(p);
}

// void LeakTest(){
//     char *p1,*p2,*p3,*p4,*p5;

//     p3 = (char *)malloc(100);
//     if(p3 == NULL) return;
//     g_GlobalCallBack(p3);
// }

// void tt(){
//     g_GlobalCallBack = MyFree;
//     LeakTest();
// }

// void doublefree(){
//     char *p;
//     p = (char *)malloc(100);
//     if(p != NULL) free(p);
//     if(p != NULL) free(p);
// }

// //ifcond judge the memory
// void MyAlloc1()
// {
//     char *p;
//     p = (char *)malloc(100);
//     if(p == NULL) return;
//     MyFree(p);
// }

// //double free
// void MyDoubleFree1(char* p)
// {
//     if(p != NULL)free(p);
//     free(p);
// }

// //double free
// void MyDoubleFree2(char* p)
// {
//     if(p != NULL){
//         free(p);
//         return;
//     }
//     free(p);
// }

//int main()
//{  
    // char* str = (char *)malloc(15);
    // int x = 10;
    // int y = 7;
    // if(x == 10){
    //     free(str);
    // } else if(x == 5){
    //     free(str);
    // }
    // mem_leak1();
    // mem_leak2();
    // mem_leak3();
    // mem_leak4();
    // double_free1();
    // double_free2();

    // amem_leak1();
    // amem_leak2();
    // adouble_free1();
    // adouble_free2();
    // adouble_free3();

    // switch(x){
    //     case 5:
    //         free(str);
	//         x = 3;
    //         break;
    //     case 3:
    //         x = 7;
    //         break;
    //     case 7:
    //         str = (char *)malloc(15);
    //         x = 10;
    //     default:
    //         x = 0;
    // }
//}