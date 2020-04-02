#pragma once

#include <memory>
#include <string>

//class Token;
//
//class Error {
//public:
//    virtual ~Error();
//
//    virtual std::string what();
//
//    void emit();
//
//    Error() = default;
//
//    explicit Error(std::string what);
//
//private:
//    bool mEmitted{};
//    std::string mWhat;
//};
//
//class TokenError : public Error {
//public:
//    enum Reason {
//        INVALID_ESCAPE
//    };
//
//    explicit TokenError(const Token &token, Reason reason);
//
//    std::string what() override;
//
//    ~TokenError() override = default;
//
//private:
//    const Token &mToken;
//    const Reason mReason;
//};

void *Error(const std::string &what);
