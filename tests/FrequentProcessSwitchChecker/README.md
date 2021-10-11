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
	ProcessSwitchingFrequentlyChecker = true 
}

ProcessSwitchingFrequentlyChecker
{
    limit = 3                   //连续系统调用数量
    threshold = 20              //系统调用之间指令阈值
	func = test_func:socket:    //系统调用函数表
}
```
## 检测逻辑
1、发现代码连续n次相同的系统调用，两次相邻的调用之间指令数或者cycles数少于某个阈值，给出提示