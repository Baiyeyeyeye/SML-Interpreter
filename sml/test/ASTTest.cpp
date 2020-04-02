#include <memory>
#include "gtest/gtest.h"
#include "AST/AST.h"
#include "AST/ASTVisitor.h"
#include "ASTPrinter.h"

using namespace std;

class ASTTest : public testing::Test {
protected:
    template <typename ASTT>
    auto create(ASTT &&p) {
        return std::make_shared<ASTT>(std::forward<ASTT>(p));
    }
};

TEST_F(ASTTest, ASTTest_Print_Test) {
    auto exp =
            create(ConstantExpAST(
                    create(IntConAST(2))));
    auto pat =
            create(TypeAnnotationPatAST(
                    create(VariablePatAST(
                            create(SymbolicIdAST("i")))),
                    create(ConstructorTypAST(
                            create(LongIdAST({create(AlphanumericIdAST("int"))}))))));
    auto dec =
            create(ValueDecAST(
                    create(DestructuringValBindAST(pat, exp))));
    ASTPrinter astPrinter;
    dec->accept(&astPrinter);
}
