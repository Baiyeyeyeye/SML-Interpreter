#include <src/Common/AST/ASTProperty.h>
#include "llvm/IR/Value.h"
#include "JIT.h"
#include "JITModule/JITModule.h"
#include "KaleidoscopeJIT.h"
#include "Symbol/SymbolTable.h"

/*
 * 为了实现每个函数的优化，我们需要设置一个FunctionPassManager来保存和组织要运行的LLVM优化。
 * 一旦我们有了它，我们就可以添加一组优化来运行。对于要优化的每个模块，我们都需要一个新的FunctionPassManager，
 * 因此我们将编写一个函数来为我们创建和初始化模块和pass manager：
 */

///需要创建FunctionAST作为入口
void JIT::run(llvm::Function* theFun) {
    theFun->print(llvm::errs());
    auto H = TheJIT->addModule(std::move(TheModule));
    InitializeModuleAndPassManager();

    if(auto ExprSymbol = TheJIT->findSymbol("__anon_expr")){
        auto temType = SymbolTable::getInstance()->getPatternType("it");
        if(temType->getTypeId() == Type::INT){
            int (*FP)() = (int (*)()) (intptr_t) cantFail(ExprSymbol.getAddress());
            fprintf(stderr, "Evaluated to %i\n", FP());
        }else if(temType->getTypeId() == Type::REAL){
            double (*FP)() = (double (*)()) (intptr_t) cantFail(ExprSymbol.getAddress());
            fprintf(stderr, "Evaluated to %f\n", FP());
        }

//        FP =(intptr_t) cantFail(ExprSymbol.getAddress());
//        fprintf(stderr, "Evaluated to %i\n", FP);

        TheJIT->removeModule(H);
    }
}