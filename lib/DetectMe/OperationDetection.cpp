/*
  Author: Chen Haodong (shinkunyachen@outlook.com)
  -----
  Copyright (c)2020 - present Chen Haodong. All Rights Reserved.
 */

#include "DetectMe/OperationDetection.h"
#include "DetectMe/Expression.h"
#include<string>
// helper functions

std::string getStringOfExpr(const Expr *expr)
{
    LangOptions L0;
    L0.CPlusPlus = 1;
    std::string buffer1;
    llvm::raw_string_ostream strout1(buffer1);
    expr->printPretty(strout1, nullptr, PrintingPolicy(L0));
    return strout1.str();
}
static const clang::Expr *
find_equal_expr(ClangExprValueHolder *vh,
                const std::vector<const clang::Expr *> &set) {
  for (auto i : set) {
    /* i->dump(llvm::errs());
    vh->expr()->dump(llvm::errs()); */
    if (vh->is_equal(i))
      return i;
  }
  return nullptr;
}
// TODO support pointer analysis
// TODO support array e.g. AStruct sts[10]; int y = sts[5].arr[3];
static bool is_equal_impl(const clang::DeclRefExpr *lhs,
                          const clang::DeclRefExpr *rhs) {
  return lhs->getDecl() == rhs->getDecl();
}
static bool is_equal_impl(const clang::DeclRefExpr *expr,
                          const clang::ValueDecl *decl) {
  return expr->getDecl() == decl;
}

static bool is_equal_impl(const clang::MemberExpr *lhs,
                          const clang::MemberExpr *rhs) {
  if (lhs->getMemberDecl() == rhs->getMemberDecl()) {
    const clang::Expr *lhs_base = remove_cast(lhs->getBase());
    const clang::Expr *rhs_base = remove_cast(rhs->getBase());
    if (lhs_base->getStmtClass() == rhs_base->getStmtClass()) {
      switch (lhs_base->getStmtClass()) {
      case clang::Stmt::DeclRefExprClass:
        return is_equal_impl(llvm::cast<clang::DeclRefExpr>(lhs_base),
                             llvm::cast<clang::DeclRefExpr>(rhs_base));
      case clang::Stmt::MemberExprClass:
        return is_equal_impl(llvm::cast<clang::MemberExpr>(lhs_base),
                             llvm::cast<clang::MemberExpr>(rhs_base));
      default:
        break;
      }
    }
  }
  return false;
}
// value holder
bool ClangDeclRefExprValueHolder::is_equal(const clang::Expr *expr) const {
  if (expr->getStmtClass() == clang::Stmt::DeclRefExprClass)
    return is_equal_impl(_expr, llvm::cast<clang::DeclRefExpr>(expr));
  return false;
}
bool ClangDeclRefExprValueHolder::is_equal(const clang::ValueDecl *decl) const {
  return is_equal_impl(_expr, decl);
}
bool ClangMemberExprValueHolder::is_equal(const clang::Expr *expr) const {
  if (expr->getStmtClass() == clang::Stmt::MemberExprClass)
    return is_equal_impl(_expr, llvm::cast<clang::MemberExpr>(expr));
  return false;
}

//adding part
clang::Expr *
find_equal_expr(Expr *vh,
                const std::vector<const clang::Expr *> &set) {
  for (auto i : set) {
    if(DeclRefExpr *vh_expr = llvm::dyn_cast<DeclRefExpr>(vh)){
      if(DeclRefExpr *i_expr = const_cast<DeclRefExpr *>(llvm::dyn_cast<DeclRefExpr>(i))){
       if (vh_expr->getDecl() == i_expr->getDecl())
      return vh;
    }
    }
  }
  return nullptr;
}
//adding part
// operation
bool BranchOperation::matched(const clang::CFGBlock *blk,
                              const clang::Stmt *stmt, ValueHolder *v) {
  const clang::Stmt *terminator = blk->getTerminatorStmt();
  if (!terminator)
    return false;
  switch (terminator->getStmtClass()) {
  case clang::Stmt::IfStmtClass:
    if (llvm::cast<clang::IfStmt>(terminator)->getCond() == stmt)
      return true;
    break;
  default:
    break;
  }
  return false;
}
// BUG Now only support the simple expression, it means we can't match the
// expression that is a sub-expression of other expressions for example, a =
// func(b), this is an BinaryOperator, not a CallExpr, if we want to match the
// CallExpr, we need to examine all the sub-expressions of it.
bool CallOperation::matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                            ValueHolder *v) {
  if (stmt->getStmtClass() == clang::Stmt::CallExprClass) {
    const clang::CallExpr *call_expr = llvm::cast<clang::CallExpr>(stmt);
    const clang::FunctionDecl *func_decl = call_expr->getDirectCallee();
    if (!func_decl)
      return false;
    if (func_decl->getQualifiedNameAsString() != _function_name)
      return false;
    if (v->is_expr_value_holder()) {
      ClangExprValueHolder *expr_val_holder =
          static_cast<ClangExprValueHolder *>(v);
      std::vector<const clang::Expr *> exprs;
      const clang::Expr *vh_expr = expr_val_holder->expr();
      if (_arg_idx < 0)
        collect_toplevel_expr(call_expr, vh_expr->getStmtClass(), exprs);
      else {
        collect_toplevel_expr(call_expr->getArg(_arg_idx),
                              vh_expr->getStmtClass(), exprs);
      }
      return find_equal_expr(expr_val_holder, exprs) != nullptr;
    }
  }
  return false;
}
bool AllMemberAssignOperation::matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v){
    if(stmt->getStmtClass() == clang::Stmt::BinaryOperatorClass){
      const clang::BinaryOperator *expr = llvm::cast<clang::BinaryOperator>(stmt);
      if(expr->getOpcode() == clang::BinaryOperatorKind::BO_Assign){
        const clang::Expr *lhs = extract_simple_value_expr(expr->getLHS());
        const clang::Expr *rhs = extract_simple_value_expr(expr->getRHS());
        if(MemberExpr *lhs_memberexpr = llvm::dyn_cast<MemberExpr>(expr->getLHS())){
          if(static_cast<ClangExprValueHolder *>(v)->is_equal(remove_cast(lhs_memberexpr->getBase())) && (!rhs || rhs->getStmtClass() != clang::Stmt::MemberExprClass)){
            ValueDecl *value_decl = lhs_memberexpr->getMemberDecl();
            if(FieldDecl *lhs_field_decl = llvm::dyn_cast<FieldDecl>(value_decl)){
              if(lhs_field_decl == field_decl)
                  return true;
            }
          }
        }
      }
    }
    return false; 

}
bool AssignOperation::matched(const clang::CFGBlock *blk,
                                  const clang::Stmt *stmt, ValueHolder *v) {//找到左值为value的赋值语句
    if(stmt->getStmtClass() == clang::Stmt::BinaryOperatorClass){
      const clang::BinaryOperator *expr = llvm::cast<clang::BinaryOperator>(stmt);
      if(expr->getOpcode() == clang::BinaryOperatorKind::BO_Assign){
        const clang::Expr *lhs = extract_simple_value_expr(expr->getLHS());
        const clang::Expr *rhs = extract_simple_value_expr(expr->getRHS());
        if((rhs && rhs->getStmtClass() != clang::Stmt::MemberExprClass) || rhs == nullptr){
          // if(lhs->getStmtClass() == clang::Stmt::UnaryOperatorClass)
            // const clang::Expr *lhs_simple = extract_simple_value_expr(lhs);
          if(lhs &&static_cast<ClangExprValueHolder *>(v)->is_equal(lhs))
            return true;
          else if(lhs && static_cast<ClangMemberExprValueHolder *>(v)->is_equal(lhs))
            return true;
        }
      }
    }
    else if (stmt->getStmtClass() == clang::Stmt::CallExprClass) {//找到参数为成员函数引用的函数func(&a->val);
      const clang::CallExpr *call_expr = llvm::cast<clang::CallExpr>(stmt);
      const clang::FunctionDecl *func_decl = call_expr->getDirectCallee();
      if (!func_decl)
        return false;
      if (v->is_expr_value_holder()) {
        ClangExprValueHolder *expr_val_holder =
            static_cast<ClangExprValueHolder *>(v);
        std::vector<const clang::Expr *> exprs;
        const clang::Expr *vh_expr = expr_val_holder->expr();
        for (int i = 0; i < call_expr->getNumArgs(); i++) {
          Expr *arg = const_cast<Expr *>(call_expr->getArg(i));
          if(arg->getStmtClass() == clang::Stmt::UnaryOperatorClass){
            collect_toplevel_expr(arg,vh_expr->getStmtClass(), exprs);
          }
        }
        return find_equal_expr(expr_val_holder, exprs) != nullptr;
      }
  }
    return false;              

}
bool StructInitOperation::matched(const clang::CFGBlock *blk,
                              const clang::Stmt *stmt, ValueHolder *v) {
                                // stmt->dump();
  if (stmt->getStmtClass() == clang::Stmt::CallExprClass) {//func(&a);
    const clang::CallExpr *call_expr = llvm::cast<clang::CallExpr>(stmt);
    const clang::FunctionDecl *func_decl = call_expr->getDirectCallee();
    if (!func_decl)
      return false;
    if (v->is_expr_value_holder()) {
      ClangExprValueHolder *expr_val_holder =
          static_cast<ClangExprValueHolder *>(v);
      std::vector<const clang::Expr *> exprs;
      const clang::Expr *vh_expr = expr_val_holder->expr();
      for (int i = 0; i < call_expr->getNumArgs(); i++) {
        Expr *arg = const_cast<Expr *>(call_expr->getArg(i));
        if(arg->getStmtClass() == clang::Stmt::UnaryOperatorClass){
          collect_toplevel_expr(arg,vh_expr->getStmtClass(), exprs);
        }
      }
      return find_equal_expr(expr_val_holder, exprs) != nullptr;
    }
  }                           
  else if(stmt->getStmtClass() == clang::Stmt::DeclStmtClass){//A p = ;
    const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
    const clang::Decl *decl = decl_stmt->getSingleDecl();
    if (decl->getKind() == clang::Decl::Var) {
      const clang::VarDecl *var_decl = llvm::cast<clang::VarDecl>(decl);
      const clang::Expr *init_expr = var_decl->getInit();
      
      if (v->type() == ValueHolder::ClangDeclRefExpr) {
            if (static_cast<ClangDeclRefExprValueHolder *>(v)->is_equal(
                    llvm::cast<clang::ValueDecl>(decl))){
                      // remove_cast(init_expr)->dump();
            if(remove_cast(init_expr) && remove_cast(init_expr)->getStmtClass() != clang::Stmt::CXXConstructExprClass)
            {
              return true;      
            }
         }
       }
     }
  }
  else if(stmt->getStmtClass() == clang::Stmt::CXXOperatorCallExprClass){//*p = ;
    auto cxxop_expr = llvm::dyn_cast<clang::CXXOperatorCallExpr>(stmt);
      if(cxxop_expr->isAssignmentOp()){
        if(static_cast<ClangDeclRefExprValueHolder *>(v)->is_equal(remove_cast(extract_simple_value_expr(cxxop_expr->getArg(0)))))
          return true;
      }
  }
  else if(stmt->getStmtClass() == clang::Stmt::BinaryOperatorClass){//p=fun();
    const clang::BinaryOperator *expr = llvm::cast<clang::BinaryOperator>(stmt);
    if (expr->getOpcode() == clang::BinaryOperatorKind::BO_Assign) {
        const clang::Expr *lhs = extract_simple_value_expr(expr->getLHS());
        const clang::Expr *rhs = extract_simple_value_expr(expr->getRHS());
        if(lhs &&static_cast<ClangExprValueHolder *>(v)->is_equal(lhs) && (rhs && rhs->getStmtClass() != clang::Stmt::DeclRefExprClass) 
        || (lhs &&static_cast<ClangExprValueHolder *>(v)->is_equal(lhs) && rhs == nullptr))//not p=q;
          return true;
    }
  }
  return false;
}
bool ZeroAssignOperation::matched(const clang::CFGBlock *blk,
                                  const clang::Stmt *stmt, ValueHolder *v) {
  if (stmt->getStmtClass() == clang::Stmt::BinaryOperatorClass) {
    const clang::BinaryOperator *expr = llvm::cast<clang::BinaryOperator>(stmt);
    if (expr->getOpcode() == clang::BinaryOperatorKind::BO_Assign) {
      const clang::Expr *rhs_expr = expr->getRHS();
      if (rhs_expr->getStmtClass() == clang::Stmt::IntegerLiteralClass) {
        const clang::IntegerLiteral *integer =
            llvm::cast<clang::IntegerLiteral>(rhs_expr);
        if (integer->getValue().isNullValue())
          return true;
      }
    }
  } else if (stmt->getStmtClass() == clang::Stmt::DeclStmtClass) {
    const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
    const clang::Decl *decl = decl_stmt->getSingleDecl();
    if (decl->getKind() == clang::Decl::Var) {
      const clang::VarDecl *var_decl = llvm::cast<clang::VarDecl>(decl);
      const clang::Expr *init_expr = var_decl->getInit();
      if (init_expr) {
        if (init_expr->getStmtClass() == clang::Stmt::IntegerLiteralClass) {
          const clang::IntegerLiteral *integer =
              llvm::cast<clang::IntegerLiteral>(init_expr);
          if (integer->getValue().isNullValue())
            return true;
        }
      }
    }
  }
  return false;
}
bool ExplicitCastOperation::matched(const clang::CFGBlock *blk,
                                    const clang::Stmt *stmt, ValueHolder *v) {
  if (stmt->getStmtClass() == clang::Stmt::BinaryOperatorClass) {
    const clang::BinaryOperator *expr = llvm::cast<clang::BinaryOperator>(stmt);
    if (expr->getOpcode() == clang::BinaryOperatorKind::BO_Assign) {
      const clang::Expr *rhs_expr = expr->getRHS();
      if (rhs_expr->getStmtClass() >=
              clang::Stmt::firstExplicitCastExprConstant &&
          rhs_expr->getStmtClass() <=
              clang::Stmt::lastExplicitCastExprConstant) {
        if (v->is_expr_value_holder()) {
          const clang::Expr *lhs_expr =
              extract_simple_value_expr(expr->getLHS());
          if (lhs_expr) {
            return static_cast<ClangExprValueHolder *>(v)->is_equal(lhs_expr);
          }
        }
      }
    }
  } else if (stmt->getStmtClass() == clang::Stmt::DeclStmtClass) {
    const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
    const clang::Decl *decl = decl_stmt->getSingleDecl();
    if (decl->getKind() == clang::Decl::Var) {
      const clang::VarDecl *var_decl = llvm::cast<clang::VarDecl>(decl);
      const clang::Expr *init_expr = var_decl->getInit();
      if (init_expr) {
        if (init_expr->getStmtClass() >=
                clang::Stmt::firstExplicitCastExprConstant &&
            init_expr->getStmtClass() <=
                clang::Stmt::lastExplicitCastExprConstant) {
          if (v->type() == ValueHolder::ClangDeclRefExpr) {
            if (static_cast<ClangDeclRefExprValueHolder *>(v)->is_equal(
                    llvm::cast<clang::ValueDecl>(decl_stmt->getSingleDecl())))
              return true;
          }
        }
      }
    }
  }
  return false;
}

bool Traverse::VisitExpr(const clang::Expr *_expr)
  {
    if(expr_val_holder && expr_val_holder->expr()){
      if(const clang::Expr *value_holder = llvm::dyn_cast<Expr>(expr_val_holder->expr())){
        // _expr->dump();
        // value_holder->dump();
        if (expr_val_holder->is_equal(_expr)){
          has_value_holder = true;
          exprs.push_back(_expr);
        }
      }
    }
    return true;
  }
// main analysis framework
bool OperationDetection::detect(DetectionKind kind) {
  DetectionContext ctx(_ast_manager, _function, 1, kind);
  // check argument
  if (!_operation || !_use_point || !_use_point->_value_holder) {
    throw std::invalid_argument("No operation or use point or value holder");
  }
  return detect_impl(_use_point->_block, ++_use_point->_it,
                     _use_point->_value_holder.get(), ctx);
}
bool OperationDetection::detect_impl(const clang::CFGBlock *blk,
                                     clang::CFGBlock::const_reverse_iterator it,
                                     ValueHolder *val, DetectionContext &ctx) {
  // dfs
  ctx.color_map[blk->getBlockID()] = DetectionContext::Gray;//delete
  auto end = blk->rend();
  bool halt;
  while (it != end) {
    if (it->getKind() == clang::CFGElement::Statement) {
      CFGStmt s = it->castAs<CFGStmt>();
      // s.getStmt()->dump();
      // if we detected it, stop this branch, or we has to stop because of we
      // find an untrackable assignment
      if (detect_stmt(blk, s.getStmt(), val, ctx, halt))
        return true;
      else if (halt)
        return false;
    }
    ++it;
  }
  // blk->dump();
  // auto empty = blk->empty();
  // blk->getTerminatorStmt()->dump();
  // auto b = blk->pred_size();
   if (blk->pred_size() == 0 ||((blk->empty() && blk->getTerminatorStmt() == NULL && blk->pred_size() == 1))) {
    // we are in ENTRY block!
    // can we continue the analysis?
    // first, we need to check whether the tracking is too deep
    if (ctx.call_depth >= _max_call_depth)
      return false;
    // then, we need to check whether the value holder is now a parameter
    int param_idx;
    clang::FunctionDecl *func_decl;
    const MemberExpr * memexpr;
    if (val->type() == ValueHolder::ClangDeclRefExpr)
    {
      ClangDeclRefExprValueHolder *decl_value_holder =
          static_cast<ClangDeclRefExprValueHolder *>(val);
      const clang::ValueDecl *value_decl =
          llvm::cast<clang::DeclRefExpr>(decl_value_holder->expr())->getDecl();
      if (value_decl->getKind() != clang::Decl::ParmVar){
        if (value_decl->getKind() == clang::Decl::Var) {
          const clang::VarDecl *var_decl = llvm::cast<clang::VarDecl>(value_decl);
          const clang::Expr *init_expr = var_decl->getInit();
          if(init_expr && init_expr->getStmtClass() == clang::Stmt::InitListExprClass){
            return true;      
          }
        }
          return false;
      }
      func_decl =
          _ast_manager->getFunctionDecl(ctx.current_function);
      param_idx = -1;
      for (int i = 0, sz = func_decl->param_size(); i < sz; ++i) {
        if (func_decl->getParamDecl(i) ==
            llvm::cast<clang::ParmVarDecl>(value_decl)) {
          param_idx = i;
          break;
        }
      }
    }
    //adding part
    else if(val->type() == ValueHolder::ClangMemberExpr)
    {
      ClangMemberExprValueHolder *memexprval =
        static_cast<ClangMemberExprValueHolder *>(val);
      memexpr = llvm::cast<MemberExpr>(memexprval->expr());
      MemberExpr *meme = const_cast<MemberExpr *>(memexpr);
      func_decl =
          _ast_manager->getFunctionDecl(ctx.current_function);
      param_idx = -1;
      if(clang::DeclRefExpr *declrefexpr = llvm::dyn_cast<clang::DeclRefExpr>(remove_cast(meme->getBase()))){
        const clang::ValueDecl *value_decl =declrefexpr->getDecl();
        for (int i = 0, sz = func_decl->param_size(); i < sz; ++i) {
          if(value_decl->getKind() == clang::Decl::ParmVar)
          {
            if (func_decl->getParamDecl(i) ==
                llvm::cast<clang::ParmVarDecl>(value_decl)){
                param_idx = i;
              break;
            }
          }
        }
      }
    }
    //adding part
    else
      return false;
    if (param_idx < 0)
      return false;
    // oh, it's a parameter, now we have to find all the call site of this
    // function, why it cannot be non-parameter?
    std::vector<std::pair<ASTFunction *, int64_t>> callers =
        _call_graph->getParentsWithCallsite(ctx.current_function);
    auto p = ctx.current_function->getFunctionName();
    if (callers.size() == 0)
      return false;
    std::vector<const clang::Expr *> call_exprs;
    for (auto caller : callers) {
      // find call site
      if(_ast_manager->getFunctionDecl(caller.first)->hasBody()){
        FunctionDecl *tmp_fd=_ast_manager->getFunctionDecl(caller.first)->getDefinition();
        caller.first=_ast_manager->getASTFunction(tmp_fd);
      }
      Stmt *stmts = common::getStmtInFunctionWithID(_ast_manager->getFunctionDecl(caller.first),caller.second);
      // stmts->dump();
      
      /*A调用B两次，会检查两处参数expr的初始化情况*/
      clang::CFG *cfg = _ast_manager->getCFG(caller.first).get();
      for (auto caller_blk_it = cfg->begin(), caller_blk_end = cfg->end();//traverse caller stmt
           caller_blk_it != caller_blk_end; ++caller_blk_it) {
        clang::CFGBlock *caller_block = *caller_blk_it;
        for (auto caller_elm_it = caller_block->rbegin(),
                  caller_elm_end = caller_block->rend();
             caller_elm_it != caller_elm_end; ++caller_elm_it) {
          if (caller_elm_it->getKind() == clang::CFGElement::Statement) {
            CFGStmt caller_stmt = caller_elm_it->castAs<CFGStmt>();
            if(caller_stmt.getStmt() != stmts)
              continue;
            call_exprs.clear();
            collect_expr(stmts, clang::Stmt::CallExprClass,
                         call_exprs);
            for (auto call_expr : call_exprs) {
              // call_expr->dump();
              const clang::CallExpr *clang_call_expr =
                  llvm::cast<clang::CallExpr>(call_expr);
             if (call_exprs.size() == 1 || func_decl == clang_call_expr->getDirectCallee()) {
                // we find a call site
                /*adding part*/
                /*A调用B两次，会检查两处参数expr的初始化情况*/
                const clang::Expr *simple_expr = extract_simple_value_expr(clang_call_expr->getArg(param_idx));
                ValueHolder *arg_v;
                if(val->type() == ValueHolder::ClangMemberExpr)
                {
                  clang::MemberExpr *memexpr_cast = const_cast<MemberExpr *>(memexpr);
                  // memexpr_cast->dump();
                  // simple_expr->dump();
                  if(simple_expr && simple_expr->getStmtClass() == clang::Stmt::DeclRefExprClass){
                    memexpr_cast->setBase(const_cast<Expr *>(simple_expr));
                  arg_v = dispatch_new_value_holder(memexpr_cast);
                  }
                  else
                    return false;
                }/*adding part*/
                else
                  arg_v = dispatch_new_value_holder(simple_expr);
                if (!arg_v)
                  return false; // untrackable call
                // construct new context
                DetectionContext new_ctx(_ast_manager, caller.first,
                                         ctx.call_depth + 1, ctx.kind);
                new_ctx.add_value_holder(arg_v);
                auto caller_elm_it_tmp = caller_elm_it;
                bool caller_detected = detect_impl(
                    caller_block, ++caller_elm_it_tmp, arg_v, new_ctx);
                 if (caller_detected)
                  continue;
                else
                  return false;
              }
            }
          }
        }
      }
    }
    if (ctx.kind == DetectionKind::Exists)
      return false;
    else
      return true;
   } else {
   bool all_detected=false;//有一条路径没detected到就是false
   for (auto blk_it = blk->pred_begin(), blk_end = blk->pred_end();
         blk_it != blk_end; ++blk_it) {
      all_detected=true;
      const clang::CFGBlock *pred = *blk_it;
      if(pred != nullptr){
        if(pred->empty() &&blk->pred_size() != 1){//是空的会导致下面detect到返回false，除非下一个就是entryblock
          continue;
        }
        if (ctx.color_map[pred->getBlockID()] == DetectionContext::White) {
          bool detected = detect_impl(pred, pred->rbegin(), val, ctx);
          // short-circuit evaluation
          if (ctx.kind == DetectionKind::Exists && detected)
            return true;
          else if (ctx.kind == DetectionKind::Forall && !detected)
            return false;
        }
      }
    }
    if(all_detected)
      return true;//每一条路径都detect到了，如果有一条路径没有detect到，就看参数有没有
    else
      return false;
  }
}
// TODO assignment may not be a binaryoperator, it may in declstmt
bool OperationDetection::detect_stmt(const clang::CFGBlock *blk,
                                     const clang::Stmt *stmt, ValueHolder *&val,
                                     DetectionContext &ctx, bool &halt) {
  halt = false;
  if (val->is_expr_value_holder()) {
    ClangExprValueHolder *expr_val_holder =
        static_cast<ClangExprValueHolder *>(val);
      std::vector<const clang::Expr *> exprs;
    const clang::Expr *vh_expr = expr_val_holder->expr();
    bool has_value_holder = false;
    //  collect_toplevel_expr(stmt, vh_expr->getStmtClass(), exprs);

    // if (find_equal_expr(expr_val_holder, exprs)) {
      // has_value_holder = true;
    // }//deleted part
    
    Traverse *traverse = new Traverse(expr_val_holder);
    traverse->TraverseStmt(const_cast<clang::Stmt *>(stmt));
    exprs = traverse->exprs;
    if(traverse->has_value_holder)
      has_value_holder = true;
    if (stmt->getStmtClass() == clang::Stmt::DeclStmtClass &&
        val->type() == ValueHolder::ClangDeclRefExpr) {
      const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
      if (val->type() == ValueHolder::ClangDeclRefExpr) {
        if (static_cast<ClangDeclRefExprValueHolder *>(val)->is_equal(
                llvm::cast<clang::ValueDecl>(decl_stmt->getSingleDecl())))
          has_value_holder = true;
      }
    }
    if (!has_value_holder && (val->type() != ValueHolder::ClangMemberExpr))//adding the latter condition
      return false;
    // a statement has something interesting(contains a value we need to track)
    // can it match our operation?
    bool matched = _operation->matched(blk, stmt, val);
    /*
    if (matched) {
      halt = true;
      return true;
    }
  }
  return false;
}*/
    if (matched) {
      halt = true;
      return true;
    }
    // stmt->dump();
    if (stmt->getStmtClass() == clang::Stmt::BinaryOperatorClass) {
      const clang::BinaryOperator *expr =
          llvm::cast<clang::BinaryOperator>(stmt);
      if (expr->getOpcode() == clang::BinaryOperatorKind::BO_Assign) {
        const clang::Expr *lhs_expr = extract_simple_value_expr(expr->getLHS());
        const clang::Expr *rhs_expr = extract_simple_value_expr(expr->getRHS());
        if(rhs_expr && lhs_expr && lhs_expr->getType()->isPointerType()){
        // lhs_expr->getType()->dump();
          clang::CFGBlock *detect_block;
          clang::CFGBlock::reverse_iterator detect_it;
          if(static_cast<ClangMemberExprValueHolder *>(val)->is_equal(lhs_expr)){//dangqianhanshu?
            clang::CFG *func_cfg = _ast_manager->getCFG(ctx.current_function).get();
            for(clang::CFG::reverse_iterator func_it = func_cfg->rbegin();func_it!=func_cfg->rend();func_it++){
              clang::CFGBlock *func_end_block = *func_it;
              for(clang::CFGBlock::reverse_iterator it = func_end_block->rbegin();it!=func_end_block->rend();it++){
                if (it->getKind() == clang::CFGElement::Statement) {
                // CFGStmt func_stmt = it->castAs<CFGStmt>();
                // func_stmt.getStmt()->dump();
                detect_block = func_end_block;
                detect_it = it;
                }
                else
                  continue;
              }
            }
            // DetectionContext new_ctx(_ast_manager, ctx.current_function,0,ctx.kind);
            ValueHolder *alias_v;
            alias_v = dispatch_new_value_holder(rhs_expr);
            bool alias_detected = detect_impl(detect_block,detect_it,alias_v,ctx);
            if(alias_detected)
              return true;
          }
        }
        if (lhs_expr) {
          exprs.clear();
          bool has_find_equal = false;
          if(val->type() == ValueHolder::ClangMemberExpr && lhs_expr->getStmtClass() == clang::Stmt::DeclRefExprClass){//p=q;使用点为p->val,将p->val传递给q->val
            const clang::MemberExpr *member_tmp = llvm::dyn_cast<MemberExpr>(vh_expr);
            collect_toplevel_expr(lhs_expr, remove_cast(member_tmp->getBase())->getStmtClass(), exprs);
            if(find_equal_expr(remove_cast(member_tmp->getBase()), exprs))
              has_find_equal=true;
          }
          collect_toplevel_expr(lhs_expr, vh_expr->getStmtClass(), exprs);
          if (find_equal_expr(expr_val_holder, exprs) || has_find_equal) {
            ValueHolder *v = dispatch_new_value_holder(expr->getRHS());
            if (!v)
            {//adding part
                if(vh_expr->getType()->isStructureType())
                  halt = false; 
                else 
              //adding part
                  halt = true;// halt, it's an untrackable assignment
            }
            else {
              if(val->type() == ValueHolder::ClangMemberExpr && v->type() == ValueHolder::ClangDeclRefExpr){
                MemberExpr *member_tmp = const_cast<MemberExpr *>(llvm::dyn_cast<MemberExpr>(vh_expr));
                member_tmp->setBase(remove_cast(expr->getRHS()));
                // member_tmp->dump();
                ValueHolder *v = dispatch_new_value_holder(member_tmp);
                val = v;
                ctx.add_value_holder(v);
              }
              else{
                val = v;
                ctx.add_value_holder(v);
              }
            }
          }
        }
      }
    }
    else if (stmt->getStmtClass() == clang::Stmt::DeclStmtClass) {
        const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
        if (val->type() == ValueHolder::ClangDeclRefExpr) {
          if (static_cast<ClangDeclRefExprValueHolder *>(val)->is_equal(
                  llvm::cast<clang::ValueDecl>(decl_stmt->getSingleDecl())))
            halt = true;
        }
      }
    }
  return false;
}
ValueHolder *
OperationDetection::dispatch_new_value_holder(const clang::Expr *expr) {
  if(expr){
    const clang::Expr *simple_expr = extract_simple_value_expr(expr);
    ValueHolder *val = nullptr;
    if (simple_expr) {
      // we allow duplicate value holder for the same expr
      switch (simple_expr->getStmtClass()) {
      case clang::Stmt::DeclRefExprClass:
        val = new ClangDeclRefExprValueHolder(
            llvm::cast<clang::DeclRefExpr>(simple_expr));
        break;
      case clang::Stmt::MemberExprClass:
        val = new ClangMemberExprValueHolder(
            llvm::cast<clang::MemberExpr>(simple_expr));
        break;
      default:
        break;
      }
    }
    return val;
  }
  return nullptr;
}