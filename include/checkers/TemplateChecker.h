#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <stack>
#include <string>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Attr.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
// #include <clang/Analysis/CFG.h>
// #include "CFG/CFG.h"
#include "CFG/SACFG.h"

#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/raw_ostream.h>

#include "CFG/InterProcedureCFG.h"
#include "framework/BasicChecker.h"
#include "framework/Report.h"

using namespace clang;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

class TemplateChecker : public BasicChecker {
public:
  TemplateChecker(ASTResource *resource, ASTManager *manager,
                  CallGraph *call_graph, Config *configure)
      : BasicChecker(resource, manager, call_graph, configure){
            /*
            InterProcedureCFG *icfgPtr =
                new InterProcedureCFG(manager, resource, call_graph, configure);
            icfg = icfgPtr;
            */
        };
  void check();

private:
  void getEntryFunc();
  void readConfig();
  ASTFunction *entryFunc;

  int request_fun;
  int maxPathInFun;

  InterProcedureCFG *icfg;
};
