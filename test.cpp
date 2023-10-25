#include "./include/lexer.h"
#include "include/parser.h"
#include <cstdlib>
#include <fstream>

int main()
{
    std::filebuf foutbuf, errorbuf;
    errorbuf.open("error.txt", std::ios::out);
    foutbuf.open("output.txt", std::ios::out);
    std::ostream fout(&foutbuf);
    std::ostream eout(&errorbuf);
    // std::ostream& fout = (std::cout);
    ExceptionController* exceptionController = new ExceptionController(&eout);
    Lexer* lexer = new Lexer(&fout, exceptionController, "testfile.txt");
    TableManager* tableManager = new TableManager();
    Parser parser = *new Parser(lexer, &fout, tableManager, exceptionController);
    parser.CompUnit();
    return 0;
}