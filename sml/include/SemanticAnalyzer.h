#pragma once

#include <memory>

class AST;

class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    ~SemanticAnalyzer();

    /**
     * Do a context check for built ast.
     * @param ast The ast built by parser.
     * @return The original AST if no semantic error, otherwise a nullptr.
     */
    std::shared_ptr<AST> check(const std::shared_ptr<AST> &ast);

private:
    struct Impl;

    std::unique_ptr<Impl> _impl;
};
