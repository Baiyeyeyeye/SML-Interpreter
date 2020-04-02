#pragma once

#include <iostream>
#include <memory>

class AST;

class Token;

class Scanner;

class Parser;

class SemanticAnalyzer;

class CodeGenerator;

class JIT;

class Interpreter {
public:
    explicit Interpreter(std::istream &in);

    ~Interpreter();

    void interpret();

    friend class ParserInterface;

    class ParserInterface {
        friend class Parser;

        static inline auto checkAndRun(
                Interpreter *interpreter,
                const std::shared_ptr<AST> &ast) {
            interpreter->checkAndRun(ast, false);
        }

        static inline auto getNextToken(Interpreter *interpreter) {
            return interpreter->getNextToken();
        }
    };

protected:
    struct Impl;

    const std::unique_ptr<Impl> _impl;

    void checkAndRun(const std::shared_ptr<AST> &ast, bool output);

    [[nodiscard]] virtual Scanner *getScanner() const;

    [[nodiscard]] virtual Parser *getParser() const;

    [[nodiscard]] virtual SemanticAnalyzer *getSemanticAnalyzer() const;

    [[nodiscard]] virtual CodeGenerator *getCodeGenerator() const;

    [[nodiscard]] virtual JIT *getJIT() const;

    [[nodiscard]] virtual bool eof() const;

    Interpreter();

    std::shared_ptr<Token> getNextToken();
};
