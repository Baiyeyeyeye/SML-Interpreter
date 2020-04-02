#pragma once

#include <any>
#include <string>

class Token {
public:
    /** NONE and END is only for scanner convenience to handle spaces and
     *  would not appear after the scanning process.
     */
    enum Type {
        INT,
        STRING,
        CHAR,
        REAL,
        BOOL,
        OPERATOR, ///< ; : [ ] { } ( )
        KEYWORD, ///< val type fun fn
        ID, ///< int a + ###
    };

    Token(int row, int column, int type, std::string value);

    [[nodiscard]] double getReal() const;

    [[nodiscard]] bool getBool() const;

    [[nodiscard]] int getInt() const;

    [[nodiscard]] char getChar() const;

    [[nodiscard]] const std::string &getString() const;

    [[nodiscard]] int getRow() const;

    [[nodiscard]] int getColumn() const;

    [[nodiscard]] Type getType() const;

    [[nodiscard]] const std::string &getValue() const;

private:
    struct Impl;

    std::shared_ptr<Impl> mImpl;
};
