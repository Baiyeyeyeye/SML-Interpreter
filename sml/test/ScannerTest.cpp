#include <memory>
#include <sstream>
#include "gtest/gtest.h"
#include "Scanner.h"
#include "Token.h"

using namespace std;

class ScannerTest : public ::testing::Test {

};

TEST_F(ScannerTest, ScannerTest_FreeScanTokens_Test) {
    auto scanTokens = [](const char *str) {
        istringstream iss(str);
        Scanner scanner(iss);

        cout << "----------------" << '\n';
        cout << str << '\n';
        while (auto token = scanner.getNextToken()) {
            cout << token->getValue() << '\n';
        }
        cout << "----------------" << endl;
    };

    scanTokens("fun f(x, y) = x + y;");
    scanTokens("fun f(x,y)=x+y;");
}

TEST_F(ScannerTest, ScannerTest_Declaration_Test) {
    const char *decl;
    istringstream in;

    auto newScan = [&](auto &&str) {
        decl = str;
        in = istringstream(decl);
        Scanner scanner(in);
        return scanner.scan();
    };

    decltype(newScan("")) res;

    auto assertSizeOrPrintElems = [&](size_t size) {
        ASSERT_EQ(res.size(), size) << "elems: " << [&res]() -> string {
                        string ret;
                        ret += '[';
                        for (auto &&elem : res) {
                            ret += elem->getValue();
                            ret += ", ";
                        }
                        ret.pop_back();
                        return ret + ']';
                    }();
    };

    {
        res = newScan("val valval:int=100000;");
        auto i = 0;
        const char *const values[] = {
                "val", "valval", ":", "int", "=", "100000"
        };
        assertSizeOrPrintElems(size(values));
        for (auto &&val : values) {
            ASSERT_EQ(res[i++]->getValue(), val);
        }
    }

    {
        res = newScan(
                "let val ==== =0X12BC val ## = 0.1E~2 in val i= ==== "
                " val j = ## end;");
        auto i = 0;
        const char *const values[] = {
                "let", "val", "====", "=", "0X12BC", "val", "##", "=",
                "0.1E~2", "in", "val", "i", "=", "====", "val", "j", "=", "##",
                "end"
        };
        assertSizeOrPrintElems(size(values));
        for (auto &&val : values) {
            ASSERT_EQ(res[i++]->getValue(), val);
        }
    }
}
