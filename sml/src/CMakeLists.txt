include_directories(Common)
add_subdirectory(Common)

add_subdirectory(CodeGenerator)
add_subdirectory(Error)
add_subdirectory(Interpreter)
add_subdirectory(JIT)
add_subdirectory(Parser)
add_subdirectory(Scanner)
add_subdirectory(SemanticAnalyzer)
add_subdirectory(Token)

add_library(SMLLib INTERFACE)
target_link_libraries(SMLLib
		INTERFACE
		SMLCodeGenerator
		SMLError
		SMLInterpreter
		SMLJIT
		SMLParser
		SMLScanner
		SMLSemanticAnalyzer
		SMLToken
		LLVM-6.0)

target_link_libraries(SMLLib INTERFACE SMLCommon)
