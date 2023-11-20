#include "../../include/ir_gen/function.h"
#include "../../include/util/util.h"

VarType Function::getRegType(const std::string& reg_or_num, bool& is_num, int* value)
{
    if (reg_or_num.empty()) {
        is_num = false;
        return VarType(VarType::WRONG);
    }

    auto&& ite = reg_or_num.begin();
    unsigned int times = 0;
    int ret_value = 0;
    is_num = true;
    while (ite != reg_or_num.end() && (*ite == '+' || *ite == '-')) {
        ite++;
        times += *ite == '-';
    }
    while (ite != reg_or_num.end()) {
        if (!isdigit(*ite)) {
            is_num = false;
            break;
        }
        ret_value = ret_value * 10 + *ite - '0';
        ite++;
    }

    if (is_num) {
        if (value)
            *value = times & 1 ? -ret_value : ret_value;
        return VarType(VarType::i32);
    }

    auto&& reg_ite = this->reg_map.find(reg_or_num);

    if (reg_ite == this->reg_map.end()) {
        return VarType(VarType::WRONG);
    }

    return (*reg_ite).second;
}

// distribute a reg and declare the parameter as a local variable
bool Function::declareParam(const std::string& var_name, const VarType& var_type)
{
    // distribute a reg
    std::string&& reg = "%reg" + std::to_string(this->total_reg_quantity++);
    this->type.paramsType.push_back(var_type);
    this->reg_map.insert({ reg, var_type });

    return this->declareVariable(var_name, var_type, {var_type, reg});
}

// declare an array with initial values
// return false if redefine
bool Function::declareVariable(const std::string& var_name, const VarType& var_type, const std::vector<VarInf>& initial_value)
{
    if (var_type.array_degrees.size() == 0) {
        return this->declareVariable(var_name, var_type, initial_value.empty() ? VarInf{VarType::VOID} : initial_value.front());
    }
    VarType&& array_type = var_type.getVisit(-1, nullptr);
    std::string fix_var_name;
    bool check = this->var_closure_block_manager.insertVariable(array_type, '%' + var_name, fix_var_name);

    if (!check) {
        return false;
    }

    // store content in global
    if (var_type.is_const) {
        fix_var_name = this->func_name + '.' + fix_var_name.erase(0, 1);
        this->global_var_map.insert({ fix_var_name, { array_type, initial_value } });
        return check;
    }

    if (this->cur_block != this->entry_block)
    // if not constant
        this->entry_block->insertAlloca(AllocaIR(LlvmIR::ALLOCA, fix_var_name, var_type));
    else{
        this->cur_block->addIR(AllocaIR(LlvmIR::ALLOCA, fix_var_name, var_type));
    }


    if (initial_value.empty()) {
        return check;
    }

    // Note: initial array
    unsigned int offset = 0;
    std::vector<unsigned int> indexes(var_type.array_degrees.size());
    std::vector<VarInf> str_indexes(indexes.size() + 1, {VarType(VarType::i32), "0"});
    std::string reg;

    VarType&& gep_reg_type = VarType(var_type.type, var_type.pointer_depth + 1);

    unsigned int i;
    while (offset < initial_value.size() && indexes[0] < array_type.array_degrees[0]) {
        i = indexes.size() - 1;

        // distribute a reg
        reg = "%reg" + std::to_string(this->total_reg_quantity++);
        this->reg_map.insert({ reg, gep_reg_type });

        this->cur_block->addIR(GetElementIR(reg, LlvmIR::GETELEMENTPTR, array_type, fix_var_name, str_indexes));
        //Note:
        this->cur_block->addIR(StorageIR(LlvmIR::STORE, gep_reg_type, reg, initial_value[offset].type, initial_value[offset].name));

        indexes[i]++;
        while (indexes[i] >= array_type.array_degrees[i] && i > 0) {
            indexes[i] -= array_type.array_degrees[i];
            str_indexes[i + 1].name = std::to_string(indexes[i]);
            indexes[--i]++;
        }
        str_indexes[i + 1].name = std::to_string(indexes[i]);
        offset++;
    }
    return check;
}

// return false if redefine
bool Function::declareVariable(const std::string& var_name, const VarType& var_type, const VarInf& initial_reg_or_num)
{
    VarType&& var_type_pointer = var_type.getVisit(-1, nullptr);
    std::string fix_var_name;
    bool check = this->var_closure_block_manager.insertVariable(var_type_pointer, '%' + var_name, fix_var_name);

    if (!check) {
        return false;
    }

    // store the value in global
    if (var_type.is_const) {
        fix_var_name = this->func_name + '.' + fix_var_name.erase(0, 1);
        this->global_var_map.insert({ fix_var_name, { var_type_pointer, {initial_reg_or_num}}});
        return check;
    }


    if (this->entry_block != this->cur_block)
        this->entry_block->insertAlloca(AllocaIR(LlvmIR::ALLOCA, fix_var_name, var_type));
    else
        this->cur_block->addIR(AllocaIR(LlvmIR::ALLOCA, fix_var_name, var_type));


    if (initial_reg_or_num.type != VarType::VOID) {
        this->cur_block->addIR(StorageIR(LlvmIR::STORE, var_type_pointer, fix_var_name, initial_reg_or_num.type, initial_reg_or_num.name));
    }
    return check;
}

// TODO:
// set get_value tag to be true will generate load ir
VarInf Function::getVariableRegister(bool left_value_tag, const std::string& var_name, const std::vector<VarInf>& reg_or_num_indexes, ErrorCode* error_code)
{
    if (var_name.empty()) {
        if (error_code)
            *error_code = ErrorCode::OtherError;
        return VarInf{VarType::WRONG, ""};
    }

    // get variable address register
    VarType ret_var_type;
    auto&& fix_var_name = this->var_closure_block_manager.getCurBlock()->getVariable('%' + var_name, ret_var_type);
    if (error_code)
        *error_code = ErrorCode::None;
    // is const
    if (ret_var_type.type == VarType::CONSTANT) {
        return VarInf{ret_var_type, var_name};
    }
    // check global var
    bool global_var = false;
    if (ret_var_type.type == VarType::WRONG) {
        auto&& ite = this->global_var_map.find('@' + var_name);
        if (ite == this->global_var_map.end()) {
            if (error_code)
                *error_code = ErrorCode::Nodefine;
            return VarInf{VarType::WRONG, ""};
        } else {
            ret_var_type = (*ite).second.type;
            fix_var_name = '@' + var_name;
            global_var = true;
        }
    }

    // try to get value directly
    if (ret_var_type.is_const) {
        if (!global_var)
            fix_var_name = this->func_name + '.' + fix_var_name.erase(0, 1);
        GlobalVarInf& var_inf = this->global_var_map.at(fix_var_name);

        if (var_inf.type.getVisit(reg_or_num_indexes.size(), error_code).pointer_depth == 1) {
            if (reg_or_num_indexes.empty()) {
                //Note: add other constant type here
                return var_inf.initial_values.front();
            } else {
                std::vector<unsigned int> indexes = { 0 };
                int idx;
                bool check_const = true;
                for (auto&& index : reg_or_num_indexes) {
                    if (Util::stringToInt(index.name, idx)) {
                        indexes.push_back(idx);
                    } else {
                        check_const = false;
                        break;
                    }
                }
                if (check_const) {
                    var_inf.getVisit(indexes, &fix_var_name, error_code);
                    return VarInf{VarType(), fix_var_name};
                }
            }
        }
    }

    // handle address
    if (left_value_tag && reg_or_num_indexes.empty()) {
        return VarInf{ret_var_type, fix_var_name};
    }
    ErrorCode check_bound;
    VarType&& cal_type = ret_var_type.getVisit(reg_or_num_indexes.size() + 1, &check_bound);
    if (check_bound != ErrorCode::None) {
        if (error_code)
            *error_code = check_bound;
        return VarInf{VarType(VarType::WRONG), ""};
    }

    std::string reg;
    std::vector<VarInf> indexes = reg_or_num_indexes;
    if (ret_var_type.needLoadFirst()) {
        reg = "%reg" + std::to_string(this->total_reg_quantity++);
        VarType && temp_type = ret_var_type.getVisit(1, nullptr);
        this->reg_map.insert({reg, temp_type});
        this->cur_block->addIR(StorageIR(LlvmIR::LOAD, ret_var_type, fix_var_name, temp_type, reg));
        fix_var_name = reg;
        ret_var_type = temp_type;
    } else if (!ret_var_type.array_degrees.empty()) {
        indexes.emplace(indexes.begin(), VarInf{VarType::i32, "0"});
    }
    if (cal_type.needEndZero()) {
        indexes.push_back(VarInf{VarType::i32, "0"});
        cal_type.toPointer();
    }

    if (!indexes.empty()) {
        reg = "%reg" + std::to_string(this->total_reg_quantity++);
        this->cur_block->addIR(GetElementIR(reg, LlvmIR::GETELEMENTPTR, ret_var_type, fix_var_name, indexes));
        ret_var_type = ret_var_type.getVisit(indexes.size() - 1, nullptr);
        ret_var_type.toPointer();
        this->reg_map.insert({reg, ret_var_type});
        fix_var_name = reg;
    }

    if (!left_value_tag && cal_type.pointer_depth == 0 && cal_type.array_pointer_depth == 0 && cal_type.array_degrees.empty()) {
        reg = "%reg" + std::to_string(this->total_reg_quantity++);
        VarType && temp_type = ret_var_type.getVisit(1, nullptr);
        this->reg_map.insert({reg, cal_type});
        this->cur_block->addIR(StorageIR(LlvmIR::LOAD, ret_var_type, fix_var_name, temp_type, reg));
        ret_var_type = temp_type;
    }

    return VarInf{ret_var_type, reg};
}

VarInf Function::calculate(const std::string& action, const VarInf& reg_or_num1, const VarInf& reg_or_num2, ErrorCode* error_code)
{
    if (reg_or_num1.type == VarType::WRONG || reg_or_num2.type == VarType::WRONG) {
        // Note: check this
        if (error_code) *error_code = ErrorCode::OtherError;
        return VarInf{VarType::WRONG};
    }

    // initial
    if (error_code)
        *error_code = ErrorCode::None;

    if (action.compare("ASSIGN") == 0) {
        // Note: check modify const and assign type
        if (reg_or_num1.type.is_const || reg_or_num1.type.type == VarType::CONSTANT) {
            if (error_code)
                *error_code = ErrorCode::ModifyConst;
        } else if (reg_or_num1.type.getVisit(1, nullptr) != reg_or_num2.type) {
            if (error_code && *error_code == ErrorCode::None)
                *error_code = ErrorCode::CalculateTypeNotMatch;
        } else {
            this->cur_block->addIR(StorageIR(LlvmIR::STORE, reg_or_num1.type, reg_or_num1.name, reg_or_num2.type, reg_or_num2.name));
        }
        return VarInf{VarType::VOID};
    }


    bool is_num1, is_num2;
    int value1, value2;
    is_num1 = Util::stringToInt(reg_or_num1.name, value1);
    is_num2 = Util::stringToInt(reg_or_num2.name, value2);
    VarType var_type1 = reg_or_num1.type;
    VarType var_type2 = reg_or_num2.type;

    // select ir_action
    LlvmIR::Action ir_action;
    if (action.compare("PLUS") == 0) {
        ir_action = LlvmIR::ADD;
    } else if (action.compare("MINUS") == 0) {
        ir_action = LlvmIR::SUB;
    } else if (action.compare("MULT") == 0) {
        ir_action = LlvmIR::MUL;
    } else if (action.compare("DIV") == 0) {
        ir_action = LlvmIR::SDIV;
    } else if (action.compare("MOD") == 0) {
        ir_action = LlvmIR::SREM;
    } else if (action.compare("NOT") == 0) {
        ir_action = LlvmIR::ICMP_EQ;
        is_num2 = true;
        value2 = 0;
        var_type2 = VarType(VarType::i1);
    } else if (action.compare("EQL") == 0) {
        ir_action = LlvmIR::ICMP_EQ;
    } else if (action.compare("NEQ") == 0) {
        ir_action = LlvmIR::ICMP_NE;
    } else if (action.compare("GRE") == 0) {
        ir_action = LlvmIR::ICMP_SGT;
    } else if (action.compare("LSS") == 0) {
        ir_action = LlvmIR::ICMP_SLT;
    } else if (action.compare("LEQ") == 0) {
        ir_action = LlvmIR::ICMP_SLE;
    } else if (action.compare("GEQ") == 0) {
        ir_action = LlvmIR::ICMP_SGE;
    } else if (action.compare("AND") == 0) {
        ir_action = LlvmIR::AND;
    } else if (action.compare("OR") == 0) {
        ir_action = LlvmIR::OR;
    }

    // error check

    if (var_type1 != var_type2) {
        if (error_code)
            *error_code = ErrorCode::CalculateTypeNotMatch;
        return VarInf{VarType::WRONG};
    }
    std::string ret_reg;
    // handle ir
    if (!is_num1 || !is_num2) {
        ret_reg = "%reg" + std::to_string(this->total_reg_quantity++);
        this->cur_block->addIR(CalculateIR(ret_reg, ir_action, is_num1 ? var_type2 : var_type1, reg_or_num1.name, reg_or_num2.name));
    }
    // return value directly
    switch (ir_action) {
    case LlvmIR::ADD:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 + value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    case LlvmIR::SUB:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 - value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    case LlvmIR::MUL:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 * value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    case LlvmIR::SDIV:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 / value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    case LlvmIR::SREM:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 % value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    case LlvmIR::ICMP_EQ:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 == value2);
        } else {
            this->reg_map.insert({ ret_reg, VarType(VarType::i1) });
        }
        break;
    case LlvmIR::ICMP_NE:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 != value2);
        } else {
            this->reg_map.insert({ ret_reg, VarType(VarType::i1) });
        }
        break;
    case LlvmIR::ICMP_SGT:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 > value2);
        } else {
            this->reg_map.insert({ ret_reg, VarType(VarType::i1) });
        }
        break;
    case LlvmIR::ICMP_SLT:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 < value2);
        } else {
            this->reg_map.insert({ ret_reg, VarType(VarType::i1) });
        }
        break;
    case LlvmIR::ICMP_SLE:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 <= value2);
        } else {
            this->reg_map.insert({ ret_reg, VarType(VarType::i1) });
        }
        break;
    case LlvmIR::ICMP_SGE:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 >= value2);
        } else {
            this->reg_map.insert({ ret_reg, VarType(VarType::i1) });
        }
        break;
    case LlvmIR::AND:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 && value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    case LlvmIR::OR:
        if (is_num1 && is_num2) {
            ret_reg = std::to_string(value1 || value2);
        } else {
            this->reg_map.insert({ ret_reg, var_type1.type == VarType::CONSTANT ? var_type2 : var_type1 });
        }
        break;
    default:
        break;
    }

    return {is_num1 && is_num2 ? VarType::CONSTANT : this->reg_map.at(ret_reg), ret_reg};
}

VarInf Function::callFunction(const std::string& func_name, const std::vector<VarInf>& params, ErrorCode* error_code)
{
    auto&& ite = this->function_map.find(func_name);
    if (ite == this->function_map.end()) {
        if (error_code)
            *error_code = ErrorCode::Nodefine;
        return {VarType::WRONG};
    }
    if (error_code)
        *error_code = ErrorCode::None;
    Function& callee = (*ite).second;
    if (params.size() != callee.type.paramsType.size()) {
        if (error_code)
            *error_code = ErrorCode::WrongParameterNumber;
    }
    bool is_num;
    for (unsigned int i = 0; i < params.size(); i++) {
        if (params[i].type != callee.type.paramsType[i]) {
            if (error_code)
                *error_code = ErrorCode::WrongParameterType;
            return {VarType::WRONG};
        }
    }
    if (callee.type.retType.type != VarType::VOID) {
        std::string reg = "%reg" + std::to_string(this->total_reg_quantity++);
        this->reg_map.insert({ reg, callee.type.retType });
        this->cur_block->addIR(CallIR(reg, LlvmIR::CALL, callee.type, callee.func_name, params));
        return {callee.type.retType, reg};
    } else {
        this->cur_block->addIR(CallIR(LlvmIR::CALL, callee.type, callee.func_name, params));
        return {VarType::VOID};
    }


}

void Function::setNextBasicBlock(const std::string& name, BasicBlock::Type type)
{
    std::string&& fix_label_name = BasicBlock::fixLabelName(name, type);
    this->need_ret = true;
    this->basicBlocks.insert({ fix_label_name, BasicBlock(name, type, this->fout) });
    this->cur_block = &basicBlocks[fix_label_name];
}

void Function::setReturnStatement(const VarInf& reg_or_num, ErrorCode* error_code)
{
    if (error_code)
        *error_code = ErrorCode::None;

    if (reg_or_num.type != this->type.retType) {
        if (error_code)
            *error_code = ErrorCode::OtherError;
    }
    this->cur_block->addIR(RetIR(LlvmIR::RET, reg_or_num.type, reg_or_num.name));
    this->need_ret = false;
}

void Function::setReturnStatement(ErrorCode* error_code)
{
    if (error_code)
        *error_code = ErrorCode::None;
    if (VarType::VOID != this->type.retType.type) {
        if (error_code)
            *error_code = ErrorCode::MissingReturnValue;
    }
    this->cur_block->addIR(RetIR(LlvmIR::RET));
    this->need_ret = false;
}

void Function::setBranchStatement(const VarInf& condition, const std::string& label1, BasicBlock::Type block_type1, const std::string& label2, BasicBlock::Type block_type2)
{
    int ret_value;
    bool is_num;
    // VarType&& condition_type = this->getRegType(condition, is_num, &ret_value);
    is_num = Util::stringToInt(condition.name, ret_value);
    if (is_num) {
        ret_value ? this->setBranchStatement(label1, block_type1) : this->setBranchStatement(label2, block_type2);
        return;
    }

    std::string&& fix_label1 = BasicBlock::fixLabelName(label1, block_type1);
    std::string&& fix_label2 = BasicBlock::fixLabelName(label2, block_type2);


    this->cur_block->addIR(BranchIR(LlvmIR::BR, condition.name, '%' + fix_label1, '%' + fix_label2));
    this->need_ret = false;
}

void Function::setBranchStatement(const std::string& label, BasicBlock::Type block_type)
{
    std::string&& fix_label = BasicBlock::fixLabelName(label, block_type);

    this->cur_block->addIR(BranchIR(LlvmIR::BR, '%' + fix_label));
    this->need_ret = false;
}

VarInf Function::callThirdPartyFunction(const std::string& func_name, const std::vector<VarInf>& params, ErrorCode* error_code)
{
    if (error_code)
        *error_code = ErrorCode::None;
    if (func_name.compare("@getint") == 0) {
        if (!params.empty()) {
            if (error_code)
                *error_code = ErrorCode::WrongParameterNumber;
            return {VarType::WRONG};
        } else {
            std::string reg = "%reg" + std::to_string(this->total_reg_quantity++);
            this->reg_map.insert({ reg, VarType(VarType::i32) });
            this->cur_block->addIR(CallIR(reg, LlvmIR::CALL, FunctionType{VarType::i32, {}}, func_name, {}));
            return {VarType::i32, reg};
        }
    } else if (func_name.compare("@putint") == 0) {
        if (params.size() != 1) {
            if (error_code) *error_code = ErrorCode::WrongParameterNumber;
            return {VarType::WRONG};
        } else if (params.front().type != VarType::i32){
            if (error_code) *error_code = ErrorCode::WrongParameterType;
            return {VarType::WRONG};
        } else {
            this->cur_block->addIR(CallIR(LlvmIR::CALL, {VarType::VOID, {VarType::i32}}, func_name, params));
            return {VarType::VOID};
        }
    } else if (func_name.compare("@putstr") == 0) {
        if (params.size() != 1) {
            if (error_code) *error_code = ErrorCode::WrongParameterNumber;
            return {VarType::WRONG};
        } else if (params.front().type != VarType::i8){
            if (error_code) *error_code = ErrorCode::WrongParameterType;
            return {VarType::WRONG};
        } else {
            this->cur_block->addIR(CallIR(LlvmIR::CALL, {VarType::VOID, {VarType::i32}}, func_name, params));
            return {VarType::VOID};
        }
    } else if (func_name.compare("@putch") == 0) {
        if (params.size() != 1) {
            if (error_code) *error_code = ErrorCode::WrongParameterNumber;
            return {VarType::WRONG};
        } else if (params.front().type != VarType::i32){
            if (error_code) *error_code = ErrorCode::WrongParameterType;
            return {VarType::WRONG};
        } else {
            this->cur_block->addIR(CallIR(LlvmIR::CALL, {VarType::VOID, {VarType::i32}}, func_name, params));
            return {VarType::VOID};
        }
    } else {
        if (error_code)
            *error_code = ErrorCode::Nodefine;
    }

    return {VarType::WRONG};
}

void Function::setPrintfStatement(const std::string& format_string, const std::vector<VarInf>& params, ErrorCode* error_code)
{
    auto i = format_string.begin();
    // GlobalVarInf str_var_inf = {VarType(true, VarType::i8)};
    // VarInf initial_str_value = {VarType(true, VarType::i8)};
    // str_var_inf.type.array_pointer_depth = initial_str_value.type.array_pointer_depth = 1;
    // std::string name;
    // std::string var;
    // unsigned int length = 0;
    int num = 0;
    while (i != format_string.end()) {
        if (*i == '%') {
            // if (length){
            //     initial_str_value.name += "\\00";
            //     initial_str_value.type.array_degrees = str_var_inf.type.array_degrees = {length + 1};
            //     str_var_inf.initial_values = {initial_str_value};
            //     name = this->func_name + ".str." + std::to_string(this->getLabelNumber("str"));
            //     this->global_var_map.insert({name, str_var_inf});
            //     var = "%reg" + std::to_string(this->total_reg_quantity++);
            //     this->cur_block->addIR(GetElementIR(var, LlvmIR::GETELEMENTPTR, initial_str_value.type, name, std::vector<VarInf>(2, {VarType::i32, "0"})));
            //     this->callThirdPartyFunction("@putstr", {VarInf{VarType(VarType::i8, 1), var}}, error_code);
            //     initial_str_value.name = "";
            //     length = 0;
            // }
            i++;
            if (*i == 'd') {
                i++;
                if (this->callThirdPartyFunction("@putint", {params[num++]}, error_code).type == VarType::WRONG) {
                    return;
                }
            } else {
                if (error_code) {
                    *error_code = ErrorCode::IllegalCharInFormatString;
                }
                return;
            }
            if (i == format_string.end()) {
                break;
            }
        } else if (*i == '\\') {
            i++;
            if (*i == 'n') {
                // initial_str_value.name += "\\0A";
                // length++;
                if (this->callThirdPartyFunction("@putch", {VarInf{VarType::i32, std::to_string(10)}}, error_code).type == VarType::WRONG) {
                    return;
                }
                i++;
            } else {
                if (error_code) {
                    *error_code = ErrorCode::IllegalCharInFormatString;
                }
                return;
            }
            if (i == format_string.end()) {
                break;
            }
        } else if (*i == ' ' || *i == '!' || (*i >= 40 && *i <= 126)) {
            // initial_str_value.name += *i;
            if (this->callThirdPartyFunction("@putch", {VarInf{VarType::i32, std::to_string((int)(*i))}}, error_code).type == VarType::WRONG) {
                return;
            }
            i++;
        } else {
            if (error_code) {
                *error_code = ErrorCode::IllegalCharInFormatString;
                return;
            }
        }
    }

    // if (length){
    //     initial_str_value.name += "\\00";
    //     initial_str_value.type.array_degrees = str_var_inf.type.array_degrees = {length + 1};
    //     str_var_inf.initial_values = {initial_str_value};
    //     name = this->func_name + ".str." + std::to_string(this->getLabelNumber("str"));
    //     this->global_var_map.insert({name, str_var_inf});
    //     var = "%reg" + std::to_string(this->total_reg_quantity++);
    //     this->cur_block->addIR(GetElementIR(var, LlvmIR::GETELEMENTPTR, initial_str_value.type, name, std::vector<VarInf>(2, {VarType::i32, "0"})));
    //     this->callThirdPartyFunction("@putstr", {VarInf{VarType(VarType::i8, 1), var}}, error_code);
    // }
    if (num != params.size()) {
        if (error_code) {
            *error_code = ErrorCode::WrongParameterNumber;
        }
        return;
    }

    return;
}

void Function::print()
{
    (*this->fout) << "define dso_local " << this->type.retType.toString() << ' ' << this->func_name << '(';
    unsigned int num = 0;
    if (!this->type.paramsType.empty()) {
        (*this->fout) << this->type.paramsType[0].toString() << " %reg" << num;
        num ++;
        for (unsigned int i = 1; i < this->type.paramsType.size(); i++, num++) {
            (*this->fout) << ", " << this->type.paramsType[i].toString() << " %reg" << num;
        }
    }
    (*this->fout) << "){" << std::endl;

    for (auto&& block_pair : basicBlocks) {
        block_pair.second.print();
    }

    (*this->fout) << '}' << std::endl;
}