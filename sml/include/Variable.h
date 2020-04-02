#pragma once

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "llvm/IR/Value.h"

class [[deprecated("Use SymbolTable/Value -s instead.")]] Variable {
public:
    enum Type {
        INT,
        REAL,
        CHAR,
        STRING,
        BOOL,
        UNIT,
        FUNCTION,
        TYPE,
    };

    enum InfixType {
        NONFIX,
        INFIX,
        INFIXR,
    };

    [[nodiscard]] int getPriority() const { return _priority; }

    void setPriority(int n) { _priority = n; }

    [[nodiscard]] InfixType getInfixType() const { return _infixType; }

    void setInfixType(InfixType infixType) { _infixType = infixType; }

    [[nodiscard]] Type getType() const { return _type; }

    virtual void getValue(void *ptr) const = 0;

protected:
    int _priority {};

    InfixType _infixType{};

    Type _type {};
};

//region define builtin variables with simple types
#define DEFINE_SIMPLE_VARIABLE(CLASS, ENUM, POD) \
class CLASS : public Variable {\
public:\
    explicit CLASS(POD value) : \
    __v(std::move(value)) { _priority = 0; _type = ENUM; _infixType = NONFIX; }\
    void getValue(void *p) const override { *static_cast<POD*>(p) = __v; } \
private:\
    POD __v;\
};

DEFINE_SIMPLE_VARIABLE(IntVariable, INT, int)

DEFINE_SIMPLE_VARIABLE(BoolVariable, BOOL, bool)

DEFINE_SIMPLE_VARIABLE(CharVariable, CHAR, char)

DEFINE_SIMPLE_VARIABLE(RealVariable, REAL, double)

DEFINE_SIMPLE_VARIABLE(StringVariable, STRING, std::string)

#undef DEFINE_SIMPLE_VARIABLE
//endregion

class UnitVariable : public Variable {
public:
    explicit UnitVariable() = default;
    void getValue(void *p) const override {}
};

class FunctionVariable : public Variable {
public:

};

class StructureVariable : public Variable {
public:

};
