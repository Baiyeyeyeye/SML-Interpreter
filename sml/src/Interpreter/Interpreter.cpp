#include "CodeGenerator.h"
#include "Interpreter.h"
#include "JIT.h"
#include "Parser.h"
#include "Scanner.h"
#include "SemanticAnalyzer.h"
#include "Symbol/SymbolTable.h"

struct Interpreter::Impl {
    Interpreter *const interpreter;
    Scanner scanner;
    Parser parser;
    SemanticAnalyzer semanticAnalyzer;
    CodeGenerator codeGenerator;
    JIT jit;

    explicit Impl(Interpreter *interp, std::istream &in) :
            interpreter(interp),
            scanner(in),
            parser(interp),
            semanticAnalyzer(),
            codeGenerator(),
            jit() {

    }
};

Interpreter::Interpreter(std::istream &in)
        : _impl(std::make_unique<Impl>(this, in)) {

}

Interpreter::Interpreter() = default;

Interpreter::~Interpreter() = default;

void Interpreter::interpret() {
    while (!eof()) {
        checkAndRun(getParser()->parse(), true);
    }
    SymbolTable::reset();
}

void Interpreter::checkAndRun(const std::shared_ptr<AST> &ast, bool output) {
    if (auto &&sem = getSemanticAnalyzer()) {
        if (auto &&ast1 = sem->check(ast)) {
            if (auto &&codegen = getCodeGenerator()) {
                auto &&value = (llvm::Function*)codegen->generate(ast1);
                if (auto &&jit = getJIT()) {
                    jit->run(value);
                }
            }
        }
    }
}

std::shared_ptr<Token> Interpreter::getNextToken() {
    if (auto &&sc = getScanner()) {
        return sc->getNextToken();
    }
    return nullptr;
}

Scanner *Interpreter::getScanner() const {
    return &_impl->scanner;
}

Parser *Interpreter::getParser() const {
    return &_impl->parser;
}

SemanticAnalyzer *Interpreter::getSemanticAnalyzer() const {
    return &_impl->semanticAnalyzer;
}

CodeGenerator *Interpreter::getCodeGenerator() const {
    return &_impl->codeGenerator;
}

JIT *Interpreter::getJIT() const {
    return &_impl->jit;
}

bool Interpreter::eof() const {
    return _impl->scanner.eof();
}
