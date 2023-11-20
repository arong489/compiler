#ifndef SELF_UTIL_H
#define SELF_UTIL_H

#include<string>

class Util
{
private:
    /* data */
public:
    Util(/* args */){
        throw "this is a static class";
    }

    static bool stringToInt(const std::string& string_value, int& ret_value);
};

#endif