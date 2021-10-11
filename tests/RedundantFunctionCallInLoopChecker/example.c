
#include <stdio.h>
#include <stdlib.h>
int a;
int *b;
char c[5];
struct d_type{
	double a;
	char *b;
	float c[1];
	struct d *next;
}d;
// all kinds of redundant functions

//1. empty functions
void func1(){
}
//2. return a global variable
int func2(){
    return a;
}
int * func3(){
    return b;
}
int func4(){
	return c[3];
}
double func5(){
	return d.a;
}
//3 only local variable
void func6(){
	int a=10;
	int *b=&a;
	char c[5]={0};
	a++;
	*b--;
}
//4 read global
int func7(int f){
	f++;
	return f;
}
//5. pointee symbol
char func8(char *p){
	return *p;
}
//6. arguments
void func9(int a,char*b,struct d_type c){
	a=*b;
	c.a++;
}

// these are not redundant function
char func10(char *p){
	(*p)++;
	return *p;
}
void func11(){
	c[1]=(c[2]++);
}
void func12(int a,char*b,struct d_type c){
	a=*b;
	*b=c.a;
	c.a++;
}

// predefined functions
int main(){
	while(1){
		func1();
		func2();
		func3();
		func4();
		func5();
		func6();
		func7(a);
		func8(&a);
		for(;;){
			func9(a,b,d);
			func10(b);
		}
		func11();
		func12(a,b,d);
	}
	char *c="WORLD";
	do{
		size_t Size=strlen("Hello,World");
		size_t Size1=strlen(b);
		size_t Size2=strlen(c);
		c++;
	}while(1);
	return 0;
}
