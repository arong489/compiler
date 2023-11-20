#ifndef _LEXER
#define _LEXER
#define CONTENT_SIZE 100

#include "./exception/exception.h"
#include "token.h"
#include <fstream>
#include <queue>
// #define isLineFeed(c) ((c) == '\n' || (c) == '\r')
#define isLineFeed(c) ((c) == '\n')
class Lexer {
private:
    Token curToken;
    std::istream* fin;
    std::filebuf* buf;
    std::ostream* fout;
    char curChar;
    int inner_char_line, inner_char_column;
    int inner_line, inner_column;
    bool peek, save, load;
    std::queue<Token> tokenBuffer;
    std::string* sout;
    char nextChar();
    Token digitHeader(char) noexcept;
    Token alphaHeader(char) noexcept;
    Token signHeader(char) noexcept;
    void exceptionHandler(std::string&);
    int tokenLineKeeper, tokenColumnKeeper;

    ExceptionController *exceptionController;

public:
    Lexer(std::ostream* fout, ExceptionController *exceptionController,std::string = "");
    // if stop at current token
    Token nextToken() noexcept;
    Token peekToken() noexcept;
    void hold(std::string* sout);
    void discharge(bool = false);
    bool endInput;
    int line, column;
};
#endif