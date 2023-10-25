#include "../include/token.h"
std::string TKTYPEOUTSTR[] = {
    "MAINTK",
    "IFTK",
    "ELSETK",
    "FORTK",
    "CONTINUETK",
    "RETURNTK",
    "INTTK",
    "CONSTTK",
    "VOIDTK",
    "BREAKTK",
    "PRINTFTK",
    "GETINTTK",
    "COMMA",
    "SEMICN",
    "LBRACK",
    "RBRACK",
    "LBRACE",
    "RBRACE",
    "ASSIGN",
    "LPARENT",
    "RPARENT",
    "PLUS",
    "MINU",
    "MULT",
    "DIV",
    "MOD",
    "NOT",
    "EQL",
    "NEQ",
    "GRE",
    "LSS",
    "LEQ",
    "GEQ",
    "AND",
    "OR",
    "INTCON",
    "STRCON",
    "IDENFR"
};

std::string TKTYPEORISTR[] = {
    "main", "if", "else", "for", "continue", "return", "int", "const", "void", "break", "printf", "getint", ",", ";", "[", "]", "{", "}", "=", "(", ")", "+", "-", "*", "/", "%", "!", "==", "!=", ">", "<", "<=", ">=", "&&", "||"
};

std::ostream& operator<<(std::ostream& fout, const TKTYPE& output)
{
    fout << TKTYPEOUTSTR[static_cast<int>(output)];
    return fout;
}

bool Token::operator==(const Token& another)const
{
    if (this->type == TKTYPE::EOFTK || another.type == TKTYPE::EOFTK)
    {
        return true;
    }
    return this->type == another.type;
}
bool Token::operator==(const TKTYPE& type)const
{
    if (this->type == TKTYPE::EOFTK || type == TKTYPE::EOFTK)
    {
        return true;
    }
    return this->type == type;
}
bool Token::operator!=(const TKTYPE& type)const
{
    if (this->type == TKTYPE::EOFTK || type == TKTYPE::EOFTK)
    {
        return false;
    }
    return this->type != type;
}
bool Token::operator!=(const Token& another)const
{
    if (this->type == TKTYPE::EOFTK || another.type == TKTYPE::EOFTK)
    {
        return true;
    }
    return this->type != another.type;
}

std::ostream& operator<<(std::ostream& fout, const Token& token)
{
    fout << token.type << ' ';
    if (token.value.empty() && token.type != TKTYPE::STRCON)
        fout << TKTYPEORISTR[static_cast<int>(token.type)];
    else {
        if (token.type == TKTYPE::STRCON) {
            fout << '\"' << token.value << '\"';
        } else {
            fout << token.value;
        }
    }
    return fout;
}

std::string& operator+=(std::string& sout, const Token& token)
{
    sout += TKTYPEOUTSTR[static_cast<int>(token.type)] + " ";
    if (token.value.empty())
        sout += TKTYPEORISTR[static_cast<int>(token.type)];
    else {
        if (token.type == TKTYPE::STRCON) {
            sout += '\"' + token.value + '\"';
        } else {
            sout += token.value;
        }
    }
    return sout;
}

Token::Token(TKTYPE type, int line, int column, std::string value)
    : type(type)
    , line(line)
    , column(column)
    , value(value)
{
}

unsigned int Token::worldLength() const
{
    return this->value.empty() ? TKTYPEORISTR[static_cast<int>(this->type)].length() : this->value.length();
}