#ifndef _TABLE_H
#define _TABLE_H

#include "myType.h"
#include <map>
#include <variant>
#include<list>

enum class BlockType {
    Normal,
    Global,
    Loop,
    IntFunction,
    VoidFunction,
};

class Table {
private:
    std::map<std::string, std::variant<VariableInf, FunctionInf>> map;
    Table* upperTable;
    BlockType blockType;
    bool hasReturnStatement;

public:
    Table(Table* upperTable = nullptr, BlockType blockType = BlockType::Global)
        : map()
        , upperTable(upperTable)
        , blockType(blockType)
        , hasReturnStatement(false)
    {
#ifdef debug_memory
        std::cout<<"new table\t"<<this<<std::endl;
    }
    ~Table(){
        std::cout<<"destroy table\t"<<this<<std::endl;
#endif
    }
    Table* getUpperTable();
    const BlockType& getBlockType() const;
    unsigned int getSize();

    void setReturnStatement();
    bool ifHasReturnStatement();

    int get(const std::string& ident, std::variant<VariableInf, FunctionInf>& feedback);
    bool insert(const std::string& ident, std::variant<VariableInf, FunctionInf>& value, std::variant<VariableInf, FunctionInf>*& feedback);
    bool remove(const std::string& ident);
};

class TableManager {
private:
    std::list<Table> tables;
    Table* curTable;
    FunctionInf* upperFunctionInf;

public:
    TableManager();
    void setNextBlock(BlockType BlockType);
    // check if have 'Return' statement in curTable Block
    bool setUpperBlock();

    // insert [const] variable
    bool insertVariable(const MyType& type, const std::string& ident, const std::vector<int>& initialValue = std::vector<int>());
    bool insertVariable(const MyType& type, const std::string& ident, bool isParameter);
    bool insertFunction(const MyType& type, const std::string& ident); // maintain 'upperFunctionInf' in 'insertFunction'
    // void fixUpperFunctionParameters();
    void setReturnStatement();
    bool ifHasReturnStatement();

    int getFunctionType(const std::string& ident, MyType& feedback);
    /**
     * @brief checkFunctionParameters
     *
     * @param parameters the real parameters type
     * @return 1 for wrong number of param; 2 for unmatched param type; 3 for no found; 0 for matched
     */
    int checkFunctionParameters(const std::string& ident, const std::vector<MyType>& parameters = std::vector<MyType>());
    int getVariableType(const std::string& ident, MyType& feedback);
    int getVariableValue(const std::string& ident, int& feedback, const std::vector<int> indexes);

    // check block type
    bool inLoop();
    bool inIntFunction();
    bool inVoidFunction();
};
#endif