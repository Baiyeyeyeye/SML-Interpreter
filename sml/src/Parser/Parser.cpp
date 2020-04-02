#include "AST/AST.h"
#include "Error.h"
//#include "Interpreter.h"
#include "Parser.h"
#include "Token.h"
#include <unordered_set>

Parser::Parser(Scanner &scanner) {

}

Parser::Parser(Interpreter *interpreter) : interpreter(interpreter) {

}

//Produce error meessage
void Parser::syntaxErrMsg(std::string msg) {
    std::string headMsg = "Syntax Error: row " + std::to_string(tokRow) + ", column ";
    headMsg += std::to_string(tokCol);
    headMsg +=  ": '";
    headMsg += msg;
    headMsg += "' is missing";
    Error(headMsg);
}

//Read a token from user input to Token-Buffer
void Parser::getNextToken() {
    auto tok = Interpreter::ParserInterface::getNextToken(this->interpreter);
    tokBuf.push_back(std::move(tok));
    //将tok存到缓冲区
}

//Get a token from Token-Buffer
void Parser::eat() {
    if(tokPos + 1 < tokBuf.size()){
        curTok = std::move(tokBuf[++tokPos]);
    }else{
        getNextToken();
        curTok = std::move(tokBuf[++tokPos]);
    }
    tokVal = curTok->getValue();
    tokType = curTok->getType();
    tokRow = curTok->getRow();
    tokCol = curTok->getColumn();
}

bool Parser::isOperator() {
    return (tokVal == ";" ||
            tokVal == "," ||
            tokVal == "=" ||
            tokVal == "(" ||
            tokVal == ")" ||
            tokVal == "=>" ||
            tokVal == "end" ||
            tokVal == "in");
}

//Judge current token
bool Parser::isDec() {
    return (tokVal == "fun" ||
            tokVal == "val" ||
            tokVal =="nonfix" ||
            tokVal =="infix" ||
            tokVal =="infixr" ||
            tokVal == "local" ||
            tokVal == "type");
}

bool Parser::isExp() {
    return (isCon()||
            tokType == Token::ID||
            tokVal == "fun" ||
            tokVal == "val" ||
            tokVal =="nonfix" ||
            tokVal =="infix" ||
            tokVal =="infixr" ||
            tokVal == "fn" ||
            tokVal == "while" ||
            tokVal == "if" ||
            tokVal == "(" ||
            tokVal == "let" ||
            tokVal == "#" ||
            tokVal == "[");
}

bool Parser::isPat() {
    return (tokVal == "_" ||
            (tokType == Token::ID and !isOperator()) ||
            isCon() ||
            tokVal == "(" ||
            tokVal =="[" ||
            tokVal == "}"||
            tokVal == "op"
    );
}

//The interface exposed to Interpreter;
std::shared_ptr<AST> Parser::parse() {
    eat();
    std::shared_ptr<AST> result  = parseProg();
    return result;
}


//The entry of the program
std::shared_ptr<AST> Parser::parseProg() {
    std::shared_ptr<AST> root;
    if(isDec()){
        std::shared_ptr<DecAST> decAST(parseDec());
        if(decAST == nullptr) return nullptr;
        root = std::move(decAST);
    }else if(isExp() ){
        //Top-Level Expression
        std::shared_ptr<ExpAST> decAST(parseExp());
        if(decAST == nullptr) return nullptr;
        root = std::move(decAST);
    }

    if(root == nullptr){
        Error("Error occurs!");
    }
    if(tokVal == ";"){
        return root;
    } else{
        syntaxErrMsg("; is missing");
        return nullptr;
    }
}


//parse constant to ConAST
std::shared_ptr<ConAST> Parser::genConsAST() {
    std::shared_ptr<ConAST> result = nullptr;
    switch (tokType){
        case Token::INT:
            result.reset(new IntConAST(curTok->getInt()));
            break;
        case Token::STRING:
            result.reset(new StringConAST(curTok->getString()));
            break;
        case Token::CHAR:
            result.reset(new CharConAST(curTok->getChar()));
            break;
        case Token::REAL:
            result.reset(new FloatConAST(curTok->getReal()));
            break;
        case Token::BOOL:
            result.reset(new BoolConAST(curTok->getBool()));
            break;
    }
    eat();
    return result;
}

/************************************
 * parse Declaraction
 * @return std::shared_ptr<DecAST>
 ***********************************/

std::shared_ptr<DecAST> Parser::parseDec() {
    std::shared_ptr<DecAST> decAST = nullptr;
    if (tokVal == "val") {
        //valDec
        decAST = std::move(parseValueDec());
    } else if (tokVal == "fun") {
        decAST = std::move(parseFunDec());
    } else if(tokVal == "type"){
        std::shared_ptr<TypeDecAST> typDec;
        decAST = std::move(parseTypeDec());
    } else if(tokVal == "local") {
        decAST = std::move(parseLocalDec());
    }else if(tokVal == "infix" ) {
        decAST = std::move(parseLeftAssociativeDec());
    }else if(tokVal == "infixr") {
        decAST = std::move(parseRightAssociativeDec());
    }else if(tokVal == "nonfix") {
        decAST = std::move(parseNonfixDec());
    }else {
        syntaxErrMsg("dec");
        return nullptr;
    }
    return decAST;

}
//Parse valueDec : val valbind
std::shared_ptr<ValueDecAST> Parser::parseValueDec() {
    std::shared_ptr<ValueDecAST> valDec;
    eat();
    std::shared_ptr<ValBindAST> vbind(parseValbind()); //接管返回的ValbindAST
    if (vbind == nullptr)return nullptr;
    valDec.reset(new ValueDecAST(vbind));
    return valDec;
}

std::shared_ptr<ValBindAST> Parser::parseValbind() {
    std::shared_ptr<DestructuringValBindAST> vbind;
    std::shared_ptr<PatAST> pat(parsePat());
    if(pat == nullptr) return nullptr;
    if (tokVal != "=") {
        syntaxErrMsg("= is missing");
        return nullptr;
    }
    eat();
    std::shared_ptr<ExpAST> exp(parseExp());
    if (exp == nullptr) return nullptr;
    vbind.reset(new DestructuringValBindAST(pat, exp));
    if (tokVal != "and") return vbind;
    //recursive invoking parseVbind to generate and valbind
    eat();
    vbind->setAndValBind(parseValbind());
    return vbind;
}

std::shared_ptr<FunctionDecAST> Parser::parseFunDec() {
    std::shared_ptr<FunctionDecAST> funDec;
    eat();
    std::shared_ptr<FunBindAST> fbind(parseFunbind());
    if (fbind == nullptr)return nullptr;
    funDec.reset(new FunctionDecAST(fbind));
    return funDec;
}

std::shared_ptr<FunBindAST> Parser::parseFunbind() {
    std::shared_ptr<FunBindAST> fbind;
    std::shared_ptr<FunMatchAST> fmatch(parseFunmatch());
    if(fmatch == nullptr)return nullptr;
    fbind.reset(new FunBindAST(std::move(fmatch)));
    if(tokVal != "and") return fbind;
    eat();
    fbind->setAndFunBind(parseFunbind());
    return fbind;
}

std::shared_ptr<FunMatchAST> Parser::parseFunmatch() {
    std::shared_ptr<FunMatchAST> funMatch;
    if (tokType == Token::ID) {
        //Nonfix
        funMatch = std::move(parseNonfixFunmatch());
    } else if (isPat()) {
        //Infix
        funMatch = std::move(parseInfixFunmatch());
    } else {
        syntaxErrMsg(tokVal + "is invalid");
        return nullptr;
    }
    return funMatch;

}

std::shared_ptr<NonFixFunMatchAST> Parser::parseNonfixFunmatch() {
    std::shared_ptr<NonFixFunMatchAST> nonfixFMatch;
    std::shared_ptr<IdAST> id(parseId());
    std::vector<std::shared_ptr<PatAST>> pats;
    std::shared_ptr<TypAST> typ;
    while (true) {
        pats.push_back(parsePat());
        if (!isPat()) break;
    }
    if (tokVal == ":") {
        eat();
        typ = std::move(parseTyp());
    }
    if (tokVal == "=") {
        eat();
    } else {
        syntaxErrMsg("= is missing");
        return nullptr;
    }
    std::shared_ptr<ExpAST> exp(parseExp());
    if(exp == nullptr) return nullptr;
    nonfixFMatch.reset(new NonFixFunMatchAST(id, pats, exp, typ));
    if (tokVal == "|") {
        eat();
        nonfixFMatch->setOrFunMatch(parseFunmatch());
    }
    return nonfixFMatch;
}

std::shared_ptr<InfixFunMatchAST> Parser::parseInfixFunmatch() {
    std::shared_ptr<InfixFunMatchAST> infixFMatch;
    std::shared_ptr<PatAST> pat1;
    std::shared_ptr<PatAST> pat2;
    pat1 = std::move(parsePat());
    if (tokType != Token::ID) {
        syntaxErrMsg(tokVal + "is an invalid id");
        return nullptr;
    }
    std::shared_ptr<IdAST> id(parseId());
    pat2 = std::move(parsePat());
    if(id == nullptr or pat1 == nullptr or pat2== nullptr) return nullptr;
    std::shared_ptr<TypAST> typ;
    if (tokVal == ":") {
        eat();
        typ = std::move(parseTyp());
    }
    if (tokVal != "=") {
        syntaxErrMsg("= is missing");
        return nullptr;
    }
    eat();
    std::vector<std::shared_ptr<PatAST>> pats{std::move(pat1), std::move(pat2)};
    std::shared_ptr<ExpAST> exp(parseExp());
    if(exp == nullptr)return nullptr;
    infixFMatch.reset(new InfixFunMatchAST(id, pats, exp, typ));
    if (tokVal == "|") {
        eat();
        infixFMatch->setOrFunMatch(parseFunmatch());
    }
    return infixFMatch;
}

std::shared_ptr<LocalDecAST> Parser::parseLocalDec() {
    std::shared_ptr<LocalDecAST> localDec;
    eat();
    std::shared_ptr<DecAST> dec1(parseDec());
    if(dec1 == nullptr) return nullptr;
    std::vector<std::shared_ptr<DecAST>> decs1;
    decs1.push_back(std::move(dec1)); //压进第一个dec
    if(tokVal == ";"){
        while(true){
            eat();
            decs1.push_back(std::move(parseDec()));
            if(tokVal != ";") break;
        }
    }
    dec1.reset(new SequenceDecAST(decs1));//处理多个dec
    if(tokVal != "in"){
        syntaxErrMsg("'in' is missing");
        return nullptr;
    }
    eat();
    std::shared_ptr<DecAST> dec2(parseDec());
    if(dec2 == nullptr)return nullptr;
    std::vector<std::shared_ptr<DecAST>> decs2;
    decs2.push_back(std::move(dec2)); //压进第一个dec
    if(tokVal == ";"){
        while(true){
            eat();
            decs2.push_back(std::move(parseDec()));
            if(tokVal != ";") break;
        }
    }
    if(!decs2.empty()){
        dec2.reset(new SequenceDecAST(decs2));
    } //处理多个dec
    if(tokVal != "end"){
        genErrMsg();
        return nullptr;
    }
    eat();
    localDec.reset(new LocalDecAST(dec1, dec2));
    return localDec;
}

std::shared_ptr<LeftAssociativeInfixDecAST> Parser::parseLeftAssociativeDec() {
    std::shared_ptr<LeftAssociativeInfixDecAST> leftAss;
    eat();
    int priority = -10;
    if(tokType == Token::INT){
        priority = curTok->getInt();
        eat();
    }
    if(tokType != Token::ID){
        genErrMsg();
        return nullptr;
    }
    std::vector<std::shared_ptr<IdAST>> ids;
    if(priority >= 0){
        while(true){
            SymbolTable::getInstance()->setOperator(tokVal, {SymbolTable::Operator::INFIX,  priority});
            ids.push_back(std::move(parseId()));
            if(tokType!= Token::ID) break;
        }
        leftAss.reset(new LeftAssociativeInfixDecAST(ids));
        return leftAss;
    }else {
        while (true) {
            SymbolTable::getInstance()->setOperator(tokVal, {SymbolTable::Operator::INFIX});
            ids.push_back(std::move(parseId()));
            if (tokType != Token::ID) break;
        }

    }
    leftAss.reset(new LeftAssociativeInfixDecAST(ids));
    return leftAss;
}

std::shared_ptr<RightAssociativeInfixDecAST> Parser::parseRightAssociativeDec() {
    std::shared_ptr<RightAssociativeInfixDecAST> rightAss;
    eat();

    if(tokType != Token::ID){
        genErrMsg();
        return nullptr;
    }
    std::vector<std::shared_ptr<IdAST>> ids;
    while(true){
        SymbolTable::getInstance()->setOperator(tokVal, {SymbolTable::Operator::INFIXR});
        ids.push_back(std::move(parseId()));
        if(tokType!= Token::ID) break;
    }
    rightAss.reset(new RightAssociativeInfixDecAST(ids));
    return rightAss;
}

std::shared_ptr<NonfixDecAST> Parser::parseNonfixDec() {
    std::shared_ptr<NonfixDecAST> nonfixDec;
    this->eat();
    int priority = -10;
    if(tokType == Token::INT){
        priority = curTok->getInt();
        eat();
    }
    if(tokType != Token::ID){
        genErrMsg();
        return nullptr;
    }
    std::vector<std::shared_ptr<IdAST>> ids;
    if(priority >= 0){
        while (true) {
            SymbolTable::getInstance()->setOperator(tokVal, {SymbolTable::Operator::NONFIX,priority});
            ids.push_back(std::move(parseId()));
            if (tokType != Token::ID) break;
        }
    }else {
        while (true) {
            SymbolTable::getInstance()->setOperator(tokVal, {SymbolTable::Operator::NONFIX});
            ids.push_back(std::move(parseId()));
            if (tokType != Token::ID) break;
        }
    }
    nonfixDec.reset(new NonfixDecAST(ids));
    return nonfixDec;
}

/************************************
 * parse Pattern
 * @return std::shared_ptr<PatAST>
 ***********************************/

//todo: consider longid
std::shared_ptr<PatAST> Parser::parsePat() {
    std::shared_ptr<PatAST> patAST = nullptr;
    //判断其他pat
    if(isCon()){
        std::shared_ptr<ConstantPatAST> conPat(new ConstantPatAST(genConsAST()));
        patAST = std::move(conPat);
    }
    else if(this->tokVal == "_"){
        //判断wildcard
        eat();
        patAST.reset(new WildCardPatAST());
    }else if(this->tokVal == "("){
        eat();
        std::shared_ptr tempPat(parsePat());
        if(tokVal == ")"){
            eat();
            patAST = std::move(tempPat);
            return patAST;
        }else if(this->tokVal == ",") {
            std::vector<std::shared_ptr<PatAST>> tuplePats;
            tuplePats.push_back(std::move(tempPat));
            while(true){
                eat();
                tuplePats.push_back(parsePat());
                if(tokVal != ",")break;
            }
            patAST.reset(new TuplePatAST(tuplePats));
            if(tokVal != ")") {
                genErrMsg();
                return nullptr;
            }
            eat();
        }else{
            genErrMsg();
            return nullptr;
        }
    }else if(this->tokVal == "{"){

    }else if(this->tokType == Token::ID){
        if(tokVal[0] == '\'') {
            std::shared_ptr<IdAST> tempIdAST(parseId());
            patAST.reset(new VariablePatAST(tempIdAST));
        }else{
            std::shared_ptr<LongIdAST> longid(parseLongId());
            patAST.reset(new ConstructionPatAST(longid));
        }
    }else if(this->tokVal == "op"){
        eat();
        if(tokType != Token::ID or  this->isNonfixId()){
            genErrMsg();
            return nullptr;
        }
    }
    if(patAST == nullptr) return nullptr;
    if((tokVal != ":") or tokVal == "=") return patAST;

    if(this->tokVal == ":"){
        eat();
        std::shared_ptr<TypAST> tempTypAST(parseTyp());
        if(tempTypAST == nullptr) return nullptr;
        patAST.reset(new TypeAnnotationPatAST(patAST, tempTypAST));
    }else if(this->tokType == Token::ID and tokVal != "="){
        std::shared_ptr<IdAST> tempIdAST(parseId());
        std::shared_ptr<PatAST> tempPat(parsePat());
        if(tempPat != nullptr){
            patAST.reset(new InfixConstructionPatAST(patAST,tempIdAST,tempPat));
        }else{
            //报错
            return nullptr;
        }
    } else if(tokVal == "=") return patAST;


    return patAST;
}

//已完成
std::shared_ptr<TypAST> Parser::parseTyp() {
    std::shared_ptr<TypAST> typ;
    if (tokVal[0] == '\'') {
        std::shared_ptr<VarAST> varAst(new VarAST(tokVal));
        typ.reset(new VariableTypAST(std::move(varAst)));
    } else if (tokType == Token::ID) {
        std::shared_ptr<LongIdAST> longId(parseLongId());
        typ.reset(new ConstructorTypAST(longId));
    } else if (tokVal == "(") {
        eat();
        std::shared_ptr<ParenthesesTypAST> ptyp(new ParenthesesTypAST(parseTyp()));
        if (tokVal != ")") {
            genErrMsg();
            return nullptr;
        }
        eat();
        typ = std::move(ptyp);
    } else {
        genErrMsg();
        return nullptr;
    }
    if (tokVal != "->" and tokVal != "*") return typ;
    if (tokVal == "->") {
        eat();
        std::shared_ptr<FunctionTypAST> ftype(new FunctionTypAST(typ, parseTyp()));
        return ftype;
    } else if (tokVal == "*") {
        //此处遇到阻碍
        std::vector<std::shared_ptr<TypAST>> typs;
        typs.push_back(std::move(typ));//压入第一个元素
        while (true) {
            eat();
            std::shared_ptr<TypAST> nextTyp;
            if (tokVal[0] == '\'') {
                std::shared_ptr<VarAST> varAst(new VarAST(tokVal));
                nextTyp.reset(new VariableTypAST(std::move(varAst)));
            } else if (tokType == Token::ID) {
                std::shared_ptr<LongIdAST> longId(parseLongId());
                nextTyp.reset(new ConstructorTypAST(longId));
            } else if (tokVal == "(") {
                eat();
                std::shared_ptr<ParenthesesTypAST> ptyp(new ParenthesesTypAST(parseTyp()));
                if (tokVal != ")") {
                    genErrMsg();
                    return nullptr;
                }
                eat();
                nextTyp = std::move(ptyp);
            } else {
                genErrMsg();
                return nullptr;
            }
            typs.push_back(nextTyp);
            if (tokVal != "*") {
                return std::shared_ptr<TupleTypAST>(new TupleTypAST(typs));
            }
        }
    }
    return nullptr;
}



std::shared_ptr<ExpAST> Parser::parseExp() {
    std::shared_ptr<ExpAST> LHS = parsePrimaryExp();
    if (LHS == nullptr) return nullptr;
    if(shouldRetLHSexp())
        return LHS;

    if(isInfixFunction(curTok)){
        //中缀运算符；
        return parseBinOpRHS(0, LHS);
    }else if(tokVal == "andalso"){
        eat();
        std::shared_ptr<ExpAST> exp(parseExp());
        if(exp == nullptr) return nullptr;
        return std::shared_ptr<ExpAST>(new ConjunctionExpAST(LHS,exp));
    }else if(tokVal == "orelse"){
        eat();
        std::shared_ptr<ExpAST> exp(parseExp());
        if(exp == nullptr) return nullptr;
        return std::shared_ptr<ExpAST>(new DisjunctionExpAST(LHS,exp));
    }else{
        genErrMsg();
        return nullptr;
    }
}


//EXP-Con
std::shared_ptr<ConstantExpAST> Parser::parseConExp() {
    std::shared_ptr<ConstantExpAST> result(new ConstantExpAST(genConsAST()));
    //在genConAST中已经吃掉
    return result;
}


std::shared_ptr<IdAST> Parser::parseId() {
    //不清楚两种id使用上有无区别，逻辑上分离两种id已完成
    std::shared_ptr<IdAST> idAST = nullptr;
    int distS = tokVal[0] - 'a';
    int distL = tokVal[0] - 'A';
    //把两种Id分开看
    if((distS >= 0 && distS <= 26) || (distL >= 0 && distL <= 26) ){
        idAST.reset(new AlphanumericIdAST(this->tokVal));
    }else{
        idAST.reset(new SymbolicIdAST(this->tokVal));
    }
    eat();
    return idAST;
}





bool Parser::isCon() {
    //已完成
    return (this->tokType == Token::CHAR ||
            this->tokType == Token::REAL ||
            this->tokType == Token::INT ||
            this->tokType == Token::STRING ||
            this->tokType == Token::BOOL);
}



//EXP-longid
std::shared_ptr<ValueOrConstructorIdentifierExpAST> Parser::parseValueOrConstructorIdentifierExp() {
    //已完成
    return std::shared_ptr<ValueOrConstructorIdentifierExpAST>(new ValueOrConstructorIdentifierExpAST(parseLongId()));
}

//EXP-if_then_else
std::shared_ptr<ConditionalExpAST> Parser::parseConditionalExp() {
    //已完成
    //读到if进来
    eat();//吃掉if
    std::shared_ptr<ConditionalExpAST> condiExp;
    std::shared_ptr<ExpAST> c1(parseExp());
    if(c1 == nullptr) return nullptr;
    if(tokVal != "then") {
        syntaxErrMsg("'then' is missing");
        return nullptr;
    }
    eat(); //吃掉then
    std::shared_ptr<ExpAST> c2(parseExp());
    if(c2 == nullptr) return nullptr;
    if(tokVal != "else") {
        syntaxErrMsg("'else' is missing");
        return nullptr;
    }
    eat();//吃掉else
    std::shared_ptr<ExpAST> c3(parseExp());
    if(c3 == nullptr) return nullptr;
    condiExp.reset(new ConditionalExpAST(std::move(c1),std::move(c2), std::move(c3)));
    return condiExp;
}

//EXP-while_do_
std::shared_ptr<IterationExpAST> Parser::parseIterationExp() {
    //读到while进来
    std::shared_ptr<IterationExpAST> itraExp;
    eat();//吃掉while
    std::shared_ptr<ExpAST> c1(parseExp());
    if(c1 == nullptr) return nullptr;
    if(tokVal != "do") {
        syntaxErrMsg("'do' is lost");
    }
    std::shared_ptr<ExpAST> c2(parseExp());
    if(c2 == nullptr) return nullptr;
    itraExp.reset(new IterationExpAST(std::move(c1),std::move(c2)));
    return itraExp;
}

std::shared_ptr<FunctionExpAST> Parser::parseFuntionExp() {
    //已完成
    std::shared_ptr<FunctionExpAST> result = nullptr;
    eat();//吃掉fn
    std::shared_ptr<MatchAST> tempMatch (std::move( parseMatch()));
    if(tempMatch == nullptr) return nullptr;
    result.reset(new FunctionExpAST(tempMatch));

    return result;
}

//match已经完成
std::shared_ptr<MatchAST> Parser::parseMatch() {
    std::shared_ptr<PatAST> patAST(parsePat());
    if(tokVal != "=>"){
        syntaxErrMsg("'=>' is missing");
        return nullptr;
    }
    eat();
    std::shared_ptr<ExpAST> expAST(parseExp());
    if(patAST == nullptr ||  expAST == nullptr) return nullptr;
    std::shared_ptr<MatchAST> firstMatch(new MatchAST(patAST,expAST));
    if(tokVal != "|") return firstMatch;
    std::shared_ptr<MatchAST> curLevel = firstMatch;
    while(true){
        eat();
        std::shared_ptr<PatAST> orPatAST(parsePat());
        if(orPatAST == nullptr) return nullptr;
        if(tokVal != "=>"){
            syntaxErrMsg("'=>' is missing");
            return nullptr;
        }
        eat();
        std::shared_ptr<ExpAST> orExpAST(parseExp());
        if(orExpAST == nullptr) return nullptr;
        std::shared_ptr<MatchAST> tempMatch(new MatchAST(std::move(patAST),std::move(orExpAST)));
        curLevel->setMatch(tempMatch);
        curLevel = tempMatch;
        if(tokVal != "|"){
            return firstMatch;
        }
    }
}



//已完成
int Parser::getIdPrecedence() {
    //查阅符号表，先读出是否为infix，且判断一下结合性
    //再读取一下优先级
    auto symbolTable = SymbolTable::getInstance();
    auto thisOp = symbolTable->getOperator(tokVal);
    if(thisOp == nullptr) return -1;
    auto typ = thisOp->operatorType;
    if(typ != SymbolTable::Operator::INFIX and typ != SymbolTable::Operator::INFIXR){
        return -1;
    }
    int priority = thisOp->priority;
    return priority;
}


//其实是构造f args 实际调用的例子
std::shared_ptr<ApplicationExpAST> Parser::parseApplicationExp() {
    //nonfix Id可以进来
    std::shared_ptr<ApplicationExpAST> app;
    std::shared_ptr<ValueOrConstructorIdentifierExpAST> LHS(parseValueOrConstructorIdentifierExp());
    std::shared_ptr<ExpAST> exp2;

    std::shared_ptr<ApplicationExpAST> tmpRoot(new ApplicationExpAST(std::move(LHS), parsePrimaryExp()));
    while(true){
        if(isExp() and !isInfixFunction(curTok) and !followExp(curTok)){
            tmpRoot.reset(new ApplicationExpAST(std::move(tmpRoot), parsePrimaryExp()));
        }else{
            app = std::move(tmpRoot);
            break;
        }
    }
    return app;
}


//算符优先构造ApplicationAST
std::shared_ptr<ExpAST> Parser::parseBinOpRHS(int ExprPrec, std::shared_ptr<ExpAST> LHS) {
    while (true) {
        int TokPrec = getIdPrecedence();
        if(TokPrec < ExprPrec) return LHS;//除掉nonfix的情况
        auto BinOp = parseId();
        auto RHS = parsePrimaryExp();
        if (!RHS) return nullptr;
        int NextPrec = getIdPrecedence();
        if (TokPrec < NextPrec) {
            RHS = std::move(parseBinOpRHS(TokPrec + 1, std::move(RHS)));
            if (!RHS) return nullptr;
        }
        LHS.reset(new InfixApplicationExpAST(std::move(LHS), std::move(BinOp),  std::move(RHS)));
    }
}

std::shared_ptr<ExpAST> Parser::parsePrimaryExp() {
    std::shared_ptr<ExpAST> frontExp;
    if (isCon()) {
        std::shared_ptr<ConstantExpAST> con(new ConstantExpAST(genConsAST()));
        frontExp  = std::move(con);
    } else if(tokType == Token::ID ){
        getNextToken();
        if(!isExp() or isInfixFunction(tokBuf[tokPos+1]) or followExp(tokBuf[tokPos+1])){
            frontExp = std::move(parseValueOrConstructorIdentifierExp());
            return frontExp;
        }
        //后面处理是否是f a b c 的情况，也就是applicationAST
        auto appRoot = std::move(parseApplicationExp());
        if(appRoot == nullptr)return nullptr;
        frontExp = std::move(appRoot);
    } else if (tokVal == "(") {
        auto pExp = parseParenthesesExp();
        if(pExp == nullptr) return nullptr;
        frontExp = std::move(pExp);
    }else if(tokVal == "["){
        auto listExp = parseListExp();
        if(listExp == nullptr) return nullptr;
        frontExp = std::move(listExp);
    }else if(tokVal == "{"){
        //todo:补全Record的AST
    }else if(tokVal == "#"){
        eat();
        std::shared_ptr<RecordSelectorExpAST> selector(new RecordSelectorExpAST(parseLab()));
        frontExp = std::move(selector);
    }else if(tokVal == "fn") {
        eat();
        std::shared_ptr<FunctionExpAST> funExp(new FunctionExpAST(parseMatch()));
        frontExp = std::move(funExp);
    }else if(tokVal == "if"){
       auto condi = parseConditionalExp();
       if(condi == nullptr) return nullptr;
       frontExp = std::move(condi);
    }else if(tokVal == "while"){
        auto itera = parseIterationExp();
        if(itera == nullptr) return nullptr;
        frontExp = std::move(itera);
    }else if(tokVal == "let"){
       auto localDecExp = parseLocalDeclarationExp();
       if(localDecExp == nullptr)return nullptr;
       frontExp = std::move(localDecExp);
    }else{
        return nullptr;
    }
    if(tokVal == ":") {
        eat();
        std::shared_ptr<TypAST> typ(parseTyp());
        if (typ == nullptr) return nullptr;
        return std::shared_ptr<ExpAST>(new TypeAnnotationExpAST(frontExp, typ));
    }
    return frontExp;
}

bool Parser::isNonfixId() {
    auto symbolTB = SymbolTable::getInstance();
    return FunctionValue::NONFIX == ( (FunctionValue *) symbolTB->getValue(tokVal) )->getOperationType();
}

std::shared_ptr<LabAST> Parser::parseLab() {
    return std::shared_ptr<LabAST>();
}

std::shared_ptr<VarAST> Parser::parseVar() {
    return std::shared_ptr<VarAST>();
}

std::shared_ptr<ExpAST> Parser::parseParenthesesExp() {
    if(tokVal != "(") {
        syntaxErrMsg("(");
        return nullptr;
    }
    eat();
    auto exp = parseExp();
    if(exp == nullptr) return nullptr;
    if(tokVal == ",") {
        //处理TupleExp
        std::vector<std::shared_ptr<ExpAST>> exps;
        exps.push_back(std::move(exp));
        while (true) {
            eat(); //吃掉 ,
            std::shared_ptr<ExpAST> newexp(parseExp());
            if (newexp == nullptr) return nullptr;
            exps.push_back(std::move(newexp));
            if (tokVal != ",") break;
        }
        if (tokVal != ")") {
            genErrMsg();
            return nullptr;
        }
        eat(); //匹配就把)吃掉
        std::shared_ptr<TupleExpAST> tupleExp(new TupleExpAST(exps));
        return  tupleExp;
    }else if(tokVal == ")"){
        eat();
        return exp;
    }else{
        syntaxErrMsg(")");
    }
}

std::shared_ptr<LocalDeclarationExpAST> Parser::parseLocalDeclarationExp() {
    std::shared_ptr<LocalDeclarationExpAST> localDec;
    if (tokVal != "let") {
        syntaxErrMsg("'let' is missing");
        return nullptr;
    }
    eat();
    std::shared_ptr<DecAST> dec(parseDec());
    if (dec == nullptr) return nullptr;
    std::vector<std::shared_ptr<DecAST>> decs;
    decs.push_back(std::move(dec)); //压进第一个dec
    if (tokVal == ";") {
        while (true) {
            eat();
            auto tmpDec = parseDec();
            if(tmpDec == nullptr) return nullptr;
            decs.push_back(std::move(tmpDec));
            if (tokVal != ";") break;
        }
    }
    dec.reset(new SequenceDecAST(std::move(decs)));//处理多个dec
    if (tokVal != "in") {
        syntaxErrMsg("in");
        return nullptr;
    }
    eat();
    std::vector<std::shared_ptr<ExpAST>> exps;
    std::shared_ptr<ExpAST> exp(parseExp());
    if (exp == nullptr) return nullptr;
    exps.push_back(std::move(exp));
    if (tokVal == ";") {
        while (true) {
            eat();
            exps.push_back(std::move(parseExp()));
            if (tokVal != ";")break;
        }
    }

    if (tokVal != "end") {
        syntaxErrMsg("end");
        return nullptr;
    }
    eat();
    localDec.reset(new LocalDeclarationExpAST(dec,exps));
    return localDec;
}

std::shared_ptr<ListExpAST> Parser::parseListExp() {
    eat();
    std::shared_ptr<ExpAST> exp1(parseExp());
    std::vector<std::shared_ptr<ExpAST>> ListExps;
    ListExps.push_back(std::move(exp1));
    while(true){
        if(tokVal != ",") break;
        eat();
        std::shared_ptr<ExpAST> oneexp(parsePrimaryExp());
        if (oneexp == nullptr) return nullptr;
        ListExps.push_back(std::move(oneexp));
    }
    if(tokVal != "]"){
        syntaxErrMsg("]");
        return nullptr;
    }
    eat();
    std::shared_ptr<ListExpAST> list(new ListExpAST(ListExps));
    return list;
}


void Parser::genErrMsg() {
    std::string row = std::to_string(tokRow);
    std::string col = std::to_string(tokCol);
    Error("Syntax Error: row " + row + ", column " + col + ": Invalid token: \t" + tokVal);
}

bool Parser::isInfixFunction(std::shared_ptr<Token> tok) {
    auto SymTb = SymbolTable::getInstance();
    auto getOp = SymTb->getOperator(tok->getValue());
    if (getOp == nullptr) return false;
    return getOp->operatorType == SymbolTable::Operator::INFIX or getOp->operatorType == SymbolTable::Operator::INFIXR;
}


bool Parser::followExp(const std::shared_ptr<Token>& tok) {
    std::unordered_set<std::string> expFollow{
            ")",",",";","]","end",":","andalso","orelse","then","else","do","and","="
    };


    bool isInFollow = expFollow.count(tok->getValue()) > 0;
    return isInFollow;
}

bool Parser::shouldRetLHSexp() {
    std::unordered_set<std::string> should{
            ")",",", ";" , "]", "end" , "then","else","do","and","=", "in"
    };
    bool ret = should.count(tokVal) > 0;
    return ret;
}

std::shared_ptr<LongIdAST> Parser::parseLongId() {
    auto id = parseId();
    if(id == nullptr) return nullptr;
    std::vector<std::shared_ptr<IdAST>> ids{id};
    std::shared_ptr<LongIdAST> longid(new LongIdAST(ids));
    return longid;
}



std::shared_ptr<TypeDecAST> Parser::parseTypeDec() {
    std::shared_ptr<TypeDecAST> typc;
    eat();
    auto typb = parseTypBind();
    if(typb == nullptr) return nullptr;
    typc.reset(new TypeDecAST(typb));
    return typc;
}

std::shared_ptr<TypBindAST> Parser::parseTypBind() {
    std::shared_ptr<TypBindAST> typb;
    auto id = parseId();
    if(id == nullptr) return nullptr;
    if(tokVal != "=")return nullptr;
    eat();
    auto typ = parseTyp();
    if(typ == nullptr) return nullptr;
    typb.reset(new TypBindAST(id,typ));
    if(tokVal == "and"){
        eat();
        typb->setTypBind(parseTypBind());
    }
    return typb;
}





















