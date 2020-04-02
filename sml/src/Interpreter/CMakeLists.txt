project(SMLInterpreter)

add_library(${PROJECT_NAME} Interpreter.cpp)
target_link_libraries(${PROJECT_NAME}
        INTERFACE
        SMLCodeGenerator
        SMLError
        SMLInterpreter
        SMLJIT
        SMLParser
        SMLScanner
        SMLSemanticAnalyzer
        SMLToken)
