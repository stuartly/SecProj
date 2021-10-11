/*************************RedundantFunctionCallInLoopChecker.h**************
 * @file RedundantFunctionCallInLoopChecker.h                              *
 * @author Comphsics                                                       *
 * @date 2021.1.19                                                         *
 * @details                                                                *
 *     This file implements redundant function call in loop checker        *
 * @todo                                                                   *
 *      For nested functions, we do not analyze the inner functions which  *
 *      have parameters, they would be very complicated                    *
 ***************************************************************************/

#ifndef REDUNDANTFUNCTIONCALLINLOOPCHECKER_H
#define REDUNDANTFUNCTIONCALLINLOOPCHECKER_H
#include <vector>
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include <tuple>
#include "framework/VariableAnalyzer.h"
#include "framework/BasicChecker.h"
#include "framework/Report.h"


class RedundantFunctionCallInLoopChecker;
class RedundantFunctionCallInLoopAnalyzer final: public VariableAnalyzer{
    friend class RedundantFunctionCallInLoopChecker;
private:

    /// predefined redundant functions.
    std::vector<llvm::StringRef> PredefinedFunctions;
    enum RedundantType{
		Redundant,
		USELESS,
		Other
	};
	enum AccessType{
		AT_NO_RW,
		AT_R,
		AT_W,
		AT_RW
	};
	struct PatchInfo{
		const clang::Stmt* LS;
		const clang::FunctionDecl *FD;
		const clang::CallExpr *CE;
	};
    /// @brief A tiny struct that holds a function triple info.
    /// @details
    /// The triple which is consisted of five parts:
    /// A FunctionDecl denotes the function information;
    /// A RedundantType object which shows the redundant type this function is;
    /// A Footprints which records all the global symbols this function reads and writes.
    /// A Footprints which records all the parameter symbols this function reads and writes.
    /// A Footprints which record the symbols of the inner functions, in this case,those inner
    ///     functions all have no arguments.
	struct FunctionTripleInfo{
        const clang::FunctionDecl*FD;
        RedundantType RT;
        llvm::DenseMap<Symbol::SymbolType*,bool> GlobalFP;// true is Read
        std::vector<Symbol::SymbolType*> ArgsFP;
        std::vector<FunctionTripleInfo*> InnerFPs;
	};
	/// @brief A tiny struct that holds a CallExpr and a set of its arguments symbols,
	/// and the index of it.
	struct CallExprInfo{
	    const clang::CallExpr *CE;
		std::vector<std::pair<Symbol::SymbolType*,size_t>> ArgsFP;
	};
	/// records all global footprints inside loops,may be nested.
	// the integer field has three values:
	// 0: not used,1 read,2 write 3, read and write
	std::vector<llvm::DenseMap<Symbol::SymbolType*,unsigned char>> LoopFPs;
	llvm::StringMap<FunctionTripleInfo*> FunctionTriples;
	/// @brief Denotes the current function triple info.
	llvm::StringMapIterator<FunctionTripleInfo*> CurrentFunctionTriple;
	/// @brief Denotes whether in the call expr context.
	bool IsInCallExpr;
	/// @brief Denotes some FunctionDecl that waiting for judging redundancy after
	/// we exiting the current loop.
	std::vector<CallExprInfo*> CurrentUnresolvedCallExprs;
	/// @brief Records the patches we had resolved.
	std::vector<PatchInfo*> Patches;
	/// @brief Records the stack which contains of a loop and its scope level.
	std::stack<std::pair<const clang::Stmt*,int>> LoopStack;


	Symbol::SymbolType* copy(Symbol::SymbolType *Src);
	void checkRedundantFunctionCall();
	/// @brief Check the footprints of the given loop scope with the given
	///		function of the global symbols.
	/// @retval true if redundancy,otherwise false
	bool checkGlobalSymbolsRedundancy(FunctionTripleInfo*FTI,llvm::DenseMap<Symbol::SymbolType*,unsigned char> &LoopGlobal);
	/// @brief Whether A global of two scope is seen redundancy according to the rule.
	/// @details
	/// Two kind of usage:
	///     1): compare a function and a loop
	///     2): compare a function and another function
	///
	/// Rule: If applying read on the <code>L</code>,then a redundancy occurs when applying
	///     no write on the <code>R</code>,  If applying write on the <code>L</code>,then a redundancy occurs when applying
	///     no read on the <code>R</code>.
	/// @return If redundant, returns true. Otherwise false.
	bool checkGlobalSymbolRedundancy(const Symbol::SymbolType*L ,const Symbol::SymbolType *R);

	/// Check the symbols redundancy,note that these symbols are arguments.
    /// @param FTI The argument symbol of the function
    /// @param CEI The argument symbol of the call expr
    bool checkArgSymbolsRedundancy(FunctionTripleInfo*FTI,CallExprInfo*CEI );
	// these are a set of checks.
	bool checkNormalSymbolRedundancy(Symbol::Normal *L,Symbol::Normal *R,bool IsSame);
	bool checkPointerSymbolRedundancy(Symbol::Pointer*L,Symbol::Pointer*R,bool IsSame);
	bool checkRecordSymbolRedundancy(Symbol::Record *L,Symbol::Record *R,bool IsSame);
	bool checkArraySymbolRedundancy(Symbol::Array *L,Symbol::Array *R,bool IsSame);
	template<bool IsArg>
	AccessType access(Symbol::SymbolType *ST);
	// If IsArg is true, then the R symbol can be nullptr.
	bool checkArgSymbolRedundancy(Symbol::SymbolType*L,Symbol::SymbolType*R );
    bool isPredefinedRedundantFunction(CallExprInfo *CEI);

	bool checkSymbolRedundancy(Symbol::SymbolType*L,Symbol::SymbolType*R,bool IsSame);
    /// @brief get all predefined functions that are redundant
    /// which are record in the *PREDEFINEDFUNCTIONS.def*
    void initializePredefinedRedundantFunctions(){
#ifndef PREDEFINEDREDUNDANT
#define PREDEFINEDREDUNDANT(FUNC,DESCRIPTION) PredefinedFunctions.push_back(#FUNC);
#include "checkers/PREDEFINEDFUNCTIONS.def"
#endif
    }

    /// @brief Given a symbol,returns its level in the scope.
    int getScopeLevelOfSymbol(Symbol::SymbolType*ST)const;

    /// @brief Resolved the outer function redundancy when an inner function call inside.
    /// @param Inner The <code>FunctionTripleInfo</code> of the inner function call.
    /// @param Outer The <code>FunctionTripleInfo</code> of the outer function.
    void resolvedNestedFunctionRedundancy(FunctionTripleInfo*Inner,FunctionTripleInfo*Outer);


	void insertNewScopeCount();
	void eraseCurrentScopeCount();

	int getCountOfLoopStmt(const clang::ForStmt *FS) override;
protected:

    // A set of overridden methods.

    void enterForScope(const clang::ForStmt*FS)override;
    void enterWhileScope(const clang::WhileStmt *WS)override;
    void enterDoScope(const clang::DoStmt *DS) override;
    void enterFunctionScope(const clang::FunctionDecl*FD) override;
    void finishScope(Footprints *FP) override;
    void enterCallExpr(const clang::CallExpr *CE) override;
    void exitCallExpr(const clang::CallExpr*CE)override;

    void visitSymbol(Symbol::SymbolType*ST,bool Read,const clang::Expr*E) override;

public:
	explicit RedundantFunctionCallInLoopAnalyzer(const clang::ASTContext *AC);
	ReportSpace::Defect* convertToReport(PatchInfo *Patch);
	void reportToConsole(PatchInfo *Patch);

};
class RedundantFunctionCallInLoopChecker:BasicChecker{
private:
	IgnLibPathConfig IgnorePaths;
    std::vector<RedundantFunctionCallInLoopAnalyzer*> Checkers;
    void readConfig(RedundantFunctionCallInLoopAnalyzer *Analyzer);

public:
	void addSingleChecker(RedundantFunctionCallInLoopAnalyzer *C);
	RedundantFunctionCallInLoopChecker(ASTResource *resource, ASTManager *manager,
                                       CallGraph *callGraph, Config *configure);
	
	void check();

};
#endif