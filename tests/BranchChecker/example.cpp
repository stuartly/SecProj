
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNLIKELY(x) __builtin_expect(!!(x),0)
#define LIKELY(x) __builtin_expect(!!(x),1)

enum State{
    OK,
    ERROR
};
int main() { return 0; }

void critical(){
}
void non_critical(){

}
void func1(){
    int a,size;
    int *p;
    if(a < size){
        critical();
        return;
    }
    if(!p){
        non_critical();
        non_critical();
    }
}

void func2(){
    int a = OK;
    if(a != OK){
        non_critical();
        non_critical();
    }
    if(a == ERROR){
        non_critical();
        non_critical();
    }
}

void func3(){
    int a = 1;
    if(UNLIKELY(a == 1)){
        non_critical();
        non_critical();
    }
    if(LIKELY(a == 1)){
        critical();
        critical();
    }
}

void func4(){
    int a = 0;
    if(a != 0){
        non_critical();
        non_critical();
    }
}
/* void func2(){
    return;
}
int func1(int a) {
    if(a > 2){
        return 0;
    }
    else{
        func2();
        return 1;
    }
}
int main() {
    int a;
    if(a > 2){
        return 0;
    }
    else{
        func1(a);
        return 1;
    }
} */