project(SMLSemanticAnalyzer)

add_library(
		${PROJECT_NAME}
		TypeCheck.cpp
		SemanticAnalyzer.cpp
		SemanticAnalyzerImpl.cpp
)
