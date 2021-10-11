# 2.9	 消除Cache“伪共享”
特征：限定在同一个数据结构中，频繁读和频繁写位于同一个CacheLine中
(假设代码总是多核并行执行的)

影响：某个core频繁写会导致另外core发生CacheMiss

## 配置项
```
CheckerEnable
{
  	FakeCachelineShareChecker = true // 开启配置项
}
FakeCachelineShareChecker
{
    min_gap = 0.5 // 报告的阈值（0到1之间），越小越容易报告缺陷
    min_access = 5 // 某个成员的读或写访问次数少于该值将会被忽略
    DefaultLoopCount=2 //遇到无法分析的循环上界时，选用的循环次数

}
```

min_gap的的用处为：如果一个结构体某个成员的写次数记为Wi，如果存在另一个成员，它的读次数为Rj，且min(Wi, Rj)/max(Wi, Rj)>min_gap，则报告存在伪共享问题。
