#ifndef MODULE_H
#define MODULE_H

#include "./function.h"
#include <map>

class Module {
private:
    std::map<std::string, GlobalVarInf> global_var_infs;
    std::map<std::string, Function> functions;
    Function* cur_function = nullptr;
    std::ostream* fout;

public:
    // "pre_read_mode" just check the token structure
    bool pre_read_mode = false;
    Module(std::ostream* fout)
        : fout(fout)
    {
    }

    bool delcareFunction(const std::string& func_name, const VarType& ret_type = VarType(VarType::VOID));
    bool declareParam(const std::string& var_name, const VarType& var_type);

    // if "ret" is needed in current fuction
    bool closeFunctionNeedRet();

    VarInf callFunction(const std::string& func_name, const std::vector<VarInf>& params = {}, ErrorCode* error_code = nullptr);
    VarInf callFunction(const std::string& func_name, ErrorCode* error_code) { return this->cur_function->callFunction(func_name, {}, error_code); }
    VarInf callThirdPartyFunction(const std::string& func_name, const std::vector<VarInf>& params = {}, ErrorCode* error_code = nullptr);

    bool getFuction(const std::string& func_name, Function* ret_function);

    bool declareVariable(const std::string& var_name, const VarType& var_type, const std::vector<VarInf>& initial_regs_or_nums = {});
    bool declareVariable(const std::string& var_name, const VarType& var_type, const VarInf& initial_regs_or_num) { return this->declareVariable(var_name, var_type, { initial_regs_or_num }); }

    VarInf getVariableRegister(bool left_value_tag, const std::string& var_name, const std::vector<VarInf>& reg_or_num_indexes = {}, ErrorCode* error_code = nullptr);
    VarInf getVariableRegister(bool left_value_tag, const std::string& var_name, ErrorCode* error_code, const std::vector<VarInf>& reg_or_num_indexes = {}) { return this->getVariableRegister(left_value_tag, var_name, reg_or_num_indexes, error_code); }

    /**
     * @brief calculate
     *
     * @param action string literal
     * @note ASSIGN PLUS MINUS MULT DIV MOD NOT EQL NEQ GRE LSS LEQ GEQ AND OR
     * @param reg_or_num1
     * @param reg_or_num2
     * @param error_code [selectable]
     * @return std::string
     */
    VarInf calculate(const std::string& action, const VarInf& reg_or_num1, const VarInf& reg_or_num2, ErrorCode* error_code = nullptr);

    void setNextBasicBlock(const std::string& name, BasicBlock::Type type);
    void setReturnStatement(const VarInf& reg_or_num, ErrorCode* error_code = nullptr);
    void setReturnStatement(ErrorCode* error_code = nullptr);
    void setBranchStatement(const VarInf& condition, const std::string& label1, BasicBlock::Type block_type1, const std::string& label2, BasicBlock::Type block_type2);
    void setBranchStatement(const std::string& label, BasicBlock::Type block_type) { this->cur_function->setBranchStatement(label, block_type); }

    void setPrintfStatement(const std::string& format_string, const std::vector<VarInf>& params, ErrorCode* error_code = nullptr);
    void setPrintfStatement(const std::string& format_string, ErrorCode* error_code = nullptr) { this->cur_function->setPrintfStatement(format_string, {}, error_code); }
    void print();

    // void setBreakOrContinue(ErrorCode* error_code = nullptr);
    //|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    void setClosureBlock(BasicBlock::Type block_type, const std::string& block_name = ""){this->setClosureBlock(block_type, false, false, block_name);}
    void setClosureBlock(BasicBlock::Type block_type, const bool empty_condition, const bool empty_inc, const std::string& block_name = ""){
        if (this->pre_read_mode) {
            return;
        }
        this->cur_function->setClosureBlock(block_type, empty_condition, empty_inc, block_name);
    }

    void outClosureBlock(){
        if (this->pre_read_mode) {
            return;
        }
        this->cur_function->outClosureBlock();
    }

    bool inLoop(BasicBlock::Type& loop_type, bool& empty_condition, bool& empty_inc, std::string& loop_name){
        if (this->pre_read_mode) {
            return true;
        }
        return this->cur_function->inLoop(loop_type, empty_condition, empty_inc, loop_name);
    }

    unsigned int getLabelNumber(const std::string& label_name){
        if (this->pre_read_mode) {
            return 0;
        }
        return this->cur_function->getLabelNumber(label_name);
    }

};

#endif