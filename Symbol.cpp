#include "Symbol.hpp"


//Symbol
Symbol::Symbol(IDtype id) : id(id) {}
Symbol::Symbol(IDtype id, Exp_t exp) : id(id), exp(exp) {
    output::printLog("Symbol ctor: " + this->exp.t.getStr());
}
Symbol::Symbol(IDtype id, Type t) : id(id), exp(Exp_t(t)) {
    output::printLog("Symbol ctor(type): " + this->exp.t.getStr());
}
Symbol::Symbol(const Symbol& _sym) : id(_sym.id), exp(_sym.exp){}

Type Symbol::getType() {
    return exp.t;
}

string Symbol::getReg() {
    return exp.regName;
}

string Symbol::getId() {
    return id.id;
}

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