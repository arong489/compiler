#ifndef LLVM_TYPE_H
#define LLVM_TYPE_H

#include <map>
#include <vector>
#include "../exception/exception.h"


struct VarType {
    bool is_const;
    enum Type {
        WRONG,
        CONSTANT,
        VOID,
        i1,
        i8,
        i32,
    } type;
    unsigned int pointer_depth = 0;
    std::vector<unsigned int> array_degrees;
    unsigned int array_pointer_depth = 0;
    VarType()
        : is_const(false)
        , type(VarType::i32)
        , pointer_depth(0)
    {
    }
    
    VarType(VarType::Type type, const std::vector<unsigned int>& array_degrees = std::vector<unsigned int>(), unsigned int array_pointer_depth = 0)
        : is_const(false)
        , type(type)
        , pointer_depth(0)
        , array_degrees(array_degrees)
        , array_pointer_depth(array_pointer_depth)
    {
    }
    VarType(VarType::Type type, unsigned int pointer_depth, const std::vector<unsigned int>& array_degrees = std::vector<unsigned int>(), unsigned int array_pointer_depth = 0)
        : is_const(false)
        , type(type)
        , pointer_depth(pointer_depth)
        , array_degrees(array_degrees)
        , array_pointer_depth(array_pointer_depth)
    {
    }
    VarType(bool is_const)
        : is_const(is_const)
        , type(VarType::i32)
        , pointer_depth(0)
    {
    }
    
    VarType(bool is_const, VarType::Type type, const std::vector<unsigned int>& array_degrees = std::vector<unsigned int>(), unsigned int array_pointer_depth = 0)
        : is_const(is_const)
        , type(type)
        , pointer_depth(0)
        , array_degrees(array_degrees)
        , array_pointer_depth(array_pointer_depth)
    {
    }
    VarType(bool is_const, VarType::Type type, unsigned int pointer_depth, const std::vector<unsigned int>& array_degrees = std::vector<unsigned int>(), unsigned int array_pointer_depth = 0)
        : is_const(is_const)
        , type(type)
        , pointer_depth(pointer_depth)
        , array_degrees(array_degrees)
        , array_pointer_depth(array_pointer_depth)
    {
    }

    std::string toString() const;
    VarType getVisit(int times, ErrorCode* error_code) const;
    bool needEndZero() const;
    void toPointer();
    bool needLoadFirst() const;

    // VarType calculate(const int action_enum, const VarType& another, ErrorCode* error_code) const;
    bool operator==(const VarType&) const;
    bool operator!=(const VarType&) const;
};

struct VarInf {
    VarType type;
    std::string name;
};

struct GlobalVarInf {
    VarType type;
    std::vector<VarInf> initial_values;
    GlobalVarInf(VarType type, const std::vector<VarInf>& initial_values = {}): type(type), initial_values(initial_values){}
    VarType getVisit(const std::vector<unsigned int>& indexes, std::string* value, ErrorCode* error_code) const;
};

struct FunctionType {
    VarType retType;
    std::vector<VarType> paramsType;
};

#endif