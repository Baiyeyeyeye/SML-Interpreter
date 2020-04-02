#include <iostream>
#include "Error.h"

using namespace std;

//Error::~Error() {
//    emit();
//}
//
//void Error::emit() {
//    if (!mEmitted) {
//        mEmitted = true;
//        cerr << what() << endl;
//    }
//}
//
//std::string Error::what() {
//    return std::move(mWhat);
//}
//
//Error::Error(std::string what) : mWhat(std::move(what)) {
//
//}
//
//TokenError::TokenError(const Token &token, Reason reason)
//        : mToken(token), mReason(reason) {
//
//}
//
//string TokenError::what() {
//    const char *reasonStr{};
//    switch (mReason) {
//        case INVALID_ESCAPE:
//            reasonStr = "Invalid escape";
//            break;
//    }
//    return "";
//}

void *Error(const std::string &what) {
    cerr << what << endl;
    return nullptr;
}
