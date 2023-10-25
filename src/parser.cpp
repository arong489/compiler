#include "../include/parser.h"
#include "../include/exception.h"

// #define parser_debug_output

#define _marco_string(x) #x
#define parser_output(content) ({            \
    if (this->sout == nullptr)               \
        (*fout) << #content << std::endl;    \
    else                                     \
        (*sout) += _marco_string(content\n); \
})

#define parser_exception_handler(line, column, errorCode) ({                           \
    if (this->sout == nullptr)                                                         \
        this->exceptionController->handle(CompilerException(line, column, errorCode)); \
    else                                                                               \
        this->exceptionController->hold(CompilerException(line, column, errorCode));   \
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
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
    }

    std::vector<int> degrees, initialValues;
    Token ident;
    MyType type = MyType(true);
    int result;

    do {
        //* ident
        if (this->lexer->peekToken() != TKTYPE::IDENFR) {
            //[wrong ConstDecl]wrong identifier
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        } else
            ident = this->lexer->nextToken();

        //* {[<exp>]}
        degrees.clear();
        while (this->lexer->peekToken() == TKTYPE::LBRACK) {
            this->lexer->nextToken();

            this->ConstExp(result);

            degrees.push_back(result);

            if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                //[wrong ConstDecl]wrong identifier
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
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
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        }

        if (!this->tableManager->insertVariable(type, ident.value, initialValues)) {
            parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
        }

#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<ConstDef>" << std::endl;
        else
            (*sout) += "<ConstDef>\n";
#endif

        //* ,
        if (this->lexer->peekToken() == TKTYPE::COMMA)
            this->lexer->nextToken();
        else
            break;
    } while (true);

    if (this->lexer->peekToken() != TKTYPE::SEMICN) {
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
    } else {
        this->lexer->nextToken();
    }

#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<ConstDecl>" << std::endl;
    else
        (*sout) += "<ConstDecl>\n";
#endif
}

void Parser::VarDecl(const Token& type)
{
    Token ident;
    std::vector<int> degrees;
    int result;
    MyType VarType;
    do {
        ident = this->lexer->nextToken();
        if (ident != TKTYPE::IDENFR) {
            //[Wrong VarDecl]need ident"
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        }

        degrees.clear();
        while (this->lexer->peekToken() == TKTYPE::LBRACK) {
            this->lexer->nextToken();

            this->ConstExp(result);

            degrees.push_back(result);

            if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
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
            parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
        }

#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<VarDef>" << std::endl;
        else
            (*sout) += "<VarDef>\n";
#endif

        if (this->lexer->peekToken() == TKTYPE::COMMA)
            this->lexer->nextToken();
        else
            break;
    } while (true);

    if (this->lexer->peekToken() != TKTYPE::SEMICN) {
        //[Wrong VarDecl]need ;
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
    } else
        this->lexer->nextToken();

#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<VarDecl>" << std::endl;
    else
        (*sout) += "<VarDecl>\n";
#endif
}

std::vector<int> Parser::InitVal(bool constCheck)
{
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
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            else
                this->lexer->nextToken();
        } else
            this->lexer->nextToken();

    } else {
        int result;
        if (constCheck)
            this->ConstExp(result);
        else
            this->Exp(constCheck, result);

        if (constCheck) {
            initialValues.push_back(result);
        }
    }
#ifdef parser_debug_output
    if (constCheck) {
        if (this->sout == nullptr)
            (*fout) << "<ConstInitVal>" << std::endl;
        else
            (*sout) += "<ConstInitVal>\n";
    } else {
        if (this->sout == nullptr)
            (*fout) << "<InitVal>" << std::endl;
        else
            (*sout) += "<InitVal>\n";
    }
#endif
    return initialValues;
}

MyType Parser::ConstExp(int& feedback)
{
    MyType&& ans = this->AddExp(true, feedback);

#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<ConstExp>" << std::endl;
    else
        (*sout) += "<ConstExp>\n";
#endif
    return ans;
}

MyType Parser::Exp(bool constCheck, int& feedback)
{
    MyType&& ans = this->AddExp(constCheck, feedback);
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<Exp>" << std::endl;
    else
        (*sout) += "<Exp>\n";
#endif
    return ans;
}
MyType Parser::AddExp(bool constCheck, int& feedback)
{
    MyType&& ans = this->MulExp(constCheck, feedback);
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<AddExp>" << std::endl;
    else
        (*sout) += "<AddExp>\n";
#endif
    int result;
    Token tempToken;
    while (this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS) {
        tempToken = this->lexer->nextToken();
        MyType&& tempType = this->MulExp(constCheck, result);

        if (constCheck) {
            feedback += tempToken == TKTYPE::PLUS ? +result : -result;
        }

        try {
            ans = ans.calculate(tempToken, tempType);
        } catch (const ErrorCode& e) {
            parser_exception_handler(tempToken.line, tempToken.column, e);
        }

#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<AddExp>" << std::endl;
        else
            (*sout) += "<AddExp>\n";
#endif
    }
    return ans;
}

MyType Parser::MulExp(bool constCheck, int& feedback)
{
    MyType&& ans = this->UnaryExp(constCheck, feedback);
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<MulExp>" << std::endl;
    else
        (*sout) += "<MulExp>\n";
#endif
    Token tempToken;
    int tempInt;
    while (this->lexer->peekToken() == TKTYPE::MULT || this->lexer->peekToken() == TKTYPE::DIV || this->lexer->peekToken() == TKTYPE::MOD) {
        tempToken = this->lexer->nextToken();
        MyType&& tempType = this->UnaryExp(constCheck, tempInt);
        if (constCheck) {
            if (tempToken == TKTYPE::MULT) {
                feedback *= tempInt;
            } else {
                if (!tempInt) {
                    parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::OtherError);
                } else {
                    feedback = tempToken == TKTYPE::DIV ? feedback / tempInt : feedback % tempInt;
                }
            }
        }
        try {
            ans = ans.calculate(tempToken, tempType);
        } catch (const ErrorCode& e) {
            parser_exception_handler(tempToken.line, tempToken.column, e);
        }

#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<MulExp>" << std::endl;
        else
            (*sout) += "<MulExp>\n";
#endif
    }
    return ans;
}

MyType Parser::UnaryExp(bool constCheck, int& feedback)
{
    Token tempToken = this->lexer->nextToken();
    int check;
    MyType ans;
    switch (tempToken.type) {
    case TKTYPE::PLUS:
    case TKTYPE::MINUS:
    case TKTYPE::NOT:
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<UnaryOp>" << std::endl;
        else
            (*sout) += "<UnaryOp>\n";
#endif
        ans = this->UnaryExp(constCheck, feedback);
        if (constCheck)
            feedback = tempToken == TKTYPE::MINUS ? -feedback : tempToken == TKTYPE::NOT ? !feedback
                                                                                         : feedback;
        ans = ans.calculate(tempToken);
        break;
    case TKTYPE::LPARENT:
    case TKTYPE::INTCON:
        ans = this->PrimaryExp(tempToken, constCheck, feedback);
        break;
    case TKTYPE::IDENFR:
        if (this->lexer->peekToken() == TKTYPE::LPARENT) {
            if (this->tableManager->getFunctionType(tempToken.value, ans)) {
                parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::Nodefine);
            }
            this->lexer->nextToken();
            if (this->lexer->peekToken() == TKTYPE::IDENFR || this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS || this->lexer->peekToken() == TKTYPE::NOT || this->lexer->peekToken() == TKTYPE::LPARENT || this->lexer->peekToken() == TKTYPE::INTCON) {
                // std::vector<MyType&>& parameters = this->FuncRParams();

                if (check = this->tableManager->checkFunctionParameters(tempToken.value, this->FuncRParams())) {
                    if (check == 1)
                        parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::WrongParameterNumber);
                    else if (check == 2)
                        parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::WrongParameterType);
                    else
                        parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::OtherError);
                }
            }

            if (this->lexer->peekToken() != TKTYPE::RPARENT)
                //[wrong UnaryExp]need )
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            else
                this->lexer->nextToken();
            feedback = 0;
        } else {
            ans = this->PrimaryExp(tempToken, constCheck, feedback);
        }
        break;
    default:
        //[wrong UnaryExp]
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        break;
    }
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<UnaryExp>" << std::endl;
    else
        (*sout) += "<UnaryExp>\n";
#endif
    return ans;
}

MyType Parser::PrimaryExp(const Token& head, bool constCheck, int& feedback)
{
    MyType ans;
    switch (head.type) {
    case TKTYPE::LPARENT:
        ans = this->Exp(constCheck, feedback);
        if (this->lexer->peekToken() != TKTYPE::RPARENT)
            //[wrong PrimaryExp]need )
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
        else
            this->lexer->nextToken();
        break;
    case TKTYPE::IDENFR:
        ans = this->LVal(head, constCheck, feedback);
        break;
    case TKTYPE::INTCON:
        if (constCheck)
            feedback = std::stoi(head.value);
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<Number>" << std::endl;
        else
            (*sout) += "<Number>\n";
#endif
        // TODO: here
        ans = MyType(true);
        break;
    default:
        // [design error]
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        break;
    }
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<PrimaryExp>" << std::endl;
    else
        (*sout) += "<PrimaryExp>\n";
#endif
    return ans;
}

std::vector<MyType> Parser::FuncRParams()
{
    int result;
    std::vector<MyType> ans;
    ans.push_back(this->Exp(false, result));
    while (this->lexer->peekToken() == TKTYPE::COMMA) {
        this->lexer->nextToken();
        ans.push_back(this->Exp(false, result));
    }
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<FuncRParams>" << std::endl;
    else
        (*sout) += "<FuncRParams>\n";
#endif
    return ans;
}

void Parser::FuncDef(const Token& type, const Token& ident)
{
    if (this->lexer->peekToken() != TKTYPE::LPARENT) {
        //[wrong FuncDef]need (
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
    } else
        this->lexer->nextToken();

    if (!this->tableManager->insertFunction(MyType(type == TKTYPE::INTTK ? VarType::IntType : VarType::VoidType), ident.value)) {
        parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
    }
    this->tableManager->setNextBlock(type == TKTYPE::INTTK ? BlockType::IntFunction : BlockType::VoidFunction);

    //*<FuncFParams>
    MyType tempType;
    Token tempToken;
    std::vector<int> degrees;
    unsigned int tempPointerDepth;
    int result;
    if (this->lexer->peekToken() == TKTYPE::INTTK) {
        while (this->lexer->peekToken() == TKTYPE::INTTK) {
            this->lexer->nextToken();
            if ((tempToken = this->lexer->nextToken()) != TKTYPE::IDENFR) {
                //[wrong FuncDef]need identifier"
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            }
            tempPointerDepth = 0;
            if (this->lexer->peekToken() == TKTYPE::LBRACK) {
                this->lexer->nextToken();
                tempPointerDepth++;
                if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                    //[wrong FuncDef]need ]
                    parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                } else
                    this->lexer->nextToken();
            }

            degrees.clear();
            while (this->lexer->peekToken() == TKTYPE::LBRACK) {
                this->lexer->nextToken();

                this->ConstExp(result);

                degrees.push_back(result);
                if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                    // [wrong FuncDef]need ]
                    parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                } else
                    this->lexer->nextToken();
            }
            tempType = MyType(VarType::IntType, tempPointerDepth, degrees);
            if (!this->tableManager->insertVariable(tempType, tempToken.value, true)) {
                parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::Redefine);
            }

#ifdef parser_debug_output
            if (this->sout == nullptr)
                (*fout) << "<FuncFParam>" << std::endl;
            else
                (*sout) += "<FuncFParam>\n";
#endif
            if (this->lexer->peekToken() == TKTYPE::COMMA) {
                this->lexer->nextToken();
            }
        }
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<FuncFParams>" << std::endl;
        else
            (*sout) += "<FuncFParams>\n";
#endif
    }
    if (this->lexer->peekToken() != TKTYPE::RPARENT) {
        //[wrong FuncDef]need )
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
    } else {
        this->lexer->nextToken();
        // this->tableManager->fixUpperFunctionParameters();
    }

    this->Block();
    if (!this->tableManager->setUpperBlock()) {
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingReturnStatement);
    }

#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << (ident != TKTYPE::MAINTK ? "<FuncDef>" : "<MainFuncDef>") << std::endl;
    else
        (*sout) += (ident != TKTYPE::MAINTK ? "<FuncDef>\n" : "<MainFuncDef>\n");
#endif
}

void Parser::Block()
{
    if (this->lexer->nextToken() != TKTYPE::LBRACE) {
        // [wrong Block]need {
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
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
        parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
    }
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<Block>" << std::endl;
    else
        (*sout) += "<Block>\n";
#endif
}

void Parser::Stmt(BlockType blockType)
{
    if (this->lexer->peekToken() == TKTYPE::IFTK) {
        this->lexer->nextToken();
        this->lexer->nextToken(); //(
        this->LOrExp();
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<Cond>" << std::endl;
        else
            (*sout) += "<Cond>\n";
#endif
        if (this->lexer->peekToken() != TKTYPE::RPARENT)
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
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
            MyType&& tempType = this->LVal(ident, false, result);
            if (tempType.isConst) {
                parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
            }
            if (this->lexer->nextToken() != TKTYPE::ASSIGN) {
                // [wrong Stmt]need =
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            }
            this->Exp(false, result);
#ifdef parser_debug_output
            if (this->sout == nullptr)
                (*fout) << "<ForStmt>" << std::endl;
            else
                (*sout) += "<ForStmt>\n";
#endif
        }
        if (this->lexer->nextToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        }
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            this->LOrExp();
#ifdef parser_debug_output
            if (this->sout == nullptr)
                (*fout) << "<Cond>" << std::endl;
            else
                (*sout) += "<Cond>\n";
#endif
        }

        if (this->lexer->nextToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        }

        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            int result;
            Token ident = this->lexer->nextToken();
            MyType&& tempType = this->LVal(ident, false, result);
            if (tempType.isConst) {
                parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
            }
            if (this->lexer->nextToken() != TKTYPE::ASSIGN) {
                // [wrong Stmt]need =
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            }
            this->Exp(false, result);
#ifdef parser_debug_output
            if (this->sout == nullptr)
                (*fout) << "<ForStmt>" << std::endl;
            else
                (*sout) += "<ForStmt>\n";
#endif
        }
        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            //[wrong Stmt]need )
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
        } else {
            this->lexer->nextToken();
        }

        this->Stmt(BlockType::Loop);
    } else if (this->lexer->peekToken() == TKTYPE::BREAKTK
        || this->lexer->peekToken() == TKTYPE::CONTINUETK) {
        Token&& cbToken = this->lexer->nextToken();
        if (blockType != BlockType::Loop && !this->tableManager->inLoop()) {
            parser_exception_handler(cbToken.line, cbToken.column, ErrorCode::BreakOrContinueOutOfLoop);
        }
        
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        } else
            this->lexer->nextToken();
    } else if (this->lexer->peekToken() == TKTYPE::RETURNTK) {
        Token&& returnToken = this->lexer->nextToken();
        this->tableManager->setReturnStatement();
        int result;
        if (this->lexer->peekToken() == TKTYPE::IDENFR || this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS || this->lexer->peekToken() == TKTYPE::NOT || this->lexer->peekToken() == TKTYPE::LPARENT || this->lexer->peekToken() == TKTYPE::INTCON) {
            this->Exp(false, result);
            if (!this->tableManager->inIntFunction()) {
                parser_exception_handler(returnToken.line, returnToken.column, ErrorCode::SuperfluousReturnValue);
            }
        } else {
            if (!this->tableManager->inVoidFunction()) {
                parser_exception_handler(returnToken.line, returnToken.column, ErrorCode::MissingReturnValue);
            }
        }
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        } else {
            this->lexer->nextToken();
        }
    } else if (this->lexer->peekToken() == TKTYPE::PRINTFTK) {
        Token&& printfToken = this->lexer->nextToken();
        Token tempToken;
        int result;
        if (this->lexer->nextToken() != TKTYPE::LPARENT) {
            // [wrong Stmt]need (");
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        }
        // TODO:check printf here
        unsigned int formatParamNum = 0;
        if ((tempToken = this->lexer->peekToken()) != TKTYPE::STRCON) {
            // [wrong Stmt]need format string");
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        } else {
            if (!checkFormatString(tempToken, formatParamNum)) {
                parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::IllegalCharInFormatString);
            }
            this->lexer->nextToken();
        }
        unsigned int paramNum = 0;
        while (this->lexer->peekToken() == TKTYPE::COMMA) {
            this->lexer->nextToken();
            paramNum++;
            this->Exp(false, result);
        }
        if (paramNum != formatParamNum) {
            parser_exception_handler(printfToken.line, printfToken.column, ErrorCode::UnmatchedPrintArgs);
        }

        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            // [wrong Stmt]need )");
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
        } else
            this->lexer->nextToken();

        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;");
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        } else
            this->lexer->nextToken();

    } else if (this->lexer->peekToken() == TKTYPE::SEMICN) {
        this->lexer->nextToken();
    } else if (this->lexer->peekToken() == TKTYPE::IDENFR) {

        this->sout = &this->outBuffer;
        this->lexer->hold(&this->outBuffer);
        int result;
        Token ident = this->lexer->nextToken();
        MyType&& ans = this->LVal(ident, false, result);
        if (ans.isConst) {
            parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
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
                    parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                else
                    this->lexer->nextToken();
                if (this->lexer->peekToken() != TKTYPE::RPARENT)
                    // [wrong FuncDef]need )");
                    parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
                else
                    this->lexer->nextToken();
                if (this->lexer->peekToken() != TKTYPE::SEMICN)
                    // [wrong FuncDef]need ;");
                    parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
                else
                    this->lexer->nextToken();
            } else {
                this->Exp(false, result);
                if (this->lexer->peekToken() != TKTYPE::SEMICN)
                    // [wrong FuncDef]need ;");
                    parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
                else
                    this->lexer->nextToken();
            }
        } else {

            this->lexer->discharge();
            this->exceptionController->discharge();
            // guess wrong
            this->sout = nullptr;
            int result;
            this->Exp(false, result);

            if (this->lexer->peekToken() != TKTYPE::SEMICN)
                // [wrong FuncDef]need ;");
                parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            else
                this->lexer->nextToken();
        }
    } else if (this->lexer->peekToken() == TKTYPE::LBRACE) {
        this->tableManager->setNextBlock(blockType);
        this->Block();
        if (!this->tableManager->setUpperBlock()) {
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingReturnStatement);
        }
    } else {
        int result;
        this->Exp(false, result);
        if (this->lexer->peekToken() != TKTYPE::SEMICN)
            // [wrong FuncDef]need ;");
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
        else
            this->lexer->nextToken();
    }
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<Stmt>" << std::endl;
    else
        (*sout) += "<Stmt>\n";
#endif
}

MyType Parser::LVal(const Token& ident, bool constCheck, int& feedback)
{
    int result;
    std::vector<int> indexes;
    MyType ans;
    Token tempToken;
    if (this->tableManager->getVariableType(ident.value, ans)) {
        parser_exception_handler(ident.line, ident.column, ErrorCode::Nodefine);
    }
    while (this->lexer->peekToken() == TKTYPE::LBRACK) {
        tempToken = this->lexer->nextToken();

        MyType&& tempType = this->Exp(constCheck, result);
        ans = ans.calculate(tempToken, tempType);
        if (constCheck)
            indexes.push_back(result);

        if (this->lexer->peekToken() != TKTYPE::RBRACK)
            parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
        else
            this->lexer->nextToken(); // ]
    }
    if (constCheck) {
        this->tableManager->getVariableValue(ident.value, feedback, indexes);
    }
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<LVal>" << std::endl;
    else
        (*sout) += "<LVal>\n";
#endif
    return ans;
}

void Parser::RelExp()
{
    int result;
    this->AddExp(false, result);
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<RelExp>" << std::endl;
    else
        (*sout) += "<RelExp>\n";
#endif
    while (this->lexer->peekToken() == TKTYPE::LSS
        || this->lexer->peekToken() == TKTYPE::GRE
        || this->lexer->peekToken() == TKTYPE::LEQ
        || this->lexer->peekToken() == TKTYPE::GEQ) {
        this->lexer->nextToken();
        this->AddExp(false, result);
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<RelExp>" << std::endl;
        else
            (*sout) += "<RelExp>\n";
#endif
    }
}

void Parser::EqExp()
{
    this->RelExp();
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<EqExp>" << std::endl;
    else
        (*sout) += "<EqExp>\n";
#endif
    while (this->lexer->peekToken() == TKTYPE::EQL
        || this->lexer->peekToken() == TKTYPE::NEQ) {
        this->lexer->nextToken();
        this->RelExp();
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<EqExp>" << std::endl;
        else
            (*sout) += "<EqExp>\n";
#endif
    }
}

void Parser::LAndExp()
{
    this->EqExp();
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<LAndExp>" << std::endl;
    else
        (*sout) += "<LAndExp>\n";
#endif
    while (this->lexer->peekToken() == TKTYPE::AND) {
        this->lexer->nextToken();
        this->EqExp();
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<LAndExp>" << std::endl;
        else
            (*sout) += "<LAndExp>\n";
#endif
    }
}

void Parser::LOrExp()
{
    this->LAndExp();
#ifdef parser_debug_output
    if (this->sout == nullptr)
        (*fout) << "<LOrExp>" << std::endl;
    else
        (*sout) += "<LOrExp>\n";
#endif
    while (this->lexer->peekToken() == TKTYPE::OR) {
        this->lexer->nextToken();
        this->LAndExp();
#ifdef parser_debug_output
        if (this->sout == nullptr)
            (*fout) << "<LOrExp>" << std::endl;
        else
            (*sout) += "<LOrExp>\n";
#endif
    }
}
