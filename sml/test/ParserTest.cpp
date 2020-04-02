#include <sstream>
#include <vector>
#include "gtest/gtest.h"
#include "AST/AST.h"
#include "ASTPrinter.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

using namespace std;

class ParserTest : public testing::Test {
protected:
    class MyScanner : public ::Scanner {
    public:
        explicit MyScanner(vector<shared_ptr<Token>> ___tokens)
                : Scanner(getWildCardStream()), tokens(move(___tokens)) {
            it = this->tokens.begin();
        }

        shared_ptr<Token> getNextToken() override {
            return it == tokens.end() ? nullptr : *it++;
        }

    private:
        vector<shared_ptr<Token>> tokens;
        decltype(tokens)::iterator it;

        static istream &getWildCardStream() noexcept {
            static istringstream w;
            return w;
        }
    };

    class MyParser: public ::Parser{

    };

    class MyInterpreter : public ::Interpreter {
    public:
        friend class ParserTest;

        explicit MyInterpreter(vector<shared_ptr<Token>> tokens)
                : myScanner(std::move(tokens)),
                  parser(this) {

        }

    protected:
        [[nodiscard]] Scanner *getScanner() const override {
            return const_cast<MyScanner *>(&myScanner);
        }

        [[nodiscard]] Parser *getParser() const override {
            return const_cast<Parser *>(&parser);
        }

        [[nodiscard]] SemanticAnalyzer *getSemanticAnalyzer() const override {
            return nullptr;
        }

        [[nodiscard]] CodeGenerator *getCodeGenerator() const override {
            return nullptr;
        }

        [[nodiscard]] JIT *getJIT() const override {
            return nullptr;
        }

        [[nodiscard]] bool eof() const override {
            return myScanner.eof();
        }

    private:
        MyScanner myScanner;
        Parser parser;
    };

    template<typename Type, typename Str>
    auto newToken(Type &&type, Str &&str) {
        return make_shared<Token>(1, 1, type, str);
    }

    static auto parse(const vector<shared_ptr<Token>> &tokens) {
        MyInterpreter interpreter(tokens);
        return interpreter.getParser()->parse();
    }

    template<typename T, typename U>
    auto dyn_cast(shared_ptr<U> p) {
        return dynamic_pointer_cast<T>(p);
    }

    template<typename ASTT>
    auto create(ASTT *p) {
        return std::shared_ptr<ASTT>(p);
    }

    template<typename ASTT>
    auto create(ASTT &&p) {
        return std::make_shared<ASTT>(std::forward<ASTT>(p));
    }

    static std::string printAST(const std::shared_ptr<AST> &ast) {
        ostringstream ss;
        ASTPrinter astPrinter(ss);
        ast->accept(&astPrinter);
        return std::move(ss.str());
    }

    vector<shared_ptr<Token>> tokens;
};


TEST_F(ParserTest, ParserTest_Dec_Test) {
    decltype(parse(tokens)) pAST;
    // simple int assignment
    tokens = {
            newToken(Token::KEYWORD, "val"),
            newToken(Token::ID, "i"),
            newToken(Token::OPERATOR, "="),
            newToken(Token::INT, "1"),
            newToken(Token::ID, "+"),
            newToken(Token::INT, "1"),
            newToken(Token::OPERATOR,";")
    };

    pAST = parse(tokens);
}

TEST_F(ParserTest, Let_Test) {
    decltype(parse(tokens)) pAST;
    // simple int assignment
    tokens = {
            newToken(Token::INT, "1"),
            newToken(Token::ID, "+"),
            newToken(Token::INT, "1"),
            newToken(Token::OPERATOR,";")
    };

    pAST = parse(tokens);
}

TEST_F(ParserTest, ParserTest_ASTParse_Test) {
    decltype(parse(tokens)) pAST;
    // simple int assignment
    tokens = {
            newToken(Token::KEYWORD, "val"),
            newToken(Token::ID, "###"),
            newToken(Token::OPERATOR, ":"),
            newToken(Token::ID, "int"),
            newToken(Token::OPERATOR, "="),
            newToken(Token::INT, "2"),
            newToken(Token::OPERATOR,";")
    };

    pAST = parse(tokens);
}
TEST_F(ParserTest, ParserTest_ASTParse__and_Test2) {
    decltype(parse(tokens)) pAST;
    // simple int assignment
    tokens = {
            newToken(Token::KEYWORD, "val"),
            newToken(Token::ID, "i"),
            newToken(Token::OPERATOR, ":"),
            newToken(Token::ID, "int"),
            newToken(Token::OPERATOR, "="),
            newToken(Token::INT, "42"),
            newToken(Token::OPERATOR, ";"),
    };

    pAST = parse(tokens);
}



TEST_F(ParserTest, ParserTest_CodeChecking_Test) {
    const char *inputStr;
    istringstream iss;
    Scanner scanner(iss);
    decltype(scanner.scan()) tokens;
    decltype(parse(scanner.scan())) pAST;

    // type checking
    inputStr = "val i: int = 0.2;";
    iss = istringstream(inputStr);
    tokens = scanner.scan();
    pAST = parse(tokens);
//	auto &&children = ast->getChildren();
}

TEST_F(ParserTest, ParserTest_ParseSimpleValueDec_Test) {
    std::shared_ptr<AST> ast;
    ast = parse
            ({
                     newToken(Token::INT, "42"),
                     newToken(Token::ID, "+"),
                     newToken(Token::REAL, "3.14"),
                     newToken(Token::ID, "*"),
                     newToken(Token::REAL, "8"),
                     newToken(Token::ID, "+"),
                     newToken(Token::REAL, "3.14"),
                     newToken(Token::OPERATOR, ";"),
             });
    ASSERT_EQ(printAST(ast), "42 + 3.14;");


    tokens = {
            newToken(Token::KEYWORD, "val"),
            newToken(Token::ID, "i"),
            newToken(Token::OPERATOR, "="),
            newToken(Token::INT, "42"),
            newToken(Token::OPERATOR, ";"),
    };
    //region val i = 42;
    ast = parse(tokens);
    ASSERT_EQ(printAST(ast), "val i = 42;");
    //endregion



    //region val i : int = 42;
    ast = parse
            ({
                     newToken(Token::KEYWORD, "val"),
                     newToken(Token::ID, "i"),
                     newToken(Token::OPERATOR, ":"),
                     newToken(Token::ID, "int"),
                     newToken(Token::OPERATOR, "="),
                     newToken(Token::INT, "42"),
                     newToken(Token::OPERATOR, ";"),
             });
    ASSERT_EQ(printAST(ast), "val i : int = 42;\n");
    //endregion

    //region val i : int = 42 : int;
    ast = parse
            ({
                     newToken(Token::KEYWORD, "val"),
                     newToken(Token::ID, "i"),
                     newToken(Token::OPERATOR, ":"),
                     newToken(Token::ID, "int"),
                     newToken(Token::OPERATOR, "="),
                     newToken(Token::INT, "42"),
                     newToken(Token::OPERATOR, ":"),
                     newToken(Token::ID, "int"),
                     newToken(Token::OPERATOR, ";"),
             });
    ASSERT_EQ(printAST(ast), "val i : int = 42 : int;");
    //endregion

    //region val i = 42 : int;
    ast = parse
            ({
                     newToken(Token::KEYWORD, "val"),
                     newToken(Token::ID, "i"),
                     newToken(Token::OPERATOR, "="),
                     newToken(Token::INT, "42"),
                     newToken(Token::OPERATOR, ":"),
                     newToken(Token::ID, "int"),
                     newToken(Token::OPERATOR, ";"),
             });
    ASSERT_EQ(printAST(ast), "val i = 42 : int;");
    //endregion

    //region val i : int = 42 + 3.14 * 2.718 - 0 : int;
    ast = parse
            ({
                     newToken(Token::KEYWORD, "val"),
                     newToken(Token::ID, "i"),
                     newToken(Token::OPERATOR, "="),
                     newToken(Token::INT, "42"),
                     newToken(Token::OPERATOR, ":"),
                     newToken(Token::ID, "int"),
                     newToken(Token::ID, "+"),
                     newToken(Token::REAL, "3.14"),
                     newToken(Token::ID, "*"),
                     newToken(Token::REAL, "3.14"),
                     newToken(Token::ID, "-"),
                     newToken(Token::INT, "0"),
                     newToken(Token::OPERATOR, ":"),
                     newToken(Token::ID, "int"),
                     newToken(Token::OPERATOR, ";"),
             });
    ASSERT_EQ(printAST(ast), "val i : int = 42 + 3.14 * 2.718 - 0;");
    //endregion

//    auto valDec =
//            dyn_cast<ValueDecAST>(ast);
//    ASSERT_TRUE(valDec);
//
//    auto destValBind = dyn_cast<DestructuringValBindAST>(
//            valDec->getValBind());
//    ASSERT_TRUE(destValBind);
//
//    auto varPat = dyn_cast<VariablePatAST>(
//            destValBind->getPat());
//    ASSERT_TRUE(varPat);
//
//    auto alphaId = dyn_cast<AlphanumericIdAST>(
//            varPat->getId());
//    ASSERT_TRUE(alphaId);
//    ASSERT_EQ(alphaId->get(), "i");
//
//    auto conExp = dyn_cast<ConstantExpAST>(destValBind->getExp());
//    ASSERT_TRUE(conExp);
//
//    auto intCon = dyn_cast<IntConAST>(conExp->getCon());
//    ASSERT_TRUE(intCon);
//    ASSERT_EQ(intCon->get(), 42);
}
