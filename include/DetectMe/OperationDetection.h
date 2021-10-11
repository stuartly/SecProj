/*
  Author: Chen Haodong (shinkunyachen@outlook.com)
  -----
  Copyright (c)2020 - present Chen Haodong. All Rights Reserved.
 */

#ifndef OPERATION_DETECTION_H
#define OPERATION_DETECTION_H

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
// #include <clang/Analysis/CFG.h>
#include "CFG/SACFG.h"
#include "framework/ASTManager.h"
#include "framework/CallGraph.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class ValueHolder {
  friend class OperationDetection;

public:
  enum Type { ClangDeclRefExpr, ClangMemberExpr, ClangArraySubscriptExpr };

public:
  ValueHolder(Type t) : _type(t) {}
  virtual ~ValueHolder() = default;
  Type type() const { return _type; }
  bool is_expr_value_holder() {
    return _type >= ClangDeclRefExpr && _type <= ClangArraySubscriptExpr;
  }

protected:
  Type _type;
};
class ClangExprValueHolder : public ValueHolder {
public:
  ClangExprValueHolder(Type t) : ValueHolder(t) {}
  virtual const clang::Expr *expr() = 0;
  virtual bool is_equal(const clang::Expr *expr) const = 0;
};
class ClangDeclRefExprValueHolder : public ClangExprValueHolder {
public:
  ClangDeclRefExprValueHolder(const clang::DeclRefExpr *expr)
      : ClangExprValueHolder(ValueHolder::ClangDeclRefExpr), _expr(expr) {}

  virtual const clang::Expr *expr() override { return _expr; }
  virtual bool is_equal(const clang::Expr *expr) const override;
  bool is_equal(const clang::ValueDecl *decl) const;

protected:
  const clang::DeclRefExpr *_expr;
};
class ClangMemberExprValueHolder : public ClangExprValueHolder {
public:
  ClangMemberExprValueHolder(const clang::MemberExpr *expr)
      : ClangExprValueHolder(ValueHolder::ClangMemberExpr), _expr(expr) {}

  virtual const clang::Expr *expr() override { return _expr; }
  virtual bool is_equal(const clang::Expr *expr) const override;

protected:
  const clang::MemberExpr *_expr;
};
// TODO class ClangArraySubscriptExprValueHolder it's granularity is so small,
// useless now, maybe can use in propagating
class UsePoint {
  friend class OperationDetection;

public:
  UsePoint(const clang::CFGBlock *blk,
           clang::CFGBlock::const_reverse_iterator it)
      : _block(blk), _it(it) {}
  template <typename _T, typename... _Args>
  void set_value_holder(_Args &&... args) {
    _value_holder = std::make_unique<_T>(std::forward<_Args>(args)...);
  }

protected:
  const clang::CFGBlock *_block;
  clang::CFGBlock::const_reverse_iterator _it;
  std::unique_ptr<ValueHolder> _value_holder;
};

class Operation {
  friend class OperationDetection;

public:
  virtual ~Operation() {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) = 0;
};
class BranchOperation : public Operation {
public:
  BranchOperation() {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;
};
class CallOperation : public Operation {
public:
  // if arg_idx < 0, we will check whether the value is used as
  // argument(position doesn't matter)
  CallOperation(std::string func, int arg_idx)
      : _function_name(func), _arg_idx(arg_idx) {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;

protected:
  std::string _function_name;
  int _arg_idx;
};
class AssignOperation : public Operation {
public:
  AssignOperation() {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;
};
class AllMemberAssignOperation : public Operation {
public:
  AllMemberAssignOperation(clang::FieldDecl *field):field_decl(field) {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;
protected:
  clang::FieldDecl * field_decl;
};
class StructInitOperation : public Operation {
public:
  StructInitOperation() {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;
};

class ZeroAssignOperation : public Operation {
public:
  ZeroAssignOperation() {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;
};
class ExplicitCastOperation : public Operation {
public:
  ExplicitCastOperation() {}

protected:
  virtual bool matched(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                       ValueHolder *v) override;
};
/*
Operation Detection Analysis:
This analysis enumerates all the paths from the value's definition to use point,
to find whether an interesing operation happens.
*/
class OperationDetection {
public:
  enum DetectionKind {
    Exists,
    Forall,
  };
  ValueHolder *alias;
  bool has_alias;

private:
  struct DetectionContext {
    enum Color {
      White = 0,
      Gray = 1,
    };
    //
    DetectionContext(ASTManager *mgr, ASTFunction *func, uint32_t depth,
                     DetectionKind knd)
        : current_function(func), call_depth(depth),
          color_map(mgr->getCFG(func)->size(), Color::White), kind(knd) {}
    ~DetectionContext() {
      for (auto i : _values) {
        delete i;
      }
    }
    void add_value_holder(ValueHolder *v) { _values.push_back(v); }
    DetectionKind kind;
    std::vector<Color> color_map;
    ASTFunction *current_function;
    uint32_t call_depth;

  private:
    std::vector<ValueHolder *> _values;
  };

public:
  // If the use point IS NOT from the function, analysis behavior is UNDEFINED
  // Usually, you will get a segmentation fault
  OperationDetection(ASTManager *mgr, CallGraph *cg, ASTFunction *func)
      : _ast_manager(mgr), _call_graph(cg), _function(func) {}
  bool detect(DetectionKind kind);
  void set_use_point(const clang::CFGBlock *blk,
                     clang::CFGBlock::const_reverse_iterator it) {
    _use_point = std::make_unique<UsePoint>(blk, it);
  }
  template <typename _T, typename... _Args>
  void set_operation(_Args &&... args) {
    _operation = std::make_unique<_T>(std::forward<_Args>(args)...);
  }
  UsePoint *use_point() { return _use_point.get(); }
  void set_max_call_depth(uint32_t d) { _max_call_depth = d; }

private:
  bool detect_impl(const clang::CFGBlock *blk,
                   clang::CFGBlock::const_reverse_iterator it, ValueHolder *val,
                   DetectionContext &ctx);
  bool detect_stmt(const clang::CFGBlock *blk, const clang::Stmt *stmt,
                   ValueHolder *&val, DetectionContext &ctx, bool &halt);
  ValueHolder *dispatch_new_value_holder(const clang::Expr *expr);

private:
  ASTManager *_ast_manager;
  CallGraph *_call_graph;
  ASTFunction *_function;
  std::unique_ptr<UsePoint> _use_point;
  std::unique_ptr<Operation> _operation;
  uint32_t _max_call_depth = 1;
};

class Traverse : public RecursiveASTVisitor<Traverse>
{
public:
  Traverse() {expr_val_holder = nullptr;}
  Traverse(ClangExprValueHolder *value){expr_val_holder = value;}
  bool VisitInitListExpr(InitListExpr *initlist_expr){
    if (initlist_expr){
      has_InitListExpr = true;
    }
    return true;
  }
  bool VisitExpr(const clang::Expr *_expr);
  // const clang::Expr *value_holder_expr = nullptr;
  ClangExprValueHolder *expr_val_holder;
  bool has_value_holder = false;
  bool has_InitListExpr = false;
  std::vector<const clang::Expr *> exprs;
};
#endif