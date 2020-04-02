#pragma once

#include <memory>
#include "AST/ASTVisitor.h"

namespace llvm {
    class Value;
    class Function;
}

class AST;

class CodeGenerator : public ASTVisitor {
public:
    CodeGenerator();

    ~CodeGenerator();

    llvm::Function *generate(const std::shared_ptr<AST>& ast);

    void *visit(ExpAST *ast) override;

//    void *visit(LocalDeclarationExpAST *ast) override ;

    void *visit(FunctionDecAST *ast) override ;

    void *visit(ValueDecAST *ast) override;

private:
    std::shared_ptr<AST> _ast;

    struct Impl;

    std::shared_ptr<Impl> _impl;
};
