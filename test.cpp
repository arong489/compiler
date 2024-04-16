#include "./include/lexer.h"
#include "include/parser.h"
#include <cstdlib>
#include <fstream>
#include "include/ir_gen/module.h"

int main()
{
    std::filebuf foutbuf, errorbuf, llvmbuf;
    errorbuf.open("error.txt", std::ios::out);
    foutbuf.open("output.txt", std::ios::out);
    llvmbuf.open("llvm_ir.txt", std::ios::out);
    std::ostream iout(&llvmbuf);
    std::ostream fout(&foutbuf);
    std::ostream eout(&errorbuf);
    // std::ostream& fout = (std::cout);
    ExceptionController* exceptionController = new ExceptionController(&eout);
    Lexer* lexer = new Lexer(&fout, exceptionController, "testfile.txt");
    // TableManager* tableManager = new TableManager();
    Module* ir_module = new Module(&iout);
    Parser parser = *new Parser(lexer, &fout, ir_module, exceptionController);
    parser.CompUnit();
    if (exceptionController->correct)
        ir_module->print();
    return 0;
}