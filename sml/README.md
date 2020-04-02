# SML Interpreter 

## 项目结构

- sml
    - include 外部调用公共包含头文件
    - src 具体实现源码
        - CodeGenerator 代码生成实现
        - Common 私有公共库，包括AST，JITModule，Symbol
        - Error 错误产生
        - Interpreter 顶层外观模式解释器实现，将各部分组织串联
        - JIT 运行
        - Parser Parser实现，建立AST
        - Scanner Scanner实现，从输入流获得Token序列
        - SemanticAnalyzer SemanticAnalyzer实现，类型检查
        - Token Token类定义与相关函数实现
    - test 单元测试
        - CodeGenTest.cpp 代码生成测试
        - FreeTest.cpp 自由测试
        - ParserTest.cpp Parser功能测试
        - ScannerTest.cpp Scanner功能测试
        - SemaTest.cpp Semantic Analyzer功能测试
        - TokenTest.cpp Token加载测试
    - main.cpp 程序总入口
    - CMakeLists.txt 顶层CMake工程定义

项目调用LLVM库，没有对LLVM库源码进行修改。
