#include "../../include/ir_gen/module.h"
#include "../../include/util/util.h"


bool Module::delcareFunction(const std::string& func_name, const VarType& ret_type)
{
    if (this->pre_read_mode) {
        return true;
    }

    std::string fix_func_name = "@" + func_name;
    bool ret_bool = true;
    auto&& ite1 = this->global_var_infs.find(fix_func_name);
    if (ite1 != this->global_var_infs.end()) {
        ret_bool = false;
    }
    auto&& ite2 = this->functions.find(fix_func_name);
    if (ite2 != this->functions.end()) {
        ret_bool = false;
    }
    if (ret_bool){
        this->functions.insert({ fix_func_name, Function(this->global_var_infs, this->functions, fix_func_name, ret_type, this->fout) });
        this->cur_function = & this->functions.at(fix_func_name);
    } else {
        this->cur_function = new Function(this->global_var_infs, this->functions, fix_func_name, ret_type, this->fout);
    }
    return ret_bool;
}
bool Module::getFuction(const std::string& func_name, Function* ret_function)
{

    if (this->pre_read_mode) {
        return true;
    }

    std::string fix_func_name = "@" + func_name;

    auto&& ite = this->functions.find(fix_func_name);
    if (ite == this->functions.end()) {
        return false;
    } else {
        if (ret_function)
            ret_function = &(*ite).second;
        return true;
    }
}

VarInf Module::callFunction(const std::string& func_name, const std::vector<VarInf>& params, ErrorCode* error_code)
{

    if (this->pre_read_mode) {
        return {VarType::CONSTANT, "0"};
    }

    return this->cur_function->callFunction("@"+func_name, params, error_code);
}

VarInf Module::callThirdPartyFunction(const std::string& func_name, const std::vector<VarInf>& params, ErrorCode* error_code)
{
    if (this->pre_read_mode) {
        return {VarType::CONSTANT ,"0"};
    }

    return this->cur_function->callThirdPartyFunction("@"+func_name, params, error_code);
}

bool Module::declareVariable(const std::string& var_name, const VarType& var_type, const std::vector<VarInf>& initial_regs_or_nums)
{

    if (this->pre_read_mode) {
        return true;
    }

    if (this->cur_function) {
        return this->cur_function->declareVariable(var_name, var_type, initial_regs_or_nums);
    }

    std::string fix_var_name = '@' + var_name;

    auto&& ite1 = this->global_var_infs.find(fix_var_name);
    if (ite1 != this->global_var_infs.end()) {
        return false;
    }
    auto&& ite2 = this->functions.find(fix_var_name);
    if (ite2 != this->functions.end()) {
        return false;
    }

    this->global_var_infs.insert({ fix_var_name, { var_type.getVisit(-1, nullptr), initial_regs_or_nums } });
    return true;
}

VarInf Module::getVariableRegister(bool left_value_tag, const std::string& var_name, const std::vector<VarInf>& reg_or_num_indexes, ErrorCode* error_code)
{

    if (error_code) *error_code = ErrorCode::None;

    if (this->pre_read_mode) {
        return {VarType::CONSTANT, "0"};
    }

    if (this->cur_function) {
        return this->cur_function->getVariableRegister(left_value_tag, var_name, reg_or_num_indexes, error_code);
    }


    // when get var in global
    if (var_name.empty()) {
        return {VarType::WRONG, ""};
    }
    int value;
    if (Util::stringToInt(var_name, value)) {
        return {VarType::CONSTANT ,var_name};
    }

    auto&& ite = this->global_var_infs.find('@'+var_name);
    if (ite == this->global_var_infs.end()) {
        if (error_code)
            *error_code = ErrorCode::Nodefine;
        return {VarType::WRONG, ""};
    }

    if (!(*ite).second.type.is_const) {
        if (error_code)
            *error_code = ErrorCode::OtherError;
        return {VarType::WRONG, ""};
    }

    VarInf ret_var;
    if (reg_or_num_indexes.empty()) {
        return (*ite).second.initial_values.front();
    } else {
        std::vector<unsigned int> indexes = { 0 };
        int idx;
        bool check_const = true;
        for (auto&& index : reg_or_num_indexes) {
            check_const = Util::stringToInt(index.name, idx);
            if (!check_const) {
                break;
            }
            indexes.push_back(idx);
        }
        if (check_const) {
            ret_var.type = (*ite).second.getVisit(indexes, &ret_var.name, error_code);
            return ret_var;
        } else {
            if (error_code) *error_code = ErrorCode::OtherError;
            return {VarType::WRONG, ""};
        }
    }
}

// return if need return statement
bool Module::closeFunctionNeedRet()
{

    if (this->pre_read_mode) {
        return true;
    }

    return this->cur_function->ifNeedRet();
}

bool Module::declareParam(const std::string& var_name, const VarType& var_type)
{

    if (this->pre_read_mode) {
        return true;
    }

    return this->cur_function->declareParam(var_name, var_type);
}

VarInf Module::calculate(const std::string& action, const VarInf& reg_or_num1, const VarInf& reg_or_num2, ErrorCode* error_code)
{

    if (error_code) *error_code = ErrorCode::None;

    if (this->pre_read_mode) {
        return {VarType::CONSTANT, "0"};
    }

    return this->cur_function->calculate(action, reg_or_num1, reg_or_num2, error_code);
}
void Module::setNextBasicBlock(const std::string& name, BasicBlock::Type type)
{

    if (this->pre_read_mode) {
        return;
    }

    this->cur_function->setNextBasicBlock(name, type);
}
void Module::setReturnStatement(const VarInf& reg_or_num, ErrorCode* error_code)
{

    if (this->pre_read_mode) {
        return;
    }

    this->cur_function->setReturnStatement(reg_or_num, error_code);
}
void Module::setReturnStatement(ErrorCode* error_code)
{

    if (this->pre_read_mode) {
        return;
    }

    this->cur_function->setReturnStatement(error_code);
}

void Module::setBranchStatement(const VarInf& condition, const std::string& label1, BasicBlock::Type block_type1, const std::string& label2, BasicBlock::Type block_type2)
{

    if (this->pre_read_mode) {
        return;
    }

    this->cur_function->setBranchStatement(condition, label1, block_type1, label2, block_type2);
}

void Module::setPrintfStatement(const std::string& format_string, const std::vector<VarInf>& params, ErrorCode* error_code)
{

    if (error_code) *error_code = ErrorCode::None;

    if (this->pre_read_mode) {
        return;
    }

    this->cur_function->setPrintfStatement(format_string, params, error_code);
}


void Module::print(){

    (*this->fout) << "declare i32 @getint();\ndeclare void @putint(i32);\ndeclare void @putch(i32);\ndeclare void @putstr(i8*);\n\n";
    
    for (auto &&global_var_inf_pair : global_var_infs)
    {
        VarType& type = global_var_inf_pair.second.type;
        std::vector<VarInf>& initials = global_var_inf_pair.second.initial_values;
        (*this->fout)<<global_var_inf_pair.first<<" = "<< "dso_local " << (type.is_const ? "constant " : "global ") << type.getVisit(1, nullptr).toString();
        if (!initials.empty()) {
            if (type.array_degrees.size() == 0) {
                (*this->fout) << ' ' << initials.front().name <<std::endl;
            } else if (type.type == VarType::i8) {
                (*this->fout) << " c\"" << initials.front().type.toString() << "\\00\", align 1"<<std::endl;
            } else if (type.array_degrees.size() == 1) {
                (*this->fout) << " [i32 " << initials.front().name;
                for (unsigned int i = 1; i < initials.size(); i++)
                {
                    (*this->fout)<<", i32 "<< initials[i].name;
                }
                (*this->fout)<<"]"<<std::endl;
            } else {
                
                int num = 0;
                std::string temp = "";
                (*this->fout) << " [";
                VarType temp_type = type.getVisit(2, nullptr);
                temp = "[i32 " + initials[num++].name;
                for (unsigned int j = 1; j < type.array_degrees[1]; j++, num++) {
                    temp += ", i32 " + initials[num].name;
                }
                temp += ']';
                (*this->fout) << temp_type.toString() << ' ' << temp;
                for (unsigned i = 1; i < type.array_degrees[0]; i++)
                {
                    temp = " [i32 " + initials[num++].name;
                    for (unsigned int j = 1; j < type.array_degrees[1]; j++, num++) {
                        temp += ", i32 " + initials[num].name;
                    }
                    temp += ']';
                    (*this->fout) << ", " << temp_type.toString() << temp;
                }
                (*this->fout) << ']' << std::endl;
            }
        } else {
            (*this->fout) << " zeroinitializer" << std::endl;
        }
    }
    for (auto &&function_pair : functions)
    {
        function_pair.second.print();
    }
}