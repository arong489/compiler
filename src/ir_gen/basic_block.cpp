#include "../../include/ir_gen/basic_block.h"

void BasicBlock::addIR(const LlvmIR & ir)
{
    this->instructions.push_back(std::shared_ptr<LlvmIR>(ir.copy_alloca()));
}

void BasicBlock::insertAlloca(const LlvmIR & ir)
{
    this->instructions.insert(--this->instructions.end(), std::shared_ptr<LlvmIR>(ir.copy_alloca()));
}

std::string BasicBlock::fixLabelName(const std::string& label, BasicBlock::Type block_type)
{
    std::string fix_label;
    switch (block_type) {
    case BasicBlock::ENTRY:
        fix_label = "entry";
        break;
    case BasicBlock::IF_TRUE:
        fix_label = "if." + label + '.' + "true";
        break;
    case BasicBlock::IF_FALSE:
        fix_label = "if." + label + '.' + "false";
        break;
    case BasicBlock::IF_END:
        fix_label = "if." + label + '.' + "end";
        break;
    case BasicBlock::LOOP_BODY:
        fix_label = "loop." + label + '.' + "body";
        break;
    case BasicBlock::LOOP_CONDITION:
        fix_label = "loop." + label + '.' + "condition";
        break;
    case BasicBlock::LOOP_INC:
        fix_label = "loop." + label + '.' + "inc";
        break;
    case BasicBlock::LOOP_END:
        fix_label = "loop." + label + '.' + "end";
        break;
    case BasicBlock::BREAK_AFTER:
        fix_label = "break." + label + ".after";
        break;
    case BasicBlock::CONTINUE_AFTER:
        fix_label = "continue." + label + ".after";
        break;
    case BasicBlock::RETURN_AFTER:
        fix_label = "return." + label + ".after";
        break;
    default:
        break;
    }
    return fix_label;
}

void BasicBlock::print()
{
    (*this->fout) << BasicBlock::fixLabelName(this->name, this->type) << ":" << std::endl;
    for (auto&& ir : this->instructions) {
        (*this->fout) << '\t' << ir->toString() << std::endl;
    }
}