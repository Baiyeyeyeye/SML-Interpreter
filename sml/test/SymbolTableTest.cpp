#include "gtest/gtest.h"
#include "src/Common/Symbol/SymbolTable.h"

class SymbolTableTest : public testing::Test {

};

TEST_F(SymbolTableTest, SymbolTableTest_NewDeleteMemory_Test) {
    auto itype = IntType::create();
    delete itype;
    delete itype;
    auto ltibtype = ListType::create(
            TupleType::create(
                    {IntType::create(), BoolType::create()}));
    delete ltibtype;
    delete ltibtype;
}
