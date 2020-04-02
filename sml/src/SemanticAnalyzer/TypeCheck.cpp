#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_set>
#include "AST/AST.h"
#include "AST/ASTProperty.h"
#include "Error.h"
#include "Symbol/SymbolTable.h"
#include "TypeCheck.h"

using namespace std;

static Type *setASTType(AST *ast, Type *type) {
    ASTProperty::setType(ast, type);
    return type;
}

void *TypeCheck::visit(ValueDecAST *ast) {
    return unify(visitAsType(ast->getValBind()), ast);
}

void *TypeCheck::visit(DestructuringValBindAST *ast) {
    // TODO: multiple valbind
    if (auto &&andValBind = ast->getAndValBind()) {
        return nullptr;
    }
    // visit value first, in case that we meet
    // val i = i;
    // where i is defined before and assign it to i itself.
    NextIdToSearchGuard _2{VALUE};
    auto typeExp = visitAsType(ast->getExp());
    NextIdToSearchGuard _1{PATTERN};
    auto typePat = visitAsType(ast->getPat());
    auto res = unify(typePat, typeExp, ast);
    return res;
}

void *TypeCheck::visit(TypeAnnotationExpAST *ast) {
    NextIdToSearchGuard _1{VALUE};
    auto typeExp = visitAsType(ast->getExp());
    NextIdToSearchGuard _2{TYPE};
    auto typeTyp = visitAsType(ast->getTyp());
    return unify(typeExp, typeTyp, ast);
}

void *TypeCheck::visit(TypeAnnotationPatAST *ast) {
    NextIdToSearchGuard _1{PATTERN};
    auto typeExp = visitAsType(ast->getPat());
    NextIdToSearchGuard _2{TYPE};
    auto typeTyp = visitAsType(ast->getTyp());
    return unify(typeExp, typeTyp, ast);
}

void *TypeCheck::visit(VariablePatAST *ast) {
    return ast->getId()->accept(this);
}

void *TypeCheck::visit(IdAST *ast) {
    auto &&name = ast->get();
    Type *res{};
    switch (getNextIdToSearch()) {
        case TYPE:
            if (auto type = SymbolTable::getInstance()->getType(name)) {
                res = type;
            }
            break;
        case VALUE:
            // a value must be defined before, no matter it is a function
            // parameter or a defined variable.
            if (auto type = getPatternType(name)) {
                type = find(type);
                if (type->getTypeId() == Type::VARIABLE_TYPE_NAME) {
                    insertVarTypePatternToFill(name, type);
                }
                res = type;
            }
            // now, for sentences like fn x => y, the rhs y is not existing in
            // the symbol table, and we do not have to add the var type value
            // for it any more since the parser ensures this.
            break;
        case PATTERN:
            // if pattern not found, we give it a new temp variable type for
            // future unification.
            if (auto type = getNextVariableTypeNameType()) {
                insertPatternType(name, type);
                res = type;
            }
            break;
    }
    return unify(res, ast);
}

void *TypeCheck::visit(ConstantExpAST *ast) {
    return unify(visitAsType(ast->getCon()), ast);
}

void *TypeCheck::visit(IntConAST *ast) {
    return unify(getIntType(), ast);
}

Type *TypeCheck::find(Type *t) {
    // if t is not exists in dsu, set dsu[t] = t. the following codes do the
    // same stuff.
    if (!t) {
        return t;
    }
    if (dsu[t] == nullptr) {
        return dsu[t] = t;
    }
    return t == dsu[t] ? t : dsu[t] = find(dsu[t]);
}

Type *TypeCheck::uni(Type *t1, Type *t2) {
    t1 = find(t1);
    t2 = find(t2);
    if (t1->getTypeId() == Type::VARIABLE_TYPE_NAME) {
        return dsu[t1] = t2;
    }
    return dsu[t2] = t1;
}

Type *TypeCheck::unify(Type *s, Type *t, AST *ast) {
    // if parameter pointers are same, return one
    if (s == t) {
        return setASTType(ast, t);
    }
    // if one of the two is null, return null
    if (!s || !t) {
        return setASTType(ast, nullptr);
    }

    // if t is a type alias, find the base type of it.
    while (t->getTypeId() == Type::TYPE_NAME) {
        t = t->toTypeNameType()->getBoundType();
    }
    // same stuff as t
    while (s->getTypeId() == Type::TYPE_NAME) {
        s = s->toTypeNameType()->getBoundType();
    }

    // do a dsu find
    s = find(s);
    t = find(t);

    // if they are identical, return one
    if (s == t) {
        return setASTType(ast, t);
    }
    auto tid = t->getTypeId();
    auto sid = s->getTypeId();
    // if one of s and t is variable type, union them and return
    if (sid == Type::VARIABLE_TYPE_NAME || tid == Type::VARIABLE_TYPE_NAME) {
        return setASTType(ast, uni(s, t));
    }
    // now if s and t do not have the same type id, return null
    if (sid != tid) {
        stringstream ss;
        s->print(ss);
        ss << " and ";
        t->print(ss);
        Error("Could not match " + ss.str() + '.');
        return setASTType(ast, nullptr);
    }
    switch (sid) {
        case Type::INT:
        case Type::REAL:
        case Type::STRING:
        case Type::CHAR:
        case Type::BOOL:
        case Type::UNIT:
            // if they have the same basic type, return one
            return setASTType(ast, uni(s, t));
        case Type::LIST:
            // if both are lists, check the subtype of the list
            return setASTType(ast, unify(s->toListType()->getSubtype(),
                                         t->toListType()->getSubtype())
                                   ? uni(s, t) : nullptr);
        case Type::RECORD: {
            // both are records, unify each record entry's type
            auto rec1 = s->toRecordType();
            auto rec2 = t->toRecordType();
            const auto &ent1 = rec1->getRecordEntries();
            const auto &ent2 = rec2->getRecordEntries();
            if (ent1.size() != ent2.size()) {
                return setASTType(ast, nullptr);
            }
            for (auto &&[key, val] : ent1) {
                if (auto it = ent2.find(key); it == ent2.end()) {
                    return setASTType(ast, nullptr);
                } else if (it->second != val) {
                    return setASTType(ast, nullptr);
                }
            }
            return setASTType(ast, uni(s, t));
        }
        case Type::TUPLE: {
            // both are tuples, unify each tuple entry
            auto tuple1 = s->toTupleType();
            auto tuple2 = t->toTupleType();
            const auto &types1 = tuple1->getTypes();
            const auto &types2 = tuple2->getTypes();
            if (types1.size() != types2.size()) {
                return setASTType(ast, nullptr);
            }
            for (size_t i = 0; i < types1.size(); ++i) {
                if (!unify(types1[i], types2[i])) {
                    return setASTType(ast, nullptr);
                }
            }
            return setASTType(ast, uni(s, t));
        }
        case Type::FUNCTION: {
            auto f1 = s->toFunctionType();
            auto f2 = t->toFunctionType();
            // cannot both be overloaded
            if (f1->isOverloaded() && f2->isOverloaded()) {
                return setASTType(ast, nullptr);
            }
            // if one of which is overloaded, swap and make f2 the overloaded
            if (f1->isOverloaded()) {
                swap(f1, f2);
            }
            // now see whether one of the overloaded functions matches
            for (auto &&[retTy, paramTy] : f2->getTypes()) {
                // yes: union them and return
                if (unify(f1->getParameterType(), paramTy)
                    && (unify(f1->getReturnType(), retTy))) {
                    return setASTType(ast, uni(s, t));
                }
            }
            // no: none is matched, null is returned
            return setASTType(ast, nullptr);
        }
        default:
            break;
    }

    // other cases not considered
    return setASTType(ast, nullptr);
}

void *TypeCheck::visit(InfixApplicationExpAST *ast) {
    Type *res{};
    auto &&idAST = ast->getId();
    auto &&idStr = idAST->get();

    static unordered_set<string> arithmeticBuiltins =
            {"+", "*", "-"};
    static unordered_set<string> comparableBuiltins =
            {"=", "<>", "<", ">", "<=", ">="};
    // for overloaded arithmetic builtins, check if their types are operand
    // compatible and identical.
    if (arithmeticBuiltins.count(idStr)) {
        static auto isArithmeticType = [](Type *type) {
            if (!type) {
                return false;
            }
            auto id = type->getTypeId();
            return id == Type::INT
                   || id == Type::REAL
                   || id == Type::VARIABLE_TYPE_NAME;
        };
        auto lhsType = visitAsType(ast->getExp1());
        if (isArithmeticType(lhsType)) {
            auto rhsType = visitAsType(ast->getExp2());
            if (isArithmeticType(rhsType)) {
                if (lhsType->getTypeId() == Type::VARIABLE_TYPE_NAME
                    && rhsType->getTypeId() == Type::VARIABLE_TYPE_NAME) {
                    res = unify(getIntType(), unify(lhsType, rhsType));
                } else {
                    res = unify(lhsType, rhsType);
                }
            }
        }
        return unify(res, ast);
    }

    // for comparable builtins, check if operands' types are identical.
    if (comparableBuiltins.count(idStr)) {
        auto lhsType = visitAsType(ast->getExp1());
        auto rhsType = visitAsType(ast->getExp2());
        if (unify(lhsType, rhsType)) {
            return unify(getBoolType(), ast);
        } else {
            return unify(nullptr, ast);
        }
    }

    // otherwise, the id is not builtin, get its value from symbol table and
    // treat it as an infix applicable function. here, the infix property is
    // ensured by the parser on building the ast.
    auto value = SymbolTable::getInstance()->getValue(ast->getId()->get());
    if (value) {
        auto type = value->getType();
        if (type->getTypeId() == Type::FUNCTION) {
            auto infixType = type->toFunctionType();
            auto lhsType = visitAsType(ast->getExp1());
            auto rhsType = visitAsType(ast->getExp2());
            auto tmpFunctionType = FunctionType::create(
                    VariableTypeNameType::create(tempVarType.nextSmall()),
                    TupleType::create({lhsType, rhsType}));
            res = unify(tmpFunctionType, infixType);
        }
    }
    return unify(res, ast);
}

Type *TypeCheck::visitBoolExp(
        const std::shared_ptr<ExpAST> &exp, AST *ast) {
    if (auto type = this->visitAsType(exp)) {
        return unify(type, BoolType::create(), ast);
    }
    return nullptr;
}

void *TypeCheck::visit(ConjunctionExpAST *ast) {
    auto ty1 = visitAsType(ast->getExp1());
    auto ty2 = visitAsType(ast->getExp2());
    return unify(unify(ty1, ty2), getBoolType(), ast);
}

void *TypeCheck::visit(DisjunctionExpAST *ast) {
    auto ty1 = visitAsType(ast->getExp1());
    auto ty2 = visitAsType(ast->getExp2());
    return unify(unify(ty1, ty2), getBoolType(), ast);
}

void *TypeCheck::visit(ConditionalExpAST *ast) {
    if (unify(visitAsType(ast->getExp1()), getBoolType())) {
        return unify(visitAsType(ast->getExp2()),
                     visitAsType(ast->getExp3()),
                     ast);
    } else {
        return unify(nullptr, ast);
    }
}

void *TypeCheck::visit(ConstructorTypAST *ast) {
    return unify(visitAsType(ast->getLongId()), ast);
}

void *TypeCheck::visit(LongIdAST *ast) {
    Type *res{};
    // TODO: long id with more than 1 id
    if (ast->getIds().size() == 1) {
        res = visitAsType(ast->getIds()[0]);
    } else {
        for (auto &&id : ast->getIds()) {
            if (!id->accept(this)) {
                return nullptr;
            }
        }
    }
    return unify(res, ast);
}

void *TypeCheck::visit(FloatConAST *ast) {
    return unify(getRealType(), ast);
}

TypeCheck::NextIdToSearchType TypeCheck::getNextIdToSearch() const {
    if (NextIdToSearchGuard::_nextIdToSearch.empty()) {
        return VALUE;
    }
    return static_cast<NextIdToSearchType>
    (NextIdToSearchGuard::_nextIdToSearch.back());
}

void *TypeCheck::visit(BoolConAST *ast) {
    return unify(getBoolType(), ast);
}

void *TypeCheck::visit(CharConAST *ast) {
    return unify(getCharType(), ast);
}

void *TypeCheck::visit(StringConAST *ast) {
    return unify(getStringType(), ast);
}

void *TypeCheck::visit(ListExpAST *ast) {
    Type *res{};
    auto &&exps = ast->getExps();
    if (exps.empty()) {
        res = ListType::create(VariableTypeNameType::create("'a"));
    } else {
        auto firstType = visitAsType(exps[0]);
        for (size_t i = 1; firstType && i < exps.size(); ++i) {
            firstType = unify(firstType, visitAsType(exps[i]));
        }
        if (firstType) {
            res = ListType::create(firstType);
        }
    }
    return unify(res, ast);
}

IntType *TypeCheck::getIntType() {
    return _intType ?: _intType = IntType::create();
}

BoolType *TypeCheck::getBoolType() {
    return _boolType ?: _boolType = BoolType::create();
}

void *TypeCheck::visit(ApplicationExpAST *ast) {
    Type *res{};
    auto &&exp1 = ast->getExp1();
    auto lhsType = visitAsType(exp1);
    if (lhsType->getTypeId() == Type::FUNCTION) {
        auto rhsType = visitAsType(ast->getExp2());
        auto lhsFuncType = lhsType->toFunctionType();
        Type *tmpFuncType;
        tmpFuncType = FunctionType::create(
                VariableTypeNameType::create(tempVarType.nextSmall()),
                rhsType);
        res = unify(lhsFuncType, tmpFuncType);
    }
    return unify(res, ast);
}

RealType *TypeCheck::getRealType() {
    return _realType ?: _realType = RealType::create();
}

CharType *TypeCheck::getCharType() {
    return _charType ?: _charType = CharType::create();
}

StringType *TypeCheck::getStringType() {
    return _stringType ?: _stringType = StringType::create();
}

void *TypeCheck::visit(ValueOrConstructorIdentifierExpAST *ast) {
    return unify(visitAsType(ast->getLongId()), ast);
}

void *TypeCheck::visit(FunctionExpAST *ast) {
    return unify(visitAsType(ast->getMatch()), ast);
}

void *TypeCheck::visit(MatchAST *ast) {
    Type *res{};
    // function's argument list first
    NextIdToSearchGuard _1{PATTERN};
    auto patTyp = visitAsType(ast->getPat());
    NextIdToSearchGuard _2{VALUE};
    auto expTyp = visitAsType(ast->getExp());
    if (patTyp && expTyp) {
        fillVarTypePatterns();
        res = FunctionType::create(expTyp, patTyp);
    }
    return unify(res, ast);
}

void *TypeCheck::visit(ConstructionPatAST *ast) {
    return visitAsType(ast->getLongId());
}

void TypeCheck::insertVarTypePatternToFill(const std::string &name, Type *type) {
    _varTypePatternToFill.emplace_back(name, type);
}

void TypeCheck::fillVarTypePatterns() {
    tempVarType.reset();
    for (auto &&[name, type_] : _varTypePatternToFill) {
        if (auto type = find(type_)) {
            if (type->getTypeId() != Type::VARIABLE_TYPE_NAME) {
                SymbolTable::getInstance()->insertPatternType(name, type);
            } else {
                SymbolTable::getInstance()->insertPatternType(
                        name,
                        VariableTypeNameType::create(tempVarType.nextSmall()));
            }
        }
    }
    _varTypePatternToFill.clear();
}

void *TypeCheck::visit(TuplePatAST *ast) {
    vector<Type *> types;
    for (auto &&pat : ast->getPats()) {
        if (auto type = visitAsType(pat)) {
            types.push_back(type);
        } else {
            return nullptr;
        }
    }
    return unify(TupleType::create(types), ast);
}

void TypeCheck::TempVarType::reset() {
    curBig = 'Z';
    curBig.shrink_to_fit();
    curSmall = 'a';
    curSmall.shrink_to_fit();
}

std::string TypeCheck::TempVarType::nextBig() {
    auto ret = '\'' + curBig;
    if (curBig.back() == 'A') {
        if (all_of(curBig.rbegin(),
                   curBig.rend(),
                   [](char c) { return c == 'A'; })) {
            for (auto &c : curBig) {
                c = 'Z';
            }
            curBig += 'Z';
        } else {
            for (auto it = curBig.rbegin(); it != curBig.rend();) {
                --*it;
                if (*it < 'A') {
                    *it = 'Z';
                    --*++it;
                } else {
                    break;
                }
            }
        }
    } else {
        --curBig.back();
    }

    return ret;
}

std::string TypeCheck::TempVarType::nextSmall() {
    auto ret = '\'' + curSmall;
    if (curSmall.back() == 'z') {
        if (all_of(curSmall.rbegin(),
                   curSmall.rend(),
                   [](char c) { return c == 'z'; })) {
            for (auto &c : curSmall) {
                c = 'a';
            }
            curSmall += 'a';
        } else {
            for (auto it = curSmall.rbegin(); it != curSmall.rend();) {
                ++*it;
                if (*it > 'z') {
                    *it = 'a';
                    ++*++it;
                } else {
                    break;
                }
            }
        }
    } else {
        ++curSmall.back();
    }
    return ret;
}

TypeCheck::NextIdToSearchGuard::NextIdToSearchGuard(
        TypeCheck::NextIdToSearchType type) noexcept {
    _nextIdToSearch.push_back(type);
}

TypeCheck::NextIdToSearchGuard::~NextIdToSearchGuard() noexcept {
    _nextIdToSearch.pop_back();
}

std::vector<TypeCheck::NextIdToSearchType>
        TypeCheck::NextIdToSearchGuard::_nextIdToSearch;

void TypeCheck::insertPatternType(const std::string &name, Type *type) {
    auto &depth = IncreaseDepthGuard::_depth;
    if (depth == -1) {
        SymbolTable::getInstance()->insertPatternType(name, type);
    } else {
        IncreaseDepthGuard::_localTypes[depth][name] = type;
    }
}

Type *TypeCheck::getPatternType(const std::string &name) {
    for (auto i = IncreaseDepthGuard::_depth; i > -1; --i) {
        if (IncreaseDepthGuard::_localTypes[i].count(name)) {
            return IncreaseDepthGuard::_localTypes[i][name];
        }
    }
    return SymbolTable::getInstance()->getPatternType(name);
}

void *TypeCheck::visit(LocalDeclarationExpAST *ast) {
    IncreaseDepthGuard _1;
    auto decTyp = visitAsType(ast->getDec());
    Type *type{};
    if (decTyp) {
        NextIdToSearchGuard _2{VALUE};
        for (auto &&exp : ast->getExps()) {
            type = visitAsType(exp);
            if (!type) {
                break;
            }
        }
    }
    return unify(type, ast);
}

Type *TypeCheck::unify(void *type, AST *ast) {
    return unify(static_cast<Type *>(type), static_cast<Type *>(type), ast);
}

void TypeCheck::fillTypes() {
    tempVarType.reset();
    for (auto &&[name, type_] : IncreaseDepthGuard::_localTypes[0]) {
        if (auto type = verify(type_)) {
            if (type->getTypeId() != Type::VARIABLE_TYPE_NAME) {
                SymbolTable::getInstance()->insertPatternType(name, type);
            } else {
                SymbolTable::getInstance()->insertPatternType(
                        name,
                        VariableTypeNameType::create(tempVarType.nextSmall()));
            }
        }
    }
}

Type *TypeCheck::getNextVariableTypeNameType() {
    return VariableTypeNameType::create(tempVarType.nextSmall());
}

void *TypeCheck::visit(FunctionTypAST *ast) {
    auto typ1 = visitAsType(ast->getTyp1());
    auto typ2 = visitAsType(ast->getTyp2());
    Type *res{};
    if (typ1 && typ2) {
        res = FunctionType::create(typ2, typ1);
    }
    return unify(res, ast);
}

void *TypeCheck::visit(FunctionDecAST *ast) {
    return ast->getFunBind()->accept(this);
}

void *TypeCheck::visit(NonFixFunMatchAST *ast) {
    NextIdToSearchGuard _1{PATTERN};
    auto idTyp = visitAsType(ast->getId());
    vector<Type *> types;
    IncreaseDepthGuard _4;
    for (auto &&pat : ast->getPats()) {
        if (auto type = visitAsType(pat)) {
            types.push_back(type);
        } else {
            return nullptr;
        }
    }
    Type *res{};
    for (auto rit = types.rbegin();
         rit != types.rend();
         ++rit) {
        auto type = *rit;
        if (res) {
            res = FunctionType::create(res, type);
        } else {
            res = type;
        }
    }
    NextIdToSearchGuard _2{VALUE};
    auto expTyp = visitAsType(ast->getExp());
    res = FunctionType::create(expTyp, res);
    if (auto &&orfm = ast->getOrFunMatch()) {
        res = unify(res, visitAsType(orfm));
    }
    if (auto &&typ = ast->getTyp()) {
        NextIdToSearchGuard _3{TYPE};
        res = unify(res, visitAsType(typ));
    }
    res = unify(res, idTyp);
    return unify(res, ast);
}

void *TypeCheck::visit(InfixFunMatchAST *ast) {
    NextIdToSearchGuard _1{PATTERN};
    auto typ1 = visitAsType(ast->getPat1());
    auto typ2 = visitAsType(ast->getPat2());
    Type *paramTyp{};
    Type *res{};
    if (typ1 && typ2) {
        paramTyp = TupleType::create({typ1, typ2});
    } else {
        return nullptr;
    }
    NextIdToSearchGuard _2{VALUE};
    auto expTyp = visitAsType(ast->getExp());
    if (expTyp) {
        res = FunctionType::create(expTyp, paramTyp);
    } else {
        return nullptr;
    }
    if (auto &&orfm = ast->getOrFunMatch()) {
        res = unify(res, visitAsType(orfm));
    }
    NextIdToSearchGuard _3{PATTERN};
    res = unify(res, visitAsType(ast->getId()));
    if (auto &&typ = ast->getTyp()) {
        NextIdToSearchGuard _4{TYPE};
        res = unify(res, visitAsType(typ));
    }
    return unify(res, ast);
}

void *TypeCheck::visit(FunBindAST *ast) {
    auto fmTyp = visitAsType(ast->getFunMatch());
    if (auto &&andFunBind = ast->getAndFunBind()) {
        fmTyp = unify(fmTyp, visitAsType(andFunBind));
    }
    return unify(fmTyp, ast);
}

Type *TypeCheck::verify(Type *type) {
    if (type == nullptr) {
        return type;
    }
    switch (type->getTypeId()) {
        case Type::INT:
        case Type::REAL:
        case Type::STRING:
        case Type::CHAR:
        case Type::BOOL:
        case Type::UNIT:
            return type;
        case Type::LIST: {
            auto lty = type->toListType();
            auto sub = lty->getSubtype();
            sub = verify(sub);
            return ListType::create(sub);
        }
        case Type::RECORD:
            // not implemented
            break;
        case Type::TUPLE: {
            auto tupleTyp = type->toTupleType();
            vector<Type *> types;
            for (auto ty : tupleTyp->getTypes()) {
                types.emplace_back(verify(ty));
            }
            return TupleType::create(types);
        }
        case Type::FUNCTION: {
            auto funType = type->toFunctionType();
            auto n = funType->getOverloadedNum();
            vector<pair<Type *, Type *>> types;
            for (auto i = 0; i < n; ++i) {
                auto retTyp = funType->getReturnType(i);
                auto parTyp = funType->getParameterType(i);
                retTyp = verify(retTyp);
                parTyp = verify(parTyp);
                types.emplace_back(retTyp, parTyp);
            }
            return FunctionType::create(std::move(types));
        }
        case Type::TYPE_NAME:
            return type;
        case Type::VARIABLE_TYPE_NAME:
            return find(type);
    }
    return type;
}

TypeCheck::TypeCheck() {
    TypeCheck::IncreaseDepthGuard::_depth = 0;
    TypeCheck::IncreaseDepthGuard::_localTypes.clear();
    TypeCheck::IncreaseDepthGuard::_localTypes.emplace_back();
}

void *TypeCheck::visit(TupleExpAST *ast) {
    vector<Type *> types;
    for (auto &&exp : ast->getExps()) {
        if (auto type = visitAsType(exp)) {
            types.emplace_back(type);
        } else {
            return unify(nullptr, ast);
        }
    }
    return unify(TupleType::create(types), ast);
}

void *TypeCheck::visit(SequenceDecAST *ast) {
    Type *type{};
    for (auto &&dec : ast->getDecs()) {
        type = visitAsType(dec);
        if (!type) {
            break;
        }
    }
    return unify(type, ast);
}

void *TypeCheck::visit(LeftAssociativeInfixDecAST *ast) {
    return unify(nullptr, ast);
}

void *TypeCheck::visit(RightAssociativeInfixDecAST *ast) {
    return unify(nullptr, ast);
}

int TypeCheck::IncreaseDepthGuard::_depth;
std::vector<std::unordered_map<std::string, Type *>>
        TypeCheck::IncreaseDepthGuard::_localTypes;

TypeCheck::IncreaseDepthGuard::IncreaseDepthGuard() noexcept {
    _localTypes.emplace_back();
    ++_depth;
}

TypeCheck::IncreaseDepthGuard::~IncreaseDepthGuard() noexcept {
    _localTypes.pop_back();
    --_depth;
}
