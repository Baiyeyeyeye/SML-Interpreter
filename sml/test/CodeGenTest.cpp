#include "gtest/gtest.h"
#include "llvm/Support/raw_ostream.h"
#include "AST/AST.h"
#include "CodeGenerator.h"
#include "src/CodeGenerator/CodeGeneratorImpl.h"

class CodeGenTest : public testing::Test {
protected:
    template<typename ASTT>
    auto create(ASTT *p) {
        return std::shared_ptr<ASTT>(p);
    }

    template<typename ASTT>
    auto create(ASTT &&p) {
        return std::make_shared<ASTT>(std::forward<ASTT>(p));
    }

    CodeGenerator codeGenerator;

    static llvm::raw_string_ostream &getOStream() {
        static std::string out;
        static llvm::raw_string_ostream rso(out);
        return rso;
    }
};

TEST_F(CodeGenTest, CodeGenTest_VisitorInterface_Test) {
    //region val i : int = 42;
    {
        auto exp =
                create(ConstantExpAST(
                        create(IntConAST(42))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST({create(AlphanumericIdAST("int"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        auto value = codeGenerator.generate(dec);
    }
    //endregion
}
