#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "SymbolTable.h"

using namespace std;

inline namespace SymbolsCache {
    SymbolTable *symbolTableInstance;

    unordered_set<void *> pointerCache;

    unordered_map<int, unique_ptr<IntValue>> intValueCache;

    llvm::LLVMContext context;

    auto &getLLVMContext() {
        return context;
    }
}

template<typename TMap>
static void removeFromMap(const std::string &name, TMap &map) {
    auto it = map.find(name);
    if (it == map.end()) {
        return;
    }
    map.erase(it);
}

template<typename TMap, typename TVal>
static void insertToMap(const std::string &name, TVal &&val, TMap &map) {
    map[name] = val;
}

template<typename TRet, typename TMap>
static TRet getFromMap(const std::string &name, TMap &&map) {
    auto it = map.find(name);
    if (it == map.end()) {
        return nullptr;
    }
    return it->second;
}

void *MemoryCachedSymbol::operator new(size_t sz) noexcept {
    auto res = ::operator new(sz, nothrow);
    pointerCache.insert(res);
    return res;
}

void MemoryCachedSymbol::operator delete(void *p) noexcept {
    if (auto it = pointerCache.find(p); it != pointerCache.end()) {
        ::operator delete(p);
        pointerCache.erase(it);
    }
}

void MemoryCachedSymbol::clearMemory() noexcept {
    for (auto &&p : pointerCache) {
        ::operator delete(p);
    }
    pointerCache.clear();
}

void MemoryCachedSymbol::saveDeleteThis() noexcept {
    delete this;
}

std::ostream &operator<<(std::ostream &o, Type const &type) {
    return type.print(o);
}

std::ostream &operator<<(std::ostream &o, Value const &value) {
    return value.print(o);
}

llvm::Type *Type::getLLVMType() const {
    return _llvmType;
}

Type::Type(llvm::Type *llvmType)
        : _llvmType(llvmType) {

}

IntType::IntType()
        : Type(llvm::Type::getInt32Ty(getLLVMContext())) {

}

IntType *IntType::create() {
    static IntType intType;
    return &intType;
}

Type::TypeId IntType::getTypeId() const {
    return INT;
}

ostream &IntType::print(std::ostream &o) const {
    return o << "int";
}

RealType::RealType() : Type(llvm::Type::getFloatTy(getLLVMContext())) {

}

RealType *RealType::create() {
    static RealType realType;
    return &realType;
}

Type::TypeId RealType::getTypeId() const {
    return REAL;
}

ostream &RealType::print(std::ostream &o) const {
    return o << "real";
}

TupleType::TupleType(const std::vector<Type *> &types)
        : Type(nullptr), _types(types) {
    for (int i = 0; i < types.size(); i++) {
        TupleType::temTypes.push_back(types[i]->getLLVMType());
    }
    llvm::ArrayRef temArrayRef = llvm::ArrayRef(temTypes);
    this->_llvmType = llvm::StructType::get(getLLVMContext(), temArrayRef);
}

TupleType *TupleType::create(const std::vector<Type *> &types) {
    return new TupleType(types);
}

const std::vector<Type *> &TupleType::getTypes() const {
    return _types;
}

Type::TypeId TupleType::getTypeId() const {
    return TUPLE;
}

ostream &TupleType::print(std::ostream &o) const {
    o << '(';
    for (auto &&elem : this->getTypes()) {
        elem->print(o);
    }
    o << ')';
    return o;
}

SymbolTable::SymbolTable() {
    initBuiltinSymbols();
}

SymbolTable::~SymbolTable() = default;

SymbolTable *SymbolTable::getInstance() {
    if (symbolTableInstance == nullptr) {
        symbolTableInstance = new SymbolTable;
    }
    return symbolTableInstance;
}

void SymbolTable::reset() {
    delete symbolTableInstance;
    symbolTableInstance = nullptr;
}

void SymbolTable::insertValue(const std::string &name, Value *value) {
    insertToMap(name, value, _valueMap);
}

Value *SymbolTable::getValue(const std::string &name) const {
    return getFromMap<Value *>(name, _valueMap);
}

void SymbolTable::removeValue(const std::string &name) {
    removeFromMap(name, _valueMap);
}

llvm::LLVMContext &SymbolTable::getLLVMContext() {
    return *(_context ? _context : _context = make_unique<llvm::LLVMContext>());
}

Type *SymbolTable::getType(const std::string &name) const {
    return getFromMap<Type *>(name, _typeMap);
}

void SymbolTable::insertType(const std::string &name, Type *type) {
    insertToMap(name, type, _typeMap);
}

void SymbolTable::removeType(const std::string &name) {
    removeFromMap(name, _typeMap);
}

void SymbolTable::initBuiltinSymbols() {
    auto setType = [this](const char *name, Type *type) {
        _typeMap[name] = type;
    };
    setType("int", IntType::create());
    setType("real", RealType::create());
    setType("string", StringType::create());
    setType("unit", UnitType::create());
    setType("bool", BoolType::create());
    setType("char", CharType::create());

    // FIXME: pattern and value maps are duplicated.
    setOperator("*", {Operator::INFIX, 7});
    setOperator("/", {Operator::INFIX, 7});
    setOperator("div", {Operator::INFIX, 7});
    setOperator("mod", {Operator::INFIX, 7});
    setOperator("+", {Operator::INFIX, 6});
    setOperator("-", {Operator::INFIX, 6});
    setOperator("^", {Operator::INFIX, 6});
    setOperator("=", {Operator::INFIX, 4});
    setOperator("<>", {Operator::INFIX, 4});
    setOperator(">=", {Operator::INFIX, 4});
    setOperator("<=", {Operator::INFIX, 4});
    setOperator(">", {Operator::INFIX, 4});
    setOperator("<", {Operator::INFIX, 4});

    // TODO: init builtin operators & functions
    auto setValue = [this](const char *name, Value *value) {
        _valueMap[name] = value;
    };

    // infix operators' priority:
    // infix 7 * / div mod
    // infix 6 + - ^
    // infixr 5 :: @
    // infix 4 = <> > >= < <=
    // infix 3 := o
    // infix 0 before

    // the string concat operator
    setValue("^", []() {
        auto res = FunctionValue::create(
                StringType::create(),
                TupleType::create(
                        {StringType::create(), StringType::create()}),
                /*TODO: string concat fptr*/ nullptr);
        res->setPriority(6);
        res->setOperationType(FunctionValue::INFIX);
        return res;
    }());

    // the list concat operator
    setValue("@", []() {
        auto aListType = ListType::create(VariableTypeNameType::create("'a"));
        auto res = FunctionValue::create(
                aListType,
                TupleType::create({aListType, aListType}),
                /*TODO: list concat fptr*/ nullptr);
        res->setPriority(5);
        res->setOperationType(FunctionValue::INFIXR);
        return res;
    }());

    // TODO: + * - is overridden, how to?

    // for reals' and ints' + * operation.
    auto setValueSimpleArithmetic = [&setValue](const char *op, int pri) {
        setValue(op, [pri]() {
            auto intType = IntType::create();
            auto intAdd = FunctionType::create(
                    intType,
                    TupleType::create({intType, intType}));
            auto realType = RealType::create();
            auto realAdd = FunctionType::create(
                    realType,
                    TupleType::create({realType, realType}));
            auto res = FunctionValue::create(
                    {{intAdd, /*TODO: int add impl*/  nullptr},
                     {realAdd, /*TODO: real add impl*/nullptr}});
            res->setOperationType(FunctionValue::OperationType::INFIX);
            res->setPriority(pri);
            return res;
        }());
    };
    setValueSimpleArithmetic("+", 6);
    setValueSimpleArithmetic("-", 6);
    setValueSimpleArithmetic("*", 7);

    // for int's and real's negation
    setValue("~", []() {
        auto intType = IntType::create();
        auto realType = RealType::create();
        auto intNeg = FunctionType::create(
                intType,
                TupleType::create({intType}));
        auto realNeg = FunctionType::create(
                realType,
                TupleType::create({realType}));
        auto res = FunctionValue::create(
                {{intNeg, /*TODO: int neg impl*/  nullptr},
                 {realNeg, /*TODO: real neg impl*/nullptr}}
        );
        return res;
    }());
}

void SymbolTable::insertPatternType(const std::string &name, Type *type) {
    insertToMap(name, type, _patternTypeMap);
}

Type *SymbolTable::getPatternType(const std::string &name) const {
    return getFromMap<Type *>(name, _patternTypeMap);
}

void SymbolTable::removePatternType(const std::string &name) {
    removeFromMap(name, _patternTypeMap);
}

void SymbolTable::setOperator(const std::string &name,
                              SymbolTable::Operator anOperator) {
    _operatorMap[name] = anOperator;
}

const SymbolTable::Operator *
SymbolTable::getOperator(const std::string &name) const {
    return _operatorMap.count(name) ?
           &_operatorMap.find(name)->second : nullptr;
}

void SymbolTable::setTempValueMap(const unordered_map<std::string, std::string> &tempValueMap) {
    _tempValueMap = tempValueMap;
}

Value::Value(Type *type, llvm::Value *llvmValue)
        : _type(type), _llvmValue(llvmValue) {

}

llvm::Value *Value::getLLVMValue() const {
    return _llvmValue;
}

Type *Value::getType() const {
    return _type;
}

Value *Value::createDefaultOfType(Type *type) {
    if (!type) {
        return nullptr;
    }
    switch (type->getTypeId()) {
        case Type::INT:
            return IntValue::create();
        case Type::REAL:
            return RealValue::create();
        case Type::STRING:
            return StringValue::create();
        case Type::CHAR:
            return CharValue::create();
        case Type::BOOL:
            return BoolValue::create();
        case Type::UNIT:
            return UnitValue::create();
        case Type::LIST: {
            // TODO: list's type handling
            auto ltype = type->toListType();
            auto subtype = ltype->getSubtype();
            if (subtype->getTypeId() != Type::VARIABLE_TYPE_NAME) {
                vector<Value *> values;
                ListValue::create({});
            }
            return ListValue::create({});
        }
        case Type::RECORD:
            break;
        case Type::TUPLE:
            break;
        case Type::FUNCTION:
            break;
        case Type::TYPE_NAME:
            break;
        case Type::VARIABLE_TYPE_NAME:
            break;
        default:
            break;
    }
    return nullptr;
}

IntValue *IntValue::create(int v) {
    return new IntValue(v);
}

int IntValue::get() const {

}

IntValue::IntValue(int v) : Value(
        IntType::create(),
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(getLLVMContext()), v)) {

}

FunctionValue *FunctionValue::create(Type *retTy, Type *paramTy, void *fptr) {
    return new FunctionValue(retTy, paramTy, fptr);
}

int FunctionValue::getPriority() const {
    return _priority;
}

void FunctionValue::setPriority(int n) {
    _priority = n;
}

FunctionValue::OperationType FunctionValue::getOperationType() const {
    return _operationType;
}

void FunctionValue::setOperationType(FunctionValue::OperationType type) {
    _operationType = type;
}

FunctionValue::FunctionValue(Type *retTy, Type *paramTy, void *fptr)
        : Value(FunctionType::create(retTy, paramTy), nullptr) {

}

FunctionValue::FunctionValue(FunctionType *funTy, void *fptr)
        : Value(funTy, nullptr) {

}

FunctionValue *FunctionValue::create(FunctionType *funTy, void *fptr) {
    return new FunctionValue(funTy, fptr);
}

bool FunctionValue::isOverloaded() const {
    return _values.size() > 1;
}

FunctionValue::FunctionValue(
        std::vector<std::pair<FunctionType *, void *>> values) :
        Value(FunctionType::create([&values]() {
            vector<pair<Type *, Type *>> res;
            res.reserve(values.size());
            for (auto &&[funTy, _fptr] : values) {
                res.emplace_back(
                        funTy->getReturnType(), funTy->getParameterType());
            }
            return res;
        }()), nullptr),
        _values(std::move(values)) {

}

FunctionValue *FunctionValue::create(
        std::vector<std::pair<FunctionType *, void *>> values) {
    return new FunctionValue(std::move(values));
}

const vector<std::pair<FunctionType *, void *>> &FunctionValue::get() const {
    return _values;
}

RecordValue *RecordValue::create(std::map<std::string, Value *> record) {
    return nullptr;
}

const std::map<std::string, Value *> &RecordValue::get() const {

}

RecordValue::RecordValue(std::map<std::string, Value *> record)
        : Value(RecordType::create({}), nullptr) {

}

ListValue *ListValue::create(std::vector<Value *> values) {
    return new ListValue(std::move(values));
}

const std::vector<Value *> &ListValue::get() const {

}

ListValue::ListValue(std::vector<Value *> values)
        : Value([&values]() -> Type * {
    if (values.empty()) {
        return VariableTypeNameType::create("'a");
    } else {
        return values.back()->getType();
    }
}(), nullptr) {

}

UnitValue *UnitValue::create() {
    static UnitValue unitValue;
    return &unitValue;
}

UnitValue::UnitValue() : Value(UnitType::create(), nullptr) {

}

TupleValue *TupleValue::create(std::vector<Value *> values) {
    return new TupleValue(std::move(values));
}

const std::vector<Value *> &TupleValue::get() const {

}

TupleValue::TupleValue(std::vector<Value *> values)
        : Value(TupleType::create({}), nullptr) {

}

BoolValue *BoolValue::create(bool b) {
    static BoolValue b0(false), b1(true);
    return b ? &b1 : &b0;
}

bool BoolValue::get() const {
    return false;
}

BoolValue::BoolValue(bool b) : Value(BoolType::create(), nullptr) {

}

CharValue *CharValue::create(char c) {
    return new CharValue(c);
}

char CharValue::get() const {
    return 0;
}

CharValue::CharValue(char c) : Value(CharType::create(), nullptr) {

}

StringValue *StringValue::create(std::string str) {
    return new StringValue(std::move(str));
}

const std::string &StringValue::get() const {

}

StringValue::StringValue(std::string str)
        : Value(StringType::create(), nullptr) {

}

RealValue *RealValue::create(double v) {
    return new RealValue(v);
}

double RealValue::get() const {
    return 0;
}

RealValue::RealValue(double v) : Value(RealType::create(), nullptr) {

}

TypeNameType *TypeNameType::create(Type *aliasAs) {
    return new TypeNameType(aliasAs);
}

TypeNameType::TypeNameType(Type *aliasAs)
        : _aliasAs(aliasAs) {

}

Type::TypeId TypeNameType::getTypeId() const {
    return TYPE_NAME;
}

const string &TypeNameType::getName() const {
    return _name;
}

Type *TypeNameType::getBoundType() const {
    return _aliasAs;
}

ostream &TypeNameType::print(std::ostream &o) const {
    return o << this->getName();
}

FunctionType *FunctionType::create(Type *ret, Type *param) {
    return new FunctionType(ret, param);
}

Type *FunctionType::getParameterType(size_t idx) const {
    return _types.at(idx).second;
}

Type *FunctionType::getReturnType(size_t idx) const {
    return _types.at(idx).first;
}

FunctionType::FunctionType(Type *ret, Type *param)
        : _types({{ret, param}}) {

}

Type::TypeId FunctionType::getTypeId() const {
    return FUNCTION;
}

FunctionType *FunctionType::create(std::vector<std::pair<Type *, Type *>> tys) {
    return new FunctionType(std::move(tys));
}

FunctionType::FunctionType(std::vector<std::pair<Type *, Type *>> tys)
        : _types(std::move(tys)) {

}

size_t FunctionType::getOverloadedNum() const {
    return _types.size();
}

const vector<std::pair<Type *, Type *>> &FunctionType::getTypes() const {
    return _types;
}

bool FunctionType::isOverloaded() const {
    return getOverloadedNum() > 1;
}

ostream &FunctionType::print(std::ostream &o) const {
    this->getParameterType()->print(o);
    o << " -> ";
    this->getReturnType()->print(o);
    return o;
}

RecordType *RecordType::create(std::map<std::string, Type *> record) {
    return new RecordType(std::move(record));
}

const std::map<std::string, Type *> &RecordType::getRecordEntries() const {

}

RecordType::RecordType(std::map<std::string, Type *> record) {

}

Type::TypeId RecordType::getTypeId() const {
    return RECORD;
}

ListType *ListType::create(Type *subtype) {
    return new ListType(subtype);
}

Type *ListType::getSubtype() const {
    return _subType;
}

ListType::ListType(Type *subtype) : _subType(subtype) {

}

Type::TypeId ListType::getTypeId() const {
    return LIST;
}

ostream &ListType::print(std::ostream &o) const {
    return this->getSubtype()->print(o) << " list";
}

UnitType *UnitType::create() {
    static UnitType unitType;
    return &unitType;
}

UnitType::UnitType() = default;

Type::TypeId UnitType::getTypeId() const {
    return UNIT;
}

ostream &UnitType::print(std::ostream &o) const {
    return o << "unit";
}

StringType *StringType::create() {
    static StringType type;
    return &type;
}

Type::TypeId StringType::getTypeId() const {
    return STRING;
}

ostream &StringType::print(std::ostream &o) const {
    return o << "string";
}

StringType::StringType() = default;

CharType *CharType::create() {
    static CharType type;
    return &type;
}

Type::TypeId CharType::getTypeId() const {
    return CHAR;
}

ostream &CharType::print(std::ostream &o) const {
    return o << "char";
}

CharType::CharType() = default;

BoolType *BoolType::create() {
    static BoolType boolType;
    return &boolType;
}

Type::TypeId BoolType::getTypeId() const {
    return BOOL;
}

ostream &BoolType::print(std::ostream &o) const {
    return o << "bool";
}

BoolType::BoolType() = default;

Type::TypeId VariableTypeNameType::getTypeId() const {
    return VARIABLE_TYPE_NAME;
}

VariableTypeNameType *VariableTypeNameType::create(std::string var) {
    return new VariableTypeNameType(std::move(var));
}

VariableTypeNameType::VariableTypeNameType(std::string var)
        : _var(std::move(var)) {

}

const string &VariableTypeNameType::getVar() const {
    return _var;
}

ostream &VariableTypeNameType::print(std::ostream &o) const {
    return o << this->getVar();
}

VariableTypeNameValue::VariableTypeNameValue(VariableTypeNameType *type)
        : Value(type, nullptr) {

}

VariableTypeNameValue *VariableTypeNameValue::create(VariableTypeNameType *t) {
    return new VariableTypeNameValue(t);
}

VariableTypeNameValue *VariableTypeNameValue::create(std::string var) {
    return new VariableTypeNameValue(
            VariableTypeNameType::create(std::move(var)));
}
