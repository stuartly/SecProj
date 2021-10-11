# 减少运算强度
##Config

```
CheckerEnable
{
ExpensiveOperationChecker = true
}

ExpensiveOperationChecker
{
    ignoreLibPaths: 忽略自定义路径下的源文件
    enableAliasAnalysis: 开启简单别名分析
}
```
>*ignoreLibPaths*格式：
>
>``<path1>:<path2>:<path3>:...``

```shell
$ cd ${project.root}/tests/ExpensiveOperationChecker
$ clang -emit-ast -c example.cpp
$ ./../../build/tools/TemplateChecker/TemplateChecker astList.txt config.txt


```
