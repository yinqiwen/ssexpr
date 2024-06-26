// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
// Created on 2021/03/16
// Authors: qiyingwang (qiyingwang@tencent.com)
#pragma once

#include <stdint.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ssexpr2/jit_struct.h"

namespace ssexpr2 {
struct Expr {
  virtual ~Expr() {}
};

typedef Value (*ExprFunction)(...);
typedef std::function<ValueAccessor(const std::vector<std::string>&, std::shared_ptr<Xbyak::CodeGenerator>)>
    GetStructMemberAccessFunction;

using VarMap = std::unordered_map<std::string, Value>;
struct ExprOptions {
  std::map<std::string, ExprFunction> functions;
  GetStructMemberAccessFunction get_member_access;
  int jit_code_size = 8192;
  template <typename T>
  void Init() {
    T::InitJitBuilder();
    get_member_access = [](const std::vector<std::string>& names, std::shared_ptr<Xbyak::CodeGenerator> jit) {
      ValueAccessor accessor = GetFieldAccessors<T>(names, jit);
      return accessor;
    };
  }
};

struct JITEvalContext {
  VarMap vars;
  JITEvalContext() {}
};

class SpiritExpression {
 private:
  std::shared_ptr<Xbyak::CodeGenerator> jit_;
  std::shared_ptr<Expr> expr_;

  typedef Value (*EvalFunc)(const void*, const JITEvalContext*);
  EvalFunc _eval = nullptr;
  Value doEval(const void* obj, const JITEvalContext* ctx);

 public:
  int Init(const std::string& expr, const ExprOptions& options);
  int DumpAsmCode(const std::string& file);
  Value Eval() { return doEval(nullptr, nullptr); }
  template <typename T>
  Value Eval(const T& root_obj, const JITEvalContext& ctx = {}) {
    return doEval(&root_obj, &ctx);
  }
};
}  // namespace ssexpr2