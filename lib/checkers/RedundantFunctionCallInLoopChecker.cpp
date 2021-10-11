/*************************VariableAnalyzer.cpp***********************************
 * @file RedundantFunctionCallInLoopChecker.cpp                                 *
 * @brief implementation of the RedundantFunctionCallInLoopChecker.h            *
 * @author Comphsics                                                            *
 * @date 2021.1.19                                                              *
 *******************************************************************************/

#include "checkers/RedundantFunctionCallInLoopChecker.h"

extern ReportSpace::Report external_bug_report;

Symbol::SymbolType* RedundantFunctionCallInLoopAnalyzer::copy(Symbol::SymbolType *Src){
    if(!Src){
        return nullptr;
    }
    if(Src->is<Symbol::Normal*>()){
        const auto& N=Src->get<Symbol::Normal*>();
        auto Ret=new Symbol::Normal(N->VarType);
        Ret->TotalReadCounts=N->TotalReadCounts;
        Ret->TotalWriteCounts=N->TotalWriteCounts;
		Ret->ParentSymbol=N->ParentSymbol;
        return new Symbol::SymbolType(Ret);
    }else if(Src->is<Symbol::Pointer*>()){
        const auto& P=Src->get<Symbol::Pointer*>();
        auto Ret=new Symbol::Pointer(P->PointerType);
        Ret->TotalReadCounts=P->TotalReadCounts;
        Ret->TotalWriteCounts=P->TotalWriteCounts;
        Ret->PointeeSymbol=copy(P->PointeeSymbol);
		Ret->ParentSymbol=P->ParentSymbol;
        return new Symbol::SymbolType(Ret);
    }else if(Src->is<Symbol::Array*>()){
        const auto& A=Src->get<Symbol::Array*>();
        auto Ret=new Symbol::Array(A->VarType);
        Ret->UncertainElementSymbol=copy(A->UncertainElementSymbol);
        int Size=A->ElementSymbols.size();
		Ret->AggregateReadCounts=A->AggregateReadCounts;
		Ret->AggregateWriteCounts=A->AggregateWriteCounts;
        for(auto&Ele:A->ElementSymbols){
            Ret->ElementSymbols.insert({Ele.getFirst(),copy(Ele.getSecond())});
        }
		Ret->ParentSymbol=A->ParentSymbol;
        return new Symbol::SymbolType(Ret);
    }else if(Src->is<Symbol::Record*>()){
        const auto& R=Src->get<Symbol::Record*>();
        auto Ret=new Symbol::Record(R->VarType);
        int Size=R->Elements.size();
        for(size_t i=0;i<Size;i++){
            Ret->Elements.push_back({R->Elements[i].first,copy(R->Elements[i].second)});
        }
		Ret->ParentSymbol=R->ParentSymbol;
        return new Symbol::SymbolType(Ret);
    }
    return nullptr;
}

void RedundantFunctionCallInLoopAnalyzer::checkRedundantFunctionCall(){
    assert(!LoopStack.empty());
    const clang::Stmt *LS=LoopStack.top().first;
    for(auto&CE : CurrentUnresolvedCallExprs){
        // This works.
        auto FD=llvm::cast<clang::FunctionDecl>(CE->CE->getCalleeDecl());
        // If is predefined redundant function check it!
		if(isPredefinedRedundantFunction(CE)){
			const auto Patch=new PatchInfo{LS,FD,CE->CE};
            Patches.push_back(Patch);
            // add to the standard report.
            external_bug_report.addToReport(convertToReport(Patch));
		}
		auto Result=FunctionTriples.find(FD->getName());
		if(Result==FunctionTriples.end()){
			continue;
		}
        if(Result->getValue()->RT==RedundantType::USELESS){
            continue;
        }
        if(Result->getValue()->RT==RedundantType::Redundant){
            const auto Patch=new PatchInfo{LS,FD,CE->CE};
            Patches.push_back(Patch);
            // add to the standard report.
            external_bug_report.addToReport(convertToReport(Patch));
        }else{
            // This means the FD may read or write on global variables,
            // But do not both apply read and write on the same symbol belonging to
            // some global variable.
            bool Redundancy=true;
			auto LoopGlobal=LoopFPs.back();
			Redundancy&=checkGlobalSymbolsRedundancy(Result->getValue(),LoopGlobal);
            bool InnerRedundancy= true;
            // if having inner functions, check them.
            for(const auto &Inner:Result->getValue()->InnerFPs){
                InnerRedundancy&=checkGlobalSymbolsRedundancy(Inner,LoopGlobal);
                if(!InnerRedundancy){
                    break;
                }
            }
            Redundancy&=InnerRedundancy;
			Redundancy&=checkArgSymbolsRedundancy(Result->getValue(),CE);
            if(Redundancy){
                const auto Patch=new PatchInfo{LS,FD,CE->CE};
                Patches.push_back(Patch);
                // add to the standard report.
                external_bug_report.addToReport(convertToReport(Patch));
            }
        }
    }
    CurrentUnresolvedCallExprs.clear();
}

bool RedundantFunctionCallInLoopAnalyzer::checkGlobalSymbolsRedundancy(RedundantFunctionCallInLoopAnalyzer::FunctionTripleInfo*FTI,llvm::DenseMap<Symbol::SymbolType*,unsigned char> &LoopGlobal){
    bool Redundancy=true;
    const auto Global=FTI->GlobalFP;
    auto SymbolResult=LoopGlobal.end();
	for(const auto &Ele:Global){
		SymbolResult=LoopGlobal.find(Ele.getFirst());
		if(SymbolResult!=LoopGlobal.end()){
			switch(SymbolResult->getSecond()){
				case 1:{
					if(Ele.getSecond()==false){
						return false;
					}
					break;
				}
				case 2:{
					if(Ele.getSecond()==true){
						return false;
					}
					break;
				}
				case 3:{
					return false;
					break;
				}
				default:
					break;
			}
		}
    }
    return true;
}


bool RedundantFunctionCallInLoopAnalyzer::isPredefinedRedundantFunction(CallExprInfo *CEI){
    assert(llvm::isa<clang::FunctionDecl>(CEI->CE->getCalleeDecl()));
    const llvm::StringRef Name=llvm::cast<clang::FunctionDecl>(CEI->CE->getCalleeDecl())->getName();
	for(const auto &FName:PredefinedFunctions){
        if(Name==FName){
			// check the argument of the loop had been written.
			int Write=0;
			for(auto&S:CEI->ArgsFP){
				if(!S.first){
					return false;
				}else if(S.first->is<Symbol::Normal*>()){
					Symbol::Normal *N=S.first->get<Symbol::Normal*>();
					if(N->ExtraCounts.empty()){
						Write=N->TotalWriteCounts;
					}else{
						Write=N->ExtraCounts.back().first;
					}
					if(Write!=0){
						return false;
					}
				}else if(S.first->is<Symbol::Pointer*>()){
					Symbol::Pointer *P=S.first->get<Symbol::Pointer*>();
					if(P->ExtraCounts.empty()){
						Write=P->TotalWriteCounts;
					}else{
						Write=P->ExtraCounts.back().first;
					}
					if(Write!=0){
						return false;
					}
				}
			}
			// if all argument had been no written, diagnose a warning.
            // check some special situations.
			return true;
        }
    }
	return false;
}

void RedundantFunctionCallInLoopAnalyzer::resolvedNestedFunctionRedundancy(FunctionTripleInfo *Inner,
                                                                           FunctionTripleInfo *Outer) {
    assert(Inner->ArgsFP.empty());
    if(Inner->FD->getName()==Outer->FD->getName()){
        // A recursive function.
        return;
    }
    if(Outer->RT==RedundantType::USELESS){
        return;
    }
    switch(Inner->RT){
        case RedundantType::USELESS:{
            Outer->RT = RedundantType::USELESS;
            break;
        }
        default:{
            Outer->InnerFPs.push_back(Inner);
            break;
		}
    }

}

bool RedundantFunctionCallInLoopAnalyzer::checkArgSymbolsRedundancy(FunctionTripleInfo*FTI,CallExprInfo*CEI ){
	// consider the arguments.
	auto ArgOfFD=FTI->ArgsFP;
	auto ArgOfCall=CEI->ArgsFP;
	if(!ArgOfFD.empty()&&!ArgOfCall.empty()){
		size_t Index=ArgOfCall[0].second;
		size_t Size=ArgOfFD.size();
		for(int i=0;i<ArgOfCall.size();i++){
			if(Index!=ArgOfCall[i].second){
					Index=ArgOfCall[i].second;
			}
			if(Index>=Size){
				// varArgs
				return false;
			}
			if(Index==ArgOfCall[i].second){
				if(!checkArgSymbolRedundancy(ArgOfFD[Index],ArgOfCall[i].first)){
					return false;
				}
			}
		}
	}
	return true;
}

template<bool IsArg>
RedundantFunctionCallInLoopAnalyzer::AccessType RedundantFunctionCallInLoopAnalyzer::access(Symbol::SymbolType *ST){
	if(!ST||ST->isNull()){
		return AccessType::AT_NO_RW;
	}
	int R=0,W=0;
	if(ST->is<Symbol::Normal*>()){
        const auto L=ST->get<Symbol::Normal*>();
		if(L->ExtraCounts.empty()){
			R=L->TotalReadCounts;
			W=L->TotalWriteCounts;
		}else if(IsArg){
			R=L->ExtraCounts.back().second;
			W=L->ExtraCounts.back().first;
		}else{
			R=L->TotalReadCounts;
			W=L->TotalWriteCounts;
		}
		if(R==0&&W==0){
			return AccessType::AT_NO_RW;
		}else if(R==0&&W!=0){
			return AccessType::AT_W;
		}else if(R!=0&&W==0){
			return AccessType::AT_R;
		}else{
			return AccessType::AT_RW;
		}
    }else if(ST->is<Symbol::Pointer*>()){
        const auto P=ST->get<Symbol::Pointer*>();
		if(P->ExtraCounts.empty()){
			R=P->TotalReadCounts;
			W=P->TotalWriteCounts;
		}else if(IsArg){
			R=P->ExtraCounts.back().second;
			W=P->ExtraCounts.back().first;
		}else{
			R=P->TotalReadCounts;
			W=P->TotalWriteCounts;
		}
		if(R==0&&W==0){
			return AccessType::AT_NO_RW;
		}else if(R==0&&W!=0){
			return AccessType::AT_W;
		}else if(R!=0&&W==0){
			return access<IsArg>(P->PointeeSymbol);
		}else{
			return AccessType::AT_RW;
		}
    }else if(ST->is<Symbol::Array*>()){
        const auto A=ST->get<Symbol::Array*>();
		AccessType Ret=AT_NO_RW;
		if(A->AggregateReadCounts!=0&&A->AggregateWriteCounts!=0){
			Ret=AT_RW;
		}else if(A->AggregateReadCounts!=0&&A->AggregateWriteCounts==0){
			Ret=AT_R;
		}else if(A->AggregateReadCounts==0&&A->AggregateWriteCounts!=0){
			Ret=AT_W;
		}
		AccessType UncertainAT=access<IsArg>(A->UncertainElementSymbol);
		if(UncertainAT==AT_RW){
			Ret=AT_RW;
		}else if(UncertainAT==AT_R&&Ret==AT_W){
			Ret=AT_RW;
		}else if(UncertainAT==AT_W&&Ret==AT_R){
			Ret=AT_RW;
		}else if(Ret==AT_NO_RW){
			Ret=UncertainAT;
		}
		for(auto&Ele:A->ElementSymbols){
			AccessType AT=access<IsArg>(Ele.second);
			if(AT==AT_R){
				if(Ret==AT_NO_RW){
					Ret=AT_R;
				}else if(Ret==AT_W){
					Ret=AT_RW;
				}	
			}else if(AT==AT_W){
				if(Ret==AT_NO_RW){
					Ret=AT_W;
				}else if(Ret==AT_R){
					Ret=AT_RW;
				}
			}
		}
		return Ret;
    }else if(ST->is<Symbol::Record*>()){
        auto R=ST->get<Symbol::Record*>();
		AccessType Ret=AT_NO_RW;
		for(auto&Ele:R->Elements){
			AccessType AT=access<IsArg>(Ele.second);
			if(AT==AT_R){
				if(Ret==AT_NO_RW){
					Ret=AT_R;
				}else if(Ret==AT_W){
					Ret=AT_RW;
				}	
			}else if(AT==AT_W){
				if(Ret==AT_NO_RW){
					Ret=AT_W;
				}else if(Ret==AT_R){
					Ret=AT_RW;
				}
			}
		}
		return Ret;
	}
	return AT_RW;
}

bool RedundantFunctionCallInLoopAnalyzer::checkNormalSymbolRedundancy(Symbol::Normal *L,Symbol::Normal *R,bool IsSame){
	int LR=0,LW=0,RR=0,RW=0;// left read,left write,right read,right write
	if(L){
		LR=L->TotalReadCounts;
		LW=L->TotalWriteCounts;
	}
	if(R){
		if(R->ExtraCounts.empty()){
			RR=R->TotalReadCounts;
			RW=R->TotalWriteCounts;
		}else{
			RR=R->ExtraCounts.back().second;
			RW=R->ExtraCounts.back().first;
		}
	}
	if(RR==0&&RW==0){
		if(IsSame){
			if(LR!=0&&LW!=0){
				return false;
			}
		}
		return true;
	}else if(RR!=0&&RW==0){
		if(IsSame){
			if(LW!=0){
				return false;
			}else{
				return true;
			}
		}else{
			return true;
		}
	}else if(RR==0&&RW!=0){
		if(IsSame){
			if(LR!=0){
				return false;
			}else{
				return true;
			}
		}else{
			if(LR==0&&LW==0){
				return true;
			}else{
				return false;
			}
		}
	}else{
		if(IsSame){
			return false;
		}else{
			if(LR==0&&LW==0){
				return true;
			}else{
				return false;
			}
		}
	}
	return true;
}

bool RedundantFunctionCallInLoopAnalyzer::checkPointerSymbolRedundancy(Symbol::Pointer*L,Symbol::Pointer*R,bool IsSame){
	int LR=0,LW=0,RR=0,RW=0;// left read,left write,right read,right write
	if(!L||!R){
		return true;
	}
	if(L){
		LR=L->TotalReadCounts;
		LW=L->TotalWriteCounts;
	}
	if(R){
		if(R->ExtraCounts.empty()){
			RR=R->TotalReadCounts;
			RW=R->TotalWriteCounts;
		}else{
			RR=R->ExtraCounts.back().second;
			RW=R->ExtraCounts.back().first;
		}
	}
	if(!R&&L){
		if(IsSame){
			if(LR!=0&&LW!=0){
				return false;
			}
		}
		return checkSymbolRedundancy(L->PointeeSymbol,nullptr,true);
	}else if(!L&&R){
		return true;
	}else{
		if(RR==0&&LR==0){
			return true;
		}else if(RW==0&&LW==0){
			return checkSymbolRedundancy(L->PointeeSymbol,R->PointeeSymbol,true);
		}else{
			return false;
		}
	}
}

bool RedundantFunctionCallInLoopAnalyzer::checkRecordSymbolRedundancy(Symbol::Record *L,Symbol::Record *R,bool IsSame){
	if(!L&&!R){
		return true;
	}else if(!L&&R){
		for(auto&Ele:R->Elements){
			if(!checkSymbolRedundancy(nullptr,Ele.second,IsSame)){
				return false;
			}
		}
	}else if(!R&&L){
		for(auto&Ele:L->Elements){
			if(!checkSymbolRedundancy(Ele.second,nullptr,IsSame)){
				return false;
			}
		}
	}else{
		for(size_t Index=0;Index<L->Elements.size();Index++){
			if(Index>=R->Elements.size()){
				return false;
			}else if(!checkSymbolRedundancy(L->Elements[Index].second,R->Elements[Index].second,IsSame)){
				return false;
			}
		}
	}
	return true;
}

bool RedundantFunctionCallInLoopAnalyzer::checkArraySymbolRedundancy(Symbol::Array *L,Symbol::Array *R,bool IsSame){
	if(!L&&!R){
		return true;
	}else if(!L&&R){
		if(R->UncertainElementSymbol){
			return false;
		}
		for(auto&Ele:R->ElementSymbols){
			if(!checkSymbolRedundancy(nullptr,Ele.getSecond(),IsSame)){
				return false;
			}
		}
	}else if(!R&&L){
		if(L->UncertainElementSymbol){
			return false;
		}
		for(auto&Ele:L->ElementSymbols){
			if(!checkSymbolRedundancy(Ele.getSecond(),nullptr,IsSame)){
				return false;
			}
		}
	}else{
		if(L->UncertainElementSymbol||R->UncertainElementSymbol){
			return false;
		}
		if(IsSame){
			if(L->AggregateReadCounts!=0&&R->AggregateWriteCounts!=0){
				return false;
			}else if(L->AggregateWriteCounts!=0&&R->AggregateReadCounts){
				return false;
			}
		}else{
			if(L->AggregateWriteCounts!=0&&R->AggregateReadCounts){
				return false;
			}
		}
		for(auto&Ele:L->ElementSymbols){
			auto REle=R->ElementSymbols.find(Ele.getFirst());
			if(REle!=R->ElementSymbols.end()){
				if(!checkSymbolRedundancy(Ele.getSecond(),REle->getSecond(),IsSame)){
					return false;
				}
			}else{
				if(!checkSymbolRedundancy(Ele.getSecond(),nullptr,IsSame)){
					return false;
				}
			}
		}
		// actually union.
		for(auto&Ele:R->ElementSymbols){
			auto LEle=L->ElementSymbols.find(Ele.getFirst());
			if(LEle==R->ElementSymbols.end()){
				if(!checkSymbolRedundancy(LEle->getSecond(),Ele.getSecond(),IsSame)){
					return false;
				}
			}
		}
	}
	return true;
}

bool RedundantFunctionCallInLoopAnalyzer::checkArgSymbolRedundancy(Symbol::SymbolType*L,Symbol::SymbolType*R){
	assert(L);
	if(!R){
		// that means the argument R is declared inside the loop.
		if(access<true>(L)==AT_NO_RW){
			return true;
		}else{
			return false;
		}
	}
	// check the array.
	if(L->is<Symbol::Array*>()){
		return checkSymbolRedundancy(L,R,true);
	}else{
		return checkSymbolRedundancy(L,R,false);
	}
}

bool RedundantFunctionCallInLoopAnalyzer::checkSymbolRedundancy(Symbol::SymbolType*L,Symbol::SymbolType*R,bool IsSame){
	if(!L){
		return true;
	}
	if(!R){
		AccessType AT=access<false>(L);
		if(IsSame){
			if(AT!=AT_RW&&AT!=AT_W){
				return true;
			}else{
				return false;
			}
		}else{
			return true;
		}
	}
	if(L->is<Symbol::Normal*>()){
		if(R->is<Symbol::Normal*>()){
			return checkNormalSymbolRedundancy(L->get<Symbol::Normal*>(),R->get<Symbol::Normal*>()
				,false);
		}else{
			AccessType AT=access<true>(R);
			if(AT==AT_NO_RW||AT==AT_R){
				return true;
			}
		}
	}else if(L->is<Symbol::Pointer*>()){
		if(R->is<Symbol::Pointer*>()){
			return checkPointerSymbolRedundancy(L->get<Symbol::Pointer*>(),R->get<Symbol::Pointer*>()
				,false);
		}else{
			AccessType AT=access<true>(R);
			if(AT==AT_NO_RW||AT==AT_R){
				return true;
			}
		}
	}else if(L->is<Symbol::Array*>()){
		if(R->is<Symbol::Array*>()){
			return checkArraySymbolRedundancy(L->get<Symbol::Array*>(),R->get<Symbol::Array*>()
				,false);
		}else{
			AccessType AT=access<true>(R);
			if(AT==AT_NO_RW||AT==AT_R){
				return true;
			}
		}
	}else if(L->is<Symbol::Record*>()){
		if(R->is<Symbol::Record*>()){
			return checkRecordSymbolRedundancy(L->get<Symbol::Record*>(),R->get<Symbol::Record*>()
				,false);
		}else{
			AccessType AT=access<true>(R);
			if(AT==AT_NO_RW||AT==AT_R){
				return true;
			}
		}
	}
	return false;
}

void RedundantFunctionCallInLoopAnalyzer::insertNewScopeCount(){
	// insert before this scope.
	for(int Index=0;Index<=AnalyzedLevel-1;Index++){
		AnalyzedArray[Index]->insertNewScopeCount();
	}
}

void RedundantFunctionCallInLoopAnalyzer::eraseCurrentScopeCount(){
	for(int Index=0;Index<=AnalyzedLevel-1;Index++){
		AnalyzedArray[Index]->eraseCurrentScopeCount();
	}
}

RedundantFunctionCallInLoopAnalyzer::RedundantFunctionCallInLoopAnalyzer(const clang::ASTContext *AC): VariableAnalyzer(AC){
    CurrentFunctionTriple=FunctionTriples.end();
    IsInCallExpr=false;
	DefaultLoopCount=1;
    initializePredefinedRedundantFunctions();
}

int RedundantFunctionCallInLoopAnalyzer::getCountOfLoopStmt(const clang::ForStmt *FS){
	assert(DefaultLoopCount==1);
	return DefaultLoopCount;
}

ReportSpace::Defect *RedundantFunctionCallInLoopAnalyzer::convertToReport(RedundantFunctionCallInLoopAnalyzer::PatchInfo *Patch){
    // get the file of the patch location.
    const clang::SourceManager &SM=Context->getSourceManager();
    const clang::SourceLocation Loc=Patch->CE->getBeginLoc();
    unsigned int line=SM.getExpansionLineNumber(Loc);
    unsigned int column=SM.getExpansionColumnNumber(Loc);
    const llvm::StringRef FunctionName=Patch->FD->getName();
    std::string FilePathName=Loc.printToString(SM);
    ReportSpace::Defect *D=new ReportSpace::Defect(ReportSpace::RedundantFunctionCallInLoop
            ,ReportSpace::Warning,FilePathName,line,column,FunctionName.str());
    Twine extra="redundant function call "+FunctionName+" in loop statement.";
    // add extra info.
    D->addDesc(extra.str());
    return D;
}
void RedundantFunctionCallInLoopAnalyzer::reportToConsole(RedundantFunctionCallInLoopAnalyzer::PatchInfo *Patch) {
    clang::DiagnosticsEngine &DE=Context->getDiagnostics();
    const unsigned ID=DE.getCustomDiagID(clang::DiagnosticsEngine::Warning
            ,"redundant function call '%0' in loop statement.");
    const clang::DiagnosticBuilder &Builder=DE.Report(Patch->CE->getBeginLoc(),ID);
    Builder.AddString(Patch->FD->getName());
    const auto Range=clang::CharSourceRange::getCharRange(Patch->CE->getSourceRange());
    Builder.AddSourceRange(Range);
}
//---------------------------------------------------
//---------------------------------------------------
//              A set of overridden methods
//---------------------------------------------------
//---------------------------------------------------

void RedundantFunctionCallInLoopAnalyzer::enterForScope(const clang::ForStmt*FS){
    // If LoopStack is empty then we encounters no loop.
    LoopStack.push({FS,AnalyzedLevel});
	LoopFPs.push_back(llvm::DenseMap<Symbol::SymbolType*,unsigned char>());
	insertNewScopeCount();
}
void RedundantFunctionCallInLoopAnalyzer::enterWhileScope(const clang::WhileStmt *WS){
    // If LoopStack is empty then we encounters no loop.
    LoopStack.push({WS,AnalyzedLevel});
	LoopFPs.push_back(llvm::DenseMap<Symbol::SymbolType*,unsigned char>());
	insertNewScopeCount();
}
void RedundantFunctionCallInLoopAnalyzer::enterDoScope(const clang::DoStmt *DS) {
    // If LoopStack is empty then we encounters no loop.
    LoopStack.push({DS,AnalyzedLevel});
	LoopFPs.push_back(llvm::DenseMap<Symbol::SymbolType*,unsigned char>());
	insertNewScopeCount();
}
void RedundantFunctionCallInLoopAnalyzer::enterFunctionScope(const clang::FunctionDecl*FD) {
	if(FD->hasBody()){
        // main function can never be redundant.
        if(!FD->isMain()){
            CurrentFunctionTriple =FunctionTriples.insert({FD->getName(), new FunctionTripleInfo()})
				.first;
			CurrentFunctionTriple->getValue()->RT=RedundantType::Redundant;
			CurrentFunctionTriple->getValue()->FD=FD;
		}else{
			CurrentFunctionTriple =FunctionTriples.end();
		}
    }
}
void RedundantFunctionCallInLoopAnalyzer::finishScope(Footprints *FP) {
	// When this footprints is in function,
    // We must store it and not release it.
    if(AnalyzedLevel==1){
        if(CurrentFunctionTriple==FunctionTriples.end()){
            // Enter the main function, do not do anything!
            return;
        }
        auto Triple=CurrentFunctionTriple->getValue();
		if(HavingAsmStmt==true){
			Triple->RT=RedundantType::USELESS;
			CurrentFunctionTriple=FunctionTriples.end();
			return;
		}
		if(Triple->RT==RedundantType::USELESS){
			CurrentFunctionTriple=FunctionTriples.end();
			return;
		}
        const clang::FunctionDecl *FD=Triple->FD;
        for(auto&Par:FD->parameters()){
            auto &ParFPSymbols=Triple->ArgsFP;
            // XXX:Parameter should have a name?
            const char* ParName=Par->getName().data();
            auto ParSymbol=FP->getSymbol(ParName);
			ParFPSymbols.push_back(copy(ParSymbol));
			// If has parameter change the RedundantType
			Triple->RT=RedundantType::Other;
        }
        CurrentFunctionTriple=FunctionTriples.end();
    }else{
        // When exiting the for scope.
        if(!LoopStack.empty()){
            if(LoopStack.top().second==AnalyzedLevel){
                // Check all the CallExprs in this loop scope.
                checkRedundantFunctionCall();
                LoopStack.pop();
				LoopFPs.pop_back();
				eraseCurrentScopeCount();
            }
        }
    }
}


void RedundantFunctionCallInLoopAnalyzer::enterCallExpr(const clang::CallExpr *CE) {
	// NOTICE: We only consider the normal function call,
    // which means suppressing the function pointer call.
    const clang::Decl *D=CE->getCalleeDecl();
    // Entering a call function makes the top function redundant kind?
    if(D&&llvm::isa<clang::FunctionDecl>(D)){
        const auto *FD=llvm::cast<clang::FunctionDecl>(D);
        // some of the redecls of the function has body.
        // if not, maybe it is a predefined function.
        if(!LoopStack.empty()){
            auto Info=new CallExprInfo();
            Info->CE=CE;
            if(LoopStack.top().second==AnalyzedLevel){
                // add CallExpr to the current loop statement.
                CurrentUnresolvedCallExprs.push_back(Info);
				IsInCallExpr=true;
            }
        }
        if(FD->hasBody()){
            auto Inner=FunctionTriples.find(FD->getName());
            if(CurrentFunctionTriple!=FunctionTriples.end()) {
				if(Inner==FunctionTriples.end()){
					CurrentFunctionTriple->getValue()->RT = RedundantType::USELESS;
				}else if(CE->getNumArgs()==0) {
                    resolvedNestedFunctionRedundancy(Inner->getValue(), CurrentFunctionTriple->getValue());
                }else{
                    CurrentFunctionTriple->getValue()->RT = RedundantType::USELESS;
                }
            }
        }else {
            if (CurrentFunctionTriple != FunctionTriples.end()) {
                CurrentFunctionTriple->getValue()->RT = RedundantType::USELESS;
            }
        }
    }else{
        if (CurrentFunctionTriple != FunctionTriples.end()) {
            CurrentFunctionTriple->getValue()->RT = RedundantType::USELESS;
        }
    }
}

void RedundantFunctionCallInLoopAnalyzer::exitCallExpr(const clang::CallExpr*CE){
    IsInCallExpr=false;
}

int RedundantFunctionCallInLoopAnalyzer::getScopeLevelOfSymbol(Symbol::SymbolType*ST)const{
    assert(ST);
	Symbol::SymbolType *Top=Footprints::getTopSymbol(ST);
    for(int i=0;i<=AnalyzedLevel;i++){
        for(auto&Val:*(AnalyzedArray[i]->getSymbolMap())){
			if(Val.getValue()==Top){
				return i;
			}
		}
    }
	return -1;
}

void RedundantFunctionCallInLoopAnalyzer::visitSymbol(Symbol::SymbolType*ST, bool Read,const clang::Expr*E) {
	int Level=getScopeLevelOfSymbol(ST);
	assert(Level!=-1);
	if(IsInCallExpr&&!LoopStack.empty()){	
        int LoopLevel=LoopStack.top().second;
        // Check the symbol's declaration is in the for scope, otherwise
        // it may not be redundant.
        if(Level<LoopLevel){
            // Symbol is declared out of the loop scope.
            CurrentUnresolvedCallExprs.back()->ArgsFP.emplace_back(ST,IndexOfArg);
        }else{
            // Symbol is declared in this loop scope.
            CurrentUnresolvedCallExprs.back()->ArgsFP.emplace_back(nullptr,IndexOfArg);
        }
    }
	// add to the FunctionTriple and LoopFPs with global symbol
	if(Level==0){
		for(size_t i=0;i<LoopFPs.size();i++){
			auto SymbolResult=LoopFPs[i].find(ST);
			if(SymbolResult==LoopFPs[i].end()){
				if(Read){
					LoopFPs[i].insert({ST,1});
				}else{
					LoopFPs[i].insert({ST,2});
				}
			}else{
				if(!Read&&SymbolResult->getSecond()==1){
					SymbolResult->getSecond()=3;
				}else if(Read&&SymbolResult->getSecond()==2){
					SymbolResult->getSecond()=3;
				}
			}
		}
		if(CurrentFunctionTriple==FunctionTriples.end()){
			// Enter the main function, do not do anything!
			return;
		}
		// record all global symbols!
		auto Triple=CurrentFunctionTriple->getValue();
		auto TripleSymbolResult=Triple->GlobalFP.find(ST);
		if(TripleSymbolResult==Triple->GlobalFP.end()){
			Triple->GlobalFP.insert({ST,Read});
			if(Triple->RT!=RedundantType::USELESS){
				Triple->RT=RedundantType::Other;
			}
		}else{
			if(Read&&TripleSymbolResult->getSecond()==false){
				Triple->RT=RedundantType::USELESS;
			}else if(!Read&&TripleSymbolResult->getSecond()==true){
				Triple->RT=RedundantType::USELESS;
			}else{
				if(Triple->RT!=RedundantType::USELESS){
				Triple->RT=RedundantType::Other;
			}
			}
		}
	}
}


void RedundantFunctionCallInLoopChecker::addSingleChecker(RedundantFunctionCallInLoopAnalyzer *C){
    Checkers.push_back(C);
}

RedundantFunctionCallInLoopChecker::RedundantFunctionCallInLoopChecker(ASTResource *resource, ASTManager *manager,
                                                                       CallGraph *callGraph, Config *configure)
        : BasicChecker(resource, manager, callGraph, configure) {
}


void RedundantFunctionCallInLoopChecker::check(){
	size_t NumOfAnalyzedFiles=0;
	size_t NumOfTotalFiles=resource->getASTFiles().size();
    for(const auto &File:resource->getASTFiles()){
		NumOfAnalyzedFiles++;
        auto *SingleChecker=new RedundantFunctionCallInLoopAnalyzer(&(manager->getASTUnit(File)->getASTContext()));
        readConfig(SingleChecker);
        addSingleChecker(SingleChecker);
        SingleChecker->analyze();
		process_bar((float)NumOfAnalyzedFiles/NumOfTotalFiles);
    }
}

void RedundantFunctionCallInLoopChecker::readConfig(RedundantFunctionCallInLoopAnalyzer *Analyzer){
    std::unordered_map<std::string, std::string> Config =
                configure->getOptionBlock("RedundantFunctionCallInLoopChecker");
    if(Config::IsBlockConfigTrue(Config,"IgnoreCStandardLibrary")){
        Analyzer->setIgnoreCStandardLibrary(true);
    }else{
        Analyzer->setIgnoreCStandardLibrary(false);
    }
	IgnorePaths=configure->parseIgnPaths("RedundantFunctionCallInLoopChecker");
	for(auto& Path:IgnorePaths.getIgnoreLibPaths()){
		Analyzer->addIgnoreDir(Path);
	}
}