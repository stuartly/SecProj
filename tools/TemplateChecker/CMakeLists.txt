add_executable(TemplateChecker main.cpp)

# specify the libs that the target 'TemplateChecker' needs
# !!! DO NOT change the order of the libs !!!
# libs will be linked in this order, and changing the order could lead to link errors
target_link_libraries(TemplateChecker
    checkers
    point_to_analysis
    framework
    cfg
    DetectMe
    ${CLANG_LIBS}
    ${LLVM_LIBS}
    ${OTHER_LIBS}
    )
