# 循环体中冗余的函数调用

特征：循环体中冗余的函数调用
影响：可以把函数调用移到循环外

## Config

```
CheckerEnable
{
	RedundantFunctionCallInLoopChecker = true
}

RedundantFunctionCallInLoopChecker{
	IgnoreCStandardLibrary：忽略库函数
	ignoreLibPaths：忽略自定义的路径下的源文件
}
```

>*IgnoreLibPaths*格式： 
>
>``<path1>:<path2>:<path3>:...``

## How to run

```shell
$ cd ${project.root}/tests/RedundantFunctionCallInLoopChecker
$ clang -emit-ast -c example.c
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt config.txt
```