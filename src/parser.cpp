#include "../include/parser.h"
#include "../include/exception.h"

#define parser_debug_output

#define _marco_string(x) #x
#define parser_output(content) ({            \
    if (this->sout == nullptr)               \
        (*fout) << #content << std::endl;    \
    else                                     \
        (*sout) += _marco_string(content\n); \
})

bool checkFormatString(const Token& FormatString, unsigned int& paramNum)
{
    const std::string& str = FormatString.value;
    auto i = str.begin();
    paramNum = 0;
    bool ans = true;
    while (i != str.end()) {
        if (*i == '%') {
            i++;
            if (*i != 'd') {
                ans = false;
            } else {
                paramNum++;
            }
            if (i == str.end()) {
                ans = false;
                break;
            }
        } else if (*i == '\\') {
            i++;
            if (*i != 'n') {
                ans = false;
            }
            if (i == str.end()) {
                ans = false;
                break;
            }
        } else if (*i != ' ' && *i != '!' && !(*i >= 40 && *i <= 126)) {
            ans = false;
        }
        i++;
    }
    return ans;
}

/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||                                                                                |||||||||||||||
||||||||||||                                  Parser                                        |||||||||||||||
||||||||||||                                                                                |||||||||||||||
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

void Parser::CompUnit()
{
    Token type, ident;
    while (!this->lexer->endInput) {
        type = this->lexer->nextToken();

        if (type == TKTYPE::CONSTTK) {
            this->ConstDecl();
        } else {
            // held output buffer
            this->lexer->hold(&this->outBuffer);
            this->exceptionController->hold();

#ifdef parser_debug_output
            if (this->lexer->peekToken() != TKTYPE::MAINTK) {
                this->outBuffer += "<FuncType>\n";
            }
#endif

            ident = this->lexer->nextToken();
            if (this->lexer->peekToken() == TKTYPE::LPARENT) {

                this->lexer->discharge(true);
                this->exceptionController->discharge(true);

                this->FuncDef(type, ident);
            } else {

                this->lexer->discharge();
                this->exceptionController->discharge();

                this->VarDecl(type);
            }
        }
    }
#ifdef parser_debug_output
    parser_output(<CompUnit>);
#endif
}

void Parser::ConstDecl()
{
    //*int
    if (this->lexer->nextToken() != TKTYPE::INTTK) {
        //[wrong ConstDecl]unknown const type
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
    }

    std::vector<int> degrees, initialValues;
    Token ident;
    MyType type = MyType(true);
    MyType resultType;
    int resultValue;

    do {
        //* ident
        if (this->lexer->peekToken() != TKTYPE::IDENFR) {
            //[wrong ConstDecl]wrong identifier
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        } else
            ident = this->lexer->nextToken();

        //* {[<exp>]}
        degrees.clear();
        while (this->lexer->peekToken() == TKTYPE::LBRACK) {
            this->lexer->nextToken();

            this->ConstExp(resultType, resultValue);

            degrees.push_back(resultValue);

            if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                //[wrong ConstDecl]wrong identifier
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
            } else
                this->lexer->nextToken();
        }
        type.degrees = degrees;

        //* = <initialValue> (insert constant here)
        if (this->lexer->peekToken() == TKTYPE::ASSIGN) {
            this->lexer->nextToken();

            initialValues = this->InitVal(true);

        } else {
            //[wrong ConstDecl]need initialize
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        }

        if (!this->tableManager->insertVariable(type, ident.value, initialValues)) {
            // parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
            this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::Redefine));
        }

#ifdef parser_debug_output
        parser_output(<ConstDef>);
#endif

        //* ,
        if (this->lexer->peekToken() == TKTYPE::COMMA)
            this->lexer->nextToken();
        else
            break;
    } while (true);

    if (this->lexer->peekToken() != TKTYPE::SEMICN) {
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
    } else {
        this->lexer->nextToken();
    }

#ifdef parser_debug_output
    parser_output(<ConstDecl>);
#endif
}

void Parser::VarDecl(const Token& type)
{
    Token ident;
    std::vector<int> degrees;
    int resultValue;
    MyType VarType, resultType;
    do {
        ident = this->lexer->nextToken();
        if (ident != TKTYPE::IDENFR) {
            //[Wrong VarDecl]need ident"
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        }

        degrees.clear();
        while (this->lexer->peekToken() == TKTYPE::LBRACK) {
            this->lexer->nextToken();

            this->ConstExp(resultType, resultValue);

            degrees.push_back(resultValue);

            if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
            } else {
                this->lexer->nextToken();
            }
        }
        VarType.degrees = degrees;
        if (this->lexer->peekToken() == TKTYPE::ASSIGN) {
            this->lexer->nextToken();
            this->InitVal();
        }

        if (!this->tableManager->insertVariable(VarType, ident.value)) {
            // parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
            this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::Redefine));
        }

#ifdef parser_debug_output
        parser_output(<VarDef>);
#endif

        if (this->lexer->peekToken() == TKTYPE::COMMA)
            this->lexer->nextToken();
        else
            break;
    } while (true);

    if (this->lexer->peekToken() != TKTYPE::SEMICN) {
        //[Wrong VarDecl]need ;
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
    } else
        this->lexer->nextToken();

#ifdef parser_debug_output
    parser_output(<VarDecl>);
#endif
}

std::vector<int> Parser::InitVal(bool constCheck)
{
    MyType resultType;
    std::vector<int> initialValues;
    if (this->lexer->peekToken() == TKTYPE::LBRACE) {
        this->lexer->nextToken();
        if (this->lexer->peekToken() != TKTYPE::RBRACE) {
            do {
                std::vector<int>&& addInitial = this->InitVal(constCheck);

                if (constCheck) {
                    initialValues.insert(initialValues.end(), addInitial.begin(), addInitial.end());
                }

                if (this->lexer->peekToken() == TKTYPE::COMMA)
                    this->lexer->nextToken();
                else
                    break;
            } while (true);
            if (this->lexer->peekToken() != TKTYPE::RBRACE)
                // [Wrong InitVal]need }
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            else
                this->lexer->nextToken();
        } else
            this->lexer->nextToken();

    } else {
        int result;
        if (constCheck)
            this->ConstExp(resultType, result);
        else
            this->Exp(constCheck, resultType, result);

        if (constCheck) {
            initialValues.push_back(result);
        }
    }
#ifdef parser_debug_output
    if (constCheck) {
        parser_output(<ConstInitVal>);
    } else {
        parser_output(<InitVal>);
    }
#endif
    return initialValues;
}

void Parser::ConstExp(MyType& returnType, int& returnValue)
{
    this->AddExp(true, returnType, returnValue);

#ifdef parser_debug_output
    parser_output(<ConstExp>);
#endif
}

void Parser::Exp(bool constCheck, MyType& returnType, int& returnValue)
{
    this->AddExp(constCheck, returnType, returnValue);
#ifdef parser_debug_output
    parser_output(<Exp>);
#endif
}
void Parser::AddExp(bool constCheck, MyType& returnType, int& returnValue)
{
    this->MulExp(constCheck, returnType, returnValue);

#ifdef parser_debug_output
    parser_output(<AddExp>);
#endif

    int result;
    Token tempToken;
    MyType tempType;
    while (this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS) {
        tempToken = this->lexer->nextToken();

        this->MulExp(constCheck, tempType, result);

        if (constCheck) {
            returnValue += tempToken == TKTYPE::PLUS ? +result : -result;
        }

        try {
            returnType = returnType.calculate(tempToken, tempType);
        } catch (const ErrorCode& e) {
            // parser_exception_handler(tempToken.line, tempToken.column, e);
            returnType = MyType(VarType::WrongType);
            this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, e));
        }

#ifdef parser_debug_output
        parser_output(<AddExp>);
#endif
    }
}

void Parser::MulExp(bool constCheck, MyType& returnType, int& returnValue)
{
    this->UnaryExp(constCheck, returnType, returnValue);
#ifdef parser_debug_output
    parser_output(<MulExp>);
#endif
    Token tempToken;
    MyType tempType;
    int tempInt;
    while (this->lexer->peekToken() == TKTYPE::MULT || this->lexer->peekToken() == TKTYPE::DIV || this->lexer->peekToken() == TKTYPE::MOD) {
        tempToken = this->lexer->nextToken();

        this->UnaryExp(constCheck, tempType, tempInt);

        if (constCheck) {
            if (tempToken == TKTYPE::MULT) {
                returnValue *= tempInt;
            } else {
                if (!tempInt) {
                    // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::OtherError);
                    this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::OtherError));
                } else {
                    returnValue = tempToken == TKTYPE::DIV ? returnValue / tempInt : returnValue % tempInt;
                }
            }
        }

        try {
            returnType = returnType.calculate(tempToken, tempType);
        } catch (const ErrorCode& e) {
            // parser_exception_handler(tempToken.line, tempToken.column, e);
            returnType = MyType(VarType::WrongType);
            this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, e));
        }

#ifdef parser_debug_output
        parser_output(<MulExp>);
#endif
    }
}

void Parser::UnaryExp(bool constCheck, MyType& returnType, int& returnValue)
{
    Token tempToken = this->lexer->nextToken();
    int check;
    switch (tempToken.type) {
    case TKTYPE::PLUS:
    case TKTYPE::MINUS:
    case TKTYPE::NOT:
#ifdef parser_debug_output
        parser_output(<UnaryOp>);
#endif
        this->UnaryExp(constCheck, returnType, returnValue);
        if (constCheck)
            returnValue = tempToken == TKTYPE::MINUS ? -returnValue : tempToken == TKTYPE::NOT ? !returnValue
                                                                                               : returnValue;
        try {
            returnType = returnType.calculate(tempToken);
        } catch (const ErrorCode& e) {
            returnType = MyType(VarType::WrongType);
            this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, e));
        }
        break;
    case TKTYPE::LPARENT:
    case TKTYPE::INTCON:
        this->PrimaryExp(tempToken, constCheck, returnType, returnValue);
        break;
    case TKTYPE::IDENFR:
        if (this->lexer->peekToken() == TKTYPE::LPARENT) {
            if (this->tableManager->getFunctionType(tempToken.value, returnType)) {
                // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::Nodefine);
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::Nodefine));
            }
            this->lexer->nextToken();
            if (this->lexer->peekToken() == TKTYPE::IDENFR || this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS || this->lexer->peekToken() == TKTYPE::NOT || this->lexer->peekToken() == TKTYPE::LPARENT || this->lexer->peekToken() == TKTYPE::INTCON) {
                // std::vector<MyType&>& parameters = this->FuncRParams();

                if (check = this->tableManager->checkFunctionParameters(tempToken.value, this->FuncRParams())) {
                    if (check == 1)
                        // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::WrongParameterNumber);
                        this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::WrongParameterNumber));
                    else if (check == 2)
                        // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::WrongParameterType);
                        this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::WrongParameterType));
                    else
                        // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::OtherError);
                        this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::OtherError));
                }
            }

            if (this->lexer->peekToken() != TKTYPE::RPARENT)
                //[wrong UnaryExp]need )
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
            else
                this->lexer->nextToken();
            returnValue = 0;
        } else {
            this->PrimaryExp(tempToken, constCheck, returnType, returnValue);
        }
        break;
    default:
        //[wrong UnaryExp]
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        break;
    }
#ifdef parser_debug_output
    parser_output(<UnaryExp>);
#endif
}

void Parser::PrimaryExp(const Token& head, bool constCheck, MyType& returnType, int& returnValue)
{
    switch (head.type) {
    case TKTYPE::LPARENT:
        this->Exp(constCheck, returnType, returnValue);
        if (this->lexer->peekToken() != TKTYPE::RPARENT)
            //[wrong PrimaryExp]need )
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        else
            this->lexer->nextToken();
        break;
    case TKTYPE::IDENFR:
        this->LVal(head, constCheck, returnType, returnValue);
        break;
    case TKTYPE::INTCON:
        if (constCheck)
            returnValue = std::stoi(head.value);
#ifdef parser_debug_output
        parser_output(<Number>);
#endif
        // TODO: here
        returnType = MyType(true);
        break;
    default:
        // [design error]
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        break;
    }
#ifdef parser_debug_output
    parser_output(<PrimaryExp>);
#endif
}

std::vector<MyType> Parser::FuncRParams()
{
    int resultValue;
    std::vector<MyType> ans;
    MyType resultType;
    this->Exp(false, resultType, resultValue);
    ans.push_back(resultType);
    while (this->lexer->peekToken() == TKTYPE::COMMA) {
        this->lexer->nextToken();
        this->Exp(false, resultType, resultValue);
        ans.push_back(resultType);
    }
#ifdef parser_debug_output
    parser_output(<FuncRParams>);
#endif
    return ans;
}

void Parser::FuncDef(const Token& type, const Token& ident)
{
    if (this->lexer->peekToken() != TKTYPE::LPARENT) {
        //[wrong FuncDef]need (
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
    } else
        this->lexer->nextToken();

    if (!this->tableManager->insertFunction(MyType(type == TKTYPE::INTTK ? VarType::IntType : VarType::VoidType), ident.value)) {
        // parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
        this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::Redefine));
    }
    this->tableManager->setNextBlock(type == TKTYPE::INTTK ? BlockType::IntFunction : BlockType::VoidFunction);

    //*<FuncFParams>
    MyType tempType;
    Token tempToken;
    std::vector<int> degrees;
    unsigned int tempPointerDepth;
    int resultValue;
    if (this->lexer->peekToken() == TKTYPE::INTTK) {
        while (this->lexer->peekToken() == TKTYPE::INTTK) {
            this->lexer->nextToken();
            if ((tempToken = this->lexer->nextToken()) != TKTYPE::IDENFR) {
                //[wrong FuncDef]need identifier"
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }
            tempPointerDepth = 0;
            if (this->lexer->peekToken() == TKTYPE::LBRACK) {
                this->lexer->nextToken();
                tempPointerDepth++;
                if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                    //[wrong FuncDef]need ]
                    // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
                } else
                    this->lexer->nextToken();
            }

            degrees.clear();
            while (this->lexer->peekToken() == TKTYPE::LBRACK) {
                this->lexer->nextToken();

                this->ConstExp(tempType, resultValue);

                degrees.push_back(resultValue);
                if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                    // [wrong FuncDef]need ]
                    // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
                } else
                    this->lexer->nextToken();
            }

            tempType = MyType(VarType::IntType, tempPointerDepth, degrees);

            if (!this->tableManager->insertVariable(tempType, tempToken.value, true)) {
                // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::Redefine);
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::Redefine));
            }

#ifdef parser_debug_output
            parser_output(<FuncFParam>);
#endif
            if (this->lexer->peekToken() == TKTYPE::COMMA) {
                this->lexer->nextToken();
            }
        }
#ifdef parser_debug_output
        parser_output(<FuncFParams>);
#endif
    }
    if (this->lexer->peekToken() != TKTYPE::RPARENT) {
        //[wrong FuncDef]need )
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
    } else {
        this->lexer->nextToken();
        // this->tableManager->fixUpperFunctionParameters();
    }

    this->Block();
    if (!this->tableManager->setUpperBlock()) {
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingReturnStatement);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingReturnStatement));
    }

#ifdef parser_debug_output
    if (ident != TKTYPE::MAINTK) {
        parser_output(<FuncDef>);
    } else {
        parser_output(<MainFuncDef>);
    }

#endif
}

void Parser::Block()
{
    if (this->lexer->nextToken() != TKTYPE::LBRACE) {
        // [wrong Block]need {
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
    }

    //*BlockItem
    while (this->lexer->peekToken() != TKTYPE::RBRACE)
        switch (this->lexer->peekToken().type) {
        case TKTYPE::CONSTTK:
            this->lexer->nextToken();
            this->ConstDecl();
            break;
        case TKTYPE::INTTK:
            this->VarDecl(this->lexer->nextToken());
            break;
        default:
            this->Stmt();
            break;
        }
    if (this->lexer->nextToken() != TKTYPE::RBRACE) {
        //[wrong Block]need }
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
    }
#ifdef parser_debug_output
    parser_output(<Block>);
#endif
}

void Parser::Stmt(BlockType blockType)
{
    MyType returnType(VarType::VoidType);
    if (this->lexer->peekToken() == TKTYPE::IFTK) {
        this->lexer->nextToken();
        this->lexer->nextToken(); //(
        this->LOrExp();
#ifdef parser_debug_output
        parser_output(<Cond>);
#endif
        if (this->lexer->peekToken() != TKTYPE::RPARENT)
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        else
            this->lexer->nextToken(); //)

        this->Stmt(BlockType::Normal);

        if (this->lexer->peekToken() == TKTYPE::ELSETK) {
            this->lexer->nextToken();

            this->Stmt(BlockType::Normal);
        }
    } else if (this->lexer->peekToken() == TKTYPE::FORTK) {
        this->lexer->nextToken();
        this->lexer->nextToken(); // (
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            int result;
            Token ident = this->lexer->nextToken();
            this->LVal(ident, false, returnType, result);
            if (returnType.isConst) {
                // parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
                this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::ModifyConst));
            }
            if (this->lexer->nextToken() != TKTYPE::ASSIGN) {
                // [wrong Stmt]need =
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }
            this->Exp(false, returnType, result);
#ifdef parser_debug_output
            parser_output(<ForStmt>);
#endif
        }
        if (this->lexer->nextToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        }
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            this->LOrExp();
#ifdef parser_debug_output
            parser_output(<Cond>);
#endif
        }

        if (this->lexer->nextToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        }

        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            int result;
            Token ident = this->lexer->nextToken();
            this->LVal(ident, false, returnType, result);
            if (returnType.isConst) {
                // parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
                this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::ModifyConst));
            }
            if (this->lexer->nextToken() != TKTYPE::ASSIGN) {
                // [wrong Stmt]need =
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }
            this->Exp(false, returnType, result);
#ifdef parser_debug_output
            parser_output(<ForStmt>);
#endif
        }
        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            //[wrong Stmt]need )
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        } else {
            this->lexer->nextToken();
        }

        this->Stmt(BlockType::Loop);
    } else if (this->lexer->peekToken() == TKTYPE::BREAKTK
        || this->lexer->peekToken() == TKTYPE::CONTINUETK) {
        Token&& cbToken = this->lexer->nextToken();
        if (blockType != BlockType::Loop && !this->tableManager->inLoop()) {
            // parser_exception_handler(cbToken.line, cbToken.column, ErrorCode::BreakOrContinueOutOfLoop);
            this->exceptionController->handle(CompilerException(cbToken.line, cbToken.column, ErrorCode::BreakOrContinueOutOfLoop));
        }

        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        } else
            this->lexer->nextToken();
    } else if (this->lexer->peekToken() == TKTYPE::RETURNTK) {
        Token&& returnToken = this->lexer->nextToken();
        this->tableManager->setReturnStatement();
        int result;
        if (this->lexer->peekToken() == TKTYPE::IDENFR || this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS || this->lexer->peekToken() == TKTYPE::NOT || this->lexer->peekToken() == TKTYPE::LPARENT || this->lexer->peekToken() == TKTYPE::INTCON) {
            this->Exp(false, returnType, result);
            if (!this->tableManager->inIntFunction()) {
                // parser_exception_handler(returnToken.line, returnToken.column, ErrorCode::SuperfluousReturnValue);
                this->exceptionController->handle(CompilerException(returnToken.line, returnToken.column, ErrorCode::SuperfluousReturnValue));
            }
        } else {
            if (!this->tableManager->inVoidFunction()) {
                // parser_exception_handler(returnToken.line, returnToken.column, ErrorCode::MissingReturnValue);
                this->exceptionController->handle(CompilerException(returnToken.line, returnToken.column, ErrorCode::MissingReturnValue));
            }
        }
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        } else {
            this->lexer->nextToken();
        }
    } else if (this->lexer->peekToken() == TKTYPE::PRINTFTK) {
        Token&& printfToken = this->lexer->nextToken();
        Token tempToken;
        int result;
        if (this->lexer->nextToken() != TKTYPE::LPARENT) {
            // [wrong Stmt]need (");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        }
        // TODO:check printf here
        unsigned int formatParamNum = 0;
        if ((tempToken = this->lexer->peekToken()) != TKTYPE::STRCON) {
            // [wrong Stmt]need format string");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        } else {
            if (!checkFormatString(tempToken, formatParamNum)) {
                // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::IllegalCharInFormatString);
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::IllegalCharInFormatString));
            }
            this->lexer->nextToken();
        }
        unsigned int paramNum = 0;
        while (this->lexer->peekToken() == TKTYPE::COMMA) {
            this->lexer->nextToken();
            paramNum++;
            this->Exp(false, returnType, result);
        }
        if (paramNum != formatParamNum) {
            // parser_exception_handler(printfToken.line, printfToken.column, ErrorCode::UnmatchedPrintArgs);
            this->exceptionController->handle(CompilerException(printfToken.line, printfToken.column, ErrorCode::UnmatchedPrintArgs));
        }

        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            // [wrong Stmt]need )");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        } else
            this->lexer->nextToken();

        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        } else
            this->lexer->nextToken();

    } else if (this->lexer->peekToken() == TKTYPE::SEMICN) {
        this->lexer->nextToken();
    } else if (this->lexer->peekToken() == TKTYPE::IDENFR) {

        this->sout = &this->outBuffer;
        this->lexer->hold(&this->outBuffer);
        this->exceptionController->hold();
        int result;
        Token ident = this->lexer->nextToken();
        this->LVal(ident, false, returnType, result);
        if (returnType.isConst) {
            // parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
            this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::ModifyConst));
        }
        if (this->lexer->peekToken() == TKTYPE::ASSIGN) {

            this->sout = nullptr;
            // guess bingo
            this->lexer->discharge(true);
            this->exceptionController->discharge(true);
            this->lexer->nextToken();

            if (this->lexer->peekToken() == TKTYPE::GETINTTK) {
                this->lexer->nextToken();
                if (this->lexer->peekToken() != TKTYPE::LPARENT)
                    // [wrong FuncDef]need (");
                    // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
                else
                    this->lexer->nextToken();
                if (this->lexer->peekToken() != TKTYPE::RPARENT)
                    // [wrong FuncDef]need )");
                    // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
                else
                    this->lexer->nextToken();
                if (this->lexer->peekToken() != TKTYPE::SEMICN)
                    // [wrong FuncDef]need ;");
                    // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
                else
                    this->lexer->nextToken();
            } else {
                this->Exp(false, returnType, result);
                if (this->lexer->peekToken() != TKTYPE::SEMICN)
                    // [wrong FuncDef]need ;");
                    // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
                else
                    this->lexer->nextToken();
            }
        } else {

            this->lexer->discharge();
            this->exceptionController->discharge();
            // guess wrong
            this->sout = nullptr;

            int result;
            this->Exp(false, returnType, result);

            if (this->lexer->peekToken() != TKTYPE::SEMICN)
                // [wrong FuncDef]need ;");
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
            else
                this->lexer->nextToken();
        }
    } else if (this->lexer->peekToken() == TKTYPE::LBRACE) {
        this->tableManager->setNextBlock(blockType);
        this->Block();
        if (!this->tableManager->setUpperBlock()) {
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingReturnStatement);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingReturnStatement));
        }
    } else {
        int result;
        this->Exp(false, returnType, result);
        if (this->lexer->peekToken() != TKTYPE::SEMICN)
            // [wrong FuncDef]need ;");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        else
            this->lexer->nextToken();
    }
#ifdef parser_debug_output
    parser_output(<Stmt>);
#endif
}

void Parser::LVal(const Token& ident, bool constCheck, MyType& returnType, int& returnValue)
{
    int result;
    std::vector<int> indexes;
    Token tempToken;
    MyType tempType;
    if (this->tableManager->getVariableType(ident.value, returnType)) {
        // parser_exception_handler(ident.line, ident.column, ErrorCode::Nodefine);
        this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::Nodefine));
    }
    while (this->lexer->peekToken() == TKTYPE::LBRACK) {
        tempToken = this->lexer->nextToken();

        this->Exp(constCheck, tempType, result);

        try {
            returnType = returnType.calculate(tempToken, tempType);
        } catch (const ErrorCode& e) {
            returnType = MyType(VarType::WrongType);
            this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, e));
        }
        
        if (constCheck)
            indexes.push_back(result);

        if (this->lexer->peekToken() != TKTYPE::RBRACK)
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
        else
            this->lexer->nextToken(); // ]
    }
    if (constCheck) {
        this->tableManager->getVariableValue(ident.value, returnValue, indexes);
    }
#ifdef parser_debug_output
    parser_output(<LVal>);
#endif
}

void Parser::RelExp()
{
    int returnValue;
    MyType returnType;
    this->AddExp(false, returnType, returnValue);
#ifdef parser_debug_output
    parser_output(<RelExp>);
#endif
    while (this->lexer->peekToken() == TKTYPE::LSS
        || this->lexer->peekToken() == TKTYPE::GRE
        || this->lexer->peekToken() == TKTYPE::LEQ
        || this->lexer->peekToken() == TKTYPE::GEQ) {
        this->lexer->nextToken();
        this->AddExp(false, returnType, returnValue);
#ifdef parser_debug_output
        parser_output(<RelExp>);
#endif
    }
}

void Parser::EqExp()
{
    this->RelExp();
#ifdef parser_debug_output
    parser_output(<EqExp>);
#endif
    while (this->lexer->peekToken() == TKTYPE::EQL
        || this->lexer->peekToken() == TKTYPE::NEQ) {
        this->lexer->nextToken();
        this->RelExp();
#ifdef parser_debug_output
        parser_output(<EqExp>);
#endif
    }
}

void Parser::LAndExp()
{
    this->EqExp();
#ifdef parser_debug_output
    parser_output(<LAndExp>);
#endif
    while (this->lexer->peekToken() == TKTYPE::AND) {
        this->lexer->nextToken();
        this->EqExp();
#ifdef parser_debug_output
        parser_output(<LAndExp>);
#endif
    }
}

void Parser::LOrExp()
{
    this->LAndExp();
#ifdef parser_debug_output
    parser_output(<LOrExp>);
#endif
    while (this->lexer->peekToken() == TKTYPE::OR) {
        this->lexer->nextToken();
        this->LAndExp();
#ifdef parser_debug_output
        parser_output(<LOrExp>);
#endif
    }
}
