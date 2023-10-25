#ifndef _EXCEPTION
#define _EXCEPTION
#include <iostream>

enum class ErrorCode {
    IllegalCharInFormatString,
    Redefine,
    Nodefine,
    WrongParameterNumber,
    WrongParameterType,
    SuperfluousReturnValue,
    MissingReturnStatement,
    ModifyConst,
    MissingSemicolon,
    MissingCloseParentheses,
    MissingCloseSquareBracket,
    UnmatchedPrintArgs,
    BreakOrContinueOutOfLoop,
    UnknownToken,
    OtherError,
    MissingReturnValue,
    CalculateTypeNotMatch,
};

class CompilerException : std::exception {
private:
    int line, column;
    ErrorCode type;

public:
    CompilerException(int line, int column, const ErrorCode& ecode)
        : line(line)
        , column(column)
        , type(ecode)
    {
    }
    friend std::ostream& operator<<(std::ostream& fout, const CompilerException& compilerException);
    std::string toString() const;
};

class ExceptionController {
private:
    std::ostream* fout;
    std::string sout;
    bool save;

public:
    ExceptionController(std::ostream* fout)
        : fout(fout)
        , sout()
        , save(false)
    {
    }
    void handle(const CompilerException& e);

    void hold();

    void discharge(bool bingo = false);
};

#endif