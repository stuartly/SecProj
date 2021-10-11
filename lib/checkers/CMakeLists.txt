aux_source_directory(. checkers_srcs)
add_library(checkers ${checkers_srcs}
BranchChecker/LCFG.cpp
BranchChecker/BranchChecker.cpp
BranchChecker/BranchAnalyzer.cpp MemoryRewriteChecker.cpp)
