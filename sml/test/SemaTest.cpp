#include <memory>
#include "gtest/gtest.h"
#include "AST/AST.h"
#include "AST/ASTVisitor.h"
#include "SemanticAnalyzer.h"
#include "src/SemanticAnalyzer/TypeCheck.h"
#include "Symbol/SymbolTable.h"

using namespace std;

class SemaTest : public testing::Test {
protected:
    template<typename ASTT>
    inline auto create(ASTT *p) {
        return std::shared_ptr<ASTT>(p);
    }

    template<typename ASTT>
    inline auto create(ASTT &&p) {
        return std::make_shared<ASTT>(std::forward<ASTT>(p));
    }

    inline auto check(shared_ptr<AST> const &ast, bool reserved = false) {
        if (!reserved || !semanticAnalyzer) {
            semanticAnalyzer = std::make_unique<SemanticAnalyzer>();
        }
        return semanticAnalyzer->check(ast);
    }

    inline static auto addPattern(
            const std::string &str, const std::string &var = "'a") {
        SymbolTable::getInstance()->insertPatternType(
                str, VariableTypeNameType::create(var));
    }

    inline static auto resetSymbols() {
        SymbolTable::reset();
    }

    inline static auto typeOf(const std::string &str) {
        return SymbolTable::getInstance()->getPatternType(str);
    }

    inline static auto typeIdOf(const std::string &str) {
        return typeOf(str)->getTypeId();
    }

    std::unique_ptr<SemanticAnalyzer> semanticAnalyzer;
};

class ValDecTest : public SemaTest {

};

class TopLevelExpTest : public SemaTest {

};

class ConditionalExpTest : public SemaTest {

};

class FunctionDecTest : public SemaTest {

};

TEST_F(ValDecTest, ValDecTest_TypeCheck_Test) {
    //region val i : int = 2; // must be ok
    {
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
        EXPECT_TRUE(check(dec));
    }
    //endregion

    //region val i : int = 2.0; // must be wrong
    {
        auto exp =
                create(ConstantExpAST(
                        create(FloatConAST(2.0))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST({create(AlphanumericIdAST("int"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_FALSE(check(dec));
    }
    //endregion

    //region val i = 2.0 : int; // must be wrong
    {
        auto exp =
                create(TypeAnnotationExpAST(
                        create(ConstantExpAST(
                                create(FloatConAST(2.0)))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}))))));
        auto pat =
                create(VariablePatAST(
                        create(SymbolicIdAST("i"))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_FALSE(check(dec));
    }
    //endregion

    //region val i : real = 2.0 : int; // must be wrong
    {
        auto exp =
                create(TypeAnnotationExpAST(
                        create(ConstantExpAST(
                                create(FloatConAST(2.0)))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST({create(AlphanumericIdAST("real"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_FALSE(check(dec));
    }
    //endregion

    //region val i : real = 2.0 : real; // must be ok
    {
        auto exp =
                create(TypeAnnotationExpAST(
                        create(ConstantExpAST(
                                create(FloatConAST(2.0)))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("real"))}))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST({create(AlphanumericIdAST("real"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_TRUE(check(dec));
    }
    //endregion

    //region val i : string = "" : string; // must be ok
    {
        auto exp =
                create(TypeAnnotationExpAST(
                        create(ConstantExpAST(
                                create(StringConAST("")))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("string"))}))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST({create(AlphanumericIdAST("string"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_TRUE(check(dec));
    }
    //endregion

    //region val i : int = 2 + 0.2; // must be wrong
    {
        auto exp = create(InfixApplicationExpAST(
                create(ConstantExpAST(
                        create(IntConAST(2)))),
                create(AlphanumericIdAST("+")),
                create(ConstantExpAST(
                        create(FloatConAST(0.2))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_FALSE(check(dec));
    }
    //endregion

    //region val i : real = 2.0 + 0.2 : real; // must be ok
    {
        auto exp = create(TypeAnnotationExpAST(
                create(InfixApplicationExpAST(
                        create(ConstantExpAST(
                                create(FloatConAST(2.0)))),
                        create(AlphanumericIdAST("+")),
                        create(ConstantExpAST(
                                create(FloatConAST(0.2)))))),
                create(ConstructorTypAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("real"))}))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("real"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_TRUE(check(dec));
    }
    //endregion

    //region val i : int = 2.0 + 0.2; // must be wrong
    {
        auto exp = create(InfixApplicationExpAST(
                create(ConstantExpAST(
                        create(FloatConAST(2.0)))),
                create(AlphanumericIdAST("+")),
                create(ConstantExpAST(
                        create(FloatConAST(0.2))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_FALSE(check(dec));
    }
    //endregion

    //region val i : real = 2.0 + 0.2; // must be ok
    {
        auto exp = create(InfixApplicationExpAST(
                create(ConstantExpAST(
                        create(FloatConAST(2.0)))),
                create(AlphanumericIdAST("+")),
                create(ConstantExpAST(
                        create(FloatConAST(0.2))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("real"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_TRUE(check(dec));
    }
    //endregion

    //region val i : int = 40 + 2; // must be ok
    {
        auto exp = create(InfixApplicationExpAST(
                create(ConstantExpAST(
                        create(IntConAST(40)))),
                create(AlphanumericIdAST("+")),
                create(ConstantExpAST(
                        create(IntConAST(2))))));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(SymbolicIdAST("i")))),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}))))));
        auto dec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(pat, exp))));
        EXPECT_TRUE(check(dec));
    }
    //endregion
}

TEST_F(TopLevelExpTest, TopLevelExpTest_BuiltinOperators_Test) {
    //region 42; ok
    {
        auto exp = create(ConstantExpAST(
                create(IntConAST(42)
                )));
        EXPECT_TRUE(check(exp));
    }
    //endregion

    //region 42 + 42; ok
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ConstantExpAST(
                                create(IntConAST(42)
                                ))),
                        create(AlphanumericIdAST("+")),
                        create(ConstantExpAST(
                                create(IntConAST(42)
                                )))
                ));
        EXPECT_TRUE(check(infixAppExp));
    }
    //endregion

    //region 42 + 3.14; no
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ConstantExpAST(
                                create(IntConAST(42)
                                ))),
                        create(AlphanumericIdAST("+")),
                        create(ConstantExpAST(
                                create(FloatConAST(3.14)
                                )))
                ));
        EXPECT_FALSE(check(infixAppExp));
    }
    //endregion

    //region "42" + "3.14"; no
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ConstantExpAST(
                                create(StringConAST("42")
                                ))),
                        create(AlphanumericIdAST("+")),
                        create(ConstantExpAST(
                                create(StringConAST("3.14")
                                )))
                ));
        EXPECT_FALSE(check(infixAppExp));
    }
    //endregion

    //region "42" < "3.14"; ok
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ConstantExpAST(
                                create(StringConAST("42")
                                ))),
                        create(AlphanumericIdAST("<")),
                        create(ConstantExpAST(
                                create(StringConAST("3.14")
                                )))
                ));
        EXPECT_TRUE(check(infixAppExp));
    }
    //endregion

    //region "42" ^ "3.14"; ok
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ConstantExpAST(
                                create(StringConAST("42")
                                ))),
                        create(AlphanumericIdAST("^")),
                        create(ConstantExpAST(
                                create(StringConAST("3.14")
                                )))
                ));
        EXPECT_TRUE(check(infixAppExp));
    }
    //endregion

    //region [42] @ [42]; ok
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ListExpAST(
                                {create(ConstantExpAST(
                                        create(IntConAST(42))))})),
                        create(AlphanumericIdAST("@")),
                        create(ListExpAST(
                                {create(ConstantExpAST(
                                        create(IntConAST(42))))}))
                ));
        EXPECT_TRUE(check(infixAppExp));
    }
    //endregion

    //region [] @ [42]; ok
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ListExpAST({})),
                        create(AlphanumericIdAST("@")),
                        create(ListExpAST(
                                {create(ConstantExpAST(
                                        create(IntConAST(42))))}))
                ));
        EXPECT_TRUE(check(infixAppExp));
    }
    //endregion

    //region [] @ []; ok
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ListExpAST({})),
                        create(AlphanumericIdAST("@")),
                        create(ListExpAST({}))
                ));
        EXPECT_TRUE(check(infixAppExp));
    }
    //endregion

    //region [42] @ [3.14]; no
    {
        auto infixAppExp =
                create(InfixApplicationExpAST(
                        create(ListExpAST(
                                {create(ConstantExpAST(
                                        create(IntConAST(42))))})),
                        create(AlphanumericIdAST("@")),
                        create(ListExpAST(
                                {create(ConstantExpAST(
                                        create(FloatConAST(3.14))))}))
                ));
        EXPECT_FALSE(check(infixAppExp));
    }
    //endregion

    //region ~42; ok
    {
        auto appExp =
                create(ApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(SymbolicIdAST("~"))}
                                ))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(42))
                        ))
                ));
        EXPECT_TRUE(check(appExp));
    }
    //endregion

    //region ~3.14; ok
    {
        auto appExp =
                create(ApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(SymbolicIdAST("~"))}
                                ))
                        )),
                        create(ConstantExpAST(
                                create(FloatConAST(3.14))
                        ))
                ));
        EXPECT_TRUE(check(appExp));
    }
    //endregion

    //region ~"42"; no
    {
        auto appExp =
                create(ApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(SymbolicIdAST("~"))}
                                ))
                        )),
                        create(ConstantExpAST(
                                create(StringConAST("42"))
                        ))
                ));
        EXPECT_FALSE(check(appExp));
    }
    //endregion

    //region op +; ok
    {
        auto exp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(SymbolicIdAST("+"))}
                        ))
                ));
        EXPECT_TRUE(check(exp));
    }
    //endregion
}

TEST_F(TopLevelExpTest, TopLevelExpTest_FunctionMatch_Test) {
#pragma push_macro("EXPECT_EQ_TYPE")
#define EXPECT_EQ_TYPE(a, b) \
TypeCheck typecheck; \
EXPECT_TRUE(typecheck.unify(a, b)) \
<< #a " : " << *a << '\n' << #b " : " << *b

    //region fn x => x; ok 'a, 'a -> 'a
    {
        addPattern("x");
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::VARIABLE_TYPE_NAME);
        auto varTy = VariableTypeNameType::create("'a");
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(varTy, TupleType::create({varTy})));
        resetSymbols();
    }
    //endregion

    //region fn x => y; no
    {
        addPattern("x");
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("y"))}
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_FALSE(check(match));
        resetSymbols();
    }
    //endregion

    //region fn x => x + x; ok int -> int
    {
        addPattern("x");
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::INT);
        auto ty = IntType::create();
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(ty, TupleType::create({ty})));
        resetSymbols();
    }
    //endregion

    //region fn x : int => x + x; ok int -> int
    {
        addPattern("x");
        auto matchPat =
                create(TypeAnnotationPatAST(
                        create(ConstructionPatAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}
                                ))
                        ))
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::INT);
        auto ty = IntType::create();
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(ty, ty));
        resetSymbols();
    }
    //endregion

    //region fn x : real => x + x; ok real -> real
    {
        addPattern("x");
        auto matchPat =
                create(TypeAnnotationPatAST(
                        create(ConstructionPatAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("real"))}
                                ))
                        ))
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::REAL);
        auto ty = RealType::create();
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(ty, ty));
        resetSymbols();
    }
    //endregion

    //region fn x : real => 42; ok real -> int
    {
        addPattern("x");
        auto matchPat =
                create(TypeAnnotationPatAST(
                        create(ConstructionPatAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("real"))}
                                ))
                        ))
                ));
        auto matchExp =
                create(ConstantExpAST(
                        create(IntConAST(42))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::REAL);
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(
                               IntType::create(),
                               RealType::create()));
        resetSymbols();
    }
    //endregion

    //region fn (x, y) => x + y; ok int * int -> int
    {
        addPattern("x", "'Z");
        addPattern("y", "'Y");
        auto matchPat =
                create(TuplePatAST(
                        {create(ConstructionPatAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                         create(ConstructionPatAST(
                                 create(LongIdAST(
                                         {create(AlphanumericIdAST("y"))}
                                 ))
                         ))}
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("y"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::INT);
        EXPECT_EQ(typeIdOf("y"), Type::INT);
        auto ty = IntType::create();
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(ty, TupleType::create({ty, ty})));
        resetSymbols();
    }
    //endregion

    //region fn (x : real, y) => x + y; ok real * real -> real
    {
        addPattern("x", "'Z");
        addPattern("y", "'Y");
        auto matchPat =
                create(TuplePatAST(
                        {create(TypeAnnotationPatAST(
                                create(ConstructionPatAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("x"))}
                                        ))
                                )),
                                create(ConstructorTypAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("real"))}
                                        ))
                                ))
                        )),
                         create(ConstructionPatAST(
                                 create(LongIdAST(
                                         {create(AlphanumericIdAST("y"))}
                                 ))
                         ))}
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("y"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::REAL);
        EXPECT_EQ(typeIdOf("y"), Type::REAL);
        auto ty = RealType::create();
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(ty, TupleType::create({ty, ty})));
        resetSymbols();
    }
    //endregion

    //region fn (x : real, y : real) => x + y; ok real * real -> real
    {
        addPattern("x", "'Z");
        addPattern("y", "'Y");
        auto matchPat =
                create(TuplePatAST(
                        {create(TypeAnnotationPatAST(
                                create(ConstructionPatAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("x"))}
                                        ))
                                )),
                                create(ConstructorTypAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("real"))}
                                        ))
                                ))
                        )),
                         create(TypeAnnotationPatAST(
                                 create(ConstructionPatAST(
                                         create(LongIdAST(
                                                 {create(AlphanumericIdAST("y"))}
                                         ))
                                 )),
                                 create(ConstructorTypAST(
                                         create(LongIdAST(
                                                 {create(AlphanumericIdAST("real"))}
                                         ))
                                 ))
                         ))}
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("y"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::REAL);
        EXPECT_EQ(typeIdOf("y"), Type::REAL);
        auto ty = RealType::create();
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(ty, TupleType::create({ty, ty})));
        resetSymbols();
    }
    //endregion

    //region fn (x, y) => x + 1.0; ok real * 'a -> real
    {
        addPattern("x", "'Z");
        addPattern("y", "'Y");
        auto matchPat =
                create(TuplePatAST(
                        {create(ConstructionPatAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                         create(ConstructionPatAST(
                                 create(LongIdAST(
                                         {create(AlphanumericIdAST("y"))}
                                 ))
                         ))}
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ConstantExpAST(
                                create(FloatConAST(1.0))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("x"), Type::REAL);
        EXPECT_EQ(typeIdOf("y"), Type::VARIABLE_TYPE_NAME);
        auto ty = RealType::create();
        EXPECT_EQ_TYPE(
                typeOf("it"),
                FunctionType::create(ty, TupleType::create(
                        {ty, VariableTypeNameType::create("'a")})));
        resetSymbols();
    }
    //endregion

    //region fn (x : real, y) => x + y : int; no
    {
        addPattern("x", "'Z");
        addPattern("y", "'Y");
        auto matchPat =
                create(TuplePatAST(
                        {create(TypeAnnotationPatAST(
                                create(ConstructionPatAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("x"))}
                                        ))
                                )),
                                create(ConstructorTypAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("real"))}
                                        ))
                                ))
                        )),
                         create(ConstructionPatAST(
                                 create(LongIdAST(
                                         {create(AlphanumericIdAST("y"))}
                                 ))
                         ))}
                ));
        auto matchExp =
                create(TypeAnnotationExpAST(
                        create(InfixApplicationExpAST(
                                create(ValueOrConstructorIdentifierExpAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("x"))}
                                        ))
                                )),
                                create(SymbolicIdAST("+")),
                                create(ValueOrConstructorIdentifierExpAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("y"))}
                                        ))
                                ))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_FALSE(check(match));
        resetSymbols();
    }
    //endregion

    //region fn (x : real, y : int) => x + y; no
    {
        addPattern("x", "'Z");
        addPattern("y", "'Y");
        auto matchPat =
                create(TuplePatAST(
                        {create(TypeAnnotationPatAST(
                                create(ConstructionPatAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("x"))}
                                        ))
                                )),
                                create(ConstructorTypAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("real"))}
                                        ))
                                ))
                        )),
                         create(TypeAnnotationPatAST(
                                 create(ConstructionPatAST(
                                         create(LongIdAST(
                                                 {create(AlphanumericIdAST("y"))}
                                         ))
                                 )),
                                 create(ConstructorTypAST(
                                         create(LongIdAST(
                                                 {create(AlphanumericIdAST("int"))}
                                         ))
                                 ))
                         ))}
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(SymbolicIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("y"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_FALSE(check(match));
        resetSymbols();
    }
    //endregion

    //region fn it => it; ok 'a, 'a -> 'a
    {
        addPattern("it");
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("it"))}
                        ))
                ));
        auto matchExp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("it"))}
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        EXPECT_TRUE(check(match));
        EXPECT_EQ(typeIdOf("it"), Type::FUNCTION);
        auto varTy = VariableTypeNameType::create("'a");
        EXPECT_EQ_TYPE(typeOf("it"),
                       FunctionType::create(varTy, TupleType::create({varTy})));
        resetSymbols();
    }
    //endregion

#undef EXPECT_EQ_TYPE
#pragma pop_macro("EXPECT_EQ_TYPE")
}

TEST_F(ConditionalExpTest, ConditionalExpTest_ConditionalExp_Test) {
    //region val i : int = if true then 42 else 0; ok
    {
        auto condExp =
                create(ConditionalExpAST(
                        create(ConstantExpAST(
                                create(BoolConAST(true))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(42))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(0))
                        ))
                ));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(AlphanumericIdAST("i"))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}
                                ))
                        ))
                ));
        auto exp = create(DestructuringValBindAST(pat, condExp));
        EXPECT_TRUE(check(exp));
    }
    //endregion

    //region val i : real = if true then 42 else 0; no
    {
        auto condExp =
                create(ConditionalExpAST(
                        create(ConstantExpAST(
                                create(BoolConAST(true))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(42))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(0))
                        ))
                ));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(AlphanumericIdAST("i"))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("real"))}
                                ))
                        ))
                ));
        auto exp = create(DestructuringValBindAST(pat, condExp));
        EXPECT_FALSE(check(exp));
    }
    //endregion

    //region val i : int = if true then 42 else 0.0; no
    {
        auto condExp =
                create(ConditionalExpAST(
                        create(ConstantExpAST(
                                create(BoolConAST(true))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(42))
                        )),
                        create(ConstantExpAST(
                                create(FloatConAST(0.0))
                        ))
                ));
        auto pat =
                create(TypeAnnotationPatAST(
                        create(VariablePatAST(
                                create(AlphanumericIdAST("i"))
                        )),
                        create(ConstructorTypAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("int"))}
                                ))
                        ))
                ));
        auto exp = create(DestructuringValBindAST(pat, condExp));
        EXPECT_FALSE(check(exp));
    }
    //endregion

    //region val i = if true then 42 else 0.0; no
    {
        auto condExp =
                create(ConditionalExpAST(
                        create(ConstantExpAST(
                                create(BoolConAST(true))
                        )),
                        create(ConstantExpAST(
                                create(IntConAST(42))
                        )),
                        create(ConstantExpAST(
                                create(FloatConAST(0.0))
                        ))
                ));
        auto pat =
                create(VariablePatAST(
                        create(AlphanumericIdAST("i"))
                ));
        auto exp = create(DestructuringValBindAST(pat, condExp));
        EXPECT_FALSE(check(exp));
    }
    //endregion
}

TEST_F(FunctionDecTest, FunctionDecTest_SimpleFunctionDec_Test) {
    //region val f = fn x => x; ok
    {
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        auto valDecPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("f"))}
                        ))
                ));
        auto valDec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(
                                valDecPat,
                                create(FunctionExpAST(
                                        match
                                ))
                        ))
                ));
        EXPECT_TRUE(check(valDec));
    }
    //endregion

    //region val f = fn x => x + y; no
    {
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(AlphanumericIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("y"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        auto valDecPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("f"))}
                        ))
                ));
        auto valDec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(
                                valDecPat,
                                create(FunctionExpAST(
                                        match
                                ))
                        ))
                ));
        EXPECT_FALSE(check(valDec));
    }
    //endregion

    //region val f = fn x => x + x; ok
    {
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(InfixApplicationExpAST(
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        )),
                        create(AlphanumericIdAST("+")),
                        create(ValueOrConstructorIdentifierExpAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("x"))}
                                ))
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        auto valDecPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("f"))}
                        ))
                ));
        auto valDec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(
                                valDecPat,
                                create(FunctionExpAST(
                                        match
                                ))
                        ))
                ));
        EXPECT_TRUE(check(valDec));
    }
    //endregion

    //region val f = fn x => y; no
    {
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("y"))}
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        auto valDecPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("f"))}
                        ))
                ));
        auto valDec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(
                                valDecPat,
                                create(FunctionExpAST(
                                        match
                                ))
                        ))
                ));
        EXPECT_FALSE(check(valDec));
    }
    //endregion

    //region val f : int -> int = fn x => x; ok
    {
        auto matchPat =
                create(ConstructionPatAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto matchExp =
                create(ValueOrConstructorIdentifierExpAST(
                        create(LongIdAST(
                                {create(AlphanumericIdAST("x"))}
                        ))
                ));
        auto match = create(MatchAST(matchPat, matchExp));
        auto valDecPat =
                create(TypeAnnotationPatAST(
                        create(ConstructionPatAST(
                                create(LongIdAST(
                                        {create(AlphanumericIdAST("f"))}
                                ))
                        )),
                        create(FunctionTypAST(
                                create(ConstructorTypAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("int"))}
                                        ))
                                )),
                                create(ConstructorTypAST(
                                        create(LongIdAST(
                                                {create(AlphanumericIdAST("int"))}
                                        ))
                                ))
                        ))
                ));
        auto valDec =
                create(ValueDecAST(
                        create(DestructuringValBindAST(
                                valDecPat,
                                create(FunctionExpAST(
                                        match
                                ))
                        ))
                ));
        EXPECT_TRUE(check(valDec));
    }
    //endregion
}
