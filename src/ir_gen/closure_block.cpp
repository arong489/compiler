#include "../../include/ir_gen/closure_block.h"

void ClosureManager::setNextBlock(const BasicBlock::Type block_type, const bool empty_condition, const bool empty_inc, const std::string & block_name)
{
    this->closure_blocks.push_back(ClosureBlock(block_type, empty_condition, empty_inc, *this->cur_block, block_name));
    this->cur_block = &this->closure_blocks.back();
}

void ClosureManager::setUpperBlock()
{
    this->cur_block = &this->cur_block->father_block;
}

// return real name
std::string ClosureBlock::getVariable(const std::string& var_name, VarType& ret_var_type)
{
    if (var_name.empty()) {
        ret_var_type = VarType(VarType::WRONG);
        return var_name;
    }
    bool digit = true;
    auto&& c_ite = var_name.begin();
    while (c_ite != var_name.end() && (*c_ite == '+' || *c_ite == '-')) {
        c_ite++;
    }
    while (c_ite != var_name.end()) {
        if (!isdigit(*c_ite)) {
            digit = false;
            break;
        }
    }
    if (digit) {
        ret_var_type = VarType(VarType::CONSTANT);
        return var_name;
    }

    ClosureBlock* block = this;
    auto&& ite = block->var_map.find(var_name);
    if (ite != block->var_map.end()) {
        ret_var_type = (*ite).second.var_type;
        return var_name + '.' + std::to_string((*ite).second.var_number);
    }
    while (block->block_type != BasicBlock::ENTRY) {
        block = &block->father_block;
        auto&& ite = block->var_map.find(var_name);
        if (ite != block->var_map.end()) {
            ret_var_type = (*ite).second.var_type;
            return var_name + '.' + std::to_string((*ite).second.var_number);
        }
    }
    ret_var_type = VarType(VarType::WRONG);
    return "";
}

bool ClosureBlock::inLoop(BasicBlock::Type& ret_block_type, bool& empty_condition, bool& empty_inc, std::string& ret_loop_name)
{
    ClosureBlock* block = this;
    if (ClosureBlock::isLoop(block->block_type)) {
        ret_loop_name = block->loop_name;
        empty_condition = block->empty_condition;
        empty_inc = block->empty_inc;
        ret_block_type = block->block_type;
        return true;
    }
    while (block->block_type != BasicBlock::ENTRY) {
        block = &block->father_block;
        if (ClosureBlock::isLoop(block->block_type)) {
            ret_loop_name = block->loop_name;
            empty_condition = block->empty_condition;
            empty_inc = block->empty_inc;
            ret_block_type = block->block_type;
            return true;
        }
    }
    return false;
}

bool ClosureManager::insertVariable(const VarType& var_type, const std::string& var_name, std::string& ret_var_name)
{
    unsigned int size_before, size_after;
    size_before = this->cur_block->var_map.size();
    this->cur_block->var_map.insert({ var_name, { var_type, this->var_numbers[var_name] } });
    size_after = this->cur_block->var_map.size();
    ret_var_name = var_name + '.' + std::to_string(this->var_numbers[var_name]++);

    return size_before + 1 == size_after;
}