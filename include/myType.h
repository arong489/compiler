#ifndef TYPE_H
#define TYPE_H

#include "exception.h"
#include "token.h"

#include <vector>
enum class VarType {
    IntType,
    VoidType,
};

struct MyType {
    bool isConst;
    VarType varType;
    unsigned int pointerDepth;
    std::vector<int> degrees;

    MyType(VarType varType = VarType::IntType, unsigned int pointerDepth = 0, const std::vector<int>& degrees = std::vector<int>())
        : isConst(false)
        , varType(varType)
        , pointerDepth(pointerDepth)
        , degrees(degrees)
    {
#ifdef debug_memory
        std::cout<<"new MyType\t"<<this<<std::endl;
#endif
    }
    MyType(bool isConst, VarType varType = VarType::IntType, unsigned int pointerDepth = 0, const std::vector<int>& degrees = std::vector<int>())
        : isConst(isConst)
        , varType(varType)
        , pointerDepth(pointerDepth)
        , degrees(degrees)
    {
#ifdef debug_memory
        std::cout<<"new MyType\t"<<this<<std::endl;
    }
    ~MyType(){
        std::cout<<"destroy MyType\t"<<this<<std::endl;
#endif
    }
    const MyType calculate(const Token& op, const MyType& another) const;
    const MyType calculate(const Token& op) const;
    int assignCheck(const MyType& another) const;
};

struct VariableInf {
    MyType type;
    std::vector<int> initialValues;
    VariableInf(const MyType& type = MyType(), const std::vector<int> initialValues = std::vector<int>())
        : type(type)
        , initialValues(initialValues)
    {
#ifdef debug_memory
        std::cout<<"new VariableInf\t"<<this<<std::endl;
    }
    ~VariableInf(){
        std::cout<<"destroy VariableInf\t"<<this<<std::endl;
#endif
    }
    int assignCheck(const MyType& value) const;
    bool getValue(const std::vector<int>& indexes, int&feedback);
};

struct FunctionInf {
    MyType type;
    std::vector<const MyType*> parameters;
    // bool ambiguous;
    FunctionInf(const MyType& type = MyType(), const std::vector<const MyType*>& parameters = std::vector<const MyType*>())
        : type(type)
        , parameters(parameters)
        // , ambiguous(true)
    {
#ifdef debug_memory
        std::cout<<"new FunctionInf\t"<<this<<std::endl;
    }
    ~FunctionInf(){
        std::cout<<"destroy FunctionInf\t"<<this<<std::endl;
#endif
    }

    /**
     * @brief check parameters
     * @param parameters the real parameters type
     * @return 1 for wrong number of param; 2 for unmatched param type; 0 for matched
     */
    int parametersCheck(const std::vector<MyType>& parameters = std::vector<MyType>()) const;
};

#endif