#include "../include/parser.h"
#include "../include/exception/exception.h"
#include "../include/util/util.h"

// #define parser_debug_output

#define _marco_string(x) #x
#define parser_output(content) ({            \
    if (this->sout == nullptr)               \
        (*fout) << #content << std::endl;    \
    else                                     \
        (*sout) += _marco_string(content\n); \
})

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

    std::vector<unsigned int> degrees;
    std::vector<VarInf> initialValues;
    Token ident;
    VarType var_type = VarType(true);
    VarInf ret_var;
    int ret_value;

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

            this->ConstExp(ret_var);

            if (!Util::stringToInt(ret_var.name, ret_value)) {
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }

            degrees.push_back(ret_value);

            if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                //[wrong ConstDecl]wrong identifier
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
            } else
                this->lexer->nextToken();
        }
        var_type.array_degrees = degrees;

        //* = <initialValue> (insert constant here)
        if (this->lexer->peekToken() == TKTYPE::ASSIGN) {
            this->lexer->nextToken();

            initialValues = this->InitVal(true);

        } else {
            //[wrong ConstDecl]need initialize
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        }

        if (!this->ir_module->declareVariable(ident.value, var_type, initialValues)) {
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
    std::vector<unsigned int> degrees;
    std::vector<VarInf> initial_values;
    int ret_value;
    VarType var_type;
    VarInf ret_var;
    do {
        ident = this->lexer->nextToken();
        if (ident != TKTYPE::IDENFR) {
            //[Wrong VarDecl]need ident"
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        }
        ret_var = VarInf();
        // '['ConstExp']'
        degrees.clear();
        while (this->lexer->peekToken() == TKTYPE::LBRACK) {
            this->lexer->nextToken();

            this->ConstExp(ret_var);

            if (!Util::stringToInt(ret_var.name, ret_value)) {
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }

            degrees.push_back(ret_value);

            if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
            } else {
                this->lexer->nextToken();
            }
        }
        var_type.array_degrees = degrees;

        // [ selectable] '=' {initVal}
        initial_values.clear();
        if (this->lexer->peekToken() == TKTYPE::ASSIGN) {
            this->lexer->nextToken();
            initial_values = this->InitVal();
        }

        if (!this->ir_module->declareVariable(ident.value, var_type, initial_values)) {
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

std::vector<VarInf> Parser::InitVal(bool constCheck)
{
    VarInf ret_var;
    std::vector<VarInf> initialValues;
    if (this->lexer->peekToken() == TKTYPE::LBRACE) {
        //'{' exp{, exp} '}'
        this->lexer->nextToken();
        if (this->lexer->peekToken() != TKTYPE::RBRACE) {
            do {
                auto&& addInitial = this->InitVal(constCheck);

                initialValues.insert(initialValues.end(), addInitial.begin(), addInitial.end());

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
        // {exp}
        if (constCheck)
            this->ConstExp(ret_var);
        else
            this->Exp(constCheck, ret_var);

        initialValues.push_back(ret_var);
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

void Parser::ConstExp(VarInf& ret_reg)
{
    this->AddExp(true, ret_reg);

#ifdef parser_debug_output
    parser_output(<ConstExp>);
#endif
}

void Parser::Exp(bool constCheck, VarInf& ret_var)
{
    this->AddExp(constCheck, ret_var);
#ifdef parser_debug_output
    parser_output(<Exp>);
#endif
}
void Parser::AddExp(bool constCheck, VarInf& ret_var)
{
    this->MulExp(constCheck, ret_var);
    ErrorCode error_code;

#ifdef parser_debug_output
    parser_output(<AddExp>);
#endif

    Token tempToken;
    VarInf temp_var;
    while (this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS) {
        tempToken = this->lexer->nextToken();

        this->MulExp(constCheck, temp_var);
        if (constCheck) {
            int value1, value2;
            Util::stringToInt(ret_var.name, value1);
            Util::stringToInt(temp_var.name, value2);
            ret_var.name = std::to_string(tempToken==TKTYPE::PLUS ? value1 + value2 : value1 - value2);
        } else {
            ret_var = this->ir_module->calculate(tempToken == TKTYPE::PLUS ? "PLUS" : "MINUS", ret_var, temp_var, &error_code);
            if (error_code != ErrorCode::None) {
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, error_code));
            }
        }

#ifdef parser_debug_output
        parser_output(<AddExp>);
#endif
    }
}

void Parser::MulExp(bool constCheck, VarInf& ret_var)
{
    this->UnaryExp(constCheck, ret_var);
#ifdef parser_debug_output
    parser_output(<MulExp>);
#endif
    Token tempToken;
    VarInf temp_var;
    ErrorCode error_code;
    while (this->lexer->peekToken() == TKTYPE::MULT || this->lexer->peekToken() == TKTYPE::DIV || this->lexer->peekToken() == TKTYPE::MOD) {
        tempToken = this->lexer->nextToken();

        this->UnaryExp(constCheck, temp_var);
        if (constCheck) {
            int value1, value2;
            Util::stringToInt(ret_var.name, value1);
            Util::stringToInt(temp_var.name, value2);
            ret_var.name = std::to_string(tempToken == TKTYPE::MULT ? value1 * value2 : tempToken==TKTYPE::MOD ? value1 % value2 : value1 / value2);
        } else {
            if (tempToken == TKTYPE::MULT) {
                ret_var = this->ir_module->calculate("MULT", ret_var, temp_var, &error_code);
            } else if (tempToken == TKTYPE::DIV) {
                ret_var = this->ir_module->calculate("DIV", ret_var, temp_var, &error_code);
            } else {
                ret_var = this->ir_module->calculate("MOD", ret_var, temp_var, &error_code);
            }

            if (error_code != ErrorCode::None) {
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, error_code));
            }
        }

#ifdef parser_debug_output
        parser_output(<MulExp>);
#endif
    }
}

void Parser::UnaryExp(bool constCheck, VarInf& ret_var)
{
    Token tempToken = this->lexer->nextToken();
    int check;
    ErrorCode error_code;
    switch (tempToken.type) {
    //{UnaryExp} -> {UnaryOp} {UnaryExp}
    case TKTYPE::PLUS:
    case TKTYPE::MINUS:
    case TKTYPE::NOT:
#ifdef parser_debug_output
        parser_output(<UnaryOp>);
#endif
        this->UnaryExp(constCheck, ret_var);
        if (constCheck) {
            int value;
            Util::stringToInt(ret_var.name, value);
            ret_var.name = tempToken == TKTYPE::MINUS ? std::to_string(-value) : tempToken == TKTYPE::NOT ? std::to_string(!value) : ret_var.name;
        } else {
            ret_var = tempToken == TKTYPE::MINUS ? this->ir_module->calculate("MINUS", { VarType::CONSTANT, "0" }, ret_var, &error_code) : tempToken == TKTYPE::NOT ? this->ir_module->calculate("EQL", { VarType::CONSTANT, "0" }, ret_var, &error_code)
                                                                                                                                                                    : ret_var;
        }
        break;
    // {UnaryExp} -> {PrimaryExp} -> '('{Exp}')' | {Number}
    case TKTYPE::LPARENT:
    case TKTYPE::INTCON:
        this->PrimaryExp(tempToken, constCheck, ret_var);
        break;
    // {UnaryExp} -> {PrimaryExp} -> {LVal} -> Ident {'['{Exp}']'}
    // {UnaryExp} -> Ident'(' [{FuncRParams}] ')'
    case TKTYPE::IDENFR:
        // {UnaryExp} -> Ident  >>>>'('<<<<   [{FuncRParams}] ')'
        if (this->lexer->peekToken() == TKTYPE::LPARENT) {
            if (!this->ir_module->getFuction(tempToken.value, nullptr)) {
                // parser_exception_handler(tempToken.line, tempToken.column, ErrorCode::Nodefine);
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, ErrorCode::Nodefine));
            }
            this->lexer->nextToken();
            std::vector<VarInf> params = {};
            if (this->lexer->peekToken() == TKTYPE::IDENFR || this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS || this->lexer->peekToken() == TKTYPE::NOT || this->lexer->peekToken() == TKTYPE::LPARENT || this->lexer->peekToken() == TKTYPE::INTCON) {
                // std::vector<MyType&>& parameters = this->FuncRParams();
                params = this->FuncRParams();
            }
            
            ret_var = this->ir_module->callFunction(tempToken.value, params, &error_code);
            if (error_code != ErrorCode::None) {
                this->exceptionController->handle(CompilerException(tempToken.line, tempToken.column, error_code));
            }

            if (this->lexer->peekToken() != TKTYPE::RPARENT)
                //[wrong UnaryExp]need )
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
            else
                this->lexer->nextToken();
            // returnValue = 0;
        } else {
            this->PrimaryExp(tempToken, constCheck, ret_var);
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

// {PrimaryExp} -> '('{Exp}')' | {LVal} | {Number}
void Parser::PrimaryExp(const Token& head, bool constCheck, VarInf& ret_var)
{
    switch (head.type) {
    case TKTYPE::LPARENT:
        this->Exp(constCheck, ret_var);
        if (this->lexer->peekToken() != TKTYPE::RPARENT)
            //[wrong PrimaryExp]need )
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        else
            this->lexer->nextToken();
        break;
    case TKTYPE::IDENFR:
        this->LVal(head, false, constCheck, ret_var);
        break;
    case TKTYPE::INTCON:
        ret_var = { VarType::CONSTANT, head.value };
#ifdef parser_debug_output
        parser_output(<Number>);
#endif
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

std::vector<VarInf> Parser::FuncRParams()
{
    int resultValue;
    std::vector<VarInf> ret_params;
    VarInf ret_var;
    this->Exp(false, ret_var);
    ret_params.push_back(ret_var);
    while (this->lexer->peekToken() == TKTYPE::COMMA) {
        this->lexer->nextToken();
        this->Exp(false, ret_var);
        ret_params.push_back(ret_var);
    }
#ifdef parser_debug_output
    parser_output(<FuncRParams>);
#endif
    return ret_params;
}

void Parser::FuncDef(const Token& type, const Token& ident)
{
    if (this->lexer->peekToken() != TKTYPE::LPARENT) {
        //[wrong FuncDef]need (
        // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
        this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
    } else
        this->lexer->nextToken();

    if (!this->ir_module->delcareFunction(ident.type == TKTYPE::MAINTK ? "main" : ident.value, type == TKTYPE::INTTK ? VarType::i32 : VarType::VOID)) {
        // parser_exception_handler(ident.line, ident.column, ErrorCode::Redefine);
        this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::Redefine));
    }
    // TODO: check if it is needed
    // this->ir_module->setClosureBlock(BasicBlock::ENTRY);

    //*<FuncFParams>
    VarInf temp_var;
    Token temp_token;
    std::vector<unsigned int> degrees;
    unsigned int temp_pointer_depth;
    int ret_value;
    VarType var_type;
    if (this->lexer->peekToken() == TKTYPE::INTTK) {
        while (this->lexer->peekToken() == TKTYPE::INTTK) {
            var_type = VarType();
            this->lexer->nextToken();
            if ((temp_token = this->lexer->nextToken()) != TKTYPE::IDENFR) {
                //[wrong FuncDef]need identifier
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }
            temp_pointer_depth = 0;
            if (this->lexer->peekToken() == TKTYPE::LBRACK) {
                this->lexer->nextToken();
                temp_pointer_depth++;
                if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                    //[wrong FuncDef]need ]
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
                } else
                    this->lexer->nextToken();
            }

            degrees.clear();
            while (this->lexer->peekToken() == TKTYPE::LBRACK) {
                this->lexer->nextToken();

                this->ConstExp(temp_var);

                // check array index const
                if (!Util::stringToInt(temp_var.name, ret_value)) {
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
                    var_type.type = VarType::WRONG;
                }

                degrees.push_back(ret_value);
                if (this->lexer->peekToken() != TKTYPE::RBRACK) {
                    // [wrong FuncDef]need ]
                    this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
                } else
                    this->lexer->nextToken();
            }

            var_type.array_degrees = degrees;
            var_type = var_type.getVisit(-temp_pointer_depth, nullptr);

            if (!this->ir_module->declareParam(temp_token.value, var_type)) {
                this->exceptionController->handle(CompilerException(temp_token.line, temp_token.column, ErrorCode::Redefine));
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
    // this->ir_module->outClosureBlock();
    if (this->ir_module->closeFunctionNeedRet()) {
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

void Parser::Stmt(BasicBlock::Type block_type, const bool empty_condition, const bool empty_inc, const std::string& block_name)
{
    VarInf ret_var;
    ErrorCode error_code;

    if (this->lexer->peekToken() == TKTYPE::IFTK) {
        unsigned int if_label_number = this->ir_module->getLabelNumber(this->lexer->peekToken().toString());
        std::string&& label_number_string = std::to_string(if_label_number);

        this->lexer->nextToken(); // read if
        this->lexer->nextToken(); // read (

        this->LOrExp(ret_var, label_number_string, BasicBlock::IF_TRUE, label_number_string, BasicBlock::IF_FALSE);
        this->ir_module->setBranchStatement(ret_var, label_number_string, BasicBlock::IF_TRUE, label_number_string, BasicBlock::IF_FALSE);

#ifdef parser_debug_output
        parser_output(<Cond>);
#endif
        if (this->lexer->peekToken() != TKTYPE::RPARENT) // check )
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        else
            this->lexer->nextToken(); // read )

        // set if_true block
        this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::IF_TRUE);
        this->Stmt(BasicBlock::IF_TRUE, label_number_string);

        // if_false block is not empty?
        if (this->lexer->peekToken() == TKTYPE::ELSETK) {
            // end if_true block by jumping to if_false block
            this->ir_module->setBranchStatement(label_number_string, BasicBlock::IF_END);

            // set if_false block
            this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::IF_FALSE);

            this->lexer->nextToken();

            this->Stmt(BasicBlock::IF_FALSE, label_number_string);
            // end if_false block by jumping to if_end block
            this->ir_module->setBranchStatement(label_number_string, BasicBlock::IF_END);

            // set if_end block
            this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::IF_END);
        } else {
            // end if_true block by jumping to if_false block
            this->ir_module->setBranchStatement(label_number_string, BasicBlock::IF_FALSE);

            // set if_false block
            this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::IF_FALSE);
        }
    } else if (this->lexer->peekToken() == TKTYPE::FORTK) {
        unsigned for_label_number = this->ir_module->getLabelNumber(this->lexer->peekToken().toString());
        std::string label_number_string = std::to_string(for_label_number);
        VarInf temp_var;

        this->lexer->nextToken(); // read for
        this->lexer->nextToken(); // read (
        bool empty_condition = false;
        bool empty_inc = false;

        // if initial ForStmt is not empty
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // ForStmt
            Token ident = this->lexer->nextToken();
            this->LVal(ident, true, false, ret_var);

            if (this->lexer->nextToken() != TKTYPE::ASSIGN) {
                // [wrong Stmt]need =
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }

            this->Exp(false, temp_var);

            this->ir_module->calculate("ASSIGN", ret_var, temp_var, &error_code);
            if (error_code != ErrorCode::None) {
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, error_code));
            }
#ifdef parser_debug_output
            parser_output(<ForStmt>);
#endif
        }

        if (this->lexer->nextToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        }

        // if condition block is not empty
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // end initial block with jumping to condition block
            this->ir_module->setBranchStatement(label_number_string, BasicBlock::LOOP_CONDITION);

            // set condition block
            this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::LOOP_CONDITION);
            this->LOrExp(ret_var, label_number_string, BasicBlock::LOOP_BODY, label_number_string, BasicBlock::LOOP_END);
            this->ir_module->setBranchStatement(ret_var, label_number_string, BasicBlock::LOOP_BODY, label_number_string, BasicBlock::LOOP_END);
            // Note: need body, but we write inc first
#ifdef parser_debug_output
            parser_output(<Cond>);
#endif
        } else {
            // end initial block with jumping to body block
            this->ir_module->setBranchStatement(label_number_string, BasicBlock::LOOP_BODY);
            empty_condition = true;
        }

        if (this->lexer->nextToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        }

        // if inc block is not empty
        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            Token ident = this->lexer->nextToken();
            this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::LOOP_INC);
            this->LVal(ident, true, false, ret_var);
            if (this->lexer->nextToken() != TKTYPE::ASSIGN) {
                // [wrong Stmt]need =
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
            }
            this->Exp(false, temp_var);

            this->ir_module->calculate("ASSIGN", ret_var, temp_var, &error_code);
            if (error_code != ErrorCode::None) {
                // parser_exception_handler(ident.line, ident.column, ErrorCode::ModifyConst);
                this->exceptionController->handle(CompilerException(ident.line, ident.column, ErrorCode::ModifyConst));
            }
#ifdef parser_debug_output
            parser_output(<ForStmt>);
#endif
            // end inc block with jumping to condition, if it is not empty, otherwise jumping to body
            this->ir_module->setBranchStatement(label_number_string, empty_condition ? BasicBlock::LOOP_BODY : BasicBlock::LOOP_CONDITION);
        } else {
            empty_inc = true;
        }
        if (this->lexer->peekToken() != TKTYPE::RPARENT) {
            //[wrong Stmt]need )
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseParentheses));
        } else {
            this->lexer->nextToken();
        }

        this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::LOOP_BODY);
        this->Stmt(BasicBlock::LOOP_BODY, empty_condition, empty_inc, label_number_string);
        // end body with jump to inc if it is not empty, or jumping to condition if it is empty, otherwise to itself
        this->ir_module->setBranchStatement(label_number_string, !empty_inc ? BasicBlock::LOOP_INC : empty_condition ? BasicBlock::LOOP_BODY : BasicBlock::LOOP_CONDITION);

        // set for_end block
        this->ir_module->setNextBasicBlock(label_number_string, BasicBlock::LOOP_END);

    } else if (this->lexer->peekToken() == TKTYPE::BREAKTK
        || this->lexer->peekToken() == TKTYPE::CONTINUETK) {

        Token&& break_or_continue_token = this->lexer->nextToken();
        std::string real_loop_name = block_name;
        BasicBlock::Type real_loop_type = block_type;
        bool real_empty_condition = empty_condition, real_empty_inc = empty_inc;
        if (!ClosureBlock::isLoop(block_type) && !this->ir_module->inLoop(real_loop_type, real_empty_condition, real_empty_inc, real_loop_name)) {
            // parser_exception_handler(cbToken.line, cbToken.column, ErrorCode::BreakOrContinueOutOfLoop);
            this->exceptionController->handle(CompilerException(break_or_continue_token.line, break_or_continue_token.column, ErrorCode::BreakOrContinueOutOfLoop));
        }// get loop name & loop type

        this->ir_module->setBranchStatement(real_loop_name, break_or_continue_token == TKTYPE::BREAKTK ? BasicBlock::LOOP_END : real_empty_inc ? real_empty_condition ? BasicBlock::LOOP_BODY : BasicBlock::LOOP_CONDITION : BasicBlock::LOOP_INC);
        unsigned int after_number = this->ir_module->getLabelNumber(break_or_continue_token.toString());
        this->ir_module->setNextBasicBlock(std::to_string(after_number), break_or_continue_token == TKTYPE::CONTINUETK ? BasicBlock::CONTINUE_AFTER : BasicBlock::BREAK_AFTER);

        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        } else
            this->lexer->nextToken();

    } else if (this->lexer->peekToken() == TKTYPE::RETURNTK) {

        Token&& returnToken = this->lexer->nextToken();
        ErrorCode error_code;

        if (this->lexer->peekToken() == TKTYPE::IDENFR || this->lexer->peekToken() == TKTYPE::PLUS || this->lexer->peekToken() == TKTYPE::MINUS || this->lexer->peekToken() == TKTYPE::NOT || this->lexer->peekToken() == TKTYPE::LPARENT || this->lexer->peekToken() == TKTYPE::INTCON) {
            this->Exp(false, ret_var);
            this->ir_module->setReturnStatement(ret_var, &error_code);
            if (error_code != ErrorCode::None) {
                // parser_exception_handler(returnToken.line, returnToken.column, ErrorCode::SuperfluousReturnValue);
                this->exceptionController->handle(CompilerException(returnToken.line, returnToken.column, ErrorCode::SuperfluousReturnValue));
            }
        } else {
            this->ir_module->setReturnStatement(&error_code);
            if (error_code != ErrorCode::None) {
                // parser_exception_handler(returnToken.line, returnToken.column, ErrorCode::MissingReturnValue);
                this->exceptionController->handle(CompilerException(returnToken.line, returnToken.column, ErrorCode::MissingReturnValue));
            }
        }

        // TODO: after return check
        if (this->lexer->peekToken() != TKTYPE::SEMICN) {
            // [wrong Stmt]need ;
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
        } else {
            this->lexer->nextToken();
        }
    } else if (this->lexer->peekToken() == TKTYPE::PRINTFTK) {
        Token&& printf_token = this->lexer->nextToken();
        Token str_const_token;

        if (this->lexer->nextToken() != TKTYPE::LPARENT) {
            // [wrong Stmt]need (");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        }
        // TODO:check printf here

        if ((str_const_token = this->lexer->peekToken()) != TKTYPE::STRCON) {
            // [wrong Stmt]need format string");
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::OtherError);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::OtherError));
        } else {
            this->lexer->nextToken();
        }

        std::vector<VarInf> params;
        while (this->lexer->peekToken() == TKTYPE::COMMA) {
            this->lexer->nextToken();
            this->Exp(false, ret_var);
            params.push_back(ret_var);
        }

        this->ir_module->setPrintfStatement(str_const_token.value, params, &error_code);

        if (error_code != ErrorCode::None) {
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, error_code));
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

        Token ident = this->lexer->nextToken(); // read identifier
        while (this->lexer->peekToken() == TKTYPE::LBRACK) { // check '['
            this->lexer->nextToken(); // read '['
            this->ConstExp(ret_var);
            if (this->lexer->peekToken() == TKTYPE::RBRACK) // check ']'
                this->lexer->nextToken();
        }

        if (this->lexer->peekToken() == TKTYPE::ASSIGN) {
            // rollback lexer
            this->lexer->discharge();
            this->exceptionController->discharge();
            this->sout = nullptr;

            Token ident = this->lexer->nextToken(); // read identifier
            this->LVal(ident, true, false, ret_var);
            this->lexer->nextToken(); // read '='

            VarInf temp_var;
            if (this->lexer->peekToken() == TKTYPE::GETINTTK) {
                this->lexer->nextToken(); // read "getint"

                temp_var = this->ir_module->callThirdPartyFunction("getint", {}, nullptr);
                this->ir_module->calculate("ASSIGN", ret_var, temp_var, &error_code);
                if (error_code != ErrorCode::None) {
                    this->exceptionController->handle(CompilerException(ident.line, ident.column, error_code));
                }

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
                this->Exp(false, temp_var);

                this->ir_module->calculate("ASSIGN", ret_var, temp_var, &error_code);
                if (error_code != ErrorCode::None) {
                    this->exceptionController->handle(CompilerException(ident.line, ident.column, error_code));
                }

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
            this->Exp(false, ret_var);

            if (this->lexer->peekToken() != TKTYPE::SEMICN)
                // [wrong FuncDef]need ;");
                // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon);
                this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingSemicolon));
            else
                this->lexer->nextToken();
        }
    } else if (this->lexer->peekToken() == TKTYPE::LBRACE) {
        this->ir_module->setClosureBlock(block_type, empty_condition, empty_inc, block_name);
        this->Block();
        this->ir_module->outClosureBlock();
    } else {
        this->Exp(false, ret_var);
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

void Parser::LVal(const Token& ident, bool is_left_value, bool constCheck, VarInf& ret_var)
{
    int result;
    std::vector<VarInf> indexes;
    ErrorCode error_code;

    while (this->lexer->peekToken() == TKTYPE::LBRACK) {
        this->lexer->nextToken(); /// read '['

        this->Exp(constCheck, ret_var);

        indexes.push_back(ret_var);

        if (this->lexer->peekToken() != TKTYPE::RBRACK)
            // parser_exception_handler(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket);
            this->exceptionController->handle(CompilerException(this->lexer->line, this->lexer->column, ErrorCode::MissingCloseSquareBracket));
        else
            this->lexer->nextToken(); // ]
    }

    ret_var = this->ir_module->getVariableRegister(is_left_value, ident.value, indexes, &error_code);

    if (error_code != ErrorCode::None) {
        this->exceptionController->handle(CompilerException(ident.line, ident.column, error_code));
    }

#ifdef parser_debug_output
    parser_output(<LVal>);
#endif
}

void Parser::RelExp(VarInf& ret_var)
{
    VarInf temp_var;
    ErrorCode error_code;
    this->AddExp(false, ret_var);
#ifdef parser_debug_output
    parser_output(<RelExp>);
#endif
    while (this->lexer->peekToken() == TKTYPE::LSS
        || this->lexer->peekToken() == TKTYPE::GRE
        || this->lexer->peekToken() == TKTYPE::LEQ
        || this->lexer->peekToken() == TKTYPE::GEQ) {
        Token&& temp_token = this->lexer->nextToken();
        this->AddExp(false, temp_var);
        switch (temp_token.type) {
        case TKTYPE::LSS:
            ret_var = this->ir_module->calculate("LSS", ret_var, temp_var, &error_code);
            break;
        case TKTYPE::GRE:
            ret_var = this->ir_module->calculate("GRE", ret_var, temp_var, &error_code);
            break;
        case TKTYPE::LEQ:
            ret_var = this->ir_module->calculate("LEQ", ret_var, temp_var, &error_code);
            break;
        case TKTYPE::GEQ:
            ret_var = this->ir_module->calculate("GEQ", ret_var, temp_var, &error_code);
            break;
        default:
            break;
        }
        if (error_code != ErrorCode::None) {
            this->exceptionController->handle(CompilerException(temp_token.line, temp_token.column, error_code));
        }
#ifdef parser_debug_output
        parser_output(<RelExp>);
#endif
    }
}

void Parser::EqExp(VarInf& ret_var)
{
    this->RelExp(ret_var);
#ifdef parser_debug_output
    parser_output(<EqExp>);
#endif
    VarInf temp_var;
    ErrorCode error_code;
    while (this->lexer->peekToken() == TKTYPE::EQL
        || this->lexer->peekToken() == TKTYPE::NEQ) {
        auto&& temp_token = this->lexer->nextToken();
        this->RelExp(temp_var);
        switch (temp_token.type) {
        case TKTYPE::EQL:
            ret_var = this->ir_module->calculate("EQL", ret_var, temp_var, &error_code);
            break;
        case TKTYPE::NEQ:
            ret_var = this->ir_module->calculate("NEQ", ret_var, temp_var, &error_code);
            break;
        default:
            break;
        }
        if (error_code != ErrorCode::None) {
            this->exceptionController->handle(CompilerException(temp_token.line, temp_token.column, error_code));
        }
#ifdef parser_debug_output
        parser_output(<EqExp>);
#endif
    }
    if (ret_var.type == VarType::i32) {
        ret_var = this->ir_module->calculate("NEQ", ret_var, VarInf { VarType::i32, "0" }, &error_code);
    } else if (ret_var.type.type == VarType::CONSTANT) {
        ret_var.name = ret_var.name.compare("0") != 0 ? "1" : "0";
    }
}

void Parser::LAndExp(VarInf& ret_var, const std::string& label_true, BasicBlock::Type block_true_type, const std::string& label_false, BasicBlock::Type block_false_type)
{
    this->EqExp(ret_var);
    unsigned int length = 0;
    std::string land_label_number = label_true + ".land";

#ifdef parser_debug_output
    parser_output(<LAndExp>);
#endif
    while (this->lexer->peekToken() == TKTYPE::AND) {
        // and prevent
        this->ir_module->setBranchStatement(ret_var, land_label_number + std::to_string(++length), block_true_type, label_false, block_false_type);
        this->ir_module->setNextBasicBlock(land_label_number + std::to_string(length), block_true_type);

        this->lexer->nextToken();
        this->EqExp(ret_var);
#ifdef parser_debug_output
        parser_output(<LAndExp>);
#endif
    }
}

void Parser::LOrExp(VarInf& ret_var, const std::string& label_true, BasicBlock::Type block_true_type, const std::string& label_false, BasicBlock::Type block_false_type)
{
    this->sout = &this->outBuffer;
    this->lexer->hold(&this->outBuffer);
    this->exceptionController->hold();
    this->ir_module->pre_read_mode = true;

    this->LAndExp(ret_var, label_true, block_true_type, label_false, block_false_type);
    bool has_or = this->lexer->peekToken() == TKTYPE::OR;

    this->lexer->discharge();
    this->exceptionController->discharge();
    this->sout = nullptr;
    this->ir_module->pre_read_mode = false;

    unsigned int length = 1;
    std::string lor_label_true = label_true + ".lor";
    std::string lor_label_false = label_false + ".lor";
    this->LAndExp(ret_var, has_or ? lor_label_true + '1' : label_true, block_true_type, has_or ? lor_label_false + '1' : label_false, block_false_type);

#ifdef parser_debug_output
    parser_output(<LOrExp>);
#endif
    while (has_or) {
        this->lexer->nextToken();

        this->ir_module->setBranchStatement(ret_var, label_true, block_true_type, lor_label_false + std::to_string(length), block_false_type);
        this->ir_module->setNextBasicBlock(lor_label_false + std::to_string(length), block_false_type);

        // preread to check if there is "or"
        this->sout = &this->outBuffer;
        this->lexer->hold(&this->outBuffer);
        this->exceptionController->hold();
        this->ir_module->pre_read_mode = true;

        this->LAndExp(ret_var, label_true, block_true_type, label_false, block_false_type);
        has_or = this->lexer->peekToken() == TKTYPE::OR;

        this->lexer->discharge();
        this->exceptionController->discharge();
        this->sout = nullptr;
        this->ir_module->pre_read_mode = false;

        length++;
        this->LAndExp(ret_var, lor_label_true + std::to_string(length), block_true_type, has_or ? lor_label_false + std::to_string(length) : label_false, block_false_type);

#ifdef parser_debug_output
        parser_output(<LOrExp>);
#endif
    }
}
