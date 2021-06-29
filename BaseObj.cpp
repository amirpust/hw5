#include "BaseObj.hpp"

Type::Type(TYPE t) : t(t) {}
bool Type::operator==(const Type rhs) const {
    return t == rhs.t;
}
bool Type::operator==(const TYPE rhs) const {
    return t == rhs;
}
bool Type::operator!=(const Type rhs) const {
    return !(rhs == *this);
}
bool Type::operator!=(const TYPE rhs) const {
    return !(*this == rhs);
}
string Type::getStr() const {
    return typeStr[(int)(t)];
}


Num::Num(int val) : val(val) {}


String::String(string val) : val(val) {}
