# 2.13利用CPU乱序执行，多指令并发优势
特征：前后数据不相关，未充分利用CPU多流水线的优势

影响：CPU的乱序执行，多issue特性可以更好地提升性能

## Config
```
CheckerEnable
{
	CPUOutOfOrderExecutionChecker = true
}
```
## How to run

```shell
$ cd ${project.root}/tests/CPUOutOfOrderExecution
$ clang++ -emit-ast -c example.cpp
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt config.txt
```