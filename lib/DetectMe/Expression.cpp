/*
  Author: Chen Haodong (shinkunyachen@outlook.com)
  -----
  Copyright (c)2020 - present Chen Haodong. All Rights Reserved.
 */

#include "DetectMe/OperationDetection.h"
#include "DetectMe/Expression.h"
#include "iostream"
const clang::Expr *remove_cast(const clang::Expr *expr) {
  if(expr){
    if (expr->getStmtClass() >= clang::Stmt::firstCastExprConstant &&
        expr->getStmtClass() <= clang::Stmt::lastCastExprConstant) {
      return remove_cast(llvm::cast<clang::CastExpr>(expr)->getSubExpr());
    }
  }
  return expr;
}

clang::Expr *remove_cast(clang::Expr *expr) {
  if(expr){
    if (expr->getStmtClass() >= clang::Stmt::firstCastExprConstant &&
        expr->getStmtClass() <= clang::Stmt::lastCastExprConstant) {
      return remove_cast(llvm::cast<clang::CastExpr>(expr)->getSubExpr());
    }
  }
  return expr;
}

void collect_toplevel_expr(const clang::Expr *expr, clang::Stmt::StmtClass type,
                           std::vector<const clang::Expr *> &result) {
  if(expr->getStmtClass() == clang::Stmt::UnaryOperatorClass){
    auto *uOP = llvm::dyn_cast<clang::UnaryOperator>(expr);
    if(uOP->getSubExpr()->getStmtClass() == type)
      result.push_back(uOP->getSubExpr());
  }
  else if (expr->getStmtClass() == type) {
    result.push_back(expr);
    return;
  } else {
    if (expr->getStmtClass() >= clang::Stmt::firstCastExprConstant &&
        expr->getStmtClass() <= clang::Stmt::lastCastExprConstant) {
      collect_toplevel_expr(llvm::cast<clang::CastExpr>(expr)->getSubExpr(),
                            type, result);
    } else {
      switch (expr->getStmtClass()) {
      case clang::Stmt::CompoundAssignOperatorClass:
      case clang::Stmt::BinaryOperatorClass: {
        auto e = llvm::cast<clang::BinaryOperator>(expr);
        collect_toplevel_expr(e->getLHS(), type, result);
        collect_toplevel_expr(e->getRHS(), type, result);
        break;
      }
      case clang::Stmt::CallExprClass: {
        auto e = llvm::cast<clang::CallExpr>(expr);
        for (unsigned int i = 0, sz = e->getNumArgs(); i < sz; ++i)
          collect_toplevel_expr(e->getArg(i), type, result);
        break;
      }
      //adding part
      case clang::Stmt::MemberExprClass:{
         const clang::MemberExpr *memberexpr = llvm::dyn_cast<MemberExpr>(expr);
         result.push_back(remove_cast(memberexpr->getBase()));
         break;
      }
      case clang::Stmt::CXXOperatorCallExprClass:{
        auto e = llvm::dyn_cast<clang::CXXOperatorCallExpr>(expr);
        if(e->isAssignmentOp()){
          for(int i=0;i<e->getNumArgs();i++){
            if(const clang::UnaryOperator *uOP = llvm::dyn_cast<clang::UnaryOperator>(e->getArg(i))){
              result.push_back(remove_cast(uOP->getSubExpr()));
            }
          }
        }
        break;
      }
      //adding part
      // TODO more case
      default:
        break;
      }
    }
  }
}
void collect_toplevel_expr(const clang::Stmt *stmt, clang::Stmt::StmtClass type,
                           std::vector<const clang::Expr *> &result) {
  if(stmt->getStmtClass() == clang::Stmt::CallExprClass){
    const clang::CallExpr *call_expr = llvm::cast<clang::CallExpr>(stmt);
    for (int i = 0; i < call_expr->getNumArgs(); i++) {
        Expr *arg = const_cast<Expr *>(call_expr->getArg(i));
        collect_toplevel_expr(arg,type, result);
    }
  }
  else if (stmt->getStmtClass() >= clang::Stmt::firstExprConstant &&
      stmt->getStmtClass() <= clang::Stmt::lastExprConstant) {
    collect_toplevel_expr(llvm::cast<clang::Expr>(stmt), type, result);
  } else if (stmt->getStmtClass() == clang::Stmt::DeclStmtClass) {
    const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
    const clang::Decl *decl = decl_stmt->getSingleDecl();
    if (decl->getKind() == clang::Decl::Var) {
      const clang::VarDecl *var_decl = llvm::cast<clang::VarDecl>(decl);
      const clang::Expr *init_expr = var_decl->getInit();
      if (init_expr) {
        collect_toplevel_expr(init_expr, type, result);
      }
    }
  }
  // TODO more stmt
}

void collect_expr(const clang::Expr *expr, clang::Stmt::StmtClass type,
                  std::vector<const clang::Expr *> &result) {
  if (expr->getStmtClass() == type) {
    result.push_back(expr);
  }
  if (expr->getStmtClass() >= clang::Stmt::firstCastExprConstant &&
      expr->getStmtClass() <= clang::Stmt::lastCastExprConstant) {
    collect_expr(llvm::cast<clang::CastExpr>(expr)->getSubExpr(), type, result);
  } else {
    switch (expr->getStmtClass()) {
    case clang::Stmt::CompoundAssignOperatorClass:
    case clang::Stmt::BinaryOperatorClass: {
      auto e = llvm::cast<clang::BinaryOperator>(expr);
      collect_expr(e->getLHS(), type, result);
      collect_expr(e->getRHS(), type, result);
      break;
    }
    case clang::Stmt::CallExprClass: {
      auto e = llvm::cast<clang::CallExpr>(expr);
      for (unsigned int i = 0, sz = e->getNumArgs(); i < sz; ++i)
        collect_expr(e->getArg(i), type, result);
      break;
    }
    case clang::Stmt::UnaryOperatorClass: {
      collect_expr(llvm::cast<clang::UnaryOperator>(expr)->getSubExpr(), type,
                   result);
      break;
    }
    case clang::Stmt::MemberExprClass: {
      collect_expr(remove_cast(llvm::cast<clang::MemberExpr>(expr)->getBase()), type,
                   result);
      break;
    }
    // TODO more case
    default:
      break;
    }
  }
}
void collect_expr(const clang::Stmt *stmt, clang::Stmt::StmtClass type,
                  std::vector<const clang::Expr *> &result) {
  if (stmt->getStmtClass() >= clang::Stmt::firstExprConstant &&
      stmt->getStmtClass() <= clang::Stmt::lastExprConstant) {
    collect_expr(llvm::cast<clang::Expr>(stmt), type, result);
  } else if (stmt->getStmtClass() == clang::Stmt::DeclStmtClass) {
    const clang::DeclStmt *decl_stmt = llvm::cast<clang::DeclStmt>(stmt);
    const clang::Decl *decl = decl_stmt->getSingleDecl();
    if (decl->getKind() == clang::Decl::Var) {
      const clang::VarDecl *var_decl = llvm::cast<clang::VarDecl>(decl);
      const clang::Expr *init_expr = var_decl->getInit();
      if (init_expr) {
        collect_expr(init_expr, type, result);
      }
    }
  }
  // TODO more stmt
}

const clang::Expr *extract_simple_value_expr(const clang::Expr *expr) {
  if(expr){
    const clang::Expr *real_expr = remove_cast(expr);
    if(!real_expr)
      return  nullptr;
    switch (real_expr->getStmtClass()) {
    case clang::Stmt::DeclRefExprClass:
      return real_expr;
      break;
    case clang::Stmt::MemberExprClass:
      return real_expr;  
      break;
    //adding part
    case clang::Stmt::CXXConstructExprClass:
    {
      const clang::Expr *const_imexpr = llvm::cast<clang::CXXConstructExpr>(real_expr)->getArg(0);
      //Expr *imexpr = const_cast<Expr *>(const_imexpr);
      if(const_imexpr->getStmtClass() == clang::Stmt::ImplicitCastExprClass)
        return llvm::cast<ImplicitCastExpr>(const_imexpr)->getSubExpr();
      return const_imexpr;
      break;
    }
    case clang::Stmt::UnaryOperatorClass:
    {
      if(const UnaryOperator *uOP = llvm::dyn_cast<clang::UnaryOperator>(real_expr))
        return extract_simple_value_expr(uOP->getSubExpr());
      break;
    }
    case clang::Stmt::ParenExprClass:
    {
      if(const ParenExpr *paren_expr = llvm::dyn_cast<clang::ParenExpr>(real_expr))
        return extract_simple_value_expr(paren_expr->getSubExpr());
    }
    //adding part
      
    default:
      return nullptr;
    }
  }
  return nullptr;
}