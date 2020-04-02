project(SMLParser)

add_library(${PROJECT_NAME}
		Parser.cpp)

target_link_libraries(${PROJECT_NAME}
		SMLCommon
		SMLInterpreter
		SMLToken)
