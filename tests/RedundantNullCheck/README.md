# RedundantNullCheck

## Prerequisites

- LLVM >= 8.0
- CMake >= 3.10

## How to compile

There are two approaches to compile TemplateChecker:

1. If you built LLVM and installed it, you should specify variable LLVM_PREFIX which represents the path to your LLVM install directory;
2. If you built LLVM but did not install it, you should specify variable LLVM_BUILD which represents the path to your LLVM build directory.

We recommend the first approach.

```shell
$ https://github.com/DerZc/FSCA-HWRS.git
$ cd FSCA-HWRS
$ mkdir cmake-build-debug
$ cd cmake-build-debug
# use LLVM install directory
$ cmake -G Ninja -DLLVM_PREFIX=${LLVM_PREFIX} ..
# use LLVM build directory
$ cmake -G Ninja -DLLVM_BUILD=${LLVM_BUILD} ..
$ ninja
```

## How to run

```shell
$ cd ${project.root}/tests/RedundantNullCheck
$ clang++ -emit-ast -c example.cpp
$ ../../cmake-build-debug/tools/TemplateChecker/TemplateChecker astList.txt config.txt
```

## Config
```
CheckerEnable
{
  CallGraphChecker = true
  SplitBasicBlockwithFunCall = true
  PointToAnalysisChecker = true
  RedundantNullChecker = true  //Enable RedundantNullCheck
}

RedundantNullCheck
{
  pointer_analysis = true   // Support pointer_analysis
}

```
