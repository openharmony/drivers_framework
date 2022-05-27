/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_TOKEN_H
#define OHOS_HDI_TOKEN_H

#include <util/string.h>

namespace OHOS {
namespace HDI {
enum class TokenType {
    UNKNOWN = 0,
    // types
    VOID,
    BOOLEAN, // "boolean"
    BYTE,    // "byte"
    SHORT,   // "short"
    INT,     // "int"
    LONG,    // "long"
    STRING,  // "String"
    FLOAT,   // "float"
    DOUBLE,  // "double"
    FD,      // "FileDescriptor"
    ASHMEM,  // "Ashmem"
    LIST,    // "List"
    MAP,     // "Map"
    SMQ,     // "SharedMemQueue"
    CHAR,    // "char"
    // qualifier
    UNSIGNED, // "unsigned"
    // custom types
    ENUM,   // "enum"
    STRUCT, // "struct"
    UNION,  // "union"
    // keywords
    PACKAGE,              // "package"
    SEQ,                  // "sequenceable"
    IMPORT,               // "import"
    INTERFACE,            // "interface"
    EXTENDS,              // "extends"
    ONEWAY,               // "oneway"
    CALLBACK,             // "callback"
    FULL,                 // "full"
    LITE,                 // "lite"
    IN,                   // "in"
    OUT,                  // "out"
    DOT,                  // "."
    COMMA,                // ","
    COLON,                // ":"
    ASSIGN,               // "="
    SEMICOLON,            // ";"
    BRACES_LEFT,          // "{"
    BRACES_RIGHT,         // "}"
    BRACKETS_LEFT,        // "["
    BRACKETS_RIGHT,       // "]"
    PARENTHESES_LEFT,     // "("
    PARENTHESES_RIGHT,    // ")"
    ANGLE_BRACKETS_LEFT,  // "<"
    ANGLE_BRACKETS_RIGHT, // ">"
    ADD,                  // "+"
    SUB,                  // "-"
    STAR,                 // "*"
    SLASH,                // "/"
    PERCENT_SIGN,         // "%""
    LEFT_SHIFT,           // "<<"
    RIGHT_SHIFT,          // ">>"
    AND,                  // "&"
    XOR,                  // "^"
    OR,                   // "|"
    TILDE,                // "~"
    PPLUS,                // "++"
    MMINUS,               // "--"

    // others
    ID,
    NUM,
    COMMENT_BLOCK,
    COMMENT_LINE,
    END_OF_FILE,
};

struct Location {
    String filePath_;
    size_t row_;
    size_t col_;
};

struct Token {
    TokenType kind_;
    Location location_;
    String value_;

    String Dump();
};

struct TokenTypeCompare {
    bool operator()(const Token &lhs, const Token &rhs)
    {
        return lhs.kind_ > rhs.kind_;
    }
};

String LocInfo(const Token &token);

String TokenTypeToString(TokenType type);

int TokenTypeToChar(TokenType type);
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_TOKEN_H