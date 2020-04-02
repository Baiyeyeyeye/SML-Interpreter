if (SEPERATE_FROM_LLVM_SOURCE)
	project(SMLInterpreterTest)

	find_package(GTest REQUIRED)

	find_package(LLVM 6.0 REQUIRED CONFIG)
	llvm_map_components_to_libnames(llvm_libs all)
endif()

include_directories(../include ../src/Common)

if (SEPERATE_FROM_LLVM_SOURCE)
	set(gtest_lib GTest::GTest GTest::Main)
else()
	set(gtest_lib gtest_main)
endif ()

macro(add_test name)
	add_executable(${PROJECT_NAME}-${name} ${name}.cpp)

	target_link_libraries(${PROJECT_NAME}-${name}
			${gtest_lib}
			LLVM-6.0
			${ARGN})
endmacro()

add_test(TokenTest SMLToken)
add_test(ScannerTest SMLToken SMLScanner)
add_test(ParserTest SMLLib)
add_test(SemaTest SMLToken SMLScanner SMLParser SMLSemanticAnalyzer SMLCommon)
add_test(CodeGenTest SMLToken SMLScanner SMLParser SMLSemanticAnalyzer SMLCodeGenerator SMLCommon)
add_test(FreeTest SMLLib)
add_test(ASTTest SMLLib)
add_test(SymbolTableTest SMLLib)

#	add_custom_target(SMLTests)
#	set_target_properties(SMLTests PROPERTIES FOLDER "SML tests")
#	#target_link_libraries(SMLTests sml)
#
#	set(LLVM_LINK_COMPONENTS
#			Support
#			)
#
#	function(add_sml_unittest test_dir)
#		add_unittest(SMLTests ${test_dir} ${ARGN}
#				${sml_test_sources} ${sml_test_headers})
#	endfunction()
#
#	include_directories(..)
#	set(sml_test_headers)
#	foreach (file ${sml_headers})
#		set(sml_test_headers ${sml_test_headers} ../${file})
#	endforeach ()
#	set(sml_test_sources)
#	foreach (file ${sml_sources})
#		set(sml_test_sources ${sml_test_sources} ../${file})
#	endforeach ()
#
#	add_sml_unittest(SMLTest
#			TokenTest.cpp ScannerTest.cpp ParserTest.cpp SemaTest.cpp)
