#include "../../include/signTable/myType.h"

const MyType MyType::calculate(const Token& op, const MyType& another) const
{
    if (this->varType == VarType::WrongType || another.varType == VarType::WrongType) {
        return MyType(VarType::WrongType);
    }
    MyType temp;
    switch (op.type) {
    case TKTYPE::LBRACK:
        if (this->pointerDepth + this->degrees.size() == 0 || another.degrees.size() + another.pointerDepth != 0) {
            throw ErrorCode::CalculateTypeNotMatch;
        }
        temp = *this;
        if (this->pointerDepth > 0) {
            temp.pointerDepth--;
        } else {
            temp.degrees.erase(temp.degrees.begin());
        }
        break;
    case TKTYPE::MINUS:
        // 右边操作数一定为数字
        if (another.degrees.size() + another.pointerDepth != 0) {
            throw ErrorCode::CalculateTypeNotMatch;
        }
    case TKTYPE::PLUS:
        if (this->pointerDepth + this->degrees.size() != 0 && another.degrees.size() + another.pointerDepth != 0) {
            throw ErrorCode::CalculateTypeNotMatch;
        }
        temp = this->pointerDepth + this->degrees.size() != 0 ? *this : another;
        temp.isConst = this->isConst && another.isConst;
        break;
    case TKTYPE::MULT:
    case TKTYPE::DIV:
    case TKTYPE::MOD:
    case TKTYPE::EQL:
    case TKTYPE::NEQ:
    case TKTYPE::GRE:
    case TKTYPE::LSS:
    case TKTYPE::LEQ:
    case TKTYPE::GEQ:
    case TKTYPE::AND:
    case TKTYPE::OR:
        if (this->pointerDepth + this->degrees.size() != 0 || another.degrees.size() + another.pointerDepth != 0) {
            throw ErrorCode::CalculateTypeNotMatch;
        }
        temp = *this;
        temp.isConst &= another.isConst;
        break;
    default:
        throw ErrorCode::CalculateTypeNotMatch;
    }
    return temp;
}
const MyType MyType::calculate(const Token& op) const
{
    if (this->varType == VarType::WrongType) {
        return MyType(VarType::WrongType);
    }
    switch (op.type) {
    case TKTYPE::PLUS:
    case TKTYPE::MINUS:
    case TKTYPE::NOT:
        if (this->degrees.size() + this->pointerDepth != 0) {
            throw ErrorCode::CalculateTypeNotMatch;
        }
        return *this;

    default:
        throw ErrorCode::CalculateTypeNotMatch;
    }
}
int MyType::assignCheck(const MyType& another) const
{
    if (this->varType == VarType::WrongType || another.varType == VarType::WrongType) {
        return 0;
    }
    if (this->isConst) {
        // 常值不可改
        return 1;
    }
    if (this->varType != another.varType) {
        // 基础类型不匹配
        return 2;
    }
    if (this->pointerDepth == 0 && this->degrees.size() != 0) {
        // 不能对数组标签赋值
        return 3;
    }
    if (this->degrees.size() + this->pointerDepth != another.degrees.size() + another.pointerDepth) {
        // 维数不匹配
        return 4;
    }
    if (this->pointerDepth != another.pointerDepth && this->pointerDepth != another.pointerDepth + 1) {
        // 指针类型不匹配
        return 5;
    }
    auto i = this->degrees.begin(), j = another.degrees.begin();
    if (another.pointerDepth - this->pointerDepth) {
        j++;
    }
    for (; i != this->degrees.end(); i++, j++) {
        if (*i != *j) {
            // 维度不匹配
            return 6;
        }
    }
    return 0;
}

int VariableInf::assignCheck(const MyType& value) const
{
    return this->type.assignCheck(value);
}
bool VariableInf::getValue(const std::vector<int>& indexes, int& feedback)
{
    int index = 0;

    auto i = this->type.degrees.begin();
    auto j = indexes.begin();
    while (i != this->type.degrees.end()) {
        index = index * (*i) + (*j);
        i++;
        j++;
    }
    if (this->initialValues.size() <= index)
        return false;
    else {
        feedback = this->initialValues.at(index);
        return true;
    }
}
int FunctionInf::parametersCheck(const std::vector<MyType>& parameters) const
{
    // if (this->ambiguous) {
    //     return 0;
    // }
    if (this->parameters.size() != parameters.size()) {
        return 1;
    }
    auto i = this->parameters.begin();
    auto j = parameters.begin();
    while (i != this->parameters.end()) {
        if ((*(*i)).assignCheck(*j) != 0) {
            return 2;
        }
        i++;
        j++;
    }
    return 0;
}
