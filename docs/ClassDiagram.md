### 使用
1. 参考benchmark中生成astList.txt
2. config.txt: path是项目的根路径, showData表示是否展示类成员变量和函数, showDependency表示是否展示类之间依赖关系.  
showDependency为true时，showData也需要为true.  
对于mysql这样较大的项目，建议都置为false，否则生成的图片过大。
3. 编译项目，得到SAFE-HW/cmake-build-debug/tools中ClassDiagramChecker可执行文件
4. ./ClassDiagramChecker astList.txt config.txt得到out.dot
5. 通过dot -Tpng out.dot -o out.png或者dot -Tsvg out.dot -o out.svg可以将dot转化成图片

### 访问接口
1. 得到存储数据的map 
   ``` C++
   ASTContext& AST = astUnit->getASTContext();
   ClassDiagramVisitor cdv(&AST,path);
   cdv.HandleTranslationUnit(AST);
   std::map<std::string,Record*> temp = cdv.getRecordDB();
   ``` 
2. 通过遍历map可以得到父类，子类，成员变量、函数等信息
