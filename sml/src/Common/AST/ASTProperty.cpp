#include <unordered_map>
#include "ASTProperty.h"

using namespace std;

namespace {
    unordered_map<AST *, Type *> astTypeMap;
}

Type *ASTProperty::getType(AST *ast) {
    if (astTypeMap.count(ast)) {
        return astTypeMap[ast];
    }
    return nullptr;
}

void ASTProperty::setType(AST *ast, Type *type) {
    astTypeMap[ast] = type;
}
