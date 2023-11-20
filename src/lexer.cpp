#include "../include/lexer.h"
#include <iostream>
// #define lexer_debug_output

std::string operatorString = ",;[]{}=()+-*/%!<>&|\"";

Token Lexer::signHeader(char c) noexcept
{
    std::string value;
    switch (c) {
    case ',':
        return Token(TKTYPE::COMMA, this->inner_line, this->inner_column);
    case ';':
        return Token(TKTYPE::SEMICN, this->inner_line, this->inner_column);
    case '[':
        return Token(TKTYPE::LBRACK, this->inner_line, this->inner_column);
    case ']':
        return Token(TKTYPE::RBRACK, this->inner_line, this->inner_column);
    case '{':
        return Token(TKTYPE::LBRACE, this->inner_line, this->inner_column);
    case '}':
        return Token(TKTYPE::RBRACE, this->inner_line, this->inner_column);
    case '=':
        if (this->fin->peek() == '=') {
            this->fin->ignore();
            this->inner_char_column++;
            return Token(TKTYPE::EQL, this->inner_line, this->inner_column);
        } else
            return Token(TKTYPE::ASSIGN, this->inner_line, this->inner_column);
    case '(':
        return Token(TKTYPE::LPARENT, this->inner_line, this->inner_column);
    case ')':
        return Token(TKTYPE::RPARENT, this->inner_line, this->inner_column);
    case '+':
        return Token(TKTYPE::PLUS, this->inner_line, this->inner_column);
    case '-':
        return Token(TKTYPE::MINUS, this->inner_line, this->inner_column);
    case '*':
        return Token(TKTYPE::MULT, this->inner_line, this->inner_column);
    case '/':
        return Token(TKTYPE::DIV, this->inner_line, this->inner_column);
    case '%':
        return Token(TKTYPE::MOD, this->inner_line, this->inner_column);
    case '!':
        if (this->fin->peek() == '=') {
            this->fin->ignore();
            this->inner_char_column++;
            return Token(TKTYPE::NEQ, this->inner_line, this->inner_column);
        } else
            return Token(TKTYPE::NOT, this->inner_line, this->inner_column);
    case '>':
        if (this->fin->peek() == '=') {
            this->fin->ignore();
            this->inner_char_column++;
            return Token(TKTYPE::GEQ, this->inner_line, this->inner_column);
        } else
            return Token(TKTYPE::GRE, this->inner_line, this->inner_column);
    case '<':
        if (this->fin->peek() == '=') {
            this->fin->ignore();
            this->inner_char_column++;
            return Token(TKTYPE::LEQ, this->inner_line, this->inner_column);
        } else
            return Token(TKTYPE::LSS, this->inner_line, this->inner_column);
    case '&':
        if (this->fin->peek() == '&') {
            this->fin->ignore();
            this->inner_char_column++;
            return Token(TKTYPE::AND, this->inner_line, this->inner_column);
        }
        // TODO: else exception
        else {
            value += c;
            this->exceptionHandler(value);
            // this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
            return Token(TKTYPE::WRONGTK, this->inner_line, this->inner_column, value);
        }
    case '|':
        if (this->fin->peek() == '|') {
            this->fin->ignore();
            this->inner_char_column++;
            return Token(TKTYPE::OR, this->inner_line, this->inner_column);
        }
        // TODO: else exception
        else {
            value += c;
            this->exceptionHandler(value);
            // this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
            return Token(TKTYPE::WRONGTK, this->inner_line, this->inner_column, value);
        }
    case '\"':
        while (this->fin->peek() != '\"') {
            value += this->fin->get();
            if (isLineFeed(value.back())) {
                this->inner_char_line++;
                this->inner_char_column = 0;
            } else
                this->inner_char_column++;
        }
        this->fin->ignore();
        this->inner_char_column++;
        return Token(TKTYPE::STRCON, this->inner_line, this->inner_column, value);
    default:
        value += c;
        this->exceptionHandler(value);
        // this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
        return Token(TKTYPE::WRONGTK, this->inner_line, this->inner_column, value);
    }
    // return Token();
}
Token Lexer::alphaHeader(char c) noexcept
{
    std::string value;
    value += c;
    while (isalnum(c = this->fin->peek()) || c == '_') {
        value += this->fin->get();
        if (isLineFeed(value.back())) {
            this->inner_char_line++;
            this->inner_char_column = 0;
        } else
            this->inner_char_column++;
    }
    switch (value.length()) {
    case 2:
        if (value.compare("if") == 0)
            return Token(TKTYPE::IFTK, this->inner_line, this->inner_column);
        else
            return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    case 3:
        if (value.compare("for") == 0)
            return Token(TKTYPE::FORTK, this->inner_line, this->inner_column);
        else if (value.compare("int") == 0)
            return Token(TKTYPE::INTTK, this->inner_line, this->inner_column);
        else
            return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    case 4:
        if (value.compare("else") == 0)
            return Token(TKTYPE::ELSETK, this->inner_line, this->inner_column);
        else if (value.compare("void") == 0)
            return Token(TKTYPE::VOIDTK, this->inner_line, this->inner_column);
        else if (value.compare("main") == 0)
            return Token(TKTYPE::MAINTK, this->inner_line, this->inner_column);
        else
            return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    case 5:
        if (value.compare("break") == 0)
            return Token(TKTYPE::BREAKTK, this->inner_line, this->inner_column);
        else if (value.compare("const") == 0)
            return Token(TKTYPE::CONSTTK, this->inner_line, this->inner_column);
        else
            return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    case 6:
        if (value.compare("return") == 0)
            return Token(TKTYPE::RETURNTK, this->inner_line, this->inner_column);
        else if (value.compare("printf") == 0)
            return Token(TKTYPE::PRINTFTK, this->inner_line, this->inner_column);
        else if (value.compare("getint") == 0)
            return Token(TKTYPE::GETINTTK, this->inner_line, this->inner_column);
        else
            return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    case 8:
        if (value.compare("continue") == 0)
            return Token(TKTYPE::CONTINUETK, this->inner_line, this->inner_column);
        else
            return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    default:
        return Token(TKTYPE::IDENFR, this->inner_line, this->inner_column, value);
    }
}
Token Lexer::digitHeader(char c) noexcept
{
    std::string value;
    bool exception = false;
    value += c;
    while (isalnum(this->fin->peek())) {
        c = this->fin->get();
        value += c;
        if (isLineFeed(c)) {
            this->inner_char_line++;
            this->inner_char_column = 0;
        } else
            this->inner_char_column++;
        if (isalpha(c))
            exception = true;
    }
    // TODO: if (exception) exception
    if (exception) {
        this->exceptionHandler(value);
        // this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
        return Token(TKTYPE::WRONGTK, this->inner_line, this->inner_column, value);
    }
    return Token(TKTYPE::INTCON, this->inner_line, this->inner_column, value);
}
char Lexer::nextChar()
{
    char c, c1;
    while (true) {
        do {
            c = this->fin->get();
            if (isLineFeed(c)) {
                this->inner_char_line++;
                this->inner_char_column = 0;
            } else
                this->inner_char_column++;
        } while (isspace(c));

        if (c == '/') {
            c1 = this->fin->peek();
            if (c1 == '/') {
                while (!isLineFeed(c1))
                    c1 = this->fin->get();
                this->inner_char_line++;
                this->inner_char_column = 0;
            } else if (c1 == '*') {
                this->fin->ignore();
                this->inner_char_column++;
                do {
                    do {
                        c1 = this->fin->get();
                        if (isLineFeed(c1)) {
                            this->inner_char_line++;
                            this->inner_char_column = 0;
                        } else
                            this->inner_char_column++;
                    } while (c1 != '*');
                    c1 = this->fin->peek();
                } while (c1 != '/');
                this->fin->get();
                this->inner_char_column++;
            } else
                break;
        } else
            break;
    }
    return c;
}
void Lexer::exceptionHandler(std::string& feedback)
{
    char c;
    while (!isspace(c = this->fin->get())) {
        feedback += c;
        if (isLineFeed(c)) {
            this->inner_char_line++;
            this->inner_char_column = 0;
        } else {
            this->inner_char_column++;
        }
    }
    this->curChar = this->nextChar();
    this->endInput = this->curChar == -1;
}
Lexer::Lexer(std::ostream* fout, ExceptionController* exceptionController, std::string path)
    : fout(fout)
    , inner_char_line(1)
    , inner_char_column(0)
    , inner_line(1)
    , inner_column(0)
    , peek(false)
    , save(false)
    , load(false)
    , tokenBuffer()
    , sout(nullptr)
    , line(1)
    , column(1)
    , exceptionController(exceptionController)
{
    if (path.empty()) {
        this->fin = &std::cin;
        this->buf = nullptr;
    } else {
        this->buf = new std::filebuf();
        this->buf->open(path, std::ios::in);
        this->fin = new std::istream(buf);
    }
    this->curChar = this->nextChar();
    this->endInput = this->curChar == -1;
}

Token Lexer::nextToken() noexcept
{

    // load saved Token
    if (this->load) {
        Token temp = this->tokenBuffer.front();
        this->tokenBuffer.pop();
        this->load = !this->tokenBuffer.empty();

#ifdef lexer_debug_output
        if (this->sout == nullptr)
            (*fout) << temp << std::endl;
        else if (this->sout != nullptr)
            (*sout) += temp, (*sout) += '\n';
#endif

        if (temp.type == TKTYPE::WRONGTK) {
            this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
        }

        if (this->column == this->tokenColumnKeeper && this->line == this->line) {
            this->tokenColumnKeeper = this->tokenLineKeeper = -1;
        } else {
            this->line = temp.line;
            this->column = temp.column + temp.worldLength();
        }

        return temp;
    }

    // go on if has read
    if (this->peek) {
        this->peek = false;
        this->endInput = this->curChar == -1;
    } else if (!this->endInput) {
        // get new Token
        this->inner_line = this->inner_char_line;
        this->inner_column = this->inner_char_column;
        if (operatorString.find(this->curChar) != std::string::npos)
            this->curToken = this->signHeader(this->curChar);
        else if (isdigit(this->curChar))
            this->curToken = this->digitHeader(this->curChar);
        else if (isalpha(this->curChar) || this->curChar == '_')
            this->curToken = this->alphaHeader(this->curChar);
        else {
            std::string value;
            value += this->curChar;
            this->exceptionHandler(value);
            // this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
        }

        // refresh end condition
        this->curChar = this->nextChar();
        this->endInput = this->curChar == -1;
    }

#ifdef lexer_debug_output
    if (this->sout == nullptr)
        (*fout) << this->curToken << std::endl;
    else if (this->sout != nullptr)
        (*sout) += this->curToken, (*sout) += '\n';
#endif

    if (this->curToken.type == TKTYPE::WRONGTK) {
        this->exceptionController->handle(CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken));
    }

    if (this->save) {
        this->tokenBuffer.push(this->curToken);
    }

    this->line = this->curToken.line;
    this->column = this->curToken.column + this->curToken.worldLength();

    return this->curToken;
}

Token Lexer::peekToken() noexcept
{
    if (this->load) {
        return this->tokenBuffer.front();
    }

    if (!this->peek && !this->endInput) {
        this->inner_line = this->inner_char_line;
        this->inner_column = this->inner_char_column;
        if (operatorString.find(this->curChar) != std::string::npos)
            this->curToken = this->signHeader(this->curChar);
        else if (isdigit(this->curChar))
            this->curToken = this->digitHeader(this->curChar);
        else if (isalpha(this->curChar) || this->curChar == '_')
            this->curToken = this->alphaHeader(this->curChar);
        // TODO: else exception
        else {
            std::string value;
            value += this->curChar;
            this->exceptionHandler(value);
            this->curToken = Token(TKTYPE::WRONGTK, this->inner_line, this->inner_column, value);
            // throw CompilerException(this->inner_line, this->inner_column, ErrorCode::UnknownToken);
        }
        this->curChar = this->nextChar();
        this->peek = true;
    }

    return this->curToken;
}

void Lexer::hold(std::string* sout)
{
    this->sout = sout;
    this->save = true;
    this->tokenLineKeeper = this->line;
    this->tokenColumnKeeper = this->column;
}

void Lexer::discharge(bool bingo)
{
    this->save = false;
    if (bingo) {
        while (!this->tokenBuffer.empty()) {
            this->tokenBuffer.pop();
        }
        (*fout) << *(this->sout);
    }
    this->sout->clear();
    this->sout = nullptr;
    this->load = !this->tokenBuffer.empty();
    if (this->load) {
        this->line = this->tokenLineKeeper;
        this->column = this->tokenColumnKeeper;
    }
    // exceptionController->discharge(bingo);
}
