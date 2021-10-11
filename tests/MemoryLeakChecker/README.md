## How to run

```shell
$ cd ${project.root}/tests/TemplateChecker
$ clang++ -emit-ast -c example.cpp
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt ../config.txt

```
## Config
```
CheckerEnable
{
	CallGraphChecker = true

	MemoryLeakChecker = true
	DoubleFreeChecker = true
}

MemoryLeakChecker
{
	allocFunc = {"malloc"}          //内存申请函数
	freeFunc = {"free"}             //内存释放函数
	functionpointer_analysis = true //是否启用函数指针分析
}

DoubleFreeChecker
{
	allocFunc = {"malloc"}
	freeFunc = {"free"}
	functionpointer_analysis = true
}

```
## 检测逻辑
1、每个函数指针指向一个内存，内存存储被引用的数目，函数指针指向其他内存时，原内存引用数量-1

2、函数过程结束，没有被返回和引用内存数不为0的内存，发生内存泄漏；指针指向其它内存时，原有内存如果没有其他引用，发生内存泄漏

3、已经被释放的内存，再次被释放，发生重复释放