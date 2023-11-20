#ifndef BASIC_BLOCK_H
#define BASIC_BLOCK_H

#include "./instruction.h"
#include <memory>
#include <list>
#include <fstream>

class BasicBlock {
public:
    enum Type{
        ENTRY,
        LOOP_CONDITION,
        LOOP_BODY,
        LOOP_INC,
        LOOP_END,
        IF_TRUE,
        IF_FALSE,
        IF_END,
        BREAK_AFTER,
        CONTINUE_AFTER,
        RETURN_AFTER,
        ClosureBlock
    };
private:
    //inf
    std::list<std::shared_ptr<LlvmIR>> instructions;
    std::string name;
    BasicBlock::Type type;
    std::ostream* fout;

public:
    BasicBlock( const std::string& name, BasicBlock::Type type, std::ostream* fout)
        : name(name)
        , type(type)
        , fout(fout)
    {
    }
    BasicBlock(){}

    void addIR(const LlvmIR& ir);

    void insertAlloca(const LlvmIR& ir);

    static std::string fixLabelName(const std::string& label, BasicBlock::Type block_type);

    void print();
};

#endif