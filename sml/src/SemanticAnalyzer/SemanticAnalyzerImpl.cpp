#include "AST/AST.h"
#include "SemanticAnalyzerImpl.h"
#include "Symbol/SymbolTable.h"
#include "TypeCheck.h"

using namespace std;

std::shared_ptr<AST> SemanticAnalyzer::Impl::check(
        const std::shared_ptr<AST> &ast) {
    if (!ast) {
        return ast;
    }
    TypeCheck typeCheck;
    if (auto type = (Type *) ast->accept(&typeCheck)) {
        typeCheck.fillTypes();
        type = typeCheck.verify(type);
        SymbolTable::getInstance()->insertPatternType("it", type);
        return ast;
    }
    return nullptr;
}
