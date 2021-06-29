#ifndef HW3_BASEOBJ_HPP
#define HW3_BASEOBJ_HPP

#include <string>
#include "Enums.hpp"
#include <vector>
#include <utility>

using namespace std;

class BaseObj{
public:
    string regName;
    BaseObj() : regName(getNewRegister()) {};
};

class IDtype : public BaseObj{
public:
    string id;
    IDtype(string _id) : id(_id) {};
};

class Type: public BaseObj {
public:
    TYPE t;
    Type(TYPE t);

    string getStr() const;

    bool operator==(const Type rhs) const;
    bool operator==(const TYPE rhs) const;
    bool operator!=(const Type rhs) const;
    bool operator!=(const TYPE rhs) const;
};

class Num: public BaseObj{
public:
    int val;
    Num(int val);
};

class String: public BaseObj{
public:
    string val;
    int address;
    String(string val);
};

#define YYSTYPE BaseObj*



#endif //HW3_BASEOBJ_HPP
