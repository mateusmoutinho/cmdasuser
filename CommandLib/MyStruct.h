#pragma once

#include <string>

class MyStruct {
public:
    MyStruct(const std::string& str1, const std::string& str2);

    std::string serialize() const;
    static MyStruct deserialize(const std::string& data);

    const std::string& getStr1() const { return str1_; }
    const std::string& getStr2() const { return str2_; }

private:
    std::string str1_;
    std::string str2_;
};
