#include "../include/table.h"

/*||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||                                                                    ||||||||||||
||||||||||||                             Table                                  ||||||||||||
||||||||||||                                                                    ||||||||||||
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
Table* Table::getUpperTable()
{
    return this->upperTable;
}
int Table::get(const std::string& ident, std::variant<VariableInf, FunctionInf>& feedback)
{
    auto ite = this->map.find(ident);
    if (ite == this->map.end()) {
        return 1;
    } else {
        feedback = (*ite).second;
        return 0;
    }
}
bool Table::insert(const std::string& ident, std::variant<VariableInf, FunctionInf>& value, std::variant<VariableInf, FunctionInf>*& feedback)
{
    auto ret = this->map.insert({ ident, value });
    feedback = &this->map.at(ident);
    return ret.second;
}
bool Table::remove(const std::string& ident)
{
    return this->map.erase(ident);
}

void Table::setReturnStatement()
{
    this->hasReturnStatement = true;
}
bool Table::ifHasReturnStatement()
{
    return this->hasReturnStatement;
}

unsigned int Table::getSize()
{
    return this->map.size();
}
const BlockType& Table::getBlockType() const
{
    return this->blockType;
}

/*||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||                                                                    ||||||||||||
||||||||||||                          TableManager                              ||||||||||||
||||||||||||                                                                    ||||||||||||
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
TableManager::TableManager()
{
    this->tables = *new std::list<Table>();
    this->tables.push_back(Table());
    this->curTable = &this->tables.back();
    this->upperFunctionInf = nullptr;
}
void TableManager::setNextBlock(BlockType blockType)
{
    this->tables.push_back(Table(this->curTable, blockType));
    this->curTable = &this->tables.back();
}
bool TableManager::setUpperBlock()
{
    Table* curTablePointer = this->curTable;
    bool result = true;
    if (curTablePointer->getBlockType() == BlockType::IntFunction)
        result = curTablePointer->ifHasReturnStatement();
    this->curTable = curTablePointer->getUpperTable();
    if (curTablePointer->getSize() == 0) {
        this->tables.pop_back();
    }
    return result;
}

bool TableManager::insertVariable(const MyType& type, const std::string& ident, const std::vector<int>& initialValues)
{

    std::variant<VariableInf, FunctionInf> temp = VariableInf(type, initialValues), *feedback;
    return this->curTable->insert(ident, temp, feedback);
}
bool TableManager::insertVariable(const MyType& type, const std::string& ident, bool isParameter)
{
    std::variant<VariableInf, FunctionInf> temp = VariableInf(type), *feedback;
    bool ret = this->curTable->insert(ident, temp, feedback);
    if (isParameter) {
        this->upperFunctionInf->parameters.push_back(&(std::get_if<VariableInf>(feedback)->type));
    }
    return ret;
}
bool TableManager::insertFunction(const MyType& type, const std::string& ident)
{
    std::variant<VariableInf, FunctionInf> temp = FunctionInf(type), *feedback;
    bool ret = this->curTable->insert(ident, temp, feedback);
    this->upperFunctionInf = std::get_if<FunctionInf>(feedback);
    return ret;
}
// void TableManager::fixUpperFunctionParameters()
// {
//     this->upperFunctionInf->ambiguous = false;
// }
void TableManager::setReturnStatement()
{
    this->curTable->setReturnStatement();
}
bool TableManager::ifHasReturnStatement()
{
    return this->curTable->ifHasReturnStatement();
}

int TableManager::getFunctionType(const std::string& ident, MyType& feedback)
{
    Table* tablePointer = this->curTable;
    std::variant<VariableInf, FunctionInf> result;
    FunctionInf* tempFunctionInfPtr;
    if (!tablePointer->get(ident, result)) {
        tempFunctionInfPtr = std::get_if<FunctionInf>(&result);
        if (tempFunctionInfPtr != nullptr) {
            feedback = tempFunctionInfPtr->type;
            return 0;
        } else {
            return 1;
        }
    }
    while (tablePointer->getBlockType() != BlockType::Global) {
        tablePointer = tablePointer->getUpperTable();
        if (!tablePointer->get(ident, result)) {
            tempFunctionInfPtr = std::get_if<FunctionInf>(&result);
            if (tempFunctionInfPtr != nullptr) {
                feedback = tempFunctionInfPtr->type;
                return 0;
            } else {
                return 1;
            }
        }
    }
    return 2;
}
int TableManager::checkFunctionParameters(const std::string& ident, const std::vector<MyType>& parameters)
{
    Table* tablePointer = this->curTable;
    std::variant<VariableInf, FunctionInf> result;
    FunctionInf* tempFunctionInfPtr;
    if (!tablePointer->get(ident, result)) {
        tempFunctionInfPtr = std::get_if<FunctionInf>(&result);
        if (tempFunctionInfPtr != nullptr) {
            return tempFunctionInfPtr->parametersCheck(parameters);
        } else {
            return 3;
        }
    }
    while (tablePointer->getBlockType() != BlockType::Global) {
        tablePointer = tablePointer->getUpperTable();
        if (!tablePointer->get(ident, result)) {
            tempFunctionInfPtr = std::get_if<FunctionInf>(&result);
            if (tempFunctionInfPtr != nullptr) {
                return tempFunctionInfPtr->parametersCheck(parameters);
            } else {
                return 3;
            }
        }
    }
    return 4;
}
int TableManager::getVariableType(const std::string& ident, MyType& feedback)
{
    Table* tablePointer = this->curTable;
    std::variant<VariableInf, FunctionInf> result;
    VariableInf* tempVariableInfPtr;
    feedback = MyType(VarType::WrongType);
    if (!tablePointer->get(ident, result)) {
        tempVariableInfPtr = std::get_if<VariableInf>(&result);
        if (tempVariableInfPtr != nullptr) {
            feedback = tempVariableInfPtr->type;
            return 0;
        } else {
            return 1;
        }
    }
    while (tablePointer->getBlockType() != BlockType::Global) {
        tablePointer = tablePointer->getUpperTable();
        if (!tablePointer->get(ident, result)) {
            tempVariableInfPtr = std::get_if<VariableInf>(&result);
            if (!tablePointer->get(ident, result)) {
                tempVariableInfPtr = std::get_if<VariableInf>(&result);
                if (tempVariableInfPtr != nullptr) {
                    feedback = tempVariableInfPtr->type;
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }
    return 2;
}
int TableManager::getVariableValue(const std::string& ident, int& feedback, const std::vector<int> indexes)
{
    Table* tablePointer = this->curTable;
    std::variant<VariableInf, FunctionInf> result;
    VariableInf* tempVariableInfPtr;
    if (!tablePointer->get(ident, result)) {
        tempVariableInfPtr = std::get_if<VariableInf>(&result);
        if (tempVariableInfPtr != nullptr) {
            if (tempVariableInfPtr->getValue(indexes, feedback)) {
                return 0;
            } else {
                return 1;
            }
        } else {
            return 2;
        }
    }
    while (tablePointer->getBlockType() != BlockType::Global) {
        tablePointer = tablePointer->getUpperTable();
        if (!tablePointer->get(ident, result)) {
            tempVariableInfPtr = std::get_if<VariableInf>(&result);
            if (tempVariableInfPtr != nullptr) {
                if (tempVariableInfPtr->getValue(indexes, feedback)) {
                    return 0;
                } else {
                    return 1;
                }
            } else {
                return 2;
            }
        }
    }
    return 3;
}

/* check block type */
bool TableManager::inLoop()
{
    Table* tablePointer = this->curTable;
    if (tablePointer->getBlockType() == BlockType::Loop) {
        return true;
    }
    while (tablePointer->getBlockType() == BlockType::Normal) {
        tablePointer = tablePointer->getUpperTable();
        if (tablePointer->getBlockType() == BlockType::Loop) {
            return true;
        }
    }
    return false;
}
/* check block type */
bool TableManager::inIntFunction()
{
    Table* tablePointer = this->curTable;
    if (tablePointer->getBlockType() == BlockType::IntFunction) {
        return true;
    }
    while (tablePointer->getBlockType() == BlockType::Normal || tablePointer->getBlockType() == BlockType::Loop) {
        tablePointer = tablePointer->getUpperTable();
        if (tablePointer->getBlockType() == BlockType::IntFunction) {
            return true;
        }
    }
    return false;
}
/* check block type */
bool TableManager::inVoidFunction()
{
    Table* tablePointer = this->curTable;
    if (tablePointer->getBlockType() == BlockType::VoidFunction) {
        return true;
    }
    while (tablePointer->getBlockType() == BlockType::Normal || tablePointer->getBlockType() == BlockType::Loop) {
        tablePointer = tablePointer->getUpperTable();
        if (tablePointer->getBlockType() == BlockType::VoidFunction) {
            return true;
        }
    }
    return false;
}