#ifndef CLOSURE_BLOCK_H
#define CLOSURE_BLOCK_H

#include "llvm_type.h"
#include <list>
#include "basic_block.h"
struct ClosureBlock {
    
    inline static bool isLoop(BasicBlock::Type block_type) {
        return block_type == BasicBlock::LOOP_BODY;
    }

    struct VarInf {
        VarType var_type;
        unsigned int var_number;
    };

    const BasicBlock::Type block_type;
    const std::string loop_name;
    std::map<std::string, ClosureBlock::VarInf> var_map;
    ClosureBlock& father_block;
    bool empty_condition = false, empty_inc = false;

    ClosureBlock()
        : block_type(BasicBlock::ENTRY)
        , father_block(*this)
        , loop_name()
    {
    }
    ClosureBlock(BasicBlock::Type block_type, const bool empty_condition, const bool empty_inc, ClosureBlock& father_block, const std::string& loop_name)
        : block_type(block_type)
        , father_block(father_block)
        , loop_name(loop_name)
        , empty_condition(empty_condition)
        , empty_inc(empty_inc)
    {
    }

    std::string getVariable(const std::string& var_name, VarType& ret_var_type);

    bool inLoop(BasicBlock::Type& ret_block_type, bool& empty_condition, bool& empty_inc, std::string& ret_loop_name);

};

class ClosureManager {
private:
    std::map<std::string, unsigned int> var_numbers;
    std::list<ClosureBlock> closure_blocks;
    ClosureBlock* cur_block;

public:
    ClosureManager()
    {
        this->closure_blocks.push_back(ClosureBlock());
        this->cur_block = &this->closure_blocks.back();
    }
    void setNextBlock(const BasicBlock::Type block_type, const std::string& loop_name = ""){this->setNextBlock(block_type, false, false, loop_name);};
    void setNextBlock(const BasicBlock::Type block_type, const bool empty_condition, const bool empty_inc, const std::string& block_name = "");
    // check if have 'Return' statement in curTable Block
    void setUpperBlock();

    bool insertVariable(const VarType& var_type, const std::string& var_name, std::string& ret_var_name);

    ClosureBlock* getCurBlock() { return cur_block; }
};

#endif