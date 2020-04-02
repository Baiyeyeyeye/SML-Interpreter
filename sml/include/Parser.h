#pragma once

#include <map>
#include <memory>
#include <vector>

#ifndef APPLY_ALL

#include "AST/ASTApplyMacro.h"
#include "Token.h"
#include "Interpreter.h"
#include "Symbol/SymbolTable.h"


#endif

//region ast forward declarations
#define APPLY(CLASS) class CLASS;

APPLY_ALL

#undef APPLY
//endregion


class Interpreter;

class Parser {
public:
    explicit Parser(Interpreter *interpreter);

    explicit Parser(Scanner &scanner);

    std::shared_ptr<AST> parse();

private:
    Interpreter *interpreter{};
    std::shared_ptr<Token> curTok; //Store the current token
    std::string tokVal;
    Token::Type tokType;
    int tokRow;
    int tokCol;


    int tokPos = -1;
    std::vector<std::shared_ptr<Token>> tokBuf;
    void eat(); //eat current Token
    void getNextToken();
    void syntaxErrMsg(std::string msg);
    bool isCon(); //判断是否为Con
    std::shared_ptr<ConAST> genConsAST();//返回ConAST,并且吃掉这个Token

    bool isOperator();

    int getIdPrecedence();//获取id优先级

    bool isInfixFunction(std::shared_ptr<Token> tok); //判断当前id是不是中缀的function

    void genErrMsg();

    bool followExp(const std::shared_ptr<Token>& tok);

/*******************************************************************************
parse Program
*******************************************************************************/
    std::shared_ptr<AST> parseProg(); //Root node of the parsing tree
    std::shared_ptr<DecAST> parseDec();

/*******************************************************************************
parse Declaration
*******************************************************************************/
    bool isDec();//根据当前token决定是否进入dec的判断，后续可拓展

    std::shared_ptr<ValueDecAST> parseValueDec();

    std::shared_ptr<FunctionDecAST> parseFunDec();

    std::shared_ptr<ValBindAST> parseValbind();

    std::shared_ptr<FunBindAST> parseFunbind();

    std::shared_ptr<FunMatchAST> parseFunmatch();

    std::shared_ptr<NonFixFunMatchAST> parseNonfixFunmatch();

    std::shared_ptr<InfixFunMatchAST> parseInfixFunmatch();

    std::shared_ptr<LocalDecAST> parseLocalDec();

    std::shared_ptr<LeftAssociativeInfixDecAST> parseLeftAssociativeDec();

    std::shared_ptr<RightAssociativeInfixDecAST> parseRightAssociativeDec();

    std::shared_ptr<NonfixDecAST> parseNonfixDec();


    std::shared_ptr<TypeDecAST> parseTypeDec();

    std::shared_ptr<TypBindAST> parseTypBind();

    /*******************************************************************************
parse Pattern
*******************************************************************************/
    bool isPat();

    std::shared_ptr<PatAST> parsePat();

    /*******************************************************************************
parse Expression
*******************************************************************************/
    bool isExp();

    bool isNonfixId();

    bool shouldRetLHSexp();

    std::shared_ptr<ApplicationExpAST> parseApplicationExp();

    std::shared_ptr<ExpAST> parseExp();

    std::shared_ptr<ExpAST> parsePrimaryExp();

    std::shared_ptr<ExpAST> parseBinOpRHS(int ExprPrec, std::shared_ptr<ExpAST> LHS); //用于算符优先

    std::shared_ptr<ConstantExpAST> parseConExp();

    std::shared_ptr<ValueOrConstructorIdentifierExpAST> parseValueOrConstructorIdentifierExp();

    std::shared_ptr<ListExpAST> parseListExp();

    std::shared_ptr<ConditionalExpAST> parseConditionalExp();

    std::shared_ptr<IterationExpAST> parseIterationExp();

    std::shared_ptr<LocalDeclarationExpAST> parseLocalDeclarationExp();

    std::shared_ptr<FunctionExpAST> parseFuntionExp();

    std::shared_ptr<MatchAST> parseMatch();

    std::shared_ptr<ExpAST> parseParenthesesExp();
 /*******************************************************************************
parse Identifier
*******************************************************************************/
    std::shared_ptr<IdAST> parseId();

    std::shared_ptr<LongIdAST> parseLongId();

    std::shared_ptr<VarAST> parseVar();

    std::shared_ptr<LabAST> parseLab();


/*******************************************************************************
parse Type
*******************************************************************************/
    std::shared_ptr<TypAST> parseTyp();

};

#undef APPLY_ALL
