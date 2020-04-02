#pragma once

namespace llvm {
    class Value;
}

class JIT {
public:
    void run(llvm::Function* theFun);
};
