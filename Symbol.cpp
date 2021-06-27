#include "Symbol.hpp"


//Symbol
Symbol::Symbol(const IDtype id, const Type t) : id(id), t(t) {}
Symbol::Symbol(const IDtype id, const Type t, Exp_t exp) : id(id), t(t) , exp(exp) {
    output::printLog("symbol ctor: " + this->exp.t.getStr());
}
Symbol::Symbol(const Symbol& _sym) : id(_sym.id), t(_sym.t) ,exp(_sym.exp){}

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