#pragma once

#include <ostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace llvm {
    class Value;

    class Type;

    class LLVMContext;
}

/**
 * Memory cache collector for all classes declared in this header. Any new
 * operator would cache the pointer and ready to be released on clear.
 */
class MemoryCachedSymbol {
public:
    void *operator new(size_t sz) noexcept;

    void operator delete(void *) noexcept;

    virtual void saveDeleteThis() noexcept;

    static void clearMemory() noexcept;

protected:
    MemoryCachedSymbol() noexcept = default;
};

class Type;

class IntType;

class RealType;

class BoolType;

class CharType;

class StringType;

class TupleType;

class UnitType;

class ListType;

class RecordType;

class FunctionType;

class TypeNameType;

class VariableTypeNameType;

/**
 * Base Type.
 */
class Type : public MemoryCachedSymbol {
public:
    enum TypeId {
        INT, ///< int
        REAL, ///< real
        STRING, ///< string
        CHAR, ///< char
        BOOL, ///< bool
        UNIT, ///< unit ()
        LIST, ///< [...]
        RECORD, ///< {...}
        TUPLE, ///< (... * ...)
        FUNCTION, ///< param -> ret
        TYPE_NAME, ///< type alias, like n of type n = int, excluding builtins
        VARIABLE_TYPE_NAME, ///< variable type names like 'a
    };

    /**
     * Get the type id so that we could do down cast to concrete type subclass.
     * @return The type id.
     */
    [[nodiscard]] virtual TypeId getTypeId() const = 0;

    /**
     * Creator that returns a pointer to a class, respective. Each subclass
     * should implement its own create function and return a non-null pointer
     * of its instance. This function is not guaranteed to return a unique
     * pointer on each call.
     * @return A pointer to the concrete class. If subclass does not implement
     * its own create function, this would return a null pointer.
     */
    static Type *create() { return nullptr; }

    virtual std::ostream &print(std::ostream &o) const { return o; }

    //region down casts for convenience
#define DOWN_CAST_TO(SUBCLASS) \
[[nodiscard]] SUBCLASS* to##SUBCLASS() const {\
    return static_cast<SUBCLASS*>(static_cast<void*>(const_cast<Type*>(this)));\
}

    DOWN_CAST_TO(IntType)

    DOWN_CAST_TO(RealType)

    DOWN_CAST_TO(CharType)

    DOWN_CAST_TO(BoolType)

    DOWN_CAST_TO(StringType)

    DOWN_CAST_TO(TupleType)

    DOWN_CAST_TO(UnitType)

    DOWN_CAST_TO(ListType)

    DOWN_CAST_TO(RecordType)

    DOWN_CAST_TO(FunctionType)

    DOWN_CAST_TO(TypeNameType)

    DOWN_CAST_TO(VariableTypeNameType)

#undef DOWN_CAST_TO
    //endregion

    [[nodiscard]] llvm::Type *getLLVMType() const;

protected:
    /**
     * Constructor.
     *
     * Subclasses should give a type id for down cast convenience, and a pointer
     * to a concrete llvm's type.
     * @param typeId The type id.
     * @param llvmType The raw LLVM type.
     */
    explicit Type(llvm::Type *llvmType = nullptr);

protected:
    llvm::Type *_llvmType{};
};

class IntType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    /**
     * Create an int type's pointer. It is not guaranteed to return a unique
     * pointer on each call.
     * @return A non-null pointer of IntType.
     */
    static IntType *create();

    std::ostream &print(std::ostream &o) const override;

private:
    IntType();
};

class RealType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    static RealType *create();

    std::ostream &print(std::ostream &o) const override;

private:
    RealType();
};

class BoolType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    static BoolType *create();

    std::ostream &print(std::ostream &o) const override;

private:
    BoolType();
};

class CharType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    static CharType *create();

    std::ostream &print(std::ostream &o) const override;

private:
    CharType();
};

class StringType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    static StringType *create();

    std::ostream &print(std::ostream &o) const override;

private:
    StringType();
};

class TupleType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    static TupleType *create(const std::vector<Type *> &types);

    [[nodiscard]] const std::vector<Type *> &getTypes() const;

    std::ostream &print(std::ostream &o) const override;

private:
    explicit TupleType(const std::vector<Type *> &types);

    std::vector<Type *> _types;

    std::vector<llvm::Type *> temTypes;
};

class UnitType : public Type {
public:
    [[nodiscard]] TypeId getTypeId() const override;

    static UnitType *create();

    std::ostream &print(std::ostream &o) const override;

private:
    UnitType();
};

class ListType : public Type {
public:
    static ListType *create(Type *subtype);

    [[nodiscard]] Type *getSubtype() const;

    [[nodiscard]] TypeId getTypeId() const override;

    std::ostream &print(std::ostream &o) const override;

private:
    explicit ListType(Type *subtype);

    Type *_subType;
};

class RecordType : public Type {
public:
    static RecordType *create(std::map<std::string, Type *> record);

    [[nodiscard]] const std::map<std::string, Type *> &getRecordEntries() const;

    [[nodiscard]] TypeId getTypeId() const override;

private:
    explicit RecordType(std::map<std::string, Type *> record);
};

class FunctionType : public Type {
public:
    static FunctionType *create(Type *ret, Type *param);

    static FunctionType *create(std::vector<std::pair<Type *, Type *>> tys);

    [[nodiscard]] Type *getParameterType(size_t idx = 0) const;

    [[nodiscard]] Type *getReturnType(size_t idx = 0) const;

    [[nodiscard]] size_t getOverloadedNum() const;

    [[nodiscard]] bool isOverloaded() const;

    [[nodiscard]] const std::vector<std::pair<Type *, Type *>> &
    getTypes() const;

    [[nodiscard]] TypeId getTypeId() const override;

    std::ostream &print(std::ostream &o) const override;

private:
    FunctionType(Type *ret, Type *param);

    explicit FunctionType(std::vector<std::pair<Type *, Type *>> tys);

    std::vector<std::pair<Type *, Type *>> _types;
};

/**
 * Type name bind.
 *
 * A type name bind consists of the literal name and the bound type, for
 * example: `type a = int', here `a' is the literal name, and `int''s type i.e.
 * a pointer of IntType * is the bound type, or the aliased type.
 */
class TypeNameType : public Type {
public:
    static TypeNameType *create(Type *aliasAs);

    [[nodiscard]] TypeId getTypeId() const override;

    [[nodiscard]] const std::string &getName() const;

    /**
     * Get the directed bound type of current type name, i.e. if we created two
     * declarations: `type a = int and type b = a', then the directed bound type
     * of `b' should be `a' instead of `int'.
     * @return The directed bound type.
     */
    [[nodiscard]] Type *getBoundType() const;

    std::ostream &print(std::ostream &o) const override;

private:
    explicit TypeNameType(Type *aliasAs);

    Type *_aliasAs;

    std::string _name;
};

/**
 * Variable type name.
 *
 * A variable type name consists only the literal name, like 'a.
 */
class VariableTypeNameType : public Type {
public:
    static VariableTypeNameType *create(std::string var);

    [[nodiscard]] TypeId getTypeId() const override;

    /**
     * Get the variable name's literal.
     * @return The variable name.
     */
    [[nodiscard]] const std::string &getVar() const;

    std::ostream &print(std::ostream &o) const override;

private:
    explicit VariableTypeNameType(std::string var);

    std::string _var;
};

class Value;

class IntValue;

class RealValue;

class StringValue;

class BoolValue;

class CharValue;

class TupleValue;

class UnitValue;

class ListValue;

class RecordValue;

class FunctionValue;

class VariableTypeNameValue;

class Value : public MemoryCachedSymbol {
public:
    using TypeId = Type::TypeId;

    [[nodiscard]] Type *getType() const;

    virtual std::ostream &print(std::ostream &o) const { return o; }

    //region down casts for convenience
#define DOWN_CAST_TO(SUBCLASS) \
[[nodiscard]] SUBCLASS* to##SUBCLASS() const \
{return static_cast<SUBCLASS*>(static_cast<void*>(const_cast<Value*>(this)));}

    DOWN_CAST_TO(IntValue)

    DOWN_CAST_TO(RealValue)

    DOWN_CAST_TO(CharValue)

    DOWN_CAST_TO(StringValue)

    DOWN_CAST_TO(BoolValue)

    DOWN_CAST_TO(TupleValue)

    DOWN_CAST_TO(UnitValue)

    DOWN_CAST_TO(ListValue)

    DOWN_CAST_TO(RecordValue)

    DOWN_CAST_TO(FunctionValue)

    DOWN_CAST_TO(VariableTypeNameValue)

#undef DOWN_CAST_TO
    //endregion

    static Value *createDefaultOfType(Type *type);

    [[nodiscard]] llvm::Value *getLLVMValue() const;

protected:
    Value(Type *type, llvm::Value *llvmValue);

private:
    Type *_type;
    llvm::Value *_llvmValue;
};

class IntValue : public Value {
public:
    /**
     * Create an int value with an integer.
     * @param v The integer.
     * @return The int value.
     */
    static IntValue *create(int v = 0);

    /**
     * Get the stored int value.
     * @return The int value.
     */
    [[nodiscard]] int get() const;

private:
    explicit IntValue(int v);
};

class RealValue : public Value {
public:
    static RealValue *create(double v = 0);

    [[nodiscard]] double get() const;

private:
    explicit RealValue(double v);
};

class StringValue : public Value {
public:
    static StringValue *create(std::string str = "");

    [[nodiscard]] const std::string &get() const;

private:
    explicit StringValue(std::string str);
};

class CharValue : public Value {
public:
    static CharValue *create(char c = 0);

    [[nodiscard]] char get() const;

private:
    explicit CharValue(char c);
};

class BoolValue : public Value {
public:
    static BoolValue *create(bool b = false);

    [[nodiscard]] bool get() const;

private:
    explicit BoolValue(bool b);
};

class TupleValue : public Value {
public:
    static TupleValue *create(std::vector<Value *> values = {});

    [[nodiscard]] const std::vector<Value *> &get() const;

private:
    explicit TupleValue(std::vector<Value *> values);
};

class UnitValue : public Value {
public:
    static UnitValue *create();

private:
    explicit UnitValue();
};

class ListValue : public Value {
public:
    static ListValue *create(std::vector<Value *> values = {});

    [[nodiscard]] const std::vector<Value *> &get() const;

private:
    explicit ListValue(std::vector<Value *> values);
};

class RecordValue : public Value {
public:
    static RecordValue *create(std::map<std::string, Value *> record = {});

    [[nodiscard]] const std::map<std::string, Value *> &get() const;

private:
    explicit RecordValue(std::map<std::string, Value *> record);
};

class FunctionValue : public Value {
public:
    /**
     * Binary operation types.
     */
    enum OperationType {
        NONFIX,
        INFIX,
        INFIXR,
    };

    /**
     * Create a function value with a name and pointer to function.
     * @param fptr The pointer to function.
     * @return A function value.
     */
    static FunctionValue *create(Type *retTy, Type *paramTy, void *fptr);

    static FunctionValue *create(FunctionType *funTy, void *fptr);

    static FunctionValue *create(
            std::vector<std::pair<FunctionType *, void *>> _values = {});

    [[nodiscard]] const std::vector<std::pair<FunctionType *, void *>> &
    get() const;

    /**
     * Get the priority of this function.
     * @return An integer in range [0, 9].
     */
    [[nodiscard]] int getPriority() const;

    /**
     * Check if the function is overloaded.
     * @return True if function is overloaded.
     */
    [[nodiscard]] bool isOverloaded() const;

    /**
     * Set the priority of this function to min(9, max(0, n)), i.e. the priority
     * should be in range [0, 9].
     * @param n The priority.
     */
    void setPriority(int n);

    /**
     * Get the operation type of function, which only makes sense when it is a
     * binary operation.
     * @return The operation type.
     */
    [[nodiscard]] OperationType getOperationType() const;

    /**
     * Set the operation type of function, which only makes sense when it is a
     * binary operation.
     * @param type The operation type.
     */
    void setOperationType(OperationType type);

private:
    FunctionValue(Type *retTy, Type *paramTy, void *fptr);

    FunctionValue(FunctionType *funTy, void *fptr);

    explicit FunctionValue(
            std::vector<std::pair<FunctionType *, void *>> _values);

    int _priority{};

    OperationType _operationType{NONFIX};

    std::vector<std::pair<FunctionType *, void *>> _values;
};

class VariableTypeNameValue : public Value {
public:
    static VariableTypeNameValue *create(VariableTypeNameType *t);

    static VariableTypeNameValue *create(std::string var);

private:
    explicit VariableTypeNameValue(VariableTypeNameType *type);
};

class SymbolTable {
public:
    /**
     * Reset the singleton instance and all symbol caches.
     */
    static void reset();

    /**
     * Get the singleton instance.
     * @return The instance of symbol table.
     */
    static SymbolTable *getInstance();

    /**
     * Insert the value with its name to current scope.
     *
     * If the name already exists, replace it with this new one.
     * @param name The name.
     * @param value The value.
     */
    void insertValue(const std::string &name, Value *value);

    /**
     * Insert the type with its name to current scope.
     *
     * If the name already exists, replace it with this new one.
     * @param name The name.
     * @param type The type.
     */
    void insertType(const std::string &name, Type *type);

    void insertPatternType(const std::string &name, Type *type);

    /**
     * Return a value in current scope with a name.
     * @param name The name of value.
     * @return The pointer of a value, null if not defined.
     */
    [[deprecated("Use getPatternType instead.")]]
    [[nodiscard]] Value *getValue(const std::string &name) const;

    /**
     * Return a type in current scope with a name.
     * @param name The name of type.
     * @return The pointer of a type, null if not defined.
     */
    [[nodiscard]] Type *getType(const std::string &name) const;

    [[nodiscard]] Type *getPatternType(const std::string &name) const;

    /**
     * Remove the value of this name. If no such name exists, it does nothing.
     * @param name The name of value.
     */
    void removeValue(const std::string &name);

    /**
     * Remove the type of this name. If no such name exists, it does nothing.
     * @param name The name of type.
     */
    void removeType(const std::string &name);

    void removePatternType(const std::string &name);

    struct Operator {
        enum OperatorType {
            NONFIX, INFIX, INFIXR
        } operatorType{};
        int priority{};
    };

    void setOperator(const std::string &name, Operator anOperator);

    const Operator *getOperator(const std::string &name) const;

    void setTempValueMap(const std::unordered_map<std::string, std::string> &tempValueMap);

    llvm::LLVMContext &getLLVMContext();

private:
    SymbolTable();

    ~SymbolTable();

    void initBuiltinSymbols();

    std::unordered_map<std::string, Value *> _valueMap;

    std::unordered_map<std::string, Type *> _typeMap;

    std::unordered_map<std::string, Type *> _patternTypeMap;

    std::unordered_map<std::string, Operator> _operatorMap;

    std::unordered_map<std::string,std::string> _tempValueMap; //比如在let中声明的val a  = 1;存个名字就可以

    std::unordered_map<std::string, Operator> _tempFunMap; // 存名字、结合性、优先级



    std::unique_ptr<llvm::LLVMContext> _context;
};

std::ostream &operator<<(std::ostream &o, Type const &type);

std::ostream &operator<<(std::ostream &o, Value const &value);
