#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "JITModule.h"

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
std::unique_ptr<llvm::Module> TheModule;
std::map<std::string, std::shared_ptr<AST>> FunctionProtos;
std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
std::map<std::string, llvm::Value *> NamedValues;
std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
std::map<std::string, llvm::Value *> temNamedValues;

void InitializeModuleAndPassManager() {

// Open a new module.
    TheModule = llvm::make_unique<llvm::Module>("test", TheContext);
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

// Create a new pass manager attached to it.
    TheFPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

//四个add为添加优化
// Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(llvm::createInstructionCombiningPass());
// Reassociate expressions.
    TheFPM->add(llvm::createReassociatePass());
//     Eliminate Common SubExpressions.
    TheFPM->add(llvm::createGVNPass());
// Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(llvm::createCFGSimplificationPass());

    TheFPM->doInitialization();
}