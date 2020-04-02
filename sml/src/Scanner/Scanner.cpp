#include <queue>
#include <regex>
#include <tuple>
#include <set>
#include <vector>
#include "Error.h"
#include "Scanner.h"
#include "Token.h"

using namespace std;

namespace {
    enum {
        ERROR = -1, NONE = -2
    };

    const auto &getReservedOperators() {
        static set<string, greater<>> reservedOperators = {
                "(", ")", "[", "]", "{", "}", ",", ":", ";",
                "...", "_", "|", "=>", "->", "#",
        };
        return reservedOperators;
    }
}

struct Scanner::Impl {
public:
    /**
     * This should init the regexes buffer.
     */
    explicit Impl(istream &in);

    vector<shared_ptr<Token>> scan();

    /**
     * Get next token from input, if any.
     * @return A token pointer if a token is scanned from input, otherwise a
     * nullptr.
     */
    shared_ptr<Token> getNextToken();

    shared_ptr<Token> getNextCachedToken();

    void cacheTokens();

    /**
     * The customized get line. this would get a line from `in` to `str` and
     * reset `row` and `column` to 1.
     * @return The istream.
     */
    istream &getLine();

    /**
     * match next token with the regex `re`. on success it sets the matching
     * result to `matchStr` and `match` and increases the column (not
     * tokenColumn), and the `str` is advanced to the matching suffix.
     * @param re the regex.
     * @return true if matched.
     */
    bool matchNext(const regex &re);

    ///{
    // regex rules initializers: create regex for each token types.
    static regex getBoolRegex();

    static regex getKeywordRegex();

    static regex getCharRegex();

    static regex getStringRegex();

    static regex getOperatorRegex();

    static regex getRealRegex();

    static regex getIntRegex();

    static regex getIdRegex();

    static regex getSpaceRegex();

    static regex getAnyRegex();
    ///}

    /**
     * rows & columns. row == tokenRow at any time, and column is greater than
     * tokenColumn after a match, so remember to  set the tokenColumn after a
     * match.
     */
    int row = 0;
    int column = 0;
    int tokenColumn = 0;
    int tokenRow = 0;
    /**
     * the ordered regexes rules initialized by `initRegexes()`.
     */
    vector<tuple<int, regex>> regexes;

    /**
     * the buffer tokens queue, return its front on every call. if empty, start
     * a new routine of matching.
     */
    queue<shared_ptr<Token>> matchedTokens;

    /**
     * the string buf for scanning.
     */
    string str;

    /**
     * objects to hold the matching result.
     */
    string matchStr;
    smatch match;

    istream &in;
};

Scanner::Impl::Impl(istream &in) : in(in) {
    regexes.emplace_back(Token::BOOL, getBoolRegex());
    regexes.emplace_back(Token::KEYWORD, getKeywordRegex());
    regexes.emplace_back(Token::CHAR, getCharRegex());
    regexes.emplace_back(Token::STRING, getStringRegex());
    regexes.emplace_back(Token::ID, getIdRegex());
    regexes.emplace_back(Token::OPERATOR, getOperatorRegex());
    regexes.emplace_back(Token::REAL, getRealRegex());
    regexes.emplace_back(Token::INT, getIntRegex());
    regexes.emplace_back(NONE, getSpaceRegex());
    regexes.emplace_back(ERROR, getAnyRegex());
}

regex Scanner::Impl::getBoolRegex() {
    return regex("true|false");
}

regex Scanner::Impl::getKeywordRegex() {
    const char *set[] = {
            "abstype", "and", "andalso", "as", "case", "datatype", "do", "else",
            "end", "exception", "fn", "fun", "handle", "if", "in", "infix",
            "infixr", "let", "local", "nonfix", "of", "op", "open", "orelse",
            "raise", "rec", "then", "type", "val", "with", "withtype", "while",
    };
    string reStr;
    reStr += '(';
    for (auto &&elem : set) {
        reStr += elem;
        reStr += '|';
    }
    reStr.pop_back();
    reStr += ")(?=\\s)";
    return regex(reStr);
}

regex Scanner::Impl::getCharRegex() {
    return regex(R"(#\"(\\a|\\b|\\n|\\r|\\v|\\\\|\\"|\\[0-7]{3}|.)\")");
}

regex Scanner::Impl::getStringRegex() {
    return regex(R"(\"(\\a|\\b|\\n|\\r|\\v|\\\\|\\"|\\[0-7]{3}|.)*?\")");
}

regex Scanner::Impl::getOperatorRegex() {
    /**
     * we have to match the longest one, ie: >= instead of >, so use a greater
     * first ordered set.
     */
    string reStr;
//    reStr += '(';
    for (auto &&elem : getReservedOperators()) {
        for (auto &&c : elem) {
            reStr += '\\';
            reStr += elem;
        }
        reStr += '|';
    }
    reStr.pop_back();
//    reStr += R"()(?=\s|[a-zA-Z0-9\(\)\[\]\{\}\,\:\;\_]))";
    return regex(reStr);
}

regex Scanner::Impl::getRealRegex() {
    return regex(
            R"(\d+\.\d+[eE]~?\d+)"
            "|"
            R"(\d+[eE]~?\d+)"
            "|"
            R"(\d+\.\d+)");
}

regex Scanner::Impl::getIntRegex() {
    return regex("0[xX][0-9a-fA-F]+|\\d+");
}

regex Scanner::Impl::getIdRegex() {
    return regex(
            "[a-zA-Z'][0-9a-zA-Z_']*"
            "|"
            "[\\-!%&$#+/:<=>?@\\~`^|*]+");
}

regex Scanner::Impl::getSpaceRegex() {
    return regex("\\s+");
}

regex Scanner::Impl::getAnyRegex() {
    return regex(".");
}

shared_ptr<Token> Scanner::Impl::getNextToken() {
    if (auto next = getNextCachedToken()) {
        return next;
    }
    cacheTokens();
    return getNextCachedToken();
}

istream &Scanner::Impl::getLine() {
    auto &&ret = std::getline(in, str);
    str += ' ';
    tokenRow = ++row;
    column = tokenColumn = 1;
    tokenColumn = column = 1;
    return ret;
}

bool Scanner::Impl::matchNext(const regex &re) {
    if (regex_search(
            str, match, re,
            regex_constants::match_continuous)) {
        matchStr = match.str();
        str = match.suffix();
        column += matchStr.size();
        return true;
    }
    return false;
}

vector<shared_ptr<Token>> Scanner::Impl::scan() {
    vector<shared_ptr<Token>> tokens;
    while (in) {
        if (auto &&token = getNextToken()) {
            tokens.push_back(token);
        } else {
            break;
        }
    }
    return tokens;
}

void Scanner::Impl::cacheTokens() {
    /**
     * objects to handle comment.
     */
    static regex reCommentLeft(R"(\(\*)");
    static regex reCommentRight(R"(\*\))");
    static regex reAny(".");
    int eatingComment = 0;

    /**
     * process one line per call.
     */
    getLine();
    while (!str.empty() && in) {
        /**
         * handle recursive comments.
         */
        if (matchNext(reCommentLeft)) {
            ++eatingComment;
        }
        if (eatingComment) {
            while (eatingComment && in) {
                if (str.empty()) {
                    getLine();
                    continue;
                }
                if (matchNext(reCommentLeft)) {
                    ++eatingComment;
                    continue;
                }
                if (matchNext(reCommentRight)) {
                    --eatingComment;
                    continue;
                }
                matchNext(reAny);
            }
            continue;
        }

        /**
         * handle regex type & rules, get the true token.
         */
        for (auto const&[type, rule] : regexes) {
            if (matchNext(rule)) {
                if (type != ERROR && type != NONE) {
                    std::shared_ptr<Token> token;
                    // FIXME: this is only a hack, consider another solution.
                    if (getReservedOperators().count(matchStr)) {
                        token = make_shared<Token>(
                                tokenRow,
                                tokenColumn,
                                Token::OPERATOR,
                                matchStr);
                    } else {
                        token = make_shared<Token>(
                                tokenRow,
                                tokenColumn,
                                type,
                                matchStr);
                    }
                    matchedTokens.push(std::move(token));
                    tokenColumn = column;
                }
                if (type == ERROR) {
                    Error("Unrecognized token " + matchStr + '.');
                    break;
                }
                break;
            }
        }
    }
}

shared_ptr<Token> Scanner::Impl::getNextCachedToken() {
    if (!matchedTokens.empty()) {
        auto res = std::move(matchedTokens.front());
        matchedTokens.pop();
        return res;
    }
    return nullptr;
}

vector<shared_ptr<Token>> Scanner::scan() {
    return _impl->scan();
}

std::shared_ptr<Token> Scanner::getNextToken() {
    return _impl->getNextToken();
}

Scanner::Scanner(istream &in) : _impl(make_unique<Impl>(in)) {

}

bool Scanner::eof() const {
    return !_impl->in;
}

Scanner::~Scanner() = default;
