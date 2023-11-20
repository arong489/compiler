#include "../../include/ir_gen/instruction.h"

std::string CalculateIR::toString() const
{
    std::string ans;
    if (!result_reg.empty()) ans = result_reg + " = ";
    switch (this->action) {
    case LlvmIR::ADD:       ans += "add ";break;
    case LlvmIR::SUB:       ans += "sub ";break;
    case LlvmIR::MUL:       ans += "mul ";break;
    case LlvmIR::SREM:      ans += "srem ";break;
    case LlvmIR::SDIV:      ans += "sdiv ";break;
    case LlvmIR::OR:        ans += "or ";break;
    case LlvmIR::AND:       ans += "and ";break;
    case LlvmIR::ICMP_NE:   ans += "icmp ne ";break;
    case LlvmIR::ICMP_EQ:   ans += "icmp eq ";break;
    case LlvmIR::ICMP_SGT:  ans += "icmp sgt ";break;
    case LlvmIR::ICMP_SLT:  ans += "icmp slt ";break;
    case LlvmIR::ICMP_SGE:  ans += "icmp sge ";break;
    case LlvmIR::ICMP_SLE:  ans += "icmp sle ";break;
    default: throw -1;
    }
    ans += cal_type.toString() + ' ' + reg_or_num1 + ", " + reg_or_num2;
    return ans;
}

std::string BranchIR::toString() const
{
    std::string ans = "br ";
    if (condition.compare("") == 0) {
        ans += "label " + label_true;
    } else {
        ans += "i1 " + condition + ", label " + label_true + ", label " + label_false;
    }
    return ans;
}

std::string RetIR::toString() const
{
    std::string ans = ret_value.empty() ? "ret void" : "ret " + ret_type.toString() + ' ' + ret_value;
    return ans;
}

std::string StorageIR::toString() const
{
    std::string ans;
    switch (action)
    {
    case LlvmIR::LOAD:
        ans = value_reg + " = load " + value_type.toString() + ", " + pointer_type.toString() + ' ' + pointer_reg;
        break;
    case LlvmIR::STORE:
        ans = "store " + value_type.toString() + ' ' + value_reg + ", " + pointer_type.toString() + ' ' + pointer_reg;
        break;
    default:
        throw -1;
    }
    return ans;
}

std::string AllocaIR::toString() const
{
    std::string ans = result_reg + " = alloca " + alloca_type.toString();
    return ans;
}

std::string GetElementIR::toString() const
{
    VarType&& temp = this->pointer_type.getVisit(1, nullptr);
    std::string ans = result_reg + " = getelementptr " + temp.toString() + ", " + this->pointer_type.toString() + ' ' + pointer_reg;
    for (unsigned int i = 0; i < indexes.size(); i++)
    {
        ans += ", " + indexes[i].type.toString() + ' ' + indexes[i].name;
    }
    return ans;
}

std::string CallIR::toString() const
{
    std::string ans;
    if (result.compare("") != 0) {
        ans = result + " = call " + this->function_type.retType.toString() + ' ';
    } else {
        ans = "call " + this->function_type.retType.toString() + ' ';
    }
    ans += function + '(';
    if (params.empty()){
        ans += ')';
    }else{
        ans += params[0].type.toString() + ' ' + params[0].name;
        for (unsigned int i = 1; i < params.size(); i++) {
            ans += ", " + params[i].type.toString() + ' ' + params[i].name;
        }
        ans += ')';
    }
    return ans;
}

