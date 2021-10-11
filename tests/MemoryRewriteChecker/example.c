#include <string.h>
#include <stdlib.h>
enum K
{One,Two};
struct s{
	int a;
	char *b;
	struct s *next;
	float c[3];
};
struct t
{
	int a;
	char b;	
}t1;

// simple variable.
// set configuration DisableSimpleVariable to false
void func1(){
	enum K k1=One;
	k1=Two;// warning!
	int a=0;
	if(1){
		a++;
	}
	a=10;// warning!
	int *b;
	if(0){
		*b=0;
	}else{
		*b=10;
	}
	*b=10;// warning!
	double d[2];
	d[1]=0;
	d[1]=100;// warning!
}
void fun2(){
	// these are not warnings!
	int a=-1;
	if(1){
		a=10;// not a warning!
	}else{
		int b=a;
		a=100;// not a warning!
	}
	char *p;
	p[0]='B';
	p[1]='A';// not a warning!
	char *c;
	*c='A';
	*(c+1)='B';// not a warning!
}
// complicated variable
// warnings!
void func3(){
	int a[10]={0};
	memset(a,0,sizeof(a));// warning!
	struct s*s1=(struct s*)malloc(sizeof(struct s));
	memset(s1,0,sizeof(s1));
	s1->a=10;
	memset(s1->c,0,sizeof(s1->c));
	s1->next=NULL;
	s1->b=NULL;// warning!
}
// warnings!
void func4(){
	int a[10];
	a[0]=0;
	a[1]=100;
	memset(a,0,sizeof(a));// warning!
	struct s*s1=(struct s*)malloc(sizeof(struct s));
	s1->a=10;
	s1->b=0;
	if(1){
		s1->next=NULL;
	}
	memset(s1,0,sizeof(s1));// warning!
}
// these are not warnings!
void func5(){
	int a[10];
	memset(a,0,sizeof(a));
	a[0]=0;
	a[1]=100;// not a warning!
	struct s*s1=(struct s*)malloc(sizeof(struct s));
	memset(s1,0,sizeof(s1));
	s1->a=10;
	s1->b=0;// not a warning!
}

//interprocedural
void func6(){
	t1.a=0;
	t1.b='a';
}
void func7(){
	func6();
	memset(&t1,0,sizeof(struct t));//warning!
}
void func8(){
	t1.a=10;
}
void func9(){
	t1.a=0;

}
void func10(){
	func8();
	func9();// warning
}
int main(){
	return 0;
}