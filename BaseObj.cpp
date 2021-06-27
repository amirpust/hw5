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

//Symbol
Symbol::Symbol(const IDtype id, const Type t) : id(id), t(t) {}
Symbol::Symbol(const Symbol& _sym) : id(_sym.id), t(_sym.t) {}

SymList::SymList(const vector<Symbol> &symList) : symList(symList) {}
SymList::SymList() : symList() {}

void SymList::insert(Symbol sym) {
    symList.push_back(sym);
}


//Func Symbol
FuncSymbol::FuncSymbol(Type _retType, IDtype _id, SymList _symList) :   retType(_retType),
                                                                    id(_id),
                                                                    symList(_symList) {}
FuncList::FuncList(vector<FuncSymbol> &_funcList) : funcList(_funcList) {}
FuncList::FuncList() : funcList() {}

void FuncList::insert(FuncSymbol func) {
    funcList.push_back(func);
}

Num::Num(int val) : val(val) {}


