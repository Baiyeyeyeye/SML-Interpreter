#pragma once

#include "AST/ASTVisitor.h"
#include "SemanticAnalyzer.h"
#include "SemanticAnalyzerImpl.h"

class Type;

class Value;

class BoolType;

class IntType;

class CharType;

class StringType;

class UnitType;

class RealType;

class VariableTypeNameType;

// TODO: add detailed comments and sort the functions
class TypeCheck : public ASTVisitor {
public:
    TypeCheck();

    void *visit(TupleExpAST *ast) override;

    void *visit(VariablePatAST *ast) override;

    void *visit(TypeAnnotationExpAST *ast) override;

    void *visit(TypeAnnotationPatAST *ast) override;

    void *visit(DestructuringValBindAST *ast) override;

    void *visit(ValueOrConstructorIdentifierExpAST *ast) override;

    void *visit(FunctionExpAST *ast) override;

    void *visit(FunctionTypAST *ast) override;

    void *visit(ValueDecAST *ast) override;

    void *visit(FloatConAST *ast) override;

    void *visit(IntConAST *ast) override;

    void *visit(BoolConAST *ast) override;

    void *visit(CharConAST *ast) override;

    void *visit(StringConAST *ast) override;

    void *visit(ConstantExpAST *ast) override;

    void *visit(IdAST *ast) override;

    void *visit(ListExpAST *ast) override;

    void *visit(ApplicationExpAST *ast) override;

    void *visit(InfixApplicationExpAST *ast) override;

    void *visit(ConjunctionExpAST *ast) override;

    void *visit(DisjunctionExpAST *ast) override;

    void *visit(ConditionalExpAST *ast) override;

    void *visit(ConstructorTypAST *ast) override;

    void *visit(LongIdAST *ast) override;

    void *visit(MatchAST *ast) override;

    void *visit(ConstructionPatAST *ast) override;

    void *visit(TuplePatAST *ast) override;

    void *visit(LocalDeclarationExpAST *ast) override;

    void *visit(SequenceDecAST *ast) override;

    void *visit(FunctionDecAST *ast) override;

    void *visit(NonFixFunMatchAST *ast) override;

    void *visit(InfixFunMatchAST *ast) override;

    void *visit(FunBindAST *ast) override;

    void *visit(LeftAssociativeInfixDecAST *ast) override;

    void *visit(RightAssociativeInfixDecAST *ast) override;

    Type *unify(Type *t1, Type *t2, AST *ast = nullptr);

    Type *unify(void *type, AST *ast);

    Type *verify(Type *type);

    void fillTypes();

private:

    class TempVarType {
    public:
        void reset();

        std::string nextBig();

        std::string nextSmall();

    private:
        std::string curBig = "Z";
        std::string curSmall = "a";
    } tempVarType;

    // Basic type cache for type check convenience.
    IntType *getIntType();

    BoolType *getBoolType();

    RealType *getRealType();

    CharType *getCharType();

    StringType *getStringType();

    IntType *_intType{};
    RealType *_realType{};
    BoolType *_boolType{};
    CharType *_charType{};
    StringType *_stringType{};

    // Like `fn x => x + 1', we induce that x is int, then fill in the symbol
    // table a new x with int type. Also, for cases like `fn (x, y) => (y, x)',
    // still we cannot induce the type of both x and y, but we now sort their
    // type to be `'a' and `'b', respective.
    std::vector<std::pair<std::string, Type *>> _varTypePatternToFill;

    void insertVarTypePatternToFill(const std::string &name, Type *type);

    void fillVarTypePatterns();

    [[nodiscard]] Type *visitBoolExp(
            const std::shared_ptr<ExpAST> &exp, AST *ast);

    Type *find(Type *t);

    Type *uni(Type *t1, Type *t2);

    inline Type *visitAsType(const std::shared_ptr<AST> &ast) {
        return static_cast<Type *>(ast->accept(this));
    }

    std::unordered_map<Type *, Type *> dsu;

    /**
     * The next identifier to search is a type or a value.
     * val int : int = int : int;
     * Here we have two `int' to search. Store this info in the stack.
     */
    enum NextIdToSearchType {
        TYPE = 0,
        VALUE = 1, ///< must be defined
        PATTERN = 2, ///< may not be defined and to be assigned
    };

    struct NextIdToSearchGuard {
    public:
        explicit NextIdToSearchGuard(NextIdToSearchType type) noexcept;

        ~NextIdToSearchGuard() noexcept;

        static std::vector<NextIdToSearchType> _nextIdToSearch;
    };

    NextIdToSearchType getNextIdToSearch() const;

    struct IncreaseDepthGuard {
    public:
        IncreaseDepthGuard() noexcept;

        ~IncreaseDepthGuard() noexcept;

        static int _depth;

        static std::vector<std::unordered_map<std::string, Type *>> _localTypes;
    };

    Type *getNextVariableTypeNameType();

    void insertPatternType(const std::string &name, Type *type);

    Type *getPatternType(const std::string &name);
};
