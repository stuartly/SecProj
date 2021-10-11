/*
  Author: Chen Haodong (shinkunyachen@outlook.com)
  -----
  Copyright (c)2020 - present Chen Haodong. All Rights Reserved.
 */

#ifndef UTILITY_EXPRESSION_H
#define UTILITY_EXPRESSION_H

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <vector>
// this function will remove the cast operation and return its core expression
const clang::Expr *remove_cast(const clang::Expr *expr);
clang::Expr *remove_cast(clang::Expr *expr);
// this function will collect all the 'top level' expressions with required
// type, 'top level' means once it found a required expression, it WON'T check
// the expression's sub expression
void collect_toplevel_expr(const clang::Expr *expr, clang::Stmt::StmtClass type,
                           std::vector<const clang::Expr *> &result);
void collect_toplevel_expr(const clang::Stmt *stmt, clang::Stmt::StmtClass type,
                           std::vector<const clang::Expr *> &result);
// this function will collect all the expressions with required type
void collect_expr(const clang::Expr *expr, clang::Stmt::StmtClass type,
                  std::vector<const clang::Expr *> &result);
void collect_expr(const clang::Stmt *stmt, clang::Stmt::StmtClass type,
                  std::vector<const clang::Expr *> &result);
// this function will try to extract a simple value holder needed expression
// from expr, if expr is complex(such as binary operation, some arithmetic unary
// operation), this function will return nullptr
const clang::Expr *extract_simple_value_expr(const clang::Expr *expr);
#endif