set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../include include .)

#set(LLVM_LINK_COMPONENTS
#		BitWriter
#		Core
#		ExecutionEngine
#		MC
#		MCJIT
#		Support
#		nativecodegen
#		)
#
#set(sml_sources
#		src/SemanticAnalyzer/TypeCheck.cpp
#		src/SemanticAnalyzer/VariableCheck.cpp
#		src/Error/Error.cpp
#		src/Interpreter/Interpreter.cpp
#		src/JIT/JIT.cpp
#		src/Parser/Parser.cpp
#		src/Scanner/Scanner.cpp
#		src/SemanticAnalyzer/SemanticAnalyzer.cpp
#		src/Token/Token.cpp
#		)
#
#set(sml_headers
#		include/Error.h
#		include/Interpreter.h
#		include/JIT.h
#		include/Parser.h
#		include/Scanner.h
#		include/SemanticAnalyzer.h
#		include/Token.h
#		)

project(SML)

if (SEPERATE_FROM_LLVM_SOURCE)
    find_package(LLVM 6.0 REQUIRED CONFIG)
    include_directories(${LLVM_INCLUDE_DIRS})
endif ()

add_subdirectory(src)
#	add_library(SMLInterpreterLib STATIC ${sml_sources})
add_executable(${PROJECT_NAME} main.cpp ${sml_headers})
target_link_libraries(${PROJECT_NAME}
        SMLLib
        LLVM-6.0)

add_subdirectory(test)

#	add_executable(TOY toy.cxx)
#	target_link_libraries(TOY LLVMSupport)

set(LLVM_ENABLE_EH ON)
set(LLVM_ENABLE_RTTI ON)

#	add_llvm_example(sml main.cpp
#			${sml_sources} ${sml_headers})

#	add_subdirectory(test)

#project(sml)
#
#set(LLVM_LINK_COMPONENTS
#        Analysis
#        Core
#        ExecutionEngine
#        InstCombine
#        Object
#        RuntimeDyld
#        ScalarOpts
#        Support
#        native
#        Core
#        Interpreter
#        MC
#        nativecodegen
#        )
#
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
#file(GLOB sources *.cpp)
#file(GLOB headers *.h)
#
#include_directories(../include .)

#add_executable(${PROJECT_NAME} ${sources} ${headers})
#add_llvm_example(${PROJECT_NAME} ${sources} ${headers})
##add_llvm_example(main.cpp ${ARGN})
#target_link_libraries(${PROJECT_NAME} ${LLVM_PTHREAD_LIB})
##add_executable(${PROJECT_NAME}-toy toy.cxx)
