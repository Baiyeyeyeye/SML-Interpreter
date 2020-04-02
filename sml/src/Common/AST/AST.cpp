#include <memory>
#include <utility>
#include "AST.h"
#include "ASTVisitor.h"

#ifndef APPLY_ALL

#include "ASTApplyMacro.h"

#endif

#define APPLY(...) \
inline void *__VA_ARGS__::accept(ASTVisitor *____v) {return ____v->visit(this);}

APPLY_ALL

LeftAssociativeInfixDecAST::LeftAssociativeInfixDecAST(
        std::vector<std::shared_ptr<IdAST>> ids, int priority)
        : ids(std::move(ids)), priority(priority) {

}

const std::vector<std::shared_ptr<IdAST>> &
LeftAssociativeInfixDecAST::getIds() const {
    return ids;
}

int LeftAssociativeInfixDecAST::getPriority() const {
    return priority;
}

RightAssociativeInfixDecAST::RightAssociativeInfixDecAST(
        std::vector<std::shared_ptr<IdAST>> ids, int priority)
        : ids(std::move(ids)), priority(priority) {

}

const std::vector<std::shared_ptr<IdAST>> &
RightAssociativeInfixDecAST::getIds() const {
    return ids;
}

int RightAssociativeInfixDecAST::getPriority() const {
    return priority;
}

const std::shared_ptr<ExpAST> &ParenthesesExpAST::getExp() const {
    return exp;
}

ParenthesesExpAST::ParenthesesExpAST(std::shared_ptr<ExpAST> exp)
        : exp(std::move(exp)) {

}

ParenthesesPatAST::ParenthesesPatAST(std::shared_ptr<PatAST> pat)
        : pat(std::move(pat)) {

}

const std::shared_ptr<PatAST> &ParenthesesPatAST::getPat() const {
    return pat;
}


LocalDeclarationExpAST::LocalDeclarationExpAST(
        std::shared_ptr<DecAST> dec,
        std::vector<std::shared_ptr<ExpAST>> exps)
        : _dec(std::move(dec)), _exps(std::move(exps)) {

}

const std::shared_ptr<DecAST> &LocalDeclarationExpAST::getDec() const {
    return _dec;
}

const std::vector<std::shared_ptr<ExpAST>> &
LocalDeclarationExpAST::getExps() const {
    return _exps;
}

NonfixDecAST::NonfixDecAST(std::vector<std::shared_ptr<IdAST>> ids) : ids(
        std::move(ids)) {}

LocalDecAST::LocalDecAST(std::shared_ptr<DecAST> dec1,
                         std::shared_ptr<DecAST> dec2) : dec1(std::move(dec1)),
                                                         dec2(std::move(
                                                                 dec2)) {}

SequenceDecAST::SequenceDecAST(std::vector<std::shared_ptr<DecAST>> decs)
        : _decs(std::move(decs)) {}

const std::vector<std::shared_ptr<DecAST>> &SequenceDecAST::getDecs() const {
    return _decs;
}

VariableTypAST::VariableTypAST(std::shared_ptr<VarAST> var) : _var(
        std::move(var)) {}


InfixFunMatchAST::InfixFunMatchAST(std::shared_ptr<IdAST> id,
                                   std::vector<std::shared_ptr<PatAST>> pats,
                                   std::shared_ptr<ExpAST> exp,
                                   std::shared_ptr<TypAST> typ,
                                   std::shared_ptr<FunMatchAST> orFunMatch)
        : FunMatchAST(std::move(id),
                      std::move(pats),
                      std::move(exp),
                      std::move(typ),
                      std::move(orFunMatch)) {
    this->_pat1 = std::move(pats[0]);
    this->_pat2 = std::move(pats[1]);
}

const std::shared_ptr<PatAST> &InfixFunMatchAST::getPat1() const {
    return _pat1;
}

const std::shared_ptr<PatAST> &InfixFunMatchAST::getPat2() const {
    return _pat2;
}

NonFixFunMatchAST::NonFixFunMatchAST(std::shared_ptr<IdAST> id,
                                     std::vector<std::shared_ptr<PatAST>> pats,
                                     std::shared_ptr<ExpAST> exp,
                                     std::shared_ptr<TypAST> typ,
                                     std::shared_ptr<FunMatchAST> orFunMatch)
        : FunMatchAST(std::move(id),
                      std::move(pats),
                      std::move(exp),
                      std::move(typ),
                      std::move(orFunMatch)) {

}

RecordSelectorExpAST::RecordSelectorExpAST(std::shared_ptr<LabAST> lab) : _lab(
        std::move(lab)) {}

TupleExpAST::TupleExpAST(std::vector<std::shared_ptr<ExpAST>> exps) : _exps(
        std::move(exps)) {}

const std::vector<std::shared_ptr<ExpAST>> &TupleExpAST::getExps() const {
    return _exps;
}


const std::shared_ptr<IdAST> &FunMatchAST::getId() const {
    return _id;
}

const std::vector<std::shared_ptr<PatAST>> &FunMatchAST::getPats() const {
    return _pats;
}

const std::shared_ptr<ExpAST> &FunMatchAST::getExp() const {
    return _exp;
}

const std::shared_ptr<FunMatchAST> &FunMatchAST::getOrFunMatch() const {
    return _orFunMatch;
}

FunMatchAST::FunMatchAST(std::shared_ptr<IdAST> id,
                         std::vector<std::shared_ptr<PatAST>> pats,
                         std::shared_ptr<ExpAST> exp,
                         std::shared_ptr<TypAST> typ,
                         std::shared_ptr<FunMatchAST> orFunMatch) : _id(
        std::move(id)), _pats(std::move(pats)),
                                                                    _exp(std::move(
                                                                            exp)),
                                                                    _typ(std::move(
                                                                            typ)),
                                                                    _orFunMatch(
                                                                            std::move(
                                                                                    orFunMatch)) {}

const std::shared_ptr<TypAST> &FunMatchAST::getTyp() const {
    return _typ;
}

void
FunMatchAST::setOrFunMatch(const std::shared_ptr<FunMatchAST> &orFunMatch) {
    _orFunMatch = orFunMatch;
}


IterationExpAST::IterationExpAST(std::shared_ptr<ExpAST> exp1,
                                 std::shared_ptr<ExpAST> exp2)
        : exp1(std::move(exp1)), exp2(std::move(exp2)) {

}

const std::vector<std::shared_ptr<PatAST>> &TuplePatAST::getPats() const {
    return pats;
}

TuplePatAST::TuplePatAST(std::vector<std::shared_ptr<PatAST>> pats)
        : pats(std::move(pats)) {

}

ConstructionPatAST::ConstructionPatAST(
        std::shared_ptr<LongIdAST> longId, std::shared_ptr<PatAST> pat)
        : longId(std::move(longId)), pat(std::move(pat)) {

}

const std::shared_ptr<LongIdAST> &ConstructionPatAST::getLongId() const {
    return longId;
}

const std::shared_ptr<PatAST> &ConstructionPatAST::getPat() const {
    return pat;
}

InfixConstructionPatAST::InfixConstructionPatAST(
        std::shared_ptr<PatAST> pat1,
        std::shared_ptr<IdAST> id,
        std::shared_ptr<PatAST> pat2)
        : pat1(std::move(pat1)), id(std::move(id)), pat2(std::move(pat2)) {

}

ValueOrConstructorIdentifierExpAST::ValueOrConstructorIdentifierExpAST(
        std::shared_ptr<LongIdAST> longId)
        : longId(std::move(longId)) {

}

const std::shared_ptr<LongIdAST> &
ValueOrConstructorIdentifierExpAST::getLongId() const {
    return longId;
}

ApplicationExpAST::ApplicationExpAST(
        std::shared_ptr<ExpAST> exp1, std::shared_ptr<ExpAST> exp2)
        : exp1(std::move(exp1)), exp2(std::move(exp2)) {

}

const std::shared_ptr<ExpAST> &ApplicationExpAST::getExp1() const {
    return exp1;
}

const std::shared_ptr<ExpAST> &ApplicationExpAST::getExp2() const {
    return exp2;
}

ListExpAST::ListExpAST(std::vector<std::shared_ptr<ExpAST>> exps)
        : exps(std::move(exps)) {

}

const std::vector<std::shared_ptr<ExpAST>> &ListExpAST::getExps() const {
    return exps;
}

EqualityVarAST::EqualityVarAST(std::string var) : VarAST(std::move(var)) {

}

UnconstrainedVarAST::UnconstrainedVarAST(std::string var)
        : VarAST(std::move(var)) {

}

VarAST::VarAST(std::string var) : _var(std::move(var)) {

}

const std::string &VarAST::getVar() const {
    return _var;
}

const std::shared_ptr<ExpAST> &InfixApplicationExpAST::getExp1() const {
    return exp1;
}

const std::shared_ptr<IdAST> &InfixApplicationExpAST::getId() const {
    return id;
}

const std::shared_ptr<ExpAST> &InfixApplicationExpAST::getExp2() const {
    return exp2;
}

InfixApplicationExpAST::InfixApplicationExpAST(
        std::shared_ptr<ExpAST> exp1,
        std::shared_ptr<IdAST> id,
        std::shared_ptr<ExpAST> exp2)
        : exp1(std::move(exp1)), id(std::move(id)), exp2(std::move(exp2)) {

}

MatchAST::MatchAST(std::shared_ptr<PatAST> pat,
                   std::shared_ptr<ExpAST> exp,
                   std::shared_ptr<MatchAST> match)
        : pat(std::move(pat)), exp(std::move(exp)), match(std::move(match)) {

}

const std::shared_ptr<PatAST> &MatchAST::getPat() const {
    return pat;
}

const std::shared_ptr<ExpAST> &MatchAST::getExp() const {
    return exp;
}

const std::shared_ptr<MatchAST> &MatchAST::getMatch() const {
    return match;
}

void MatchAST::setMatch(const std::shared_ptr<MatchAST> &orMatch) {
    MatchAST::match = orMatch;
}

LongIdAST::LongIdAST(std::vector<std::shared_ptr<IdAST>> ids)
        : ids(std::move(ids)) {

}

const std::vector<std::shared_ptr<IdAST>> &LongIdAST::getIds() const {
    return ids;
}

ConstructorTypAST::ConstructorTypAST(std::shared_ptr<LongIdAST> longId)
        : longId(std::move(longId)) {

}

const std::shared_ptr<LongIdAST> &ConstructorTypAST::getLongId() const {
    return longId;
}

TypeAnnotationExpAST::TypeAnnotationExpAST(
        std::shared_ptr<ExpAST> exp, std::shared_ptr<TypAST> typ)
        : exp(std::move(exp)), typ(std::move(typ)) {

}

const std::shared_ptr<ExpAST> &TypeAnnotationExpAST::getExp() const {
    return exp;
}

const std::shared_ptr<TypAST> &TypeAnnotationExpAST::getTyp() const {
    return typ;
}

TupleTypAST::TupleTypAST(std::vector<std::shared_ptr<TypAST>> tuple)
        : tuple(std::move(tuple)) {

}

const std::vector<std::shared_ptr<TypAST>> &TupleTypAST::getTuple() const {
    return tuple;
}

NumberLabAST::NumberLabAST(int n) : n(n) {

}

int NumberLabAST::getN() const {
    return n;
}

IdentifierLabAST::IdentifierLabAST(std::shared_ptr<IdAST> id)
        : id(std::move(id)) {

}

const std::shared_ptr<IdAST> &IdentifierLabAST::getId() const {
    return id;
}

AlphanumericIdAST::AlphanumericIdAST(std::string id) : IdAST(std::move(id)) {

}

SymbolicIdAST::SymbolicIdAST(std::string id) : IdAST(std::move(id)) {

}

BoolConAST::BoolConAST(bool b) : b(b) {}

bool BoolConAST::get() const {
    return b;
}

VariablePatAST::VariablePatAST(std::shared_ptr<IdAST> id) : id(std::move(id)) {

}

const std::shared_ptr<IdAST> &VariablePatAST::getId() const {
    return id;
}

const std::shared_ptr<PatAST> &InfixConstructionPatAST::getPat1() const {
    return pat1;
}

const std::shared_ptr<IdAST> &InfixConstructionPatAST::getId() const {
    return id;
}

const std::shared_ptr<PatAST> &InfixConstructionPatAST::getPat2() const {
    return pat2;
}

TypeAnnotationPatAST::TypeAnnotationPatAST(
        std::shared_ptr<PatAST> pat, std::shared_ptr<TypAST> typ)
        : pat(std::move(pat)), typ(std::move(typ)) {

}

const std::shared_ptr<PatAST> &TypeAnnotationPatAST::getPat() const {
    return pat;
}

const std::shared_ptr<TypAST> &TypeAnnotationPatAST::getTyp() const {
    return typ;
}

ConstantExpAST::ConstantExpAST(std::shared_ptr<ConAST> con)
        : con(std::move(con)) {

}

const std::shared_ptr<ConAST> &ConstantExpAST::getCon() const {
    return con;
}

IdAST::IdAST(std::string id) : id(std::move(id)) {

}

const std::string &IdAST::get() const {
    return id;
}

StringConAST::StringConAST(std::string str) : str(std::move(str)) {

}

const std::string &StringConAST::get() const {
    return str;
}

FloatConAST::FloatConAST(double v) : v(v) {

}

double FloatConAST::get() const {
    return v;
}

CharConAST::CharConAST(char c) : c(c) {

}

char CharConAST::get() const {
    return c;
}

IntConAST::IntConAST(int v) : v(v) {

}

int IntConAST::get() const {
    return v;
}

ConditionalExpAST::ConditionalExpAST(
        std::shared_ptr<ExpAST> exp1,
        std::shared_ptr<ExpAST> exp2,
        std::shared_ptr<ExpAST> exp3)
        : exp1(std::move(exp1)), exp2(std::move(exp2)), exp3(std::move(exp3)) {

}

const std::shared_ptr<ExpAST> &ConditionalExpAST::getExp1() const {
    return exp1;
}

const std::shared_ptr<ExpAST> &ConditionalExpAST::getExp2() const {
    return exp2;
}

const std::shared_ptr<ExpAST> &ConditionalExpAST::getExp3() const {
    return exp3;
}

FunctionExpAST::FunctionExpAST(std::shared_ptr<MatchAST> match)
        : match(std::move(match)) {
}

const std::shared_ptr<MatchAST> &FunctionExpAST::getMatch() const {
    return match;
}

DisjunctionExpAST::DisjunctionExpAST(
        std::shared_ptr<ExpAST> exp1, std::shared_ptr<ExpAST> exp2)
        : exp1(std::move(exp1)), exp2(std::move(exp2)) {

}

const std::shared_ptr<ExpAST> &DisjunctionExpAST::getExp1() const {
    return exp1;
}

const std::shared_ptr<ExpAST> &DisjunctionExpAST::getExp2() const {
    return exp2;
}

ConjunctionExpAST::ConjunctionExpAST(
        std::shared_ptr<ExpAST> exp1,
        std::shared_ptr<ExpAST> exp2)
        : exp1(std::move(exp1)), exp2(std::move(exp2)) {

}

const std::shared_ptr<ExpAST> &ConjunctionExpAST::getExp1() const {
    return exp1;
}

const std::shared_ptr<ExpAST> &ConjunctionExpAST::getExp2() const {
    return exp2;
}

ConstantPatAST::ConstantPatAST(std::shared_ptr<ConAST> con)
        : con(std::move(con)) {

}

const std::shared_ptr<ConAST> &ConstantPatAST::getCon() const {
    return con;
}

TypRowAST::TypRowAST(
        std::shared_ptr<LabAST> lab,
        std::shared_ptr<TypAST> typ,
        std::shared_ptr<TypRowAST> typRow)
        : lab(std::move(lab)), typ(std::move(typ)), typRow(std::move(typRow)) {

}

const std::shared_ptr<LabAST> &TypRowAST::getLab() const {
    return lab;
}

const std::shared_ptr<TypAST> &TypRowAST::getTyp() const {
    return typ;
}

const std::shared_ptr<TypRowAST> &TypRowAST::getTypRow() const {
    return typRow;
}

RecordTypAST::RecordTypAST(std::shared_ptr<TypRowAST> typRow)
        : typRow(std::move(typRow)) {

}

const std::shared_ptr<TypRowAST> &RecordTypAST::getTypRow() const {
    return typRow;
}

FunctionTypAST::FunctionTypAST(
        std::shared_ptr<TypAST> typ1, std::shared_ptr<TypAST> typ2)
        : typ1(std::move(typ1)), typ2(std::move(typ2)) {

}

const std::shared_ptr<TypAST> &FunctionTypAST::getTyp1() const {
    return typ1;
}

const std::shared_ptr<TypAST> &FunctionTypAST::getTyp2() const {
    return typ2;
}

ParenthesesTypAST::ParenthesesTypAST(std::shared_ptr<TypAST> typ)
        : typ(std::move(typ)) {

}

const std::shared_ptr<TypAST> &ParenthesesTypAST::getTyp() const {
    return typ;
}

FunctionDecAST::FunctionDecAST(std::shared_ptr<FunBindAST> funBind)
        : funBind(std::move(funBind)) {

}

const std::shared_ptr<FunBindAST> &FunctionDecAST::getFunBind() const {
    return funBind;
}

RecursiveValBindAST::RecursiveValBindAST(std::shared_ptr<ValBindAST> valBind)
        : valBind(std::move(valBind)) {

}

const std::shared_ptr<ValBindAST> &RecursiveValBindAST::getValBind() const {
    return valBind;
}

TypeDecAST::TypeDecAST(std::shared_ptr<TypBindAST> typBind)
        : _typBind(std::move(typBind)) {

}

const std::shared_ptr<TypBindAST> &TypeDecAST::getTypBind() const {
    return _typBind;
}

ValueDecAST::ValueDecAST(std::shared_ptr<ValBindAST> valBind)
        : _valBind(std::move(valBind)) {

}

const std::shared_ptr<ValBindAST> &ValueDecAST::getValBind() const {
    return _valBind;
}

DestructuringValBindAST::DestructuringValBindAST(
        std::shared_ptr<PatAST> pat,
        std::shared_ptr<ExpAST> exp,
        std::shared_ptr<ValBindAST> andValBind)
        : _pat(std::move(pat)),
          _exp(std::move(exp)),
          _andValBind(std::move(andValBind)) {

}

const std::shared_ptr<PatAST> &DestructuringValBindAST::getPat() const {
    return _pat;
}

const std::shared_ptr<ExpAST> &DestructuringValBindAST::getExp() const {
    return _exp;
}

const std::shared_ptr<ValBindAST> &
DestructuringValBindAST::getAndValBind() const {
    return _andValBind;
}

void DestructuringValBindAST::setAndValBind(
        const std::shared_ptr<ValBindAST> &andValBind) {
    _andValBind = andValBind;
}

FunBindAST::FunBindAST(
        std::shared_ptr<FunMatchAST> funMatch,
        std::shared_ptr<FunBindAST> andFunBind)
        : _funMatch(std::move(funMatch)),
          _andFunBind(std::move(andFunBind)) {

}

const std::shared_ptr<FunMatchAST> &FunBindAST::getFunMatch() const {
    return _funMatch;
}

const std::shared_ptr<FunBindAST> &FunBindAST::getAndFunBind() const {
    return _andFunBind;
}

void FunBindAST::setAndFunBind(const std::shared_ptr<FunBindAST> &andFunBind) {
    _andFunBind = andFunBind;
}

TypBindAST::TypBindAST(
        std::shared_ptr<IdAST> id,
        std::shared_ptr<TypAST> typ,
        std::shared_ptr<TypBindAST> andTypBind)
        : id(std::move(id)),
          typ(std::move(typ)),
          typBind(std::move(andTypBind)) {

}

const std::shared_ptr<IdAST> &TypBindAST::getId() const {
    return id;
}

const std::shared_ptr<TypAST> &TypBindAST::getTyp() const {
    return typ;
}

const std::shared_ptr<TypBindAST> &TypBindAST::getTypBind() const {
    return typBind;
}

void TypBindAST::setTypBind(const std::shared_ptr<TypBindAST> &typBind) {
    TypBindAST::typBind = typBind;
}
