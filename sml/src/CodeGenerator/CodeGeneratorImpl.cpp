#include <src/Common/AST/ASTProperty.h>
#include "CodeGeneratorImpl.h"
#include "Error.h"
#include "JIT.h"
#include "JITModule/JITModule.h"

void *CodeGen::visit(InfixConstructionPatAST *ast) {
    auto pat1 = ast->getPat1();
    auto id = ast->getId();
    auto pat2 = ast->getPat2();
    if(!(pat1 and id and pat2))
        return nullptr;
    std::string op = id->get();
    if (op[0] == '=') {
        // Assignment requires the LHS to be an identifier.
        // This assume we're building without RTTI because LLVM builds that way by
        // default.  If you build LLVM with RTTI this can be changed to a
        // dynamic_cast for automatic error checking.
        auto *LHSE = dynamic_cast<VariablePatAST *>(pat1.get());
        if (!LHSE)
            return nullptr;
        // Codegen the RHS.
        auto Val = (llvm::Value*)pat2->accept(this);
        if (!Val)
            return nullptr;

        // Look up the name.
        llvm::Value *Variable = NamedValues[LHSE->getId()->get()];
        if (!Variable)
            return nullptr;

        Builder.CreateStore(Val, Variable);
        return Val;
    }

    auto L = (llvm::Value*)pat1->accept(this);
    auto R = (llvm::Value*)pat2->accept(this);
//    auto Op = (llvm::Value*)id->accept(this);
    if(!L || !R)
        return nullptr;

    switch(op[0]){
        case '+':
            return Builder.CreateFAdd(L, R, "addtmp");
        case '-':
            return Builder.CreateFSub(L, R, "subtmp");
        case '*':
            return Builder.CreateFMul(L, R, "multmp");
        case '/':
            return Builder.CreateFDiv(L, R, "divtem");
        default:
            Error("invalid binary operator");
            return nullptr;
    }
}

void *CodeGen::visit(FunctionTypAST *ast) {
    auto typ1 = ast->getTyp1();
    auto typ2 = ast->getTyp2();
    if(!(typ1 and typ2)){
        return nullptr;
    }
    auto tem1 = typ1->accept(this);
    auto tem2 = typ2->accept(this);
}

void *CodeGen::visit(FunctionExpAST *ast) {
    auto tem = ast->getMatch();
    if(!tem)
        return nullptr;
    return tem->accept(this);
}

void *CodeGen::visit(MatchAST *ast) {
    ///返回llvm::Value*
    auto temPat = ast->getPat();
    auto temExp = ast->getExp();
    auto temMatch = ast->getMatch();
    if(!(temPat and temExp))
        return nullptr;
    auto tem1 = (llvm::Value*)temPat->accept(this);
    auto tem2 = (llvm::Value*)temExp->accept(this);
    if(temMatch)//如果只有一个函数体就返回tem2
        return tem2;
    auto temMatchValue = (llvm::Value*)temMatch->accept(this);
    ///如何根据具体的值来返回相应的Value*

}

void *CodeGen::visit(ValBindAST *ast) {
    return nullptr;
}

void *CodeGen::visit(NumberLabAST *ast) {
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, ast->getN()));
}

void *CodeGen::visit(IdentifierLabAST *ast) {
    auto tem = ast->getId();
    tem->accept(this);
}

void *CodeGen::visit(LongIdAST *ast) {
    auto tem = ast->getIds();
    for(int i=0;i<tem.size();i++){
        tem[i]->accept(this);
    }
}

void *CodeGen::visit(VarAST *ast) {
    auto tem = ast->getVar();
    if(!tem.empty()){
        Error("Unknown var");
        return nullptr;
    }
    return &tem;
}

void *CodeGen::visit(IdAST *ast) {
    auto tem = SymbolTable::getInstance()->getValue(ast->get());
    if(!tem){
        Error("Unknown Id name!");
        return nullptr;
    }
    return tem->getLLVMValue();
}

void *CodeGen::visit(IntConAST *ast) {
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, ast->get()));
}

void *CodeGen::visit(FloatConAST *ast) {
    return llvm::ConstantFP::get(TheContext, llvm::APFloat(ast->get()));
}

void *CodeGen::visit(CharConAST *ast) {
    // Maybe something is wrong here,
    return llvm::ConstantInt::get(TheContext, llvm::APInt(8, ast->get(), false));
}

void *CodeGen::visit(StringConAST *ast) {
//    //I guess here is sonething wrong, but I am not sure.
//    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
//
//    // Create an alloca for the variable in the entry block.
//
//    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
//                     TheFunction->getEntryBlock().begin());
//    llvm::AllocaInst *Alloca = TmpB.CreateAlloca(llvm::Type::getDoubleTy(TheContext),
//                                nullptr, "StringValue");

//    std::initializer_list<std::pair<llvm::StringRef, llvm::Value*>> List;
//    llvm::StringMap map = llvm::StringMap(List);

//    return Builder.CreateGlobalString(ast->get());
    std::vector<llvm::Constant*> StringLocal;
    for(unsigned long long i = 0; i < ast->get().length();i++){
        llvm::ConstantInt* temInt = llvm::ConstantInt::get(TheContext,
                llvm::APInt(8,ast->get().c_str()[i],false));
        StringLocal.push_back(temInt);
    }
    llvm::Constant*  charArray = llvm::ConstantArray::get(llvm::ArrayType::get(
            llvm::Type::getInt64Ty(TheContext),ast->get().length()),llvm::ArrayRef(StringLocal));
    return charArray;
}

void *CodeGen::visit(BoolConAST *ast) {
    return llvm::ConstantInt::get(TheContext,llvm::APInt(1,int(ast->get())));
}

void *CodeGen::visit(DisjunctionExpAST *ast) {
    auto lhs = visitAsValue(ast->getExp1());
    auto rhs = visitAsValue(ast->getExp2());
    return llvm::BinaryOperator::CreateOr(lhs, rhs, "tmpor");
}

void *CodeGen::visit(ConjunctionExpAST *ast) {
    auto lhs = visitAsValue(ast->getExp1());
    auto rhs = visitAsValue(ast->getExp2());
    return llvm::BinaryOperator::CreateAnd(lhs, rhs, "andpor");
}

llvm::Value *CodeGen::visitAsValue(const std::shared_ptr<ExpAST>& exp) {
    return static_cast<llvm::Value *>(exp->accept(this));
}

///check this!!!
llvm::Function *getFunction(std::string Name,ASTVisitor* ASTVisitor) {
    ///返回llvm::Function*
    // First, see if the function has already been added to the current module.
    if (auto *F = TheModule->getFunction(Name))
        return F;

    // If not, check whether we can codegen the declaration from some existing
    // prototype.
    auto FI = FunctionProtos.find(Name);
    if (FI != FunctionProtos.end())
        return (llvm::Function *)FI->second->accept(ASTVisitor);

    // If no existing prototype exists, return null.
    return nullptr;
}

void *CodeGen::visit(FunctionDecAST *ast) {
    InitializeModuleAndPassManager();
    auto temAST = ast->getFunBind()->getFunMatch()->getId()->get();
    FunctionProtos[temAST] = ast->getFunBind()->getFunMatch();
    llvm::Function *TheFunction = getFunction(temAST,this);
    if(!TheFunction)
        return nullptr;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext,"entry",TheFunction);
    Builder.SetInsertPoint(BB);

    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[Arg.getName()] = &Arg;
    if(auto RetVal = (llvm::Value*)ast->getFunBind()->getFunMatch()->getExp()->accept(this)){
        // Finish off the function.
        Builder.CreateRet(RetVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        // Run the optimizer on the function.
        TheFPM->run(*TheFunction);

        return TheFunction;
    }else if(auto RetAST = dynamic_pointer_cast<ValueOrConstructorIdentifierExpAST>(ast->getFunBind()->getFunMatch()->getExp())){
        // Finish off the function.
        auto RetVal = temNamedValues[RetAST->getLongId()->getIds()[0]->get()];
        Builder.CreateRet(RetVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        // Run the optimizer on the function.
        TheFPM->run(*TheFunction);

        return TheFunction;
    }

    TheFunction->eraseFromParent();
    return nullptr;
}

void *CodeGen::visit(FunBindAST *ast) {
//    if(ast->getAndFunBind())
//        auto tem = ast->getAndFunBind()->accept(this);
    return ast->getFunMatch()->accept(this);///返回llvm::Function*
}

void *CodeGen::visit(FunMatchAST *ast) {
    auto iii = ast->getPats().size();
    std::vector<llvm::Type *>Ints(ast->getPats().size(),llvm::Type::getInt32Ty(TheContext));
    llvm::FunctionType *FT=
            llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext),Ints,false);

    llvm::Function *F = llvm::Function::Create(FT,llvm::Function::ExternalLinkage,ast->getId()->get(),TheModule.get());

    unsigned Idx =0;
    auto temPats = ast->getPats();
    vector<shared_ptr<ConstructionPatAST>> temLongId;
    auto ii = temPats.size();
    for(int i =0;i<temPats.size();i++){
        temLongId.push_back(dynamic_pointer_cast<ConstructionPatAST>(temPats[i]));
    }
//    std::vector<std::shared_ptr<ConstructionPatAST>> tem = (const vector<shared_ptr<ConstructionPatAST>> &) ast->getPats();
    for(auto &Arg:F->args())
        Arg.setName(temLongId[Idx++]->getLongId()->getIds()[0]->get());

    ///返回llvm::Function*类型
//    auto temValue = (llvm::Value*) ast->getExp()->accept(this);
//    // Finish off the function.
//    Builder.CreateRet(temValue);
//
//    // Validate the generated code, checking for consistency.
//    verifyFunction(*F);
//
//    // Run the optimizer on the function.
//    TheFPM->run(*F);

    return F;
}

void *CodeGen::visit(TuplePatAST *ast) {
    return nullptr;
}

void *CodeGen::visit(VariablePatAST *ast) {
//    auto tem = (llvm::Value*) ast->getId()->accept(this);
//    NamedValues[ast->getId()->get()] = tem;
//    return tem;
    return nullptr;
}

void *CodeGen::visit(InfixApplicationExpAST *ast) {
    char flag = 'I';
    shared_ptr<ExpAST>exp11;
    shared_ptr<ValueOrConstructorIdentifierExpAST>exp12;
    if(auto tem = dynamic_pointer_cast<ValueOrConstructorIdentifierExpAST>(ast->getExp1())){
        exp12 = tem;
    }else{
        exp11 = ast->getExp1();
    }

    if(ASTProperty::getType(exp11)->getTypeId()==Type::REAL)
        flag = 'F';

    shared_ptr<ExpAST>exp21;
    shared_ptr<ValueOrConstructorIdentifierExpAST>exp22;
    if(auto tem = dynamic_pointer_cast<ValueOrConstructorIdentifierExpAST>(ast->getExp2())){
        exp22 = tem;
    }else{
        exp21 = ast->getExp2();
    }

    auto id = ast->getId();
    if(!((exp11 or exp12) and id and (exp21 or exp22)))
        return nullptr;
    std::string op = id->get();
    if (op[0] == '=') {
        // Assignment requires the LHS to be an identifier.
        // This assume we're building without RTTI because LLVM builds that way by
        // default.  If you build LLVM with RTTI this can be changed to a
        // dynamic_cast for automatic error checking.
        ///I guess something is wrong here
        auto LHSE = dynamic_cast<VariablePatAST *>(exp12.get());
        if (!LHSE)
            return nullptr;
        // Codegen the RHS.
        auto Val = (llvm::Value*)exp21->accept(this);
        if (!Val)
            return nullptr;

        // Look up the name.
        llvm::Value *Variable = NamedValues[LHSE->getId()->get()];
        if (!Variable)
            return nullptr;

        Builder.CreateStore(Val, Variable);
        return Val;
    }
    llvm::Value* L;
    llvm::Value* R;
    if(exp11 == nullptr){
//        L = (llvm::Value*)exp12->accept(this);
        if(llvm::Value* v1 = NamedValues[exp12->getLongId()->getIds()[0]->get()])
            L = v1;
        else if(llvm::Value* v11 = temNamedValues[exp12->getLongId()->getIds()[0]->get()])
            L = v11;
        else
            Error("unknown variable name");
    }else{
        L = (llvm::Value*)exp11->accept(this);
    }
    if(exp21 == nullptr){
//        R = (llvm::Value*)exp22->accept(this);
        if(llvm::Value* v2 = NamedValues[exp22->getLongId()->getIds()[0]->get()])
            R = v2;
        else if(llvm::Value* v22 = temNamedValues[exp22->getLongId()->getIds()[0]->get()])
            R = v22;
        else
            Error("unknown variable name");
    }else{
        R = (llvm::Value*)exp21->accept(this);
    }
//    auto Op = (llvm::Value*)id->accept(this);
    if(!L || !R)
        return nullptr;

    switch(op[0]){
        case '+':
            if(flag == 'I')
                return Builder.CreateAdd(L, R, "addtmp");
            else if(flag == 'F')
                return Builder.CreateFAdd(L,R,"addtmp");
        case '-':
            if(flag == 'I')
                return Builder.CreateSub(L, R, "subtmp");
            else if(flag == 'F')
                return Builder.CreateFSub(L, R, "subtmp");
        case '*':
            if(flag == 'I')
                return Builder.CreateMul(L, R, "multmp");
            else if(flag == 'F')
                return Builder.CreateFMul(L, R, "multmp");
        case '/':
            return Builder.CreateFDiv(L, R, "divtem");
        case '>':{
            return Builder.CreateICmpSGT(L,R,"IcmpSGTtem");
//            return Builder.CreateUIToFP(temBool,llvm::Type::getDoubleTy(TheContext),"boolFTem");
        }
        case '<':{
//            llvm::Value* temBool = Builder.CreateICmpSLT(L,R,"IcmpSLTtem");
//            return Builder.CreateUIToFP(temBool,llvm::Type::getDoubleTy(TheContext),"boolFTem");
            return Builder.CreateICmpSLT(L,R,"IcmpSLTtem");
        }
        case '^':
            if(auto temStr1 = dynamic_pointer_cast<ConstantExpAST>(exp11))
            {
                if(auto temStr2 = dynamic_pointer_cast<ConstantExpAST>(exp21)){
                    if(auto temStr11 = dynamic_pointer_cast<StringConAST>(temStr1->getCon()))
                        if(auto temStr22 = dynamic_pointer_cast<StringConAST>(temStr2->getCon())){
                            string tem  =temStr11->get()+temStr22->get();
                            auto temAST = new StringConAST(tem);
                            return temAST->accept(this);
                        }
                }else{
                    Error("both of the operator must be a string");
                }
            }else{
                Error("both of the operator must be a string");
            }
        default:
            Error("invalid binary operator");
            return nullptr;
    }
}

void *CodeGen::visit(ConditionalExpAST *ast) {
    auto CondV = (llvm::Value *)ast->getExp1()->accept(this);
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing equal to 0.0.
    CondV = Builder.CreateFCmpONE(
            CondV, llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0)), "ifcond");

    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then value.
    Builder.SetInsertPoint(ThenBB);

    auto ThenV = (llvm::Value*)ast->getExp2()->accept(this);
    if (!ThenV)
        return nullptr;

    Builder.CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = Builder.GetInsertBlock();

    // Emit else block.
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    auto ElseV = (llvm::Value*)ast->getExp3()->accept(this);
    if (!ElseV)
        return nullptr;

    Builder.CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = Builder.GetInsertBlock();

    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder.CreatePHI(llvm::Type::getDoubleTy(TheContext), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}

void *CodeGen::visit(ValueDecAST *ast) {///返回llvm::Value*
    return ast->getValBind()->accept(this);
}

void *CodeGen::visit(DestructuringValBindAST *ast) {
    auto pat = ast->getPat();
    auto *LHSE = dynamic_cast<ConstructionPatAST *>(pat.get());
    if (!LHSE)
        return nullptr;
    // Codegen the RHS.
    if(auto temFunctionExpAST = dynamic_pointer_cast<FunctionExpAST>(ast->getExp())){

    }else if(auto con = dynamic_pointer_cast<ConstantExpAST>(ast->getExp())){
        if(auto tem = dynamic_pointer_cast<ConstructionPatAST>(ast->getPat())){
            auto temV = (llvm::Value*)con->getCon()->accept(this);
            temNamedValues[tem->getLongId()->getIds()[0]->get()] = temV;
            return temV;
        }
    }else{
        auto temValue = (llvm::Value*)ast->getExp()->accept(this);
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

void *CodeGen::visit(ConstantExpAST *ast) {
    return ast->getCon()->accept(this);
}

void *CodeGen::visit(ConAST *ast) {
    return ASTVisitor::visit(ast);
}

void *CodeGen::visit(ExpAST *ast) {
    return ASTVisitor::visit(ast);
}

void *CodeGen::visit(ApplicationExpAST *ast) {
    if(auto con = dynamic_pointer_cast<ValueOrConstructorIdentifierExpAST>(ast->getExp1())){
        string str = con->getLongId()->getIds()[0]->get();

        llvm::Function* callF = getFunction(str,this);
        if(!callF)
            Error("invalid Function name");

        // If argument mismatch error.
        if(auto arg = dynamic_pointer_cast<TupleExpAST>(ast->getExp2())){
            if (callF->arg_size() != arg->getExps().size())
                ////FIX THIS
                Error("Incorrect # arguments passed");

            std::vector<llvm::Value *> ArgsV;
            for (unsigned i = 0, e = arg->getExps().size(); i != e; ++i) {
                auto tem = (llvm::Value*)arg->getExps()[i]->accept(this);
                ArgsV.push_back(tem);
                if (!ArgsV.back())
                    return nullptr;
            }

            return Builder.CreateCall(callF, ArgsV, "calltmp");
        }else if(auto arg = dynamic_pointer_cast<ConstantExpAST>(ast->getExp2())){
            std::vector<llvm::Value *> ArgsV;
            for (unsigned i = 0, e = 1; i != e; ++i) {
                auto tem = (llvm::Value*)arg->accept(this);
                ArgsV.push_back(tem);
                if (!ArgsV.back())
                    return nullptr;
            }
            return Builder.CreateCall(callF, ArgsV, "calltmp");
        }
    }
}

void *CodeGen::visit(ValueOrConstructorIdentifierExpAST *ast) {
    llvm::Value* V = NamedValues[ast->getLongId()->getIds()[0]->get()];
    if(!V)
        Error("unknown variable name");
    return V;
//    auto tem = (llvm::Value*) ast->getId()->accept(this);
//    NamedValues[ast->getId()->get()] = tem;
//    return tem;
}

void *CodeGen::visit(LocalDeclarationExpAST *ast) {///返回llvm::Function*
    std::vector<llvm::Value*> tem;
    auto exps = ast->getExps();
    auto dec = ast->getDec();
    dec->accept(this);
    for(int i =0;i<ast->getExps().size();i++){
        tem.push_back((llvm::Value*)ast->getExps()[i]->accept(this));
    }
    return tem[tem.size()-1];
}

void *CodeGen::visit(SequenceDecAST *ast) {

    for(int i =0;i<ast->getDecs().size();i++){
        auto temValue = (llvm::Value*)ast->getDecs()[i]->accept(this);

        auto temDec = dynamic_pointer_cast<ValueDecAST>(ast->getDecs()[i]);
        auto temDesDec = dynamic_pointer_cast<DestructuringValBindAST>(temDec->getValBind());
        auto temConDec = dynamic_pointer_cast<ConstructionPatAST>(temDesDec->getPat());
        NamedValues[temConDec->getLongId()->getIds()[0]->get()] = temValue;
    }
}
