project(SMLCommon)

add_library(SMLAST
        AST/AST.cpp
        AST/ASTVisitor.cpp
        AST/ASTProperty.cpp)

add_library(SMLSymbol
        Symbol/SymbolTable.cpp)

add_library(SMLJITModule
        JITModule/JITModule.cpp)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE SMLAST SMLSymbol SMLJITModule)
