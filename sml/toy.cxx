#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
    tok_eof = -1,
    // commands

    tok_def = -2,
    tok_extern = -3,

    // primary

    tok_identifier = -4,
    tok_number = -5,

    tok_keyword = -6,
    tok_int = -7,
    tok_real = -8,
    tok_bool = -9,
    tok_char = -10,
    tok_string = -11
};

static auto const &getKeywords() {
    static const std::unordered_set<std::string> set = {
            "abstype", "and", "andalso", "as", "case", "datatype", "do", "else",
            "end", "exception", "fn", "fun", "handle", "if", "in", "infix",
            "infixr", "let", "local", "nonfix", "of", "op", "open", "orelse",
            "raise", "rec", "then", "type", "val", "with", "withtype", "while",
    };
    return set;
}

static bool isKeyword(const std::string &str) {
    return getKeywords().count(str) > 0;
}

static auto const &getIdentifierSymbols() {
    static std::unordered_set<char> set;
    if (set.empty()) {
        for (auto &&c : std::string_view(R"(!%&$#+-/:<=>?@\~`^|*)")) {
            set.emplace(c);
        }
    }
    return set;
}

static auto const &getReservedOperators() {
    static std::unordered_set<std::string> set = {
            "+", "-", "*", "/", "=", "%", "#", "~",
            "<", ">", "<=", ">=", "<>",
            "(", ")", "[", "]", "{", "}", ",", ":", ";", "...", "_", "|", "=",
            "=>", "->", "#",
    };
    return set;
}

static bool isReservedOperator(const std::string &str) {
    return getReservedOperators().count(str) > 0;
}

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number
static int IntNum;
static std::string RealNum;
static bool BoolVal;
static char charVal;
static std::string strVal;

// on entering this function, we have already consumed (*.
static void consumeComment() {
    int c;
    while (c = getchar(), c != '*' && c != '(') {
    }
    if (c == '*') {
        c = getchar();
        // if we got a *), then consuming comment completed. otherwise continue
        // consuming.
        if (c != ')') {
            consumeComment();
        }
        return;
    } else if (c == '(') {
        c = getchar();
        // if we got another (*, start consuming the inner comment. when we come
        // back from inner comment, continue consuming the outer comment.
        if (c == '*') {
            consumeComment();
        }
        consumeComment();
        return;
    } else if (c == EOF) {
        return;
    }
}

/// gettok - Return the next token from standard input.
static int gettok() {
    using namespace std;
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar)) {
        LastChar = getchar();
    }

    // test for comment consuming
    if (LastChar == '(') {
        // found the comment beginning
        if (auto c = getchar(); c == '*') {
            consumeComment();
            LastChar = ' ';
            return gettok();
        } else {
            // not found, put back c.
            ungetc(c, stdin);
        }
    }


    // is number constant
    if (isdigit(LastChar)) {
        // eat number
        //分为 int 和 real
        //int部分
        int tempNum = LastChar - '0';
        while (LastChar = getchar(), isdigit(LastChar)) {
            tempNum = tempNum * 10 + static_cast<int>(LastChar);
        }
        if (LastChar == '.') {
            //判断是否为real
            LastChar = getchar();
            if (!isdigit(LastChar)) {
                //小数点后不是数字,则报错
                return -100;
            } else {
                RealNum =
                        std::to_string(tempNum) + "." + std::to_string(LastChar);
                bool hasE = false;
                while (LastChar = getchar(), isdigit(LastChar) ||
                                             LastChar == 'E' ||
                                             LastChar == 'e') {
                    //含e的情况
                    if ((LastChar == 'E' || LastChar == 'e') && !hasE) {
                        hasE = true;
                        RealNum += "e";
                    } else if ((LastChar == 'E' || LastChar == 'e') && hasE) {
                        //报错
                        return -100;
                    }
                    if (LastChar == 'E' || LastChar == 'e' || hasE) {
                        //此处接着上面写,E后面可以接~或者整数
                        LastChar = getchar();
                        if (LastChar != '~' || !isdigit(LastChar)) {
                            //e后面接了其他东西，就报错
                            //报错
                            return -100;
                        } else {
                            if (LastChar == '~') {
                                RealNum += "-";
                                LastChar = getchar();
                            }
                            while (LastChar = getchar(), isdigit(LastChar)) {
                                RealNum += LastChar;
                            }
                            return tok_real;
                        }
                    }

                    //不含e的情况
                    RealNum += to_string(LastChar);
                }
                return tok_real;
            }
        } else {
            IntNum = tempNum;
            return tok_int;
        }
    }



    // is identifier alphanumeric
    if (isalpha(LastChar) || LastChar == '\'') {
        IdentifierStr = static_cast<char>(LastChar);
        while (LastChar = getchar(),
                isalnum(LastChar) || LastChar == '\'' || LastChar == '_') {
            IdentifierStr += static_cast<char>(LastChar);
        }
        if (isKeyword(IdentifierStr)) {
            return tok_keyword;
        } else if (IdentifierStr == "true") {
            BoolVal = true;
            return tok_bool;
        } else if (IdentifierStr == "false") {
            BoolVal = false;
            return tok_bool;
        }
        return tok_identifier;
    }

    // is identifier symbolic
    if (auto &&idSymbols = getIdentifierSymbols(); idSymbols.count(LastChar)) {
        IdentifierStr = static_cast<char>(LastChar);
        while (LastChar = getchar(),
                idSymbols.count(LastChar)) {
            IdentifierStr += static_cast<char>(LastChar);
        }
        if (isReservedOperator(IdentifierStr)) {
            return tok_identifier;
        }
        return tok_identifier;
    }

    //is char
    if (LastChar == '#') {
        if (auto c = getchar(); c == '\"') {
            //此处未考虑转义，后续进行添加
            //char内容为一个字母或者数字
            if (LastChar = getchar(), isdigit(LastChar) || isalpha(LastChar)) {
                charVal = static_cast<char>(LastChar);
                if (LastChar = getchar(), LastChar == '\"') {
                    return tok_char;
                } else {
                    //此处要报错，char内容有多个字符
                    return -100;
                }
            } else {
                //此处要报错,char内容不为一个数字或者字母
                return -100;
            }
        } else {
            //此处要报错,不满足char条件#"a"
            return -100;
        }
    }

    //is string
    if (LastChar == '\"') {
        strVal = static_cast<char>(LastChar);
        //此处未考虑转义
        while (LastChar = getchar(), LastChar != '\"') {
            strVal += static_cast<char>(LastChar);
        }
        if (LastChar == '\"') {
            return tok_string;
        }

        //此处要报错，如果不满足string类型的条件,”abc"
        return -100;

    }
    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF) {
        return tok_eof;
    }

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}
//static std::string IdentifierStr; // Filled in if tok_identifier
//static double NumVal;             // Filled in if tok_number
//
///// gettok - Return the next token from standard input.
//static int gettok() {
//  static int LastChar = ' ';
//
//  // Skip any whitespace. 跳过空格
//  while (isspace(LastChar))
//    LastChar = getchar();
//
//  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
//    IdentifierStr = LastChar;
//    while (isalnum((LastChar = getchar())))  // 读取直到非空格
//      IdentifierStr += LastChar;
//
//    if (IdentifierStr == "def")
//      return tok_def;
//    if (IdentifierStr == "extern")
//      return tok_extern;
//    return tok_identifier;
//  }
//
//  //下面几个if判断都是返回不同的类型判断，与上面的判断是并列的
//  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
//    std::string NumStr;
//
//    do {
//      NumStr += LastChar;
//      LastChar = getchar();
//    } while (isdigit(LastChar) || LastChar == '.');
//
//    NumVal = strtod(NumStr.c_str(), nullptr);
//    return tok_number;
//  }
//
//  if (LastChar == '#') {
//    // Comment until end of line.
//    do
//      LastChar = getchar();
//    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
//
//    if (LastChar != EOF)
//      return gettok();
//  }
//
//  // Check for end of file.  Don't eat the EOF.
//  if (LastChar == EOF)
//    return tok_eof;
//
//  // Otherwise, just return the character as its ascii value.
//  int ThisChar = LastChar;
//  LastChar = getchar();
//  return ThisChar;
//}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

namespace {

    namespace MyAST {
        class ExprAST {
        public:
            virtual ~ExprAST() = default;
        };

        class IntExprAST : public ExprAST {
            int _int;
        public:
            explicit IntExprAST(int v) : _int(v) {}
        };

        class RealExprAST : public ExprAST {
            double _real;
        public:
            explicit RealExprAST(double v) : _real(v) {}
        };

        class BoolExprAST : public ExprAST {
            bool _bool;
        public:
            explicit BoolExprAST(bool v) : _bool(v) {}
        };

        class CharExprAST : public ExprAST {
            char _char;
        public:
            explicit CharExprAST(char v) : _char(v) {}
        };

        class StringExprAST : public ExprAST {
            std::string _string;
        public:
            explicit StringExprAST(std::string v) : _string(std::move(v)) {}
        };

        class FunExprAST : public ExprAST {

        };

        class ValExprAST : public ExprAST {

        };

        class VariableExprAST : public ExprAST {

        };
    }

    /// ExprAST - Base class for all expression nodes.
    class ExprAST {
    public:
        virtual ~ExprAST() = default;
    };

/// NumberExprAST - Expression class for numeric literals like "1.0".
    class NumberExprAST : public ExprAST {
        double Val;

    public:
        NumberExprAST(double Val) : Val(Val) {
        }
    };

/// VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST : public ExprAST {
        std::string Name;

    public:
        VariableExprAST(const std::string &Name) : Name(Name) {
        }
    };

/// BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                      std::unique_ptr<ExprAST> RHS)
                : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {
        }
    };

/// CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

    public:
        CallExprAST(const std::string &Callee,
                    std::vector<std::unique_ptr<ExprAST>> Args)
                : Callee(Callee), Args(std::move(Args)) {
        }
    };

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
    class PrototypeAST {
        std::string Name;
        std::vector<std::string> Args;

    public:
        PrototypeAST(const std::string &Name, std::vector<std::string> Args)
                : Name(Name), Args(std::move(Args)) {
        }

        const std::string &getName() const {
            return Name;
        }
    };

/// FunctionAST - This class represents a function definition itself.
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                    std::unique_ptr<ExprAST> Body)
                : Proto(std::move(Proto)), Body(std::move(Body)) {
        }
    };

} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static int CurTok;

static int getNextToken() {
    return CurTok = gettok();
}

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
    if (!isascii(CurTok)) {
        return -1;
    }

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) {
        return -1;
    }
    return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = llvm::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();
    if (!V) {
        return nullptr;
    }

    if (CurTok != ')') {
        return LogError("expected ')'");
    }
    getNextToken(); // eat ).
    return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier.

    if (CurTok != '(') { // Simple variable ref.
        return llvm::make_unique<VariableExprAST>(IdName);
    }

    // Call.
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            } else {
                return nullptr;
            }

            if (CurTok == ')') {
                break;
            }

            if (CurTok != ',') {
                return LogError("Expected ')' or ',' in argument list");
            }
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
        int TokPrec = GetTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec) {
            return LHS;
        }

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS) {
            return nullptr;
        }

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }

        // Merge LHS/RHS.
        LHS = llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                                               std::move(RHS));
    }
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) {
        return nullptr;
    }

    return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != tok_identifier) {
        return LogErrorP("Expected function name in prototype");
    }

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(') {
        return LogErrorP("Expected '(' in prototype");
    }

    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier) {
        ArgNames.push_back(IdentifierStr);
    }
    if (CurTok != ')') {
        return LogErrorP("Expected ')' in prototype");
    }

    // success.
    getNextToken(); // eat ')'.

    return llvm::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken(); // eat def.
    auto Proto = ParsePrototype();
    if (!Proto) {
        return nullptr;
    }

    if (auto E = ParseExpression()) {
        return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr",
                                                     std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken(); // eat extern.
    return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
    if (ParseDefinition()) {
        fprintf(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (ParseExtern()) {
        fprintf(stderr, "Parsed an extern\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {
            case tok_eof:
                return;
            case ';': // ignore top-level semicolons.
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}
