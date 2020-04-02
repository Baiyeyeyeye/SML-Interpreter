#pragma once

/*******************************************************************************
 Referred to: https://people.mpi-sws.org/~rossberg/sml.html

 Classes' constraint:

    Some constants can be represented by basic types, like int, char, they are
    the atomic classes which hold the result of the token scanner, and there-
    fore do not have subclasses. All grammar rules except those are represented
    by an own class.

    AST is the abstract base class that all concrete classes are derived from.
    Each grammar unit that appears on any left hand side of grammar rules and
    generates more than one right hand sides are separated with subclasses for
    each productions respective.

 Naming Rules:

    For single-right-hand-sides, the left hand side plus AST.

    For multi-right-hand-sides, the left hand side plus AST becomes an abstract
    base class, and the explanation plus the left hand side becomes a concrete
    subclass of it.

*******************************************************************************/

#include <memory>
#include <vector>

//region APPLY_ALL forward declarations for all ast.
#ifndef APPLY_ALL

#include "ASTApplyMacro.h"

#define APPLY(...) class __VA_ARGS__;

APPLY_ALL

#undef APPLY
#undef APPLY_ALL
#endif
//endregion

class ASTVisitor;

/*******************************************************************************
Base AST
*******************************************************************************/

class AST;

class AST {
public:
    virtual ~AST() = default;

    /**
     * Visitor accept.
     *
     * Accept an ast visitor and let the ast visitor visit `this` ast.
     * @param visitor The visitor to accept.
     * @return By default (void*)1 if no errors occurred.
     */
    virtual void *accept(ASTVisitor *visitor);

    /**
     * Down cast an ast pointer from base.
     * @tparam TCastToPtr The derived class type.
     * @param ast The pointer to cast.
     * @return The pointer after the cast.
     */
    template<typename TCastToPtr>
    static auto cast(AST *const ast) {
        return static_cast<TCastToPtr>(ast);
    }

    /**
     * Down cast an ast shared pointer from base.
     * @tparam TCastToPtr The derived class type.
     * @param ast The pointer to cast.
     * @return The shared pointer after the cast.
     */
    template<typename TCastToPtr>
    static auto cast(const std::shared_ptr<AST> &ast) {
        return std::static_pointer_cast<TCastToPtr>(ast);
    }

    /**
     * Create a concrete ast shared pointer with constructor arguments.
     * @tparam TDerivedAST The concrete ast type.
     * @tparam Args The constructor argument types.
     * @param args The constructor argument list.
     * @return A shared pointer to the target ast.
     */
    template<
            typename TDerivedAST,
            typename ...Args,
            typename = std::enable_if_t<std::is_base_of_v<AST, TDerivedAST>>>
    static auto create(Args &&...args) {
        return std::make_shared<TDerivedAST>(std::forward<Args>(args)...);
    }

protected:
    AST() = default;
};

//region DECL_ACCEPT_VISITOR
/**
 * A helper macro to define the override function for each subclass.
 */
#define DECL_ACCEPT_VISITOR \
public: void *accept(ASTVisitor*) override;

/*******************************************************************************
Constants
*******************************************************************************/

class ConAST;

class IntConAST;

class FloatConAST;

class BoolConAST;

class CharConAST;

class StringConAST;

class ConAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    ConAST() = default;
};

class IntConAST : public ConAST {
DECL_ACCEPT_VISITOR

public:
    explicit IntConAST(int v);

    [[nodiscard]] int get() const;

private:
    int v;
};

class BoolConAST : public ConAST {
DECL_ACCEPT_VISITOR

public:
    explicit BoolConAST(bool b);

    [[nodiscard]] bool get() const;

private:
    bool b;
};

class CharConAST : public ConAST {
DECL_ACCEPT_VISITOR

public:
    explicit CharConAST(char c);

    [[nodiscard]] char get() const;

private:
    char c;
};

class FloatConAST : public ConAST {
DECL_ACCEPT_VISITOR

public:
    explicit FloatConAST(double v);

    [[nodiscard]] double get() const;

private:
    double v;
};

class StringConAST : public ConAST {
DECL_ACCEPT_VISITOR

public:
    explicit StringConAST(std::string str);

    [[nodiscard]] const std::string &get() const;

private:
    std::string str;
};

/*******************************************************************************
Identifiers
*******************************************************************************/

class IdAST;

class VarAST;

class LongIdAST;

class LabAST;

class AlphanumericIdAST;

class SymbolicIdAST;

class UnconstrainedVarAST;

class EqualityVarAST;

class IdentifierLabAST;

class NumberLabAST;

class IdAST : public AST {
DECL_ACCEPT_VISITOR

public:
    [[nodiscard]] const std::string &get() const;

protected:
    explicit IdAST(std::string id);

private:
    std::string id;
};

class SymbolicIdAST : public IdAST {
DECL_ACCEPT_VISITOR

public:
    explicit SymbolicIdAST(std::string id);
};

class AlphanumericIdAST : public IdAST {
DECL_ACCEPT_VISITOR

public:
    explicit AlphanumericIdAST(std::string id);
};

class VarAST : public AST {
DECL_ACCEPT_VISITOR

public:
    explicit VarAST(std::string var);

    [[nodiscard]] const std::string &getVar() const;

private:
    std::string _var;
};

class UnconstrainedVarAST : public VarAST {
DECL_ACCEPT_VISITOR

public:
    explicit UnconstrainedVarAST(std::string var);
};

class EqualityVarAST : public VarAST {
DECL_ACCEPT_VISITOR

public:
    explicit EqualityVarAST(std::string var);
};

class LongIdAST : public AST {
DECL_ACCEPT_VISITOR

public:
    explicit LongIdAST(std::vector<std::shared_ptr<IdAST>> ids);

    [[nodiscard]] const std::vector<std::shared_ptr<IdAST>> &getIds() const;

private:
    std::vector<std::shared_ptr<IdAST>> ids;
};

class LabAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    LabAST() = default;
};

class IdentifierLabAST : public LabAST {
DECL_ACCEPT_VISITOR

public:
    explicit IdentifierLabAST(std::shared_ptr<IdAST> id);

    [[nodiscard]] const std::shared_ptr<IdAST> &getId() const;

private:
    std::shared_ptr<IdAST> id;
};

class NumberLabAST : public LabAST {
DECL_ACCEPT_VISITOR

public:
    explicit NumberLabAST(int n);

    [[nodiscard]] int getN() const;

private:
    int n;
};

/*******************************************************************************
Expressions
*******************************************************************************/

class ExpAST;

class ConstantExpAST;

class ValueOrConstructorIdentifierExpAST;

class ApplicationExpAST;

class InfixApplicationExpAST;

class ParenthesesExpAST;

class TupleExpAST;

class RecordTupleExpAST;

class RecordSelectorExpAST;

class ListExpAST;

class LocalDeclarationExpAST;

class TypeAnnotationExpAST;

class ConjunctionExpAST;

class DisjunctionExpAST;

class ConditionalExpAST;

class IterationExpAST;

class CaseAnalysisExpAST;

class FunctionExpAST;

class ExpRowAST;

class MatchAST;

class ExpAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    ExpAST() = default;
};

class ExpRowAST : public AST {
DECL_ACCEPT_VISITOR
};

class MatchAST : public AST {
DECL_ACCEPT_VISITOR

public:
    MatchAST(std::shared_ptr<PatAST> pat,
             std::shared_ptr<ExpAST> exp,
             std::shared_ptr<MatchAST> match = nullptr);

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp() const;

    [[nodiscard]] const std::shared_ptr<MatchAST> &getMatch() const;

    void setMatch(const std::shared_ptr<MatchAST> &orMatch);

private:
    std::shared_ptr<PatAST> pat;
    std::shared_ptr<ExpAST> exp;
    std::shared_ptr<MatchAST> match;
};

class ConstantExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    explicit ConstantExpAST(std::shared_ptr<ConAST> con);

    [[nodiscard]] const std::shared_ptr<ConAST> &getCon() const;

private:
    std::shared_ptr<ConAST> con;
};

class ValueOrConstructorIdentifierExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    explicit ValueOrConstructorIdentifierExpAST(
            std::shared_ptr<LongIdAST> longId);

    [[nodiscard]] const std::shared_ptr<LongIdAST> &getLongId() const;

private:
    std::shared_ptr<LongIdAST> longId;
};

class ApplicationExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    ApplicationExpAST(std::shared_ptr<ExpAST> exp1,
                      std::shared_ptr<ExpAST> exp2);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp1() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp2() const;

private:
    std::shared_ptr<ExpAST> exp1;
    std::shared_ptr<ExpAST> exp2;
};

class InfixApplicationExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    InfixApplicationExpAST(std::shared_ptr<ExpAST> exp1,
                           std::shared_ptr<IdAST> id,
                           std::shared_ptr<ExpAST> exp2);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp1() const;

    [[nodiscard]] const std::shared_ptr<IdAST> &getId() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp2() const;

private:
    std::shared_ptr<ExpAST> exp1;
    std::shared_ptr<IdAST> id;
    std::shared_ptr<ExpAST> exp2;
};

class ParenthesesExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    explicit ParenthesesExpAST(std::shared_ptr<ExpAST> exp);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp() const;

private:
    std::shared_ptr<ExpAST> exp;
};

class TupleExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    explicit TupleExpAST(std::vector<std::shared_ptr<ExpAST>> exps);

    [[nodiscard]] const std::vector<std::shared_ptr<ExpAST>> &getExps() const;

private:
    std::vector<std::shared_ptr<ExpAST>> _exps;
};

class RecordTupleExpAST : public ExpAST {
DECL_ACCEPT_VISITOR
};

class RecordSelectorExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    explicit RecordSelectorExpAST(std::shared_ptr<LabAST> lab);

private:
    std::shared_ptr<LabAST> _lab;
};

class ListExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    explicit ListExpAST(std::vector<std::shared_ptr<ExpAST>> exps);

    [[nodiscard]] const std::vector<std::shared_ptr<ExpAST>> &getExps() const;

private:
    std::vector<std::shared_ptr<ExpAST>> exps;
};

class LocalDeclarationExpAST : public ExpAST {
DECL_ACCEPT_VISITOR


    LocalDeclarationExpAST(
            std::shared_ptr<DecAST> dec,
            std::vector<std::shared_ptr<ExpAST>> exps);

    [[nodiscard]] const std::shared_ptr<DecAST> &getDec() const;

    [[nodiscard]] const std::vector<std::shared_ptr<ExpAST>> &getExps() const;

private:
    std::shared_ptr<DecAST> _dec;
    std::vector<std::shared_ptr<ExpAST>> _exps;
};

class TypeAnnotationExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    TypeAnnotationExpAST(std::shared_ptr<ExpAST> exp,
                         std::shared_ptr<TypAST> typ);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp() const;

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp() const;

private:
    std::shared_ptr<ExpAST> exp;
    std::shared_ptr<TypAST> typ;
};

class ConjunctionExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    ConjunctionExpAST(std::shared_ptr<ExpAST> exp1,
                      std::shared_ptr<ExpAST> exp2);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp1() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp2() const;

private:
    std::shared_ptr<ExpAST> exp1;
    std::shared_ptr<ExpAST> exp2;
};

class DisjunctionExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    DisjunctionExpAST(std::shared_ptr<ExpAST> exp1,
                      std::shared_ptr<ExpAST> exp2);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp1() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp2() const;

private:
    std::shared_ptr<ExpAST> exp1;
    std::shared_ptr<ExpAST> exp2;
};

class ConditionalExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    ConditionalExpAST(std::shared_ptr<ExpAST> exp1,
                      std::shared_ptr<ExpAST> exp2,
                      std::shared_ptr<ExpAST> exp3);

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp1() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp2() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp3() const;

private:
    std::shared_ptr<ExpAST> exp1;
    std::shared_ptr<ExpAST> exp2;
    std::shared_ptr<ExpAST> exp3;
};

class IterationExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    IterationExpAST(std::shared_ptr<ExpAST> exp1,
                    std::shared_ptr<ExpAST> exp2);

private:
    std::shared_ptr<ExpAST> exp1;
    std::shared_ptr<ExpAST> exp2;
    std::shared_ptr<ExpAST> exp3;
};

class CaseAnalysisExpAST : public ExpAST {
DECL_ACCEPT_VISITOR
};

class FunctionExpAST : public ExpAST {
DECL_ACCEPT_VISITOR

public:
    FunctionExpAST(std::shared_ptr<MatchAST> match);

    [[nodiscard]] const std::shared_ptr<MatchAST> &getMatch() const;

    ~FunctionExpAST() override = default;

private:
    std::shared_ptr<MatchAST> match;
};

/*******************************************************************************
Patterns
*******************************************************************************/

class PatAST;

class ConstantPatAST;

class WildCardPatAST;

class VariablePatAST;

class ConstructionPatAST;

class InfixConstructionPatAST;

class ParenthesesPatAST;

class TuplePatAST;

class RecordPatAST;

class ListPatAST;

class TypeAnnotationPatAST;

class LayeredPatAST;

class PatRowAST;

class WildCardPatRowAST;

class PatternPatRowAST;

class VariablePatRowAST;

class PatAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    PatAST() = default;
};

class ConstantPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit ConstantPatAST(std::shared_ptr<ConAST> con);

    [[nodiscard]] const std::shared_ptr<ConAST> &getCon() const;

private:
    std::shared_ptr<ConAST> con;
};

class WildCardPatAST : public PatAST {
DECL_ACCEPT_VISITOR
};

class VariablePatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit VariablePatAST(std::shared_ptr<IdAST> id);

    [[nodiscard]] const std::shared_ptr<IdAST> &getId() const;

private:
    std::shared_ptr<IdAST> id;
};

class ConstructionPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit ConstructionPatAST(
            std::shared_ptr<LongIdAST> longId,
            std::shared_ptr<PatAST> pat = nullptr);

    [[nodiscard]] const std::shared_ptr<LongIdAST> &getLongId() const;

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat() const;

private:
    std::shared_ptr<LongIdAST> longId;
    std::shared_ptr<PatAST> pat;
};

class InfixConstructionPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit InfixConstructionPatAST(std::shared_ptr<PatAST> pat1,
                                     std::shared_ptr<IdAST> id,
                                     std::shared_ptr<PatAST> pat2);

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat1() const;

    [[nodiscard]] const std::shared_ptr<IdAST> &getId() const;

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat2() const;

private:
    std::shared_ptr<PatAST> pat1;
    std::shared_ptr<IdAST> id;
    std::shared_ptr<PatAST> pat2;
};

class ParenthesesPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit ParenthesesPatAST(std::shared_ptr<PatAST> pat);

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat() const;

private:
    std::shared_ptr<PatAST> pat;
};

class TuplePatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit TuplePatAST(std::vector<std::shared_ptr<PatAST>> pats);

    [[nodiscard]] const std::vector<std::shared_ptr<PatAST>> &getPats() const;


private:
    std::vector<std::shared_ptr<PatAST>> pats;
};

class RecordPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit RecordPatAST(std::shared_ptr<PatRowAST> patRow = nullptr);
};

class ListPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    explicit ListPatAST(std::vector<std::shared_ptr<PatAST>> pats);
};

class TypeAnnotationPatAST : public PatAST {
DECL_ACCEPT_VISITOR

public:
    TypeAnnotationPatAST(std::shared_ptr<PatAST> pat,
                         std::shared_ptr<TypAST> typ);

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat() const;

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp() const;

private:
    std::shared_ptr<PatAST> pat;
    std::shared_ptr<TypAST> typ;
};

class LayeredPatAST : public PatAST {
DECL_ACCEPT_VISITOR
};

class PatRowAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    PatRowAST() = default;
};

class WildCardPatRowAST : public PatRowAST {
DECL_ACCEPT_VISITOR
};

class PatternPatRowAST : public PatRowAST {
DECL_ACCEPT_VISITOR

public:
    PatternPatRowAST(
            std::shared_ptr<LabAST> lab,
            std::shared_ptr<PatAST> pat,
            std::shared_ptr<PatRowAST> patRow = nullptr);
};

class VariablePatRowAST : public PatRowAST {
DECL_ACCEPT_VISITOR

public:
    explicit VariablePatRowAST(std::shared_ptr<IdAST> id,
                               std::shared_ptr<TypAST> typ = nullptr,
                               std::shared_ptr<PatAST> asPat = nullptr,
                               std::shared_ptr<PatRowAST> patRow = nullptr);

private:
    std::shared_ptr<IdAST> id;
    std::shared_ptr<TypAST> typ;
    std::shared_ptr<PatAST> asPat;
    std::shared_ptr<PatRowAST> patRow;
};

/*******************************************************************************
Types
*******************************************************************************/

class TypAST;

class VariableTypAST;

class ConstructorTypAST;

class ParenthesesTypAST;

class FunctionTypAST;

class TupleTypAST;

class RecordTypAST;

class TypRowAST;

class TypAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    TypAST() = default;
};

class VariableTypAST : public TypAST {
DECL_ACCEPT_VISITOR

public:
    explicit VariableTypAST(std::shared_ptr<VarAST> var);

private:
    std::shared_ptr<VarAST> _var;
};

class ConstructorTypAST : public TypAST {
DECL_ACCEPT_VISITOR

public:
    explicit ConstructorTypAST(std::shared_ptr<LongIdAST> longId);

    [[nodiscard]] const std::shared_ptr<LongIdAST> &getLongId() const;

private:
    std::shared_ptr<LongIdAST> longId;
};

class ParenthesesTypAST : public TypAST {
DECL_ACCEPT_VISITOR

public:
    explicit ParenthesesTypAST(std::shared_ptr<TypAST> typ);

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp() const;

private:
    std::shared_ptr<TypAST> typ;
};

class FunctionTypAST : public TypAST {
DECL_ACCEPT_VISITOR

public:
    FunctionTypAST(
            std::shared_ptr<TypAST> typ1,
            std::shared_ptr<TypAST> typ2);

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp1() const;

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp2() const;

private:
    std::shared_ptr<TypAST> typ1;
    std::shared_ptr<TypAST> typ2;
};

class TupleTypAST : public TypAST {
DECL_ACCEPT_VISITOR

public:
    explicit TupleTypAST(std::vector<std::shared_ptr<TypAST>> tuple);

    [[nodiscard]] const std::vector<std::shared_ptr<TypAST>> &getTuple() const;

private:
    std::vector<std::shared_ptr<TypAST>> tuple;
};

class RecordTypAST : public TypAST {
DECL_ACCEPT_VISITOR

public:
    explicit RecordTypAST(std::shared_ptr<TypRowAST> typRow = nullptr);

    [[nodiscard]] const std::shared_ptr<TypRowAST> &getTypRow() const;

private:
    std::shared_ptr<TypRowAST> typRow;
};

class TypRowAST : public AST {
DECL_ACCEPT_VISITOR

public:
    TypRowAST(std::shared_ptr<LabAST> lab,
              std::shared_ptr<TypAST> typ,
              std::shared_ptr<TypRowAST> typRow = nullptr);

    [[nodiscard]] const std::shared_ptr<LabAST> &getLab() const;

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp() const;

    [[nodiscard]] const std::shared_ptr<TypRowAST> &getTypRow() const;

private:
    std::shared_ptr<LabAST> lab;
    std::shared_ptr<TypAST> typ;
    std::shared_ptr<TypRowAST> typRow;
};

/*******************************************************************************
Declarations
*******************************************************************************/

class DecAST;

class ValueDecAST;

class FunctionDecAST;

class TypeDecAST;

class DataTypeDecAST;

class SequenceDecAST;

class LocalDecAST;

class NonfixDecAST;

class LeftAssociativeInfixDecAST;

class RightAssociativeInfixDecAST;

class ValBindAST;

class DestructuringValBindAST;

class RecursiveValBindAST;

class ClausalFunctionValBindAST;

class FunBindAST;

class FunMatchAST;

class NonFixFunMatchAST;

class InfixFunMatchAST;

class TypBindAST;

class DatBindAST;

class ConBindAST;

class ExnBindAST;

class GenerativeExnBindAST;

class RenamingExnBindAST;

class DecAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    DecAST() = default;
};

class ValueDecAST : public DecAST {
DECL_ACCEPT_VISITOR

public:
    explicit ValueDecAST(std::shared_ptr<ValBindAST> valBind);

    [[nodiscard]] const std::shared_ptr<ValBindAST> &getValBind() const;

private:
    std::shared_ptr<ValBindAST> _valBind;
};

class FunctionDecAST : public DecAST {
DECL_ACCEPT_VISITOR

public:
    explicit FunctionDecAST(std::shared_ptr<FunBindAST> funBind);

    [[nodiscard]] const std::shared_ptr<FunBindAST> &getFunBind() const;

private:
    std::shared_ptr<FunBindAST> funBind;
};

class TypeDecAST : public DecAST {
DECL_ACCEPT_VISITOR

public:
    explicit TypeDecAST(std::shared_ptr<TypBindAST> typBind);

    [[nodiscard]] const std::shared_ptr<TypBindAST> &getTypBind() const;

private:
    std::shared_ptr<TypBindAST> _typBind;
};

class DataTypeDecAST : public DecAST {
DECL_ACCEPT_VISITOR
};

class SequenceDecAST : public DecAST {
DECL_ACCEPT_VISITOR

    explicit SequenceDecAST(std::vector<std::shared_ptr<DecAST>> decs);

    [[nodiscard]] const std::vector<std::shared_ptr<DecAST>> &getDecs() const;

private:
    std::vector<std::shared_ptr<DecAST>> _decs;
};

class LocalDecAST : public DecAST {
DECL_ACCEPT_VISITOR

    LocalDecAST(std::shared_ptr<DecAST> dec1, std::shared_ptr<DecAST> dec2);

private:
    std::shared_ptr<DecAST> dec1;
    std::shared_ptr<DecAST> dec2;
};

class NonfixDecAST : public DecAST {
DECL_ACCEPT_VISITOR

    explicit NonfixDecAST(std::vector<std::shared_ptr<IdAST>> ids);

private:
    std::vector<std::shared_ptr<IdAST>> ids;
};

class LeftAssociativeInfixDecAST : public DecAST {
DECL_ACCEPT_VISITOR

public:
    explicit LeftAssociativeInfixDecAST(
            std::vector<std::shared_ptr<IdAST>> ids,
            int priority = 0);

    [[nodiscard]] const std::vector<std::shared_ptr<IdAST>> &getIds() const;

    [[nodiscard]] int getPriority() const;

private:
    std::vector<std::shared_ptr<IdAST>> ids;
    int priority;
};

class RightAssociativeInfixDecAST : public DecAST {
DECL_ACCEPT_VISITOR

public:
    explicit RightAssociativeInfixDecAST(
            std::vector<std::shared_ptr<IdAST>> ids,
            int priority = 0);

    [[nodiscard]] const std::vector<std::shared_ptr<IdAST>> &getIds() const;

    [[nodiscard]] int getPriority() const;

private:
    std::vector<std::shared_ptr<IdAST>> ids;
    int priority;
};

class ValBindAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    ValBindAST() = default;
};

class DestructuringValBindAST : public ValBindAST {
DECL_ACCEPT_VISITOR

public:
    DestructuringValBindAST(
            std::shared_ptr<PatAST> pat,
            std::shared_ptr<ExpAST> exp,
            std::shared_ptr<ValBindAST> andValBind = nullptr);

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp() const;

    [[nodiscard]] const std::shared_ptr<ValBindAST> &getAndValBind() const;

    void setAndValBind(const std::shared_ptr<ValBindAST> &andValBind);

private:
    std::shared_ptr<PatAST> _pat;
    std::shared_ptr<ExpAST> _exp;
    std::shared_ptr<ValBindAST> _andValBind;
};

class RecursiveValBindAST : public ValBindAST {
DECL_ACCEPT_VISITOR

public:
    explicit RecursiveValBindAST(
            std::shared_ptr<ValBindAST> valBind);

    [[nodiscard]] const std::shared_ptr<ValBindAST> &getValBind() const;

private:
    std::shared_ptr<ValBindAST> valBind;
};

class FunBindAST : public AST {
DECL_ACCEPT_VISITOR

public:
    explicit FunBindAST(
            std::shared_ptr<FunMatchAST> funMatch,
            std::shared_ptr<FunBindAST> andFunBind = nullptr);

    [[nodiscard]] const std::shared_ptr<FunMatchAST> &getFunMatch() const;

    [[nodiscard]] const std::shared_ptr<FunBindAST> &getAndFunBind() const;

    void setAndFunBind(const std::shared_ptr<FunBindAST> &andFunBind);

private:
    std::shared_ptr<FunMatchAST> _funMatch;
    std::shared_ptr<FunBindAST> _andFunBind;
};

class FunMatchAST : public AST {
DECL_ACCEPT_VISITOR

public:
    FunMatchAST(std::shared_ptr<IdAST> id,
                std::vector<std::shared_ptr<PatAST>> pats,
                std::shared_ptr<ExpAST> exp,
                std::shared_ptr<TypAST> typ = nullptr,
                std::shared_ptr<FunMatchAST> orFunMatch = nullptr);

    [[nodiscard]] const std::shared_ptr<IdAST> &getId() const;

    [[nodiscard]] const std::vector<std::shared_ptr<PatAST>> &getPats() const;

    [[nodiscard]] const std::shared_ptr<ExpAST> &getExp() const;

    [[nodiscard]] const std::shared_ptr<FunMatchAST> &getOrFunMatch() const;

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp() const;

    void setOrFunMatch(const std::shared_ptr<FunMatchAST> &orFunMatch);

protected:
    std::shared_ptr<IdAST> _id;
    std::vector<std::shared_ptr<PatAST>> _pats;
    std::shared_ptr<ExpAST> _exp;
    std::shared_ptr<TypAST> _typ;
    std::shared_ptr<FunMatchAST> _orFunMatch;
};


class NonFixFunMatchAST : public FunMatchAST {
DECL_ACCEPT_VISITOR

public:
    explicit NonFixFunMatchAST(
            std::shared_ptr<IdAST> id,
            std::vector<std::shared_ptr<PatAST>> pats,
            std::shared_ptr<ExpAST> exp, std::shared_ptr<TypAST> typ = nullptr,
            std::shared_ptr<FunMatchAST> orFunMatch = nullptr);

};

class InfixFunMatchAST : public FunMatchAST {
DECL_ACCEPT_VISITOR

public:

    explicit InfixFunMatchAST(std::shared_ptr<IdAST> id,
                              std::vector<std::shared_ptr<PatAST>> pats,
                              std::shared_ptr<ExpAST> exp,
                              std::shared_ptr<TypAST> typ = nullptr,
                              std::shared_ptr<FunMatchAST> orFunMatch = nullptr);

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat1() const;

    [[nodiscard]] const std::shared_ptr<PatAST> &getPat2() const;

private:
    std::shared_ptr<PatAST> _pat1;
    std::shared_ptr<PatAST> _pat2;
};

class TypBindAST : public AST {
DECL_ACCEPT_VISITOR

public:
    TypBindAST(std::shared_ptr<IdAST> id,
               std::shared_ptr<TypAST> typ,
               std::shared_ptr<TypBindAST> andTypBind = nullptr);

    [[nodiscard]] const std::shared_ptr<IdAST> &getId() const;

    [[nodiscard]] const std::shared_ptr<TypAST> &getTyp() const;

    [[nodiscard]] const std::shared_ptr<TypBindAST> &getTypBind() const;

    void setTypBind(const std::shared_ptr<TypBindAST> &typBind);

private:
    std::shared_ptr<IdAST> id;
    std::shared_ptr<TypAST> typ;
    std::shared_ptr<TypBindAST> typBind;
};

class DatBindAST : public AST {
DECL_ACCEPT_VISITOR
};

class ConBindAST : public AST {
DECL_ACCEPT_VISITOR
};

class ExnBindAST : public AST {
DECL_ACCEPT_VISITOR

protected:
    ExnBindAST() = default;
};

class GenerativeExnBindAST : public ExnBindAST {
DECL_ACCEPT_VISITOR
};

class RenamingExnBindAST : public ExnBindAST {
DECL_ACCEPT_VISITOR
};

#undef DECL_ACCEPT_VISITOR
//endregion
