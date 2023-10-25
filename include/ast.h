#ifndef _AST_H
#define _AST_H

#include "token.h"
#include <vector>

enum class ASTNODETYPE {
    CompUnit = static_cast<int>(TKTYPE::EOFTK) + 1,
    ConstDecl/*变量类型和初始值存在表里面，语法分析只记Ident*/,
    VarDecl/*比常数多记一个初始值*/,
    InitVal,
    FuncDef/*类型记录在表中,只记录Ident和Block*/,
    MainFuncDef,
    FuncParams/*类型在符号表，只记录Ident*/,
    Block,
    Stmt,
    Lval,
    Exp,

};

struct AstNode {
    int type; // unify ASTNODETYPE and TKTYPE
    std::string content;
    size_t line, column;
    std::vector<struct AstNode*> children;

    // structor
    AstNode(const Token&);
    AstNode(const ASTNODETYPE& type, size_t line, size_t column)
        : type(static_cast<int>(type))
        , line(line)
        , column(column)
    {
    }
    // operator
    bool operator==(const ASTNODETYPE&);
    bool operator==(const TKTYPE&);
    bool operator==(const Token&);
};
#endif