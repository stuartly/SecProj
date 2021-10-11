# 2.15 空间问题发现
1. 特征: 超大全局变量，超大局部变量，不合理的数据段内容（BSS），冗余数据结构成员
2. 检测:
    1. 超大全局变量，超大局部变量，超过指定的阈值，给出提示；
    2. 不合理的大数据结构变量赋初值，造成二进制Data段过大；
    3. 数据结构中的某个成员所有的执行路径上都用不到；


## 关于 config 文件
开启 `HugeMemoryChecker = true`, 不依赖于其他 `checker`, 添加 `HugeMemoryChecker` 块的配置项:
1. `enableHugeVarCheck`: 针对问题 1，2 问题，是否检查超大全局变量，超大局部变量，不合理的数据段内容（BSS），默认不检查，即 `false`
2. `enableUnusedFieldCheck`: 针对问题 3 问题，是否检查冗余数据结构成员，默认不检查，即 `false`
3. `localVarThreshold`: 针对检测 1 问题，指定超大局部变量阈值，单位为 Bytes，不指定的话默认值为 1024
4. `globalVarThreshold`: 针对检测 1 问题，指定超大全局变量阈值，单位为 Bytes，不指定的话默认值为 1024
5. `dataSectionThreshold`: 针对检测 2 问题，指定Data段阈值，单位为 Bytes，不指定的话默认值为 1024
6. `ignoreLibPaths`: 针对检测3问题，指定忽略分析的结构体的路径
    + 第三方库在include头文件时会引入很多结构体，这些结构体可能是程序员有意预留的字段或者公共头文件中的数据结构，只有部分会在项目里使用，而使用的结构体里也可能只有部分域会在项目里使用到，因此设置该配置项可以忽略分析这些结构体的域，减少warning，可用于减少误报
    + 可以指定多个路径，用冒号分割，详细见 [docs/README.md](../../docs/README.md)
    + 无论是否给定该配置也会系统头文件，参考 clang 提供的 [`isInSystemHeader`](../lib/checkers/HugeMemory.cpp) 等接口
7. `ignoreStructs`: 针对检测3问题，指定忽略分析的结构体类型，可以指定多个，以`#`分隔，每个元素的格式为“struct 结构体类型名”，用于减少误报
8. `ignoreFields`: 针对检测3问题，指定忽略分析的域成员名字，可以指定多个，以`#`分隔，每个元素的格式为“域成员名”，例如有些在dpkg里有些结构体会用padding填充，该字段本身没有意义，也没有被使用，可以设置该配置来减少这一类的warning，用于减少误报
9. `ignoreStructFields`: 针对检测3问题，指定忽略分析的结构体类型和域成员名字，可以指定多个，以`#`分隔，每个元素的格式为“struct 结构体类型名.域成员名”，用于减少误报
10. `considerCStyleCast`: 针对检测3问题，是否考虑指针强转，值为 `True` 或 `False`，不指定默认为 `false`
    + 关于该配置项的详细说明如下

------
### `considerCStyleCast` 配置项的说明
例如：
```
typedef struct {
    PyObject ob_base;
    PyObject *start;
    PyObject *stop;
    PyObject *step;
    PyObject *length;
} rangeobject;
```
其中 `PyObject` 也是一个结构体，当 `considerCStyleCast = true` 时，以下情况也算使用了 `rangeobject` 的 `ob_base` 域：
1. `rangeobject *op; return (PyObject *)op;`
2. `PyObject *obj; rangeobject *rgob = (rangeobject *)obj;`

即对于指针强转 `(type1*)val` 语句, 其中 `val` 为 `type2*` 类型，
当满足以下情况之一，则认为***隐式***使用了结构体的域成员：
1. type1是结构体类型且type2是其第一个域的类型
2. type2是结构体类型且type1是其第一个域的类型

注意，这里只考虑指针的强转，对应的实现逻辑在`lib/checkers/HugeMemory.cpp#FieldCheckSolution::VisitCStyleCastExpr()`

-----

这么做的原因如下：当 `considerCStyleCast = false` 时运行 CPython

在分析 CPython 的检测3问题时会产生许多形如以下的报告：
+ [ ] Field idx #0 'ob_base' :'PyObject
+ [ ] Field idx #0 'ob_base' :'PyVarObject

由于当前的检测算是是遍历 ast 寻找结构体的定义以及寻找相关变量的初始化和相关域的使用点，标记对应结构体的使用情况。因此这是基于语义分析的。

通过分析报告发现这些成员在语义层面上确实没有被直接***显式***使用，这一点与 CPython 的设计和编码实现有关。以 `PyObject` 这个结构体为例，这一结构体是 CPython 对象中的基类，其他 Python 对象对应的 C 实现都基于此，并将此放置到对应结构体的第一个位置（因此Field idx 为0），以实现安全的强制类型转换。

例如上述结构体类型声明在`Python-3.8.0/build/../Objects/rangeobject.c:13`

声明一个 `rangeobject* obj` 变量并动态分配空间之后，这个变量就强转为 `PyObject*` 类型在控制流中流动。那么原本对 `((rangeobject*)obj)->ob_base` 的访问就变成了直接操控 `(PyObject*)obj`, 因此在源码中不会出现 obj->ob_base 这种直接的访问方式。这种访问方式在语法和语义分析的层面无法检查到。

所以 log 输出中会有这一类的报告，且大部分都是。

在当前的报告中，如果对报告中输出的未使用的域在源码中进行删除，是可以编译和链接的（即符合语法和语义的），但生成的二进制文件无法运行。
