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
	ReduceLockChecker = true 
}

ReduceLockChecker
{
    lock = lock:plock_lock:       //加锁函数表
    unlock = unlock:plock_unlock:   //解锁函数表
}
```
## 检测逻辑
1、检测Spinlock, WrLock之类的锁内部的临界区无关的操作，耗时操作，给出提示
2、临界区不相关代码检测，临界区内的局部变量相关的操作，认为是非临界区相关的，给出提示