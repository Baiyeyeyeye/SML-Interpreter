#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "AST/AST.h"
#include "AST/ASTVisitor.h"
#include "Error.h"
#include "SemanticAnalyzer.h"
#include "Symbol/SymbolTable.h"

using namespace std;

inline namespace MyStringType {
    class StringType : public Type {
    public:

    private:
        string str;
    };
}

class FreeTest : public testing::Test {
protected:
    llvm::LLVMContext context;
};

TEST_F(FreeTest, FreeTest_DeriveOperatorNew_Test) {
    class Base {
    public:
        void *operator new(size_t sz) {
            return ::operator new(sz);
        }

        void operator delete(void *) {

        }
    };

    class Derived : public Base {

    };

    new Derived;
}

TEST_F(FreeTest, FreeTest_ErrorTest_Test) {
    {
        Error("gkejl");
    }
    ASSERT_TRUE(1);
}

TEST_F(FreeTest, FreeTest_True_Test) {
    ASSERT_TRUE(true);
}

TEST_F(FreeTest, FreeTest_ASTTest_Test) {
    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.check(std::shared_ptr<IdAST>(new AlphanumericIdAST("w")));
}

TEST_F(FreeTest, FreeTest_TypeValueDownCast_Test) {
    auto t = IntType::create();
    ASSERT_EQ(t->toIntType()->getTypeId(), Type::INT);
}

TEST_F(FreeTest, FreeTest_TestGetType_Test) {
    using llvm::Type;
    using llvm::StructType;
    using llvm::PointerType;
    using llvm::ArrayType;

    auto structType = StructType::get(context, Type::getInt32Ty(context), false);
    structType->setName("WTF");
    cout << ((string) structType->getName()) << endl;
    cout << structType->getNumElements() << endl;
    for (int i = 0; i < structType->getNumElements(); ++i) {
        auto subType = structType->getStructElementType(i);
        cout << subType->getIntegerBitWidth() << endl;
    }
    auto structType2 = StructType::get(context, Type::getInt32Ty(context), false);

    ASSERT_EQ(structType, structType2);

    auto int32Type = Type::getInt32Ty(context);
    structType = StructType::get(context, {int32Type, int32Type, int32Type});
    auto arrayType = ArrayType::get(int32Type, 2);
    auto array2Type = ArrayType::get(int32Type, 2);
    ASSERT_EQ(arrayType, array2Type);

    auto ptr = PointerType::get(Type::getInt8Ty(context), 3);
    structType = StructType::create(context, "wtf");
    structType2 = StructType::create(context, "wtf");
    ASSERT_NE(structType, structType2);
}