#ifndef FUNCTION_H
#define FUNCTION_H

#include "./basic_block.h"
#include "./closure_block.h"

class Function {
private:
    
    std::map<std::string, GlobalVarInf>& global_var_map;
    std::map<std::string, Function>& function_map;

    std::string func_name;
    ClosureManager var_closure_block_manager;
    std::map<std::string, VarType> reg_map;
    std::map<std::string, BasicBlock> basicBlocks;
    BasicBlock* cur_block;
    BasicBlock* entry_block;
    unsigned int total_reg_quantity = 0;

    VarType getRegType(const std::string & reg_name, bool& is_num, int* value);

    std::map<std::string, unsigned int> label_num_map;
    std::ostream* fout;
    bool need_ret;

public:
    FunctionType type;

    Function(std::map<std::string, GlobalVarInf>& global_var_map, std::map<std::string, Function>& function_map, const std::string& func_name, std::ostream* fout)
        : global_var_map(global_var_map)
        , function_map(function_map)
        , func_name(func_name)
        , type({VarType(VarType::VOID)})
        , fout(fout)
        , need_ret(true)
    {
        this->basicBlocks.insert({"", BasicBlock("entry", BasicBlock::ENTRY, this->fout)});
        this->entry_block = this->cur_block = &basicBlocks[""];
    }

    Function(std::map<std::string, GlobalVarInf>& global_var_map, std::map<std::string, Function>& function_map, const std::string& func_name, const VarType& ret_type, std::ostream* fout)
        : global_var_map(global_var_map)
        , function_map(function_map)
        , func_name(func_name)
        , type({ret_type})
        , fout(fout)
        , need_ret(true)
    {
        this->basicBlocks.insert({"", BasicBlock("entry", BasicBlock::ENTRY, this->fout)});
        this->entry_block = this->cur_block = &basicBlocks[""];
    }

    // BasicBlock& getCurBasicBlock();

    bool declareParam(const std::string& var_name, const VarType& var_type);

    bool declareVariable(const std::string& var_name, const VarType& var_type, const std::vector<VarInf>& initial_regs_or_nums);
    bool declareVariable(const std::string& var_name, const VarType& var_type, const VarInf& initial_reg_or_num = {VarType::VOID});

    // return the allocated pointer register name
    VarInf getVariableRegister(bool left_value_tag, const std::string& var_name, const std::vector<VarInf>& reg_or_num_indexes = std::vector<VarInf>(), ErrorCode* error_code = nullptr);
    VarInf getVariableRegister(bool left_value_tag, const std::string& var_name, ErrorCode* error_code = nullptr, const std::vector<VarInf>& reg_or_num_indexes = {}){return this->getVariableRegister(left_value_tag, var_name, reg_or_num_indexes, error_code);}

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


    // please use Module::callFunction
    VarInf callFunction(const std::string& func_name, const std::vector<VarInf>& params = {}, ErrorCode* error_code = nullptr);
    VarInf callThirdPartyFunction(const std::string& func_name, const std::vector<VarInf>& params = {}, ErrorCode* error_code = nullptr);

    void setNextBasicBlock(const std::string& cur_name, BasicBlock::Type type);
    void setBranchStatement(const VarInf& condition, const std::string& label1, BasicBlock::Type block_type1, const std::string& label2, BasicBlock::Type block_type2);
    void setBranchStatement(const std::string& label, BasicBlock::Type block_type);
    void setReturnStatement(const VarInf& reg_or_num, ErrorCode* error_code = nullptr);
    void setReturnStatement(ErrorCode* error_code = nullptr);
    // check IllegalCharInFormatString & UnmatchedPrintArgs
    void setPrintfStatement(const std::string& format_string, const std::vector<VarInf>& params, ErrorCode* error_code);

    void setClosureBlock(const BasicBlock::Type block_type, const bool empty_condition, const bool empty_inc, const std::string& block_name = ""){this->var_closure_block_manager.setNextBlock(block_type, empty_condition, empty_inc, block_name);}
    void outClosureBlock(){this->var_closure_block_manager.setUpperBlock();}

    bool inLoop(BasicBlock::Type& loop_type, bool& empty_condition, bool& empty_inc, std::string& loop_name){return this->var_closure_block_manager.getCurBlock()->inLoop(loop_type, empty_condition, empty_inc, loop_name);}

    // return if need ret
    bool ifNeedRet() {
        if (this->need_ret) {
            if (this->type.retType == VarType::VOID) {
                ErrorCode error_code;
                this->setReturnStatement(&error_code);
                return false;
            } else {
                return true;
            }
        }
        return false;
    }
    unsigned int getLabelNumber( const std::string& label_name){
        return this->label_num_map[label_name]++;
    }

    void print();

};

#endif