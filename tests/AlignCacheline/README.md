# 2.10	 关键数据结构未按CacheLine对齐
特征：频繁访问的数据结构未按照cacheline对齐

影响：访问数据可能会导致有两次load到cache的操作

## 配置项
```
CheckerEnable
{
  AlignCachelineChecker = true //开启配置项
}
AlignCachelineChecker
{
	cacheline_size = 64 //设置cacheline的大小（bytes）
	DefaultLoopCount=2 //遇到无法分析的循环上界时，选用的循环次数
	frequent_access_threshold = 1000 //在算法第3步所用的频繁访问次数阈值，小于则不会报告
	avg_dev_threshold = 0.5 // 每个成员访问次数的average deviation的阈值，超过才会进入算法第三步。
}
```

## 算法

1. 对于频繁访问的数据结构，判断该数据结构是否有__attribute__((aligned(64)))，如没有则报错

2. 如有attribute, 计算该数据结构每个成员访问次数的average deviation，如果average deviation大于配置项的avg_dev_threshold才会进入下一步。如果不需要此功能，则将avg_dev_threshold设为0.

3. 则判断频繁访问字段地址是否CacheLine对齐(已经假设了整个结构是CacheLine对齐)，如字段没有对齐，则报错，提示用户更合理安排数据结构的字段