#include"../include/ast.h"

bool AstNode::operator==(const ASTNODETYPE& type)
{
    return this->type == static_cast<int>(type);
}
bool AstNode::operator==(const TKTYPE&type)
{
    return this->type == static_cast<int>(type);
}

bool AstNode::operator==(const Token& token)
{
    return this->type == static_cast<int>(token.type);
}

AstNode::AstNode(const Token &token)
{
    this->type == static_cast<int>(token.type);
    this->content = token.value;
    this->line = token.line;
    this->column = token.column;
}