#include "../include/exception.h"

#define exception_debug_output

std::string ErrorCodeString[] = {
    "IllegalCharInFormatString",
    "Redefine",
    "Nodefine",
    "WrongParameterNumber",
    "WrongParameterType",
    "SuperfluousReturnValue",
    "MissingReturnStatement",
    "ModifyConst",
    "MissingSemicolon",
    "MissingCloseParentheses",
    "MissingCloseSquareBracket",
    "UnmatchedPrintArgs",
    "BreakOrContinueOutOfLoop",
    "UnknownToken",
    "OtherError",
    "MissingReturnValue",
    "CalculateTypeNotMatch",
};

std::ostream& operator<<(std::ostream& fout, const CompilerException& compilerException)
{
    if (&fout == &std::cout)
        fout << "\033[1;31m"
             << "in line\t" << compilerException.line << "\tcolumn\t" << compilerException.column << '\t' << ErrorCodeString[static_cast<int>(compilerException.type)] << "\033[0m";
    else
        fout << "in line\t" << compilerException.line << "\tcolumn\t" << compilerException.column << '\t' << ErrorCodeString[static_cast<int>(compilerException.type)];
    // if (&fout == &std::cout)
    //     fout << "\033[1;31m"
    //          << compilerException.line << ' ' << (char)('a' + static_cast<int>(compilerException.type)) << "\033[0m";
    // else
    //     fout << compilerException.line << ' ' << (char)('a' + static_cast<int>(compilerException.type));
    return fout;
}
std::string CompilerException::toString() const
{
    return std::to_string(line) + ' ' + (char)('a' + static_cast<int>(this->type)) + '\n';
}

void ExceptionController::handle(const CompilerException& e)
{
#ifdef exception_debug_output
    if (this->save)
        (this->sout) += e.toString();
    else
        (*this->fout) << e << std::endl;
#endif
}

void ExceptionController::hold()
{
#ifdef exception_debug_output
    this->save = true;
#endif
}

void ExceptionController::discharge(bool bingo)
{
#ifdef exception_debug_output
    if (bingo) {
        (*this->fout) << sout;
    }
    this->save = false;
    sout.clear();
#endif
}