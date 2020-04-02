#include "AST.h"
#include "ASTVisitor.h"

#define FORWARD(...) return visit(static_cast<__VA_ARGS__*>(ast));

void *ASTVisitor::visit(ConAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(IntConAST *ast) {
    FORWARD(ConAST)
}

void *ASTVisitor::visit(FloatConAST *ast) {
    FORWARD(ConAST)
}

void *ASTVisitor::visit(BoolConAST *ast) {
    FORWARD(ConAST)
}

void *ASTVisitor::visit(CharConAST *ast) {
    FORWARD(ConAST)
}

void *ASTVisitor::visit(StringConAST *ast) {
    FORWARD(ConAST)
}

void *ASTVisitor::visit(IdAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(VarAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(LongIdAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(LabAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(ExpAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(ConstantExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ValueOrConstructorIdentifierExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ApplicationExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(InfixApplicationExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ParenthesesExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(TupleExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(RecordTupleExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(RecordSelectorExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ListExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(LocalDeclarationExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(TypeAnnotationExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ConjunctionExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(DisjunctionExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ConditionalExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(IterationExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(CaseAnalysisExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(FunctionExpAST *ast) {
    FORWARD(ExpAST)
}

void *ASTVisitor::visit(ExpRowAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(MatchAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(PatAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(ConstantPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(WildCardPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(ParenthesesPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(VariablePatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(ConstructionPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(InfixConstructionPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(AST *ast) {
    return ast;
}

void *ASTVisitor::visit(TuplePatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(RecordPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(ListPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(TypeAnnotationPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(LayeredPatAST *ast) {
    FORWARD(PatAST)
}

void *ASTVisitor::visit(PatRowAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(TypAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(VariableTypAST *ast) {
    FORWARD(TypAST)
}

void *ASTVisitor::visit(ConstructorTypAST *ast) {
    FORWARD(TypAST)
}

void *ASTVisitor::visit(ParenthesesTypAST *ast) {
    FORWARD(TypAST)
}

void *ASTVisitor::visit(FunctionTypAST *ast) {
    FORWARD(TypAST)
}

void *ASTVisitor::visit(TupleTypAST *ast) {
    FORWARD(TypAST)
}

void *ASTVisitor::visit(RecordTypAST *ast) {
    FORWARD(TypAST)
}

void *ASTVisitor::visit(TypRowAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(WildCardPatRowAST *ast) {
    FORWARD(PatRowAST)
}

void *ASTVisitor::visit(PatternPatRowAST *ast) {
    FORWARD(PatRowAST)
}

void *ASTVisitor::visit(VariablePatRowAST *ast) {
    FORWARD(PatRowAST)
}

void *ASTVisitor::visit(DecAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(ValueDecAST *ast) {
    FORWARD(DecAST)
}

void *ASTVisitor::visit(FunctionDecAST *ast) {
    FORWARD(DecAST)
}

void *ASTVisitor::visit(TypeDecAST *ast) {
    FORWARD(DecAST)
}

void *ASTVisitor::visit(DataTypeDecAST *ast) {
    FORWARD(DecAST)
}

void *ASTVisitor::visit(SequenceDecAST *ast) {
    FORWARD(DecAST)
}
void *ASTVisitor::visit(LocalDecAST *ast) {
    FORWARD(DecAST)
}

void *ASTVisitor::visit(LeftAssociativeInfixDecAST *ast) {
    FORWARD(DecAST)
}
void *ASTVisitor::visit(RightAssociativeInfixDecAST *ast) {
    FORWARD(DecAST)
}
void *ASTVisitor::visit(NonfixDecAST *ast) {
    FORWARD(DecAST)
}

void *ASTVisitor::visit(ValBindAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(TypBindAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(DestructuringValBindAST *ast) {
    FORWARD(ValBindAST)
}

void *ASTVisitor::visit(RecursiveValBindAST *ast) {
    FORWARD(ValBindAST)
}

void *ASTVisitor::visit(FunBindAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(FunMatchAST *ast) {
    FORWARD(AST)
}

void *ASTVisitor::visit(NonFixFunMatchAST *ast) {
    FORWARD(FunMatchAST)
}

void *ASTVisitor::visit(InfixFunMatchAST *ast) {
    FORWARD(FunMatchAST)
}

void *ASTVisitor::visit(AlphanumericIdAST *ast) {
    FORWARD(IdAST)
}

void *ASTVisitor::visit(SymbolicIdAST *ast) {
    FORWARD(IdAST)
}

void *ASTVisitor::visit(UnconstrainedVarAST *ast) {
    FORWARD(VarAST)
}

void *ASTVisitor::visit(EqualityVarAST *ast) {
    FORWARD(VarAST)
}

void *ASTVisitor::visit(IdentifierLabAST *ast) {
    FORWARD(LabAST)
}

void *ASTVisitor::visit(NumberLabAST *ast) {
    FORWARD(LabAST)
}


