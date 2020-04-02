#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "AST/AST.h"
#include "AST/ASTVisitor.h"
#include "CodeGenerator.h"
#include "Error.h"
#include "Symbol/SymbolTable.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "KaleidoscopeJIT.h"

using namespace std;
//using llvm::Value;

class CodeGen : public ASTVisitor {
public:
    void *visit(ValBindAST *ast) override;

    void *visit(IntConAST *ast) override;

    void *visit(FloatConAST *ast) override;

    void *visit(CharConAST *ast) override;

    void *visit(StringConAST *ast) override;

    void *visit(BoolConAST *ast) override;

    void *visit(DisjunctionExpAST *ast) override;

    void *visit(ConjunctionExpAST *ast) override;

    void *visit(IdAST *ast) override;

    void *visit(VarAST *ast) override;

    void *visit(LongIdAST *ast) override;

    void *visit(IdentifierLabAST *ast) override;

    void *visit(NumberLabAST *ast) override;

    void *visit(MatchAST *ast) override;

    void *visit(FunctionExpAST *ast) override;

    void *visit(FunctionTypAST *ast) override;

    void *visit(InfixConstructionPatAST *ast) override;

    void *visit(FunctionDecAST *ast) override;

    void *visit(FunBindAST *ast) override;

    void *visit(FunMatchAST *ast) override;

    void *visit(TuplePatAST *ast) override;

    void *visit(VariablePatAST *ast) override;

    void *visit(InfixApplicationExpAST *ast) override;

    void *visit(ConditionalExpAST *ast) override;

    void *visit(ValueDecAST *ast) override;

    void *visit(DestructuringValBindAST *ast) override;

    void *visit(ConstantExpAST *ast) override;

    void *visit(ConAST *ast) override;

    void *visit(ExpAST *ast) override;

    void *visit(ApplicationExpAST *ast) override;

    void *visit(ValueOrConstructorIdentifierExpAST *ast) override;

    void *visit(LocalDeclarationExpAST *ast) override;

    void *visit(SequenceDecAST *ast) override;

private:
    llvm::Value *visitAsValue(const std::shared_ptr<ExpAST>& exp);

    llvm::Value *_value{};
};

struct CodeGenerator::Impl {
    CodeGen codeGen;
};
