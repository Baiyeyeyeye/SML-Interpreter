#pragma once

#include <any>
#include <memory>

//region APPLY_ALL
#ifndef APPLY_ALL
#include "ASTApplyMacro.h"
#endif

//region ast forward declarations
#define APPLY(CLASS) class CLASS;

APPLY_ALL

#undef APPLY
//endregion

class ASTVisitor {
public:
//region visit function definition for every ast
    /**
     * Visit the ast.
     *
     * By default, it forwards to a super class of ast if ast is not the top
     * most base class (i.e. the AST class). If it reach the top of AST class,
     * return the ast itself.
     *
     * @param ast The ast to visit.
     * @return A pointer, by default with value of parameter ast.
     */
#define APPLY(CLASS) virtual void* visit(CLASS *ast);

    APPLY_ALL

#undef APPLY
//endregion

protected:
    ASTVisitor() = default;
};

#undef APPLY_ALL
//endregion
