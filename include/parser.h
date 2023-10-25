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
    void Exp(bool, MyType&, int&);
    void FuncDef(const Token&, const Token&);
    std::vector<MyType> FuncRParams();
    void AddExp(bool, MyType&, int&);
    void MulExp(bool, MyType&, int&);
    void UnaryExp(bool, MyType&, int&);
    void PrimaryExp(const Token&, bool, MyType&, int&);
    void LVal(const Token&, bool, MyType&, int&);
    void ConstExp(MyType&, int&);
    void Block();
    void Stmt(BlockType = BlockType::Normal);
    void RelExp();
    void EqExp();
    void LAndExp();
    void LOrExp();
};

#endif