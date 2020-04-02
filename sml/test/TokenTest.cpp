#include <iostream>
#include <memory>
#include <tuple>
#include "gtest/gtest.h"
#include "Token.h"

using namespace std;

TEST(test_case_name_example, test_name_example) {
    Token token(1, 1, Token::INT, "1");
    EXPECT_EQ(token.getInt(), 1);
}

class TokenValueTest : public ::testing::Test {

};

TEST_F(TokenValueTest, TokenValueTest_SimpleAllBasicType_Test) {
    unique_ptr<Token> token;
    auto new_token = [&](auto &&type, auto &&val) {
        token = make_unique<Token>(1, 1, type, val);
    };
    new_token(Token::INT, "0x123abc");
    EXPECT_EQ(token->getInt(), 0x123abc);
    new_token(Token::STRING, R"("123qwe\n\b\123")");
    EXPECT_EQ(token->getString(), "123qwe\n\b\x7b");
    new_token(Token::CHAR, R"(#"\t")");
    EXPECT_EQ(token->getChar(), '\t');
    new_token(Token::REAL, "1.2E~3");
    EXPECT_DOUBLE_EQ(token->getReal(), 1.2e-3);
    new_token(Token::BOOL, "true");
    EXPECT_TRUE(token->getBool());
}

TEST_F(TokenValueTest, TokenValueTest_IntValue_Test) {
    // simple ints
    for (int i = 0; i < 10000; ++i) {
        Token token(1, 1, Token::INT, to_string(i));
        EXPECT_EQ(token.getInt(), i);
    }

    // hex ints
    string hex;
    string hexLetters = "0123456789abcdef";
    for (auto i : hexLetters) {
        for (auto j : hexLetters) {
            string letters = "0x"s + i + j;
            Token token(1, 1, Token::INT, letters);
            EXPECT_EQ(token.getInt(), stoi(letters, nullptr, 16));
        }
    }
}
