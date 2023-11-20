#include "../../include/util/util.h"

bool Util::stringToInt(const std::string& string_value, int& ret_value)
{
    if (string_value.empty()) {
        return false;
    }

    bool ret_bool = true;
    ret_value = 0;
    bool minus = false;
    auto&& ite = string_value.begin();
    while (ite != string_value.end() && ((*ite) == '+' || (*ite) == '-')) {
        minus = !minus;
        ite++;
    }
    while (ite != string_value.end()) {
        if (!isdigit(*ite)) {
            ret_bool = false;
            break;
        }
        ret_value = 10 * ret_value + (*ite) - '0';
        ite++;
    }

    if (ret_bool && minus) {
        ret_value = -ret_value;
    }

    return ret_bool;
}