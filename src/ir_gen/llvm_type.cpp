#include "../../include/ir_gen/llvm_type.h"

std::string VarType::toString() const
{
    std::string front, back;
    for (auto&& degree : array_degrees) {
        front += "[" + std::to_string(degree) + " x ";
        back += ']';
    }

    switch (this->type) {
    case VarType::i1:
        front += "i1";
        break;
    case VarType::i8:
        front += "i8";
        break;
    case VarType::CONSTANT:
    case VarType::i32:
        front += "i32";
        break;
    default:
        front += "void";
        break;
    }
    for (unsigned int i = 0; i < pointer_depth; i++) {
        front += '*';
    }

    front += back;
    for (unsigned int i = 0; i < array_pointer_depth; i++) {
        front += '*';
    }
    return front;
}

VarType VarType::getVisit(int times, ErrorCode* error_code) const
{
    if (VarType::WRONG == this->type) {
        return *this;
    }

    if (error_code) *error_code = ErrorCode::None;

    if ((this->array_pointer_depth == 0 && this->array_degrees.empty()) || (times > 0 && times > this->array_pointer_depth)) {
        times -= this->array_pointer_depth;
        if (this->array_degrees.empty() || (times > 0 && times > this->array_degrees.size())) {
            times -= this->array_degrees.size();

            if (times > 0 && this->pointer_depth < times) {
                if (error_code != nullptr && *error_code == ErrorCode::None) {
                    *error_code = ErrorCode::CalculateTypeNotMatch;
                    return VarType(VarType::WRONG);
                }
            }

            return VarType(this->is_const, this->type, this->pointer_depth - times);
        } else {
            std::vector<unsigned int> temp(this->array_degrees.begin() + times, this->array_degrees.end());
            return VarType(this->is_const, this->type, this->pointer_depth, temp);
        }
    } else {
        VarType temp = *this;
        temp.array_pointer_depth -= times;
        return temp;
    }
}

bool VarType::needEndZero() const
{
    return this->array_pointer_depth == 0 && !this->array_degrees.empty();
}

void VarType::toPointer()
{
    if (this->array_pointer_depth == 0 && !this->array_degrees.empty()) {
        this->array_degrees.erase(this->array_degrees.begin());
        this->array_degrees.empty() ? this->pointer_depth ++ : this->array_pointer_depth ++;
    }
}

// if this register point to the address of another pointer, it need "load"
bool VarType::needLoadFirst() const
{
    if (this->array_pointer_depth > 1) {
        return true;
    }
    if (this->array_pointer_depth == 0 && this->array_degrees.empty() && this->pointer_depth > 1) {
        return true;
    }
    return false;
}

bool VarType::operator==(const VarType &another) const
{
    // if (this->type == VarType::WRONG || another.type == VarType::WRONG) {
    //     return true;
    // }
    if (this->type != another.type
    && !(this->type == VarType::CONSTANT && (another.type == VarType::i32 || another.type == VarType::i8 || another.type == VarType::i1))
    && !(another.type == VarType::CONSTANT && (this->type == VarType::i32 || this->type == VarType::i8 || this->type == VarType::i1))){
        return false;
    }
    if (this->pointer_depth != another.pointer_depth
    || this->array_pointer_depth != another.array_pointer_depth
    || this->array_degrees.size() != another.array_degrees.size()) {
        return false;
    }
    for (unsigned int i = 0; i < this->array_degrees.size(); i++) {
        if (this->array_degrees[i] != another.array_degrees[i]) {
            return false;
        }
    }
    return true;
}

bool VarType::operator!=(const VarType &another) const
{
    return !this->operator==(another);
}

VarType GlobalVarInf::getVisit(const std::vector<unsigned int>& indexes, std::string *value, ErrorCode *error_code) const
{
    if (error_code != nullptr) *error_code = ErrorCode::None;
    int check = this->type.array_degrees.size() + this->type.pointer_depth + this->type.array_pointer_depth - indexes.size();
    if (check < 0) {
        if (error_code != nullptr) *error_code = ErrorCode::CalculateTypeNotMatch;
        return VarType(VarType::WRONG);
    }
    unsigned int offset = 0;
    if (check == 0 && value != nullptr) {
        for (unsigned int i = 0; i < indexes.size(); i++)
        {
            offset = offset * type.array_degrees[i] + indexes[i];
        }
        *value = initial_values[offset].name;
    }
    return this->type.getVisit(indexes.size(), nullptr);
}