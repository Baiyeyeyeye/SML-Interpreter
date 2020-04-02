#pragma once

#include <iostream>
#include "AST/AST.h"
#include "AST/ASTVisitor.h"

class ASTPrinter : public ASTVisitor {
public:
    explicit ASTPrinter(std::ostream &o = std::cout) : _os(o) {}

    /**
     * Top level: insert a semicolon and a new line after each DEC.
     */
    void *visit(DecAST *ast) override {
        getOStream() << ';' ;
        return ast;
    }

    void *visit(VariablePatAST *ast) override {
        ast->getId()->accept(this);
        return ASTVisitor::visit(ast);
    }

    void *visit(TypeAnnotationExpAST *ast) override {
        ast->getExp()->accept(this);
        getOStream() << ':';
        ast->getTyp()->accept(this);
        return ASTVisitor::visit(ast);
    }

    void *visit(TypeAnnotationPatAST *ast) override {
        ast->getPat()->accept(this);
        getOStream() << " : ";
        ast->getTyp()->accept(this);
        return ast;
    }

    void *visit(DestructuringValBindAST *ast) override {
        ast->getPat()->accept(this);
        getOStream() << " = ";
        ast->getExp()->accept(this);
        return ast;
    }

    void *visit(ValueDecAST *ast) override {
        getOStream() << "val ";
        ast->getValBind()->accept(this);
        return visit(static_cast<DecAST *>(ast));
    }

    void *visit(IntConAST *ast) override {
        getOStream() << ast->get();
        return ast;
    }

    void *visit(ConstantExpAST *ast) override {
        ast->getCon()->accept(this);
        return ast;
    }

    void *visit(IdAST *ast) override {
        getOStream() << ast->get();
        return ast;
    }

    void *visit(InfixApplicationExpAST *ast) override {
        auto exp1 = ast->getExp1();
        ast->getId()->accept(this);
        auto exp2 = ast->getExp2();
        return ast;
    }

    void *visit(DisjunctionExpAST *ast) override {
        ast->getExp1()->accept(this);
        ast->getExp2()->accept(this);
        return ast;
    }

    void *visit(ConditionalExpAST *ast) override {
        ast->getExp1()->accept(this);
        ast->getExp2()->accept(this);
        ast->getExp3()->accept(this);
        return ast;
    }

    void *visit(ConstructorTypAST *ast) override {
        ast->getLongId()->accept(this);
        return ast;
    }

    void *visit(LongIdAST *ast) override {
        int cnt = 0;
        for (auto &&id : ast->getIds()) {
            id->accept(this);
            if (cnt++) {
                getOStream() << '.';
            }
        }
        return ast;
    }

    std::ostream &getOStream() {
        return _os;
    }

private:
    std::ostream &_os;
};
