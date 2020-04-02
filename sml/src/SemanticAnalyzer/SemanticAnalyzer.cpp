#include "SemanticAnalyzer.h"
#include "SemanticAnalyzerImpl.h"
using namespace std;

SemanticAnalyzer::SemanticAnalyzer() : _impl(std::make_unique<Impl>()) {

}

SemanticAnalyzer::~SemanticAnalyzer() = default;

std::shared_ptr<AST> SemanticAnalyzer::check(const std::shared_ptr<AST>& ast) {
    return _impl->check(ast);
}
