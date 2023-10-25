#ifndef _PARSER_H
#define _PARSER_H

#include "ast.h"
#include "lexer.h"
#include "table.h"

class Parser {
private:
    Lexer *lexer;
    std::ostream *fout;
    std::string *sout;
    std::string outBuffer;
    TableManager* tableManager;
    ExceptionController *exceptionController;
public:
    Parser(Lexer *lexer, std::ostream *fout, TableManager* tableManager, ExceptionController *exceptionController)
        : lexer(lexer)
        , fout(fout)
        , sout(nullptr)
        , tableManager(tableManager)
        , exceptionController(exceptionController)
    {
    }
    void CompUnit();
    void ConstDecl();
    void VarDecl(const Token&);
    std::vector<int> InitVal(bool = false);
    MyType Exp(bool, int&);
    void FuncDef(const Token&, const Token&);
    std::vector<MyType> FuncRParams();
    MyType AddExp(bool, int&);
    MyType MulExp(bool, int&);
    MyType UnaryExp(bool, int&);
    MyType PrimaryExp(const Token&, bool, int&);
    MyType LVal(const Token&, bool, int&);
    void Block();
    void Stmt(BlockType = BlockType::Normal);
    void RelExp();
    void EqExp();
    void LAndExp();
    void LOrExp();
    MyType ConstExp(int&);
};

#endif