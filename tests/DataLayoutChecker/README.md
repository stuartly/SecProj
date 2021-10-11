# 数据布局

## Config

```
CheckerEnable
{
	DataLayoutChecker = true
}

DataLayoutChecker{
	IgnoreCStandardLibrary：忽略库函数
	ignoreLibPaths：忽略自定义的路径下的源文件
	RatioThreshold:访问次数比例阈值(默认1000)
	DiffThreshold:访问次数差值阈值（默认1000）
	DefaultLoopCount:默认的循环次数
}
```

>*IgnoreLibPaths*格式： 
>
>``<path1>:<path2>:<path3>:...``

> *RatioThreshold*，*DiffThreshold*
> 是一个整数，如果两个域成员访问次数比值大于等于这个*RatioThreshold*并且访问次数差值大于等于*DiffThreshold*，就输出一个报告
## How to run

```shell
$ cd ${project.root}/tests/DataLayoutChecker
$ clang -emit-ast -c example.c
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt config.txt
```
```