project(SMLCodeGenerator)

add_library(${PROJECT_NAME} CodeGenerator.cpp CodeGeneratorImpl.cpp)
target_link_libraries(${PROJECT_NAME}
        SMLCommon
        SMLError
        LLVM-6.0)
