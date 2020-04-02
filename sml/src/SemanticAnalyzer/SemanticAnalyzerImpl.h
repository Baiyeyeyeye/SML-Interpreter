#pragma once

#include <memory>
#include <unordered_map>
#include "SemanticAnalyzer.h"

class AST;

class Type;

struct SemanticAnalyzer::Impl {
    ~Impl() = default;

    std::shared_ptr<AST> check(const std::shared_ptr<AST> &ast);
};