#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

class Token;

class Scanner {
public:
    explicit Scanner(std::istream &in);

    ~Scanner();

    [[deprecated("Use getNextToken() to get token instead.")]]
    std::vector<std::shared_ptr<Token>> scan();

    /**
     * Get next token from input, if any.
     * @return A token pointer if a token is scanned from input, otherwise a
     * nullptr.
     */
    virtual std::shared_ptr<Token> getNextToken();

    [[nodiscard]] virtual bool eof() const;

private:
    struct Impl;

    std::unique_ptr<Impl> _impl;
};
