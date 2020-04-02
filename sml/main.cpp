#include <iostream>
#include <fstream>
#include <string>
#include "Interpreter.h"

using namespace std;

int main(int argc, char *argv[]) {

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            ifstream fin(argv[i]);
            Interpreter(fin).interpret();
        }
    } else {
        Interpreter(cin).interpret();
    }

    return 0;
}
