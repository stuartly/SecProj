# 代码组织 和 总体介绍
1. 以下是和数据流分析（Data-Flow Analysis）相关的源代码组织树，其中只列举了**文件夹和一些关键的头文件**，旨在说明源代码文件是如何组织的，**使用该框架时也应该遵循该组织结构**。该组织结构参考[phasar](https://github.com/secure-software-engineering/phasar)
```
.
|-- include
|   `-- DFA
|       |-- Common.h                      DFA通用头文件，当前包含一些用于debug的宏
|       |-- ICFG                          DFA过程间控制流图（interprocedural control flow graph）接口相关头文件
|       |-- IFDSIDE                       过程间数据流分析IFDS/IDE算法相关头文件
|       |   |-- EdgeFunctions
|       |   |-- FlowFunctions
|       |   |-- IDEProblem.h              IDE算法的数据流问题基类
|       |   |-- IFDSProblem.h             IFDS算法的数据流问题基类
|       |   |-- Problems                  存放具体的过程间数据流问题头文件，如测试框架用的最简单的数据流问题IFDSSolverTest等
|       |   `-- Solver                    IFDS/IDE算法实现
|       `-- Mono                          过程内数据流分析Mono算法相关头文件
|           |-- DataFlowFact.h            Mono算法的数据流事实基类
|           |-- IntraMonoProblem.h        Mono算法的数据流问题基类
|           |-- Problems                  存放具体的过程内数据流问题头文件，如活性变量分析LiveVariables等
|           `-- Solver                    Mono算法实现
|-- lib
|   `-- DFA
|       |-- ICFG                          DFA过程间控制流图接口实现
|       |-- IFDSIDE                       过程间IFDS/IDE算法的相关实现，一般来说是具体数据流问题的实现，对应于include中定义的头文件
|       |   `-- Problems
|       `-- Mono                          过程内Mono算法的相关实现，类似于IFDS/IDE
|           `-- Problems
|-- tests
|   `-- DFA                               包含测试用的example、config.txt、astList.txt等
|       |-- IFDSIDE                       Common包含那些通用的测试example，例如可能几个不同的数据流问题能用用一个example测试， \
|       |   |-- Common                            对于这些数据流问题就不需要单独设计测试用的example
|       |   |-- xxxProblem1               以下就是具体的数据流问题专用example，下同
|       |   `-- xxxProblem2               
|       `-- Mono
|           |-- Common
|           |-- xxxProblem1
|           `-- xxxProblem2
`-- tools
    `-- DFA                               下面有特殊说明
        |-- IFDSIDE
        |   |-- xxxProblem1
        |   `-- xxxProblem2
        `-- Mono
            |-- xxxProblem1
            `-- xxxProblem2
```
2. 整体来说，参照框架固有的文件组织，在对应的目录下由统一的 `DFA` 子目录组织数据流分析模块。其中一般地， `DFA` 目录下会分成代表**过程内**的 `Mono` 和代表**过程间**的 `IFDSIDE` 子目录。
3. 数据流分析框架提供了描述数据流问题的模板以及对数据流问题的求解。 `Mono` 和 `IFDSIDE` 分别对应符合某类特征的数据流问题的求解算法，因此利用该框架的前提是保证所要求解的数据流问题符合这些特征并**编码实现**要求解的具体的数据流问题，待问题定义好后，调用对应的solver即可求解（求解部分才是该框架的核心）。
4. 所有代码都包含在***命名空间***：`namespace hwdfa`
5. 关于ICFG：过程间的数据流分析需要用到过程间控制流图 ICFG ，其中 ICFG 依赖于函数调用图（Call graph，下称 CG ）。DFA 里的 ICFG 只是根据框架代码、 CFG 和 CG 提供的接口进一步封装，使其看起来拥有一般 ICFG 所需要的接口，实际上并不会真的构造和维护一个完整的过程间控制流图（即无法直接dump出一个图结构出来）。这样做能简化实现和减少内存开销，实际上如果实际构造一个完整的ICFG，会依赖于CG算法，不同的 CG 算法所构造出来的 ICFG 也不同，因此我们**只提供接口**，不提供图的实际表示。
6. 关于程序点（Program point）的说明：数据流分析实际上是计算每个程序点的数据流事实（Data-Flow fact），不同于教科书上的解释，在该框架中无论是过程内的cfg还是过程间的icfg都是 cfgblock 粒度，因此就DFA框架默认一个 cfgblock 对应一个程序点，但是对于不同的数据流问题，可能需要 stmt 级别的，这个工作交由具体的数据流编码完成，可以参考已实现的数据流问题[活性变量分析](./lib/DFA/Mono/Problems/LiveVariables.cpp)，`LiveVariablesIMP::transferFunc` 是基本块级别的，而在实现 transferFunc 的时候调用**非框架要求必须实现**的 StmtTransferFunctions 从而能记录每个 stmt 对应的数据流事实。
    + 不使用 stmt 作为默认程序点的原因：不同于IR(intermediate representation)，clang AST 的 stmt 是个比较上层的基类，它和教科书上的stmt在形式上有着天壤之别（例如IfStmt，根本没法对应program point这个说法，即使是 Expr 也不都能作为program point），这种形式的 stmt 没法作为统一的 program point，所以使用和 cfg 一样的 cfgblock 作为 program point。同时可以留意一下活性变量分析中记录的stmt也并不是所有的 stmt 都作为程序点，而是选择合适的 stmt 作为程序点记录。
    + 即框架提供 cfgblock 级别的程序点，如果需要 stmt 级别，使用者可以在自己编码实现的数据流问题中完成，只要符合框架的要求，一切想法都是允许的

## DFA框架使用指南

为了方便参考，目前DFA框架中含有用于测试过程间内 Mono 和过程间 IFDSIDE 的数据流问题实现（从某种角度来说也是具体的数据流问题，只不过是的最简单的数据问题），分别对应于 [MonoTest](./lib/DFA/Mono/Problems/MonoTest.cpp) 和 [IFDSIDETest](./include/DFA/IFDSIDE/Problems/IFDSSolverTest.h) 。同时还实现了一些数据流问题以便使用者参考。如过程内Mono的活性变量分析（该数据流问题的描述参考[LiveVariables](https://en.wikipedia.org/wiki/Live_variable_analysis)）。这些测试用的数据流问题实现最后的输出是打印每个程序点的数据流事实。

通常来说步骤如下：（具体过程内和过程间参考下面详细说明）

1. 根据需求明确数据流问题，过程内or过程间？是否符合对应算法的要求？
2. 编码实现具体的数据流问题，以过程内为例：
    + 在`include/DFA/Mono/Problems/`下声明问题的头文件，同时需要明确该问题的数据流事实是什么，类型是什么
    + 在`lib/DFA/Mono/Problems/`下编码实现具体数据流问题，同时需要注意编码CMakeList.txt
    + 上述二者不一定同时存在，例如对于简单的数据流问题，可以在头文件里声明的同时顺便完成实现
3. 如果需要用demo测试，则编写tests
4. 在tools下（或checker中）整合上述代码，将编码好的数据流问题传参对应的solver并调用solver()即可。


--------------------


# 过程内 Mono 框架
使用过程内 Mono 框架要求数据流问题满足如下特点：
1. 数据流事实的值域应当是有界
2. transfer function 在 merge function 上应当满足单调性

因此使用过程内 Mono 框架时，需要编码符合如上要求的数据流问题，具体来说：
1. 确定**数据流事实**的类型和值域：继承[class DataFlowFact](./include/DFA/Mono/DataFlowFact.h)，该类虽然没有设定成虚基类，但必须要继承它才能使用框架，并且**必须重写虚函数**`virtual bool isEquals(const DataFlowFact &rhs) const`
    + 一般来说，该类是 set/vector 等容器类型的数据结构的一个 wrapper，例如在[活性变量中的LivenessValues](./include/DFA/Mono/Problems/LiveVariables.h)，其本质就是一个 `std::set` 的 wrapper，而具体的类型是 `const VarDecl *`（因为在活性变量数据流问题中，某个程序点的数据流事实是该程序点的活性变量）
    + 除了实现 isEquals 方法，我们还建议实现 dump/printAsString 等方法方便调试时使用
    + 其他方法可根据添加
2. 继承[IntraMonoProblem 虚基类](./include/DFA/Mono/IntraMonoProblem.h)，该类是一个模板类，需要实例化模板参数`typename DfFactType`，DfFactType 即为上述的数据流事实类型。该类构造函数接收一个`std::unique_ptr<clang::CFG> &`作为参数，代表需要分析的函数。有如下3个纯虚函数需要重写：
    + merge
     function：`void mergeFunc(const clang::CFGBlock *Node, std::map<const clang::CFGBlock *, DfFactType> &InSet, std::map<const clang::CFGBlock *, DfFactType> &OutSet)`。一般来说，若数据流事实类型是个 set 类型，则 must analysis 的 merge function 是 intersection(In[s] = Intersection(Out[s']), s' = per(s)) 操作，而 may analysis 的 merge function 是 union(Out[s] = Union(In[s']), s' = succ(s)) 操作。
    + transfer function(flow function)：`DfFactType transferFunc(const clang::CFGBlock *Node, DfFactType Src)`。描述一个程序点是如何影响数据流事实的，注意其传参和返回值是 `DfFactType` 而不是 `DfFactType &`。注意第二个参数 Src，Mono 框架规定了对于 forward analysis，Src 是对应的 Node 的 In，而对于 backward analysis，Src 是对应 Node 的 Out，如果弄混了会导致错误的结果。另外教科书上一般会引入 Gen 和 Kill 两个概念去描述 transfer function，实际上并不是所有数据流问题的 transfer function 都可以简单地用 Gen/Kill 定义，因此框架对此采用机制与策略分离的思想，只提供策略，机制由用户实现。
    + 初始化每个程序点的in和out集合，`void initialDomain(std::map<const clang::CFGBlock *, DfFactType> &in, std::map<const clang::CFGBlock *, DfFactType> &out)`：一般而而言，对于 may analysis，每个程序点的数据流事实初试化为空(bottom），对于 must analysis，每个程序点的数据流事实初试化为值域全部元素(Top)
    + 此外，我们建议该类的命名为：xxxIMP，IMP 是 implement 的缩写。
3. 定义编码完具体的数据流问题后，实例化一个数据流问题对象，并将它传参到 IntraMonoSolver 实例化一个过程内数据流分析求解器，同时应该声明该问题是 forward analysis 还是 backward analysis，调用该对象的 `solver()` 函数即可。
    + [IntraMonoSolver](./include/DFA/Mono/Solver/IntraMonoSolver.h) 是一个模板类，有两个模板参数：`template <typename DfFactType, bool IsForwardAnalysis = true>`，第一个是数据流事实类型，前面描述过了；第二个模板参数指定数据流问题是前向还是后向，默认是前向。
    + 求解后通过 `std::map<const CFGBlock *, DfFactType> &getInSet()` 或 `std::map<const CFGBlock *, DfFactType> &getOutSet()` 获取对应的结果。
    + 使用例子可参考tools/DFA/Mono/xxx/main.cpp，这里截取最重要的代码片段：
        ```c++
        #include "DFA/Mono/Problems/xxx.h"
        #include "DFA/Mono/Solver/IntraMonoSolver.h"
        // core analysis
        xxxIMP xxximp(cfg); // cfg is type of std::unique_ptr<clang::CFG>
        IntraMonoSolver<xxxDfFactType, false> lv(xxximp);
        lv.solve();
        ```
4. 注意命名空间，上述 `class DataFlowFact`/`IntraMonoProblem`/`IntraMonoSolver` 均在**命名空间 `namespace hwdfa` 中**。


--------------------

# IFDS 框架

IFDS 问题是 *Precise Interprocedural Dataflow Analysis via Graph Reachability* 这篇文章提出的一类数据流问题，全称是 ***Interprocedural, Finite, Distributive, Subset***, 从理论的角度讲，这类问题有如下特点：

1. 数据流事实必须是有限个数的
2. 在 meet operator （比如 union 或者 intersection）上必须满足分配率

因此解决过程间 IFDS 问题时，需要编码符合如上要求的数据类问题，具体来说：

1. 确定 **数据流事实** 的类型。一个数据流事实，可以是一个变量（比如活跃变量分析或者未初始化变量分析），也可以是一个表达式（比如可用表达式分析）。由于 IFDS 问题的求解是转换成 Exploded Super Graph(ESG) 的图可达性问题，有所谓的 **ZeroValue** 的概念，所以针对 clang 中的一个变量 `VarDecl` 或者表达式 `Expr`，需要先实现一个 wrapper（比如 [未初始化变量](../include/DFA/IFDSIDE/Problems/UninitializedVariables.h) 中的 `VarDeclHolder`）
2. 继承 [DefaultIFDSProblem](../include/DFA/IFDSIDE/DefaultIFDSProblem.h) 虚基类。这个类继承自 [IFDSProblem](../include/DFA/IFDSIDE/IFDSProblem.h) 类，定义了一个最基本的 IFDS 问题。继承时需要实例化模板参数`typename Fact_t`，`Fact_t` 即为上述的数据流事实类型。这个类的构造函数接收一个 `ClangBasedBBICFG &`  作为参数，而 `ClangBasedBBICFG` 的构造函数则接收 `CallGraph &, ASTManager &, ASTFunction *` 作为参数。除此之外，要继承 DefaultIFDSProblem 类，还需要实现如下几个函数：
   1. getNormalFlowFunction：`getNormalFlowFunction(const DFAICFGBlock *curr, const DFAICFGBlock *succ)` 表示数据流事实在函数内的传播。一般而言，类中会维护一个 `DFAICFGBlock*` 到数据流事实的映射；然后在这个函数中，先获取遍历之前 `curr`  的数据流事实，然后根据需要遍历语句，保存修改后的数据流事实，然后根据需要保存到 `succ` 对应的数据流事实中
   2. getCallFlowFunction：`getCallFlowFunction(const DFAICFGBlock *callStmt, const ASTFunction *destMthd)` 表示数据流事实在函数调用时向被调用函数的传播。一般而言，先获取 `callStmt` 中的数据流事实，然后根据需要遍历语句，保存修改后的数据流事实，然后根据需要向 `destMthd` 中的第一个 `DFAICFGBlock` 写入数据流事实
   3. getRetFlowFunction：`getRetFlowFunction(const DFAICFGBlock *callSite, const ASTFunction *calleeMthd, const DFAICFGBlock *exitStmt, const DFAICFGBlock *retSite)` 表示数据流事实在函数调用结束时向 caller 函数的传播。一般而言，先获取 `exitStmt` 中的数据流事实，然后根据需要遍历语句，保存修改后的数据流事实，然后根据 `return` 的内容将数据流事实写回 `retSite` 中。如果需要其他信息，可以通过 `callSite` 或者 `calleeMthd` 获取（比如函数的返回类型）
   4. getCallToRetFlowFunction：`getCallToRetFlowFunction(const DFAICFGBlock *callSite, const DFAICFGBlock *retSite, std::set<const ASTFunction *> callees)` 表示数据流事实在函数调用时跳过调用过程，直接进入 `caller` 的下一个基本块中。比如在未初始化分析时，如果在基本块 `B2` 内变量 `&x` 作为参数传入函数 `func`，而变量 `y` 没有进入 `func`，那么可以直接把 `y` 从 `callSite` 的数据流事实集合中传入 `retSite` 的数据流集合中。`callees` 是 `callSite` 中的所有 `callee` 的集合。
   5. getSummaryFlowFunction：`getSummaryFlowFunction(const DFAICFGBlock *curr, const ASTFunction *destMthd)` 是论文中提到的一个概念，用于记录 `curr` 处的数据流事实经过 `destMthd` 的计算后返回到调用点的结果。
3. 定义编码完具体的数据流问题后，实例化一个数据流问题对象，并将它传参到 `IFDSSolver` 实例化一个过程内数据流分析求解器，然后调用求解器的 solve() 即可。注意 `IFDSSolver` 也是一个模板类，其模板参数和 `DefaultIFDSProblem` 的保持一致即可。
4. 注意命名空间，上述 `DefaultIFDSProblem` 和 `IFDSSolver` 均在**命名空间 `namespace hwdfa` 中**。

