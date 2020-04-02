project(SMLScanner)

add_library(${PROJECT_NAME}
		Scanner.cpp)

target_link_libraries(${PROJECT_NAME}
		SMLCommon
		SMLError
		SMLToken)
