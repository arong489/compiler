#include "../include/exception.h"
std::ostream& operator<<(std::ostream& fout, const CompilerException& compilerException)
{
    // if (&fout == &std::cout)
    //     fout << "\033[1;31m"
    //          << "in line\t" << compilerException.line << "\tcolumn\t" << compilerException.column << '\t' << 'a' + static_cast<int>(compilerException.type) << "\033[0m";
    // else
    //     fout << "in line\t" << compilerException.line << "\tcolumn\t" << compilerException.column << '\t' << 'a' + static_cast<int>(compilerException.type);
    if (&fout == &std::cout)
        fout << "\033[1;31m"
             << compilerException.line << ' ' << (char)('a' + static_cast<int>(compilerException.type)) << "\033[0m";
    else
        fout << compilerException.line << ' ' << (char)('a' + static_cast<int>(compilerException.type));
    return fout;
}
std::string CompilerException::toString() const
{
    return std::to_string(line) + ' ' + (char)('a' + static_cast<int>(this->type)) + '\n';
}

void ExceptionController::handle(const CompilerException& e)
{
    (*this->fout) << e << std::endl;
}

void ExceptionController::hold(const CompilerException& e)
{
    sout += e.toString();
}

void ExceptionController::discharge(bool check)
{
    if (check) {
        (*this->fout) << sout;
    }
    sout.clear();
}