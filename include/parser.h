#ifndef _PARSER_H
#define _PARSER_H

#include "lexer.h"
// #include "./signTable/table.h"
#include "./ir_gen/module.h"

class Parser {
private:
    Lexer *lexer;
    std::ostream *fout;
    std::string *sout;
    std::string outBuffer;
    // TableManager* tableManager;
    ExceptionController *exceptionController;
    Module* ir_module;
    
public:
    Parser(Lexer *lexer, std::ostream *fout/*, TableManager* tableManager*/, Module* ir_module, ExceptionController *exceptionController)
        : lexer(lexer)
        , fout(fout)
        , sout(nullptr)
        // , tableManager(tableManager)
        , ir_module(ir_module)
        , exceptionController(exceptionController)
    {
    }
    void CompUnit();
    void ConstDecl();
    void VarDecl(const Token&);
    std::vector<VarInf> InitVal(bool = false);
    void Exp(bool, VarInf&);
    void FuncDef(const Token&, const Token&);
    std::vector<VarInf> FuncRParams();
    void AddExp(bool, VarInf&);
    void MulExp(bool, VarInf&);
    void UnaryExp(bool, VarInf&);
    void PrimaryExp(const Token&, bool, VarInf&);
    void LVal(const Token&, bool, bool, VarInf&);
    void ConstExp(VarInf&);
    void Block();
    void Stmt(BasicBlock::Type, const bool, const bool, const std::string& = "");
    void Stmt(BasicBlock::Type block_type = BasicBlock::ClosureBlock, const std::string& block_name = ""){this->Stmt(block_type, false, false, block_name);}
    void RelExp(VarInf&);
    void EqExp(VarInf&);
    void LAndExp(VarInf&, const std::string&, BasicBlock::Type, const std::string&, BasicBlock::Type);
    void LOrExp(VarInf&, const std::string&, BasicBlock::Type, const std::string&, BasicBlock::Type);
};

#endif