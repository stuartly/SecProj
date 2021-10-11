#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm-c/Target.h>
#include <llvm/Support/CommandLine.h>

#include "checkers/RedundantFunctionCallInLoopChecker.h"
#include "checkers/TemplateChecker.h"

#include "framework/ASTManager.h"
#include "framework/BasicChecker.h"
#include "framework/CallGraph.h"
#include "framework/Config.h"
#include "framework/Logger.h"
#include "framework/Report.h"

#include "CFG/InterProcedureCFG.h"
#include "P2A/PointToAnalysis.h"

using namespace clang;
using namespace llvm;
using namespace clang::tooling;

ReportSpace::Report external_bug_report;

int main(int argc, const char *argv[])
{
   ofstream process_file("time.txt");
   if (!process_file.is_open())
   {
      cerr << "can't open time.txt\n";
      return -1;
   }
   clock_t startCTime, endCTime;
   startCTime = clock();

   LLVMInitializeNativeTarget();
   LLVMInitializeNativeAsmParser();

   std::vector<std::string> ASTs = initialize(argv[1]);

   Config configure(argv[2]);

   ASTResource resource;
   ASTManager manager(ASTs, resource, configure);
   Logger::configure(configure);

   auto enable = configure.getOptionBlock("CheckerEnable");

   CallGraph *call_graph = nullptr;
   InterProcedureCFG *icfgPtr = nullptr;
   PointToAnalysis *p2a = nullptr;

   if (Config::IsBlockConfigTrue(enable, "CallGraphChecker"))
   {
      process_file << "Starting CallGraphChecker check" << endl;
      clock_t start, end;
      start = clock();

      call_graph =
          new CallGraph(manager, resource, configure.getOptionBlock("CallGraph"));

      end = clock();
      //call_graph->printCallGraph(std::cout);
      unsigned sec = unsigned((end - start) / CLOCKS_PER_SEC);
      unsigned min = sec / 60;
      process_file << "Time: " << min << "min" << sec % 60 << "sec" << endl;
      process_file
          << "End of CallGraphChecker "
             "check\n-----------------------------------------------------------"
          << endl;
   }


   if (Config::IsBlockConfigTrue(enable, "TemplateChecker"))
   {
      process_file << "Starting TemplateChecker check" << endl;
      clock_t start, end;
      start = clock();

      TemplateChecker checker(&resource, &manager, call_graph, &configure);
      checker.check();

      end = clock();
      unsigned sec = unsigned((end - start) / CLOCKS_PER_SEC);
      unsigned min = sec / 60;
      process_file << "Time: " << min << "min" << sec % 60 << "sec" << endl;
      process_file
          << "End of TemplateChecker "
             "check\n-----------------------------------------------------------"
          << endl;
   }


   if (Config::IsBlockConfigTrue(enable, "RedundantFunctionCallInLoopChecker"))
   {
      process_file << "Starting RedundantFunctionCallInLoopChecker" << endl;
      clock_t start, end;
      start = clock();
      RedundantFunctionCallInLoopChecker checker(&resource, &manager, call_graph, &configure);
      checker.check();
      end = clock();
      unsigned sec = unsigned((end - start) / CLOCKS_PER_SEC);
      unsigned min = sec / 60;
      process_file << "Time: " << min << "min" << sec % 60 << "sec" << endl;
      process_file
          << "End of RedundantFunctionCallInLoopChecker "
             "check\n-----------------------------------------------------------"
          << endl;
   }

   endCTime = clock();
   unsigned sec = unsigned((endCTime - startCTime) / CLOCKS_PER_SEC);
   unsigned min = sec / 60;
   process_file << "-----------------------------------------------------------"
                   "\nTotal time: "
                << min << "min" << sec % 60 << "sec" << endl;

   external_bug_report.dump();
   return 0;
}
