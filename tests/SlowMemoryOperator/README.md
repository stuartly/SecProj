# 2.14	 慢速的内存操作

## 特征

对较大的内存区域的清零，拷贝，比较等操作
使用低速逐byte方式操作, 或者每次操作内存相对于内存区域的比例过小


## 影响

逐byte方式性能较低

## 工具检测

综合考虑内存操作区域大小，发现较为低速的逐byte方式操作内存，给出提示

## 修改目标

人工审核后，使用8byte或者是SIMD方式来操作内存

## 配置项
```
CheckerEnable
{
  SlowMemoryOperationChecker = true //开启检测工具
}

SlowMemoryOperationChecker
{
  min_bytes_operated_onetime = 8 //设置循环中一次操作的字节小于阈值时报告Warning
}
```

## 当前支持检测语句

1. 普通数组操作
```
for() {
  arr[i] = 0;
}
```

2. 类、结构体数组操作
```
class C {
  int a;
};
C c[100];
for() {
  c[i].a = 0;
}
```

3. 类、结构体成员数组操作
```
class C {
  int arr[100];
};
C c;
for() {
  c.arr[i] = 0;
}
```
