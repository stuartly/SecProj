#include "checkers/TemplateChecker.h"

extern ReportSpace::Report external_bug_report;

void TemplateChecker::check() {
  readConfig();
  getEntryFunc();
  if (entryFunc != nullptr) {
    FunctionDecl *funDecl = manager->getFunctionDecl(entryFunc);
    std::cout << "The entry function is: "
              << funDecl->getQualifiedNameAsString() << std::endl;
    std::cout << "Here is its dump: " << std::endl;
    funDecl->dump();
  } else {
    return;
  }

  LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  std::unique_ptr<CFG> &cfg = manager->getCFG(entryFunc);
  for (CFG::iterator iter = cfg->begin(); iter != cfg->end(); ++iter) {
    CFGBlock *block = *iter;
    for (CFGBlock::iterator iterblock = block->begin();
         iterblock != block->end(); ++iterblock) {
      CFGElement element = (*iterblock);
      std::cout << "the element is: " << std::endl;
      element.dump();
      if (element.getKind() != CFGElement::Kind::Statement)
        continue;
      const Stmt *it = ((CFGStmt *)&element)->getStmt();
      if (it == nullptr || !isa<CallExpr>(it))
        continue;
      if (const CallExpr *tmp = dyn_cast<CallExpr>(it)) {
        // a report demo
        ReportSpace::Defect *defect = new ReportSpace::Defect(ReportSpace::DefectName::HugeMemory, ReportSpace::DefectType::Warning,
          "/home/Desktop/main.cpp", 1, 1, "");
        external_bug_report.addToReport(defect);
      }
    }
  }
  cfg->dump(LangOpts, true);

  // std::vector<CFGBlock *> topoOrderCFG = cfg->getTopoOrder();
  // for (auto it = topoOrderCFG.begin(); it != topoOrderCFG.end(); it++) {
  //   CFGBlock *b = *it;
  //   std::cout << b->getBlockID() << std::endl;
  // }
}

void TemplateChecker::readConfig() {
  std::unordered_map<std::string, std::string> ptrConfig =
      configure->getOptionBlock("TemplateChecker");
  request_fun = stoi(ptrConfig.find("request_fun")->second);
  maxPathInFun = 10;
}

void TemplateChecker::getEntryFunc() {
  std::vector<ASTFunction *> topLevelFuncs = call_graph->getTopLevelFunctions();
  for (auto fun : topLevelFuncs) {
    const FunctionDecl *funDecl = manager->getFunctionDecl(fun);
    if (funDecl->getQualifiedNameAsString() == "main") {
      entryFunc = fun;
      return;
    }
  }
  entryFunc = nullptr;
  return;
}
