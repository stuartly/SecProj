# 2.4 函数调用开销过大
1.特征：函数参数过多，栈变量赋值过多，无意义的返回值，不变的变量

2.影响：参数过多，不能用寄存器传参；函数局部变量过多，初始化耗时；不变的变量，访问内存耗时；无意义的返回值，消耗性能；

## Config
```
CheckerEnable
{
    FunctionCallChecker = true  //开启配置项
}

FunctionCallChecker
{
    param_size = 6  //函数参数阈值
    array_size = 2048  //初始化的数组长度上限

}

```

## How to run
```shell
$ cd ${project.root}/tests/FunctionCallChecker
$ clang++ -emit-ast -c example.cpp
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt ../config.txt
```
