#include <memory>
#include "AST/AST.h"
#include "CodeGenerator.h"
#include "CodeGeneratorImpl.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
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
#include "JITModule/JITModule.h"
#include "KaleidoscopeJIT.h"

using namespace std;

CodeGenerator::CodeGenerator() : _impl(make_unique<Impl>()) {
    using namespace llvm;
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    TheJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();
}

CodeGenerator::~CodeGenerator() = default;


llvm::Function *CodeGenerator::generate(const std::shared_ptr<AST> &ast) {
    _ast = ast;
    return (llvm::Function*) ast->accept(this);

    ///ast为函数体
    if (auto exp = dynamic_pointer_cast<ExpAST>(ast)) {
        if (auto temInfixApplicationAST = dynamic_pointer_cast<InfixApplicationExpAST>(exp)) {
//            std::shared_ptr<IdAST> id( new AlphanumericIdAST(temInfixApplicationAST->getId()->get()));
            std::shared_ptr<IdAST> id(new AlphanumericIdAST("__anon_expr"));
            auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                        std::vector<std::shared_ptr<PatAST>>(), temInfixApplicationAST);
            auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
            auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
            return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
        } else if (auto appExp = dynamic_pointer_cast<ApplicationExpAST>(exp)) {
            std::shared_ptr<IdAST> id(new AlphanumericIdAST("__anon_expr"));
            auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                        std::vector<std::shared_ptr<PatAST>>(), appExp);
            auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
            auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
            return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
        } else if (auto temLet = dynamic_pointer_cast<LocalDeclarationExpAST>(exp)) {
            std::shared_ptr<IdAST> id(new AlphanumericIdAST("__anon_expr"));
            auto temSequence = dynamic_pointer_cast<SequenceDecAST>(temLet->getDec());
            std::vector<std::shared_ptr<PatAST>> temPats;
            for (int i = 0; i < temSequence->getDecs().size(); i++) {
                auto tem = dynamic_pointer_cast<ValueDecAST>(temSequence->getDecs()[i]);
                auto tem2 = dynamic_pointer_cast<DestructuringValBindAST>(tem->getValBind());
                temPats.push_back(tem2->getPat());
            }
            std::shared_ptr<TupleExpAST> temTuple(new TupleExpAST(temLet->getExps()));
            auto Proto = llvm::make_unique<FunMatchAST>(std::move(id), temPats, temTuple);

            auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
            auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
            return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
        } else {
            //创建匿名函数
            std::shared_ptr<IdAST> id(new AlphanumericIdAST("__anon_expr"));
            auto Proto = llvm::make_unique<FunMatchAST>(std::move(id), std::vector<std::shared_ptr<PatAST>>(), exp);
            auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
            auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
            return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
        }

//        auto FnValue = (llvm::Value*)FnAST->accept(&_impl->codeGen);
    } else if (auto dec = dynamic_pointer_cast<FunctionDecAST>(ast)) {
        fprintf(stderr, "Read function definition:");
        std::shared_ptr<IdAST> id(new AlphanumericIdAST(dec->getFunBind()->getFunMatch()->getId()->get()));
        auto temTuplePat = dynamic_pointer_cast<TuplePatAST>(dec->getFunBind()->getFunMatch()->getPats()[0]);
        auto Proto = llvm::make_unique<FunMatchAST>(std::move(id), temTuplePat->getPats(),
                                                    dec->getFunBind()->getFunMatch()->getExp());
        auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
        auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
        return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
    } else if (auto fnDec = dynamic_pointer_cast<ValueDecAST>(ast)) {
        if (auto val = dynamic_pointer_cast<DestructuringValBindAST>(fnDec->getValBind())) {
            if (auto fn = dynamic_pointer_cast<FunctionExpAST>(val->getExp()))
                if (auto longid = dynamic_pointer_cast<ConstructionPatAST>(val->getPat())) {
                    std::shared_ptr<IdAST> id(new AlphanumericIdAST(longid->getLongId()->getIds()[0]->get()));
                    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                                std::vector<std::shared_ptr<PatAST>>{val->getPat()},
                                                                val->getExp());
                    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
                }
        }
    } else {
        return static_cast<llvm::Function *>(ast->accept(&_impl->codeGen));
    }
}

void *CodeGenerator::visit(ExpAST *ast) {
    std::shared_ptr<IdAST> id(new AlphanumericIdAST("__anon_expr"));
    auto Proto = shared_ptr<FunMatchAST>(
            new FunMatchAST(std::move(id),
                            std::vector<std::shared_ptr<PatAST>>(),
                            dynamic_pointer_cast<ExpAST>(_ast)));
    auto FnBind = shared_ptr<FunBindAST>(new FunBindAST(std::move(Proto)));
    auto FnAST = shared_ptr<FunctionDecAST>(new FunctionDecAST(std::move(FnBind)));
    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
}

//void *CodeGenerator::visit(LocalDeclarationExpAST *ast) {
//    std::shared_ptr<IdAST> id(new AlphanumericIdAST("__anon_expr"));
//    auto temSequence = dynamic_pointer_cast<SequenceDecAST>(ast->getDec());
//    std::vector<std::shared_ptr<PatAST>> temPats;
//    for (int i = 0; i < temSequence->getDecs().size(); i++) {
//        auto tem = dynamic_pointer_cast<ValueDecAST>(temSequence->getDecs()[i]);
//        auto tem2 = dynamic_pointer_cast<DestructuringValBindAST>(tem->getValBind());
//        temPats.push_back(tem2->getPat());
//    }
//    std::shared_ptr<TupleExpAST> temTuple(new TupleExpAST(ast->getExps()));
//    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id), temPats, temTuple);
//
//    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
//    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
//    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
//}

void *CodeGenerator::visit(FunctionDecAST *ast) {
    fprintf(stderr, "Read function definition:");
    std::shared_ptr<IdAST> id(new AlphanumericIdAST(ast->getFunBind()->getFunMatch()->getId()->get()));
    if(auto temTuplePat = dynamic_pointer_cast<TuplePatAST>(ast->getFunBind()->getFunMatch()->getPats()[0])){
        auto Proto = llvm::make_unique<FunMatchAST>(std::move(id), temTuplePat->getPats(),
                                                    ast->getFunBind()->getFunMatch()->getExp());
        auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
        auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
        return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
    }else if(auto temLongIdPat = dynamic_pointer_cast<ConstructionPatAST>(ast->getFunBind()->getFunMatch()->getPats()[0])){
        std::vector<std::shared_ptr<PatAST>> temV;
        temV.push_back(static_cast<const shared_ptr<PatAST>>(new ConstructionPatAST(temLongIdPat->getLongId())));

        auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),temV,
                                                    ast->getFunBind()->getFunMatch()->getExp());
        auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
        auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
        return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
    }
}

void *CodeGenerator::visit(ValueDecAST *ast) {
    if (auto val = dynamic_pointer_cast<DestructuringValBindAST>(ast->getValBind())) {
        if (auto fn = dynamic_pointer_cast<FunctionExpAST>(val->getExp())){
            if (auto longid = dynamic_pointer_cast<ConstructionPatAST>(val->getPat())) {

                auto temName = dynamic_pointer_cast<ConstructionPatAST>(val->getPat());
                std::shared_ptr<IdAST> id(new AlphanumericIdAST(temName->getLongId()->getIds()[0]->get()));

                auto temPats = dynamic_pointer_cast<TuplePatAST>(fn->getMatch()->getPat());
//                std::vector<std::shared_ptr<ValueOrConstructorIdentifierExpAST>>temLongId;
//                for(int i =0;i<temPats->getExps().size();i++){
//                    auto tem = dynamic_pointer_cast<ValueOrConstructorIdentifierExpAST>(temPats->getExps()[i]);
//                    temLongId.push_back(tem);
//                }
                auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),temPats->getPats(),
//                                                            std::vector<std::shared_ptr<PatAST>>{val->getPat()},
                                                            fn->getMatch()->getExp());
                auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
            }else if(auto longid = dynamic_pointer_cast<TypeAnnotationPatAST>(val->getPat())){

            }
        }else{
            // Codegen the RHS.
            if(auto temFunctionExpAST = dynamic_pointer_cast<FunctionExpAST>(val->getExp())){

            }else if(dynamic_pointer_cast<ConstantExpAST>(val->getExp())){
                if(auto tem = dynamic_pointer_cast<ConstructionPatAST>(val->getPat())){
                    std::shared_ptr<IdAST> id(new AlphanumericIdAST(tem->getLongId()->getIds()[0]->get()));

                    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                                std::vector<std::shared_ptr<PatAST>>(),
                                                                val->getExp());

                    temNamedValues[tem->getLongId()->getIds()[0]->get()] = (llvm::Value*)val->getExp()->accept(&_impl->codeGen);
                    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
                }else if(auto tem = dynamic_pointer_cast<TypeAnnotationPatAST>(val->getPat())){
                    auto temPat = dynamic_pointer_cast<ConstructionPatAST>(tem->getPat());
                    std::shared_ptr<IdAST> id(new AlphanumericIdAST(temPat->getLongId()->getIds()[0]->get()));

                    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                                std::vector<std::shared_ptr<PatAST>>(),
                                                                val->getExp());

                    temNamedValues[temPat->getLongId()->getIds()[0]->get()] = (llvm::Value*)val->getExp()->accept(&_impl->codeGen);
                    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
                }
            } if(auto temVariable = dynamic_pointer_cast<ValueOrConstructorIdentifierExpAST>(val->getExp())){
                if(auto tem = dynamic_pointer_cast<ConstructionPatAST>(val->getPat())){
                    if(llvm::Value* temV = temNamedValues[temVariable->getLongId()->getIds()[0]->get()])
                        temNamedValues[tem->getLongId()->getIds()[0]->get()] = temV;
                    std::shared_ptr<IdAST> id(new AlphanumericIdAST(tem->getLongId()->getIds()[0]->get()));

                    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                                std::vector<std::shared_ptr<PatAST>>(),
                                                                val->getExp());

                    temNamedValues[tem->getLongId()->getIds()[0]->get()] = (llvm::Value*)val->getExp()->accept(&_impl->codeGen);
                    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
                }
            } if(auto temVariable = dynamic_pointer_cast<InfixApplicationExpAST>(val->getExp())){
                if(auto tem = dynamic_pointer_cast<ConstructionPatAST>(val->getPat())){
//                    if(llvm::Value* temV = temNamedValues[temVariable->getLongId()->getIds()[0]->get()])
//                        temNamedValues[tem->getLongId()->getIds()[0]->get()] = temV;
                    std::shared_ptr<IdAST> id(new AlphanumericIdAST(tem->getLongId()->getIds()[0]->get()));

                    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                                std::vector<std::shared_ptr<PatAST>>(),
                                                                val->getExp());

                    temNamedValues[tem->getLongId()->getIds()[0]->get()] = (llvm::Value*)val->getExp()->accept(&_impl->codeGen);
                    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
                }
                if(auto tem = dynamic_pointer_cast<TypeAnnotationPatAST>(val->getPat())){
                    auto temPat = dynamic_pointer_cast<ConstructionPatAST>(tem->getPat());
                    std::shared_ptr<IdAST> id(new AlphanumericIdAST(temPat->getLongId()->getIds()[0]->get()));

                    auto Proto = llvm::make_unique<FunMatchAST>(std::move(id),
                                                                std::vector<std::shared_ptr<PatAST>>(),
                                                                val->getExp());

                    temNamedValues[temPat->getLongId()->getIds()[0]->get()] = (llvm::Value*)val->getExp()->accept(&_impl->codeGen);
                    auto FnBind = llvm::make_unique<FunBindAST>(std::move(Proto));
                    auto FnAST = llvm::make_unique<FunctionDecAST>(std::move(FnBind));
                    return static_cast<llvm::Function *>(FnAST->accept(&_impl->codeGen));
                }
            }else{
                auto pat = val->getPat();
                auto *LHSE = dynamic_cast<ConstructionPatAST *>(pat.get());
                if (!LHSE)
                    return nullptr;
                auto temValue = (llvm::Value*)val->getExp()->accept(this);
                if (!temValue)
                    return nullptr;

                // Look up the name.
//        NamedValues[LHSE->getLongId()->getIds()[0]->get()] = temValue;
                llvm::Value *Variable = NamedValues[LHSE->getLongId()->getIds()[0]->get()];
                if (!Variable)
                    return nullptr;

                Builder.CreateStore(temValue, Variable);
                return temValue;
            }
        }

    }
}