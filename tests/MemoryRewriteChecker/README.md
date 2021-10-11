# 内存重复写

特征：对相同的变量重复写两次，而之间没有读

影响：删除多余的写

## Config

```
CheckerEnable
{
	MemoryRewriteChecker = true
}

MemmoryRewriteChecker{
	IgnoreCStandardLibrary：忽略库函数(默认false)
	ignoreLibPaths：忽略自定义的路径下的源文件
	DisableSimpleVariable：不检测简单变量的重复写（默认false）
	Overlap:第二次写是否覆盖第一次(默认为true)
}
```

>*DisableSimpleVariable*为真时，这种类型的重复写行为不会被报告，例如：
>
>```c++
>int a;
>a=0;
>a=0;//简单变量
>```
>
>*IgnoreLibPaths*格式： 
>
>``<path1>:<path2>:<path3>:...``
>
>*Overlap*为false，则下面这种类型会报告
>```c++
>struct s{int a;int b;int c;}s1;
>int main(){
>	memset(&s1,0,sizeof(struct s));
>	s1.a=0;
>	s1.b=0;//即使s1.c没有写，但是也会报告
>	return 0;
>}
>```
## How to run

```shell
$ cd ${project.root}/tests/MemoryRewriteChecker
$ clang -emit-ast -c example.c
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt config.txt
```

## Notice

1. 只考虑此种类型的重复写行为，对于同一个变量，两次重复的写，中间没有读行为，且 **可以安全地删除第一次写的语句**

例如：
```c++
int a;
a=1;
a=2;
```
```c++
int a;
a=10;
if(...){ a=0; }else{ a=2; }//a=0,a=2与上面的a=10共同构成了一次重复写行为
```
这些行为将不被认为是重复写：

例如：
```c++
int a;
a=10;
if(...){a=100;}else{int b=a;}
```
```c++
int a;
a=10;
if(...)
{

}else{
	a=100;
}
int b=a;
```

2. 该分析是一个过程间分析，但是无法分析部分循环调用的重复写
例如：
```c
int a;
void f1(){
	a=1;
	f1();//不会报告
}
void f3();
void f2(){
	a=1;
	f3();
}
voif f3(){
	f2();
	a=1;//不会报告
}
```
