#include <memory>
#include <utility>
#include "Token.h"
#include "TokenImpl.h"

using namespace std;

Token::Token(int row, int column, int type, std::string value)
        : mImpl(make_shared<Impl>(row, column, type, std::move(value))) {
    switch (type) {
        case INT:
            mImpl->initInt();
            break;
        case STRING:
            mImpl->initString();
            break;
        case CHAR:
            mImpl->initChar();
            break;
        case REAL:
            mImpl->initReal();
            break;
        case BOOL:
            mImpl->initBool();
            break;
        default:
            break;
    }
}

int Token::getRow() const {
    return mImpl->row;
}

int Token::getColumn() const {
    return mImpl->column;
}

Token::Type Token::getType() const {
    return static_cast<Token::Type>(mImpl->type);
}

const string &Token::getValue() const {
    return mImpl->value;
}

double Token::getReal() const {
    return mImpl->plain<double>();
}

bool Token::getBool() const {
    return mImpl->plain<bool>();
}

int Token::getInt() const {
    return mImpl->plain<int>();
}

char Token::getChar() const {
    return mImpl->plain<char>();
}

const std::string &Token::getString() const {
    return mImpl->plain<string>();
}
