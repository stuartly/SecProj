
# 变量分析使用类

## 符号值

每个变量包括局部变量与全局变量都被记录为符号值，位于命名空间``Symbol``下面，针对于C语言，共有四种符号值
  + Normal
  ```c++
  	struct Normal{
        const clang::QualType VarType;
		int TotalWriteCounts;
        int TotalReadCounts;
    	SymbolType *ParentSymbol;
        ...
	};
  ```

> 表示一个普通变量，例如``int a,const char *b``;并保存他们的类型在``VarType``类中，
> ``TotalWriteCounts,TotalReadCounts``表示该变量的读写次数
> + Array
> ```c++
> struct Array
> {
>        const clang::QualType VarType;
>		 size_t ElementSize;
>        llvm::DenseMap<size_t,SymbolType*> ElementSymbols;
>        SymbolType* UncertainElementSymbol;
>        SymbolType *ParentSymbol; 
> 		...
> };
> ```
> 表示数组变量,``ElementSymbols``记录各个下标的访问情况,
> ``UncertainElementSymbol``表示无法确定那个访问那个下标,
> 例如： int a[12], a[i]=2<br />
> 使用成员函数``isConstantArray()``来判断是否为常量数组
> + Pointer
> ```c++
> struct Pointer{
>       int TotalWriteCounts;
>       int TotalReadCounts;
>       const clang::QualType PointerType;
> 		SymbolType *PointeeSymbol;
>       SymbolType *ParentSymbol;
> 	...
> };
> ```
> 表示一个指针变量，这里没有实现精确的别名分析，因此无论该指针指向什么变量，都只维护一个
> ``PointeeSymbol``表示该指针指向的变量
> ``TotalWriteCounts``与``TotalReadCounts``表示该指针本身读写次数
> + Record
>   ```c++    
> // 结构体，共用体
> // 保存域的名字及符号值
> struct Record {
>       const clang::QualType VarType;
>       std::vector<std::pair<const char *,SymbolType*> > *Elements;
>       SymbolType *ParentSymbol;
> 	...
> };
>   ```
> ```
> 表示一个结构体，或者共用体变量，``VarType``表示类型，``Elements``记录该结构体/共用体
> 域变量的名字与符号值
> ```

每个符号类型都有一个``ParentSymbol``用来指向父符号，这出现在嵌套结构中，例如：
```c++
	struct s{
	  int a;
	  int b;
	}s1;//在这里，成员a,b是一个Normal符号，而s1是一个Record符号，因此a，b所表示的符号中的ParentSymbol都指向s1
```
顶层的符号类型``ParentSymbol``为空

## ``SymbolType``类型
> SymbolType 类型表示为`` using SymbolType=llvm::PointerUnion<Normal*,Array*,Record*,Pointer*>;``
> 为``llvm``内置的数据结构，表示安全访问的指针共用体

## ``Footprints``类型

这个类作为储存*C*语言中一个作用域的所有符号值而存在

此类有几个重要的成员以及方法

*成员*

1. ``llvm::StringMap<Symbol::SymbolType> SymbolMap;``

2. ``llvm::StringMap<const clang::VarDecl> VarDeclMap;``

> ``Footprints``类保存``llvm::StringMap<SymbolType*> SymbolMap``内容，将变量的名字哈希到对应的符> 号值，``llvm::StringMap``类是一个字符串哈希结构，将字符串哈希到模板变量中。
>
> 例如``llvm::StringMap<SymbolType*>``表示将字符串哈系到``SymbolType*``对象中
> 
> ``Footprints``类还保存``llvm::StringMap<const clang::VarDecl*> VarDeclMap``内容，将名字哈希> 到对应的声明对象中。

*方法*

```c++
const llvm::StringMap<Symbol::SymbolType*>*  getSymbolMap() //返回储存符号值的StringMap对象

const llvm::StringMap<const clang::VarDecl*> * getVarDeclMap() //返回储存所有声明的StringMap对象

const clang::VarDecl *getDeclaration(const char *Name） //根据变量的名字获得对应的声明

const clang::VarDecl *getDeclarationOfSymbol(Symbol::SymbolType *ST) //根据变量的Symbol获得对应的声明

Symbol::SymbolType* getParentSymbol(const Symbol::SymbolType *ST)//获得一个Symbol对象的父Symbol对象

Symbol::SymbolType* getSymbol(const char* Name) const; //根据变量的名字获得对应的Symbol对象

std::pair<int,int> getSymbolCounts(Symbol::SymbolType*ST) //获得一个Symbol对象的读写次数，第一个是读，第二个是写

std::pair<int,int> getRecordVariableCounts(const char* RecordName,const char*FieldName="") //获得一个结构体成员（包括所有该结构体声明的变量）的读写次数，第一个参数是结构体声明的名字，第二个是域成员的名字，如果不指定第二个参数，则返回的是该结构体声明的所有变量读写次数，第一个返回值是读，第二个是写
Symbol::SymbolType* getParentSymbol(const Symbol::SymbolType *ST);//获得父符号
Symbol::SymbolType* getTopSymbol(Symbol::SymbolType *ST);//获得顶层的符号
```

## ``VariableAnalyzer``类型

这是最重要的入口类，作用是分析``clang::ASTContext``保存的AST信息，给出每一个作用域的变量读写情况

*成员*


1. ``using AnalyzedArrayType=llvm::SmallVector<Footprints*,DEFAULTSCOPEDEPTH>``
2. ``AnalyzedArrayType AnalyzedArray``//储存所有作用域的信息
3. ``int AnalyzedLevel`` //表示当前作用域索引


注意：成员``AnalyzedArray``保存以分析完成的变量信息，其中	``llvm::SmallVector<Footprints*,DEFAULTSCOPEDEPTh>``是类似于``std::vector``的结构，对于给定的数组大小参数``DEFAULTSCOPEDEPTH``优化了，这是一个配置项，默认是10。

注意：由于作用域的复杂性，因此，每当分析完一个作用域后，该``Footprints``信息便会释放，即退出一个作用域后，``AnalyzedLevel``减1，``AnalysedArray调用pop_back()``释放


通过重写一组虚函数来进行自定义的分析操作


1. ``virtual void enterCallExpr(const clang::CallExpr*CE)``：当进入一个*CallExpr*被调用
2. ``virtual void exitCallExpr(const clang::CallExpr *CE)``：当退出一个*CallExpr*被调用
3. ``virtual void enterAnonymousScope()``：当进入一个匿名作用域被调用
4. ``virtual void enterIfScope(const clang::IfStmt*IS)``：当进入*If*作用域被调用
5. ``virtual void enterElseScope(const clang::IfStmt *IS)``：当进入*Else*作用域被调用
6. ``virtual void enterForScope(const clang::ForStmt*FS)``：当进入*For*作用域被调用
7. ``virtual void enterDoScope(const clang::DoStmt*DS)``：当进入*Do*作用域被调用
8. ``virtual void enterWhileScope(const clang::WhileStmt*WS)``：当进入*While*作用域被调用
9. ``virtual void enterSwitchScope(const clang::SwitchStmt*SS)``：当进入*Switch*作用域被调用
10. ``virtual void enterCaseScope(const clang::CaseStmt*CS)``：当进入*Case*作用域被调用
11. ``virtual void enterDefaultScope(const clang::DefaultStmt*DS)``：当进入*Default*作用域被调用
12. ``virtual void enterGlobalScope(const clang::TranslationUnitDecl*TU)``：当进入全局作用域被调用
13. ``virtual void enterFunctionScope(const clang::FunctionDecl*FD)``：当进入函数作用域被调用
14. ``virtual void visitSymbol(Symbol::SymbolType *ST,bool Read,const clang::Expr*E)``：当准备对一个*Symbol*进行读或写的操作时被调用，参数*Read*为*true*表示读，*false*表示写，第三个参数表示相关联的*Expr*对象
15.  ``virtual void visitSymbolAddress(Symbol::SymbolType *ST,const clang::Expr*E)``：当准备对一个*Symbol*的地址进行读操作时被调用，第三个参数表示相关联的*Expr*对象，存在``&``取地址时被调用
16. ``virtual void finishScope(Footprints* FP)``：当退出一个作用域的时候被调用，参数*FP*表示该作用域下声明的变量*Symbol*信息

例如：
```c++
	void finishScope(Footprints*FP){
		FP->dump();//当分析完某一个作用域，打印出来得到的信息
	}
```

*其它成员与方法*

 + ``IndexOfArg``: 当进入一个*CallExpr*的时候，该变量表示位于第几个参数中，除此之外无意义
 + ``const clang::ASTContext* getASTContext()``:获得*ASTContext*对象

*选项*
 + ``IgnoreCStandardLibrary``：此选项为真，表示不进入标准库函数内部分析
 + ``std::vector<const clang::DirectoryEntry*> IgnoreDirs``：储存自定义忽略分析的目录，
	位于此目录下的源文件除了全局变量之外其余声明将不会被分析

 > 通过方法``setIgnoreCStandardLibrary(bool Ignore)``来设置选项``IgnoreCStandardLibrary``

 > 通过方法``void addIgnoreDir(std::string& Dir)``来添加一个忽略的路径

``VariableAnalyzer(clang::ASTContext*Context)``: 构造函数<br />
``analyze()``: 调用分析函数,对于整个``ASTContext``内容分析

*构造函数*：

+ ``VariableAnalyzer(const clang::ASTContext *AC)``

*入口*
+ 调用``analyze()``方法开始分析

*Tips*
+ 成员变量``AnalyzedLevel``表示当前访问作用域索引，例如当为0的时候是一个全局作用域，为1的时候是函数作用域
