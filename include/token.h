#ifndef _TOKEN_H
#define _TOKEN_H
#include <ostream>

// #define debug_memory

enum class TKTYPE {
    // main
    MAINTK,
    // if
    IFTK,
    // else
    ELSETK,
    // for
    FORTK,
    // continue
    CONTINUETK,
    // return
    RETURNTK,
    // int
    INTTK,
    // const
    CONSTTK,
    // void
    VOIDTK,
    // break
    BREAKTK,
    // printf
    PRINTFTK,
    // getint
    GETINTTK,
    // ,
    COMMA,
    // ;
    SEMICN,
    /*[*/
    LBRACK,
    /*]*/
    RBRACK,
    /*{*/
    LBRACE,
    /*}*/
    RBRACE,
    // =
    ASSIGN,
    /*(*/
    LPARENT,
    /*)*/
    RPARENT,
    // \\+
    PLUS,
    // \-
    MINUS,
    /*\**/
    MULT,
    /*\\*/
    DIV,
    /*%*/
    MOD,
    /*!*/
    NOT,
    /*==*/
    EQL,
    /*!=*/
    NEQ,
    // &gt;
    GRE,
    // &lt;
    LSS,
    // &lt;=
    LEQ,
    // &gt;=
    GEQ,
    // &&
    AND,
    // ||
    OR,
    // const int
    INTCON,
    // const string
    STRCON,
    // identifier
    IDENFR,
    // add reserved word here
    WRONGTK,
};
std::ostream& operator<<(std::ostream&, const TKTYPE&);

struct Token {
    TKTYPE type;
    int line, column;
    std::string value;
    Token(TKTYPE, int = -1, int = -1, const std::string& = "");

    Token()
        : type()
        , line()
        , column()
        , value()
    {
        // std::cout<<"create Token"<<'\t'<<this<<'\t'<<this->type<<'\t'<<this->value<<std::endl;
    }
    friend std::ostream& operator<<(std::ostream&, const Token&);
    bool operator==(const Token&)const;
    bool operator==(const TKTYPE&)const;
    bool operator!=(const TKTYPE&)const;
    bool operator!=(const Token&)const;
    friend std::string& operator+=(std::string&, const Token&);
    unsigned int worldLength() const;
    std::string toString() const;
};

#endif