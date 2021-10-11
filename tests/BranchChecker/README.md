# 2.7 指令布局
特征：关键路径代码和非关键路径代码混杂在一起
影响：非关键路径的代码也会被加载到指令Cache中，浪费Cache，影响性能

## config
开启Checker
```
CheckerEnable
{
  InstructionLayoutChecker = true
}
```
Checker配置项
```
InstructionLayoutChecker
{
    code_count_threshold = 2
}
```
+ code_count_threshold: 报警的代码行数阈值，若非关键路径上代码行数超过该阈值则会产生警报，默认为2
# 2.11 给分支判断加上likely/unlikely的申明
特征：整个判断条件有确定性概率优势加上likely/unlikely申明
影响：编译器根据申明编译出符合CPU指令流水线的二进制，能提升性能

## config
开启Checker
```
CheckerEnable
{
  AddLikelyOrUnlikelyToBranchChecker = true
}
```
Checker配置项
```
AddLikelyOrUnlikelyToBranchChecker
{
    likely_threshold = 0.85
    unlikely_threshold = 0.15
}
```
+ likely_threshold: 报警的概率下限阈值，若If条件为true的概率超过该阈值，则产生推荐使用likely的一个警报，默认为0.85
+ unlikely_threshold: 报警的概率上限阈值，若If条件为true的概率低于该阈值，则产生推荐使用unlikely的一个警报，默认为0.15

# 关于用户自定义关键路径
本配置为上述两个Checker共用。
```
CriticalPaths
{
    path1 = main,func1,func2
}
```
每一条路径语法应满足
```
Path ::= path_name '=' CallChain
CallChain ::= function_name ',' function_name
            |  CallChain ',' function_name
```
其中`path_name`和`function_name`为满足C函数命名规则的字符串