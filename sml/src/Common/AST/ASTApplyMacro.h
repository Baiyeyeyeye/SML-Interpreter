// Be careful to undef after including this.
//region APPLY_ALL
#ifndef APPLY_ALL
#define APPLY_ALL \
APPLY(AST) \
APPLY(ConAST) \
APPLY(IntConAST) \
APPLY(FloatConAST) \
APPLY(BoolConAST) \
APPLY(CharConAST) \
APPLY(StringConAST) \
APPLY(IdAST) \
APPLY(VarAST) \
APPLY(LongIdAST) \
APPLY(LabAST) \
APPLY(AlphanumericIdAST) \
APPLY(SymbolicIdAST) \
APPLY(UnconstrainedVarAST) \
APPLY(EqualityVarAST) \
APPLY(IdentifierLabAST) \
APPLY(NumberLabAST) \
APPLY(ExpAST) \
APPLY(ConstantExpAST) \
APPLY(ValueOrConstructorIdentifierExpAST) \
APPLY(ApplicationExpAST) \
APPLY(InfixApplicationExpAST) \
APPLY(ParenthesesExpAST) \
APPLY(TupleExpAST) \
APPLY(RecordTupleExpAST) \
APPLY(RecordSelectorExpAST) \
APPLY(ListExpAST) \
APPLY(LocalDeclarationExpAST) \
APPLY(TypeAnnotationExpAST) \
APPLY(ConjunctionExpAST) \
APPLY(DisjunctionExpAST) \
APPLY(ConditionalExpAST) \
APPLY(IterationExpAST) \
APPLY(CaseAnalysisExpAST) \
APPLY(FunctionExpAST) \
APPLY(ExpRowAST) \
APPLY(MatchAST) \
APPLY(PatAST) \
APPLY(ConstantPatAST) \
APPLY(WildCardPatAST) \
APPLY(VariablePatAST) \
APPLY(ConstructionPatAST) \
APPLY(InfixConstructionPatAST) \
APPLY(ParenthesesPatAST)\
APPLY(TuplePatAST) \
APPLY(RecordPatAST) \
APPLY(ListPatAST) \
APPLY(TypeAnnotationPatAST) \
APPLY(LayeredPatAST) \
APPLY(PatRowAST) \
APPLY(TypAST) \
APPLY(VariableTypAST) \
APPLY(ConstructorTypAST) \
APPLY(ParenthesesTypAST) \
APPLY(FunctionTypAST) \
APPLY(TupleTypAST) \
APPLY(RecordTypAST) \
APPLY(TypRowAST) \
APPLY(WildCardPatRowAST) \
APPLY(PatternPatRowAST) \
APPLY(VariablePatRowAST) \
APPLY(DecAST) \
APPLY(ValueDecAST) \
APPLY(FunctionDecAST) \
APPLY(TypeDecAST) \
APPLY(DataTypeDecAST) \
APPLY(ValBindAST) \
APPLY(DestructuringValBindAST) \
APPLY(RecursiveValBindAST) \
APPLY(FunBindAST) \
APPLY(FunMatchAST) \
APPLY(NonFixFunMatchAST) \
APPLY(InfixFunMatchAST) \
APPLY(TypBindAST) \
APPLY(SequenceDecAST) \
APPLY(LocalDecAST) \
APPLY(LeftAssociativeInfixDecAST)\
APPLY(RightAssociativeInfixDecAST)\
APPLY(NonfixDecAST) \

#endif
//endregion
