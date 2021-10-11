# 简单易懂关键路径分析使用方法

如何使用关键路径分析？

1. 像checker一样实例化`BranchAnalyzer`
2. 调用`BranchAnalyzer::analyze(ASTFile*)`
3. 调用`BranchAnalyzer::results()`获得结果，结果为if语句及其对应的then分支概率的表

> 注意：API并不稳定，随时可能修改