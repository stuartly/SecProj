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
	UninitializedStructureMemberChecker = true 
}

UninitializedStructureMemberChecker
{
        depth = 3                           //配置检测深度，即过程间调用层数，depth = 3表示检测到一个结构体成员变量使用点时，向上查找3层调用函数是否对其进行了初始化。若不需要过程间分析配置depth=0。depth越大精度越高，误报越少，但效率更低。
	enqueFunc = {"push&0","push_back&0"}  //配置需检测的入队函数以及参数位置
	dequeFunc = {"pop&0,pop_back"}     
}
```
## 检测逻辑
1、若某结构体成员变量在使用前未初始化，则报错。判断其是否初始化的逻辑如下：如在某一处有a->b->c的使用点，则向上回溯查找有无a->b->c的初始化，包括声明时初始化"int a->b->c = 0;"，赋值语句"a->b->c = 0;"，以及函数内作为参数的引用func(&a->b->c),这种情况认为在func函数内对a->b->c进行了初始化。
若没有对该成员变量的初始化，则查找有无对结构体a的初始化，包括对a的声明时初始化"struct A a = (A*)malloc(sizeof(A));"、赋值语句"a = p;"，以及func(&a)。
2、过程间分析是可配置的。通过config中的depth进行配置。depth=0则只进行过程内分析，只在函数内部查找有无对成员变量进行了初始化，depth = 1表示需要在调用使用点所在的函数中查找有无对成员变量进行初始化，depth=2表示向上查找2层调用函数是否对其进行了初始化，以此类推。depth层数越多误报越少，benchmark中一般depth设为4后准确率不再有明显变化。
3、现benchmark中误报的类型大多是由于过程间分析时，有函数指针的赋值情况而无法找到该函数的调用点。
4、若遇到config中配置的需检测的入队函数并且参数位置为结构体变量类型，则对该结构体变量是否初始化进行检查，这一部分的检测是域敏感的。若该结构体变量已用初始化列表初始化或者对每一个成员变量都进行了初始化，则不报错。如上例config中。


```
UninitializedStructureMemberChecker
{
	enqueFunc = {"push&0","push_back&0"}  //配置需检测的入队函数以及参数位置
}
```
表示若遇到调用函数push_back(a)，其中参数a为结构体变量，则需要向上回溯查找a是否已经完成了对其成员变量的初始化。

"
