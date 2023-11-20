#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "./llvm_type.h"

struct LlvmIR {
    enum Action {
        RET,
        BR,
        ADD,
        SUB,
        MUL,
        SREM,
        SDIV,
        OR,
        AND,
        ICMP_NE,
        ICMP_EQ,
        ICMP_SGT,
        ICMP_SLT,
        ICMP_SGE,
        ICMP_SLE,
        LOAD,
        STORE,
        ALLOCA,
        GETELEMENTPTR,
        CALL,
    } action;
    virtual std::string toString() const = 0;
    virtual ~LlvmIR() { } // 占位, 防止析构不彻底
    virtual LlvmIR* copy_alloca() const = 0;
};

// add, sub, mul, srem, sdiv
// and, or, icmp_ne, icmp_eq, icmp_sgt, icmp_slt, icmp_sge, icmp_sle
struct CalculateIR : virtual LlvmIR {
    std::string result_reg, reg_or_num1, reg_or_num2;
    VarType cal_type;
    CalculateIR(std::string result_reg, LlvmIR::Action action, const VarType& cal_type, const std::string& reg_or_num1, const std::string& reg_or_num2)
        : result_reg(result_reg)
        , reg_or_num1(reg_or_num1)
        , reg_or_num2(reg_or_num2)
        , cal_type(cal_type)
    {
        this->action = action;
    }
    CalculateIR(LlvmIR::Action action, const VarType& cal_type, const std::string& reg_or_num1, const std::string& reg_or_num2)
        : result_reg("")
        , reg_or_num1(reg_or_num1)
        , reg_or_num2(reg_or_num2)
        , cal_type(cal_type)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new CalculateIR(*this);
    }
    std::string toString() const;
};

// br
struct BranchIR : virtual LlvmIR {
    std::string condition, label_true, label_false;
    BranchIR(LlvmIR::Action action, std::string label)
        : label_true(label)
    {
        this->action = action;
    }
    BranchIR(LlvmIR::Action action, std::string condition, std::string label_true, std::string label_false)
        : condition(condition)
        , label_true(label_true)
        , label_false(label_false)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new BranchIR(*this);
    }
    std::string toString() const;
};

// ret
struct RetIR : virtual LlvmIR {
    VarType ret_type;
    std::string ret_value;
    RetIR(LlvmIR::Action action, const VarType& ret_type, const std::string& ret_value)
        : ret_type(ret_type)
        , ret_value(ret_value)
    {
        this->action = action;
    }
    RetIR(LlvmIR::Action action)
        : ret_type(VarType::VOID)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new RetIR(*this);
    }
    std::string toString() const;
};

// load store
struct StorageIR : virtual LlvmIR {
    std::string pointer_reg, value_reg;
    VarType pointer_type, value_type;
    StorageIR(LlvmIR::Action action, const VarType& pointer_type, const std::string& pointer_reg, const VarType& value_type, const std::string& value_reg)
        : pointer_reg(pointer_reg)
        , value_reg(value_reg)
        , pointer_type(pointer_type)
        , value_type(value_type)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new StorageIR(*this);
    }
    std::string toString() const;
};

// alloca
struct AllocaIR : virtual LlvmIR {
    std::string result_reg;
    VarType alloca_type;
    AllocaIR(LlvmIR::Action action, const std::string& result_reg, const VarType& alloca_type)
        : result_reg(result_reg)
        , alloca_type(alloca_type)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new AllocaIR(*this);
    }
    std::string toString() const;
};

struct GetElementIR : virtual LlvmIR {
    std::string result_reg;
    VarType pointer_type;
    std::string pointer_reg;
    std::vector<VarInf> indexes;
    GetElementIR(const std::string& result_reg, LlvmIR::Action action, const VarType& pointer_type, const std::string& pointer_reg, const std::vector<VarInf>& indexes)
        : result_reg(result_reg)
        , pointer_type(pointer_type)
        , pointer_reg(pointer_reg)
        , indexes(indexes)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new GetElementIR(*this);
    }
    std::string toString() const;
};

struct CallIR : virtual LlvmIR {
    std::string result;
    FunctionType function_type;
    std::string function;
    std::vector<VarInf> params;
    CallIR(LlvmIR::Action action, const FunctionType& function_type, const std::string& function, const std::vector<VarInf>& params)
        : function_type(function_type)
        , function(function)
        , params(params)
    {
        this->action = action;
    }
    CallIR(const std::string& result, LlvmIR::Action action, const FunctionType& function_type, const std::string& function, const std::vector<VarInf>& params)
        : result(result)
        , function_type(function_type)
        , function(function)
        , params(params)
    {
        this->action = action;
    }
    LlvmIR* copy_alloca() const
    {
        return new CallIR(*this);
    }
    std::string toString() const;
};

#endif
