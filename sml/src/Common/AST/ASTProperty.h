#pragma once

#include <memory>

class Type;

class AST;

class ASTProperty {
public:
    static Type *getType(AST *ast);

    static Type *getType(const std::shared_ptr<AST> &ast) {
        return getType(ast.get());
    }

    static void setType(AST *ast, Type *type);

    static void setType(const std::shared_ptr<AST> &ast, Type *type) {
        setType(ast.get(), type);
    }
};
