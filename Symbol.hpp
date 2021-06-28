#ifndef HW5_SYMBOL_HPP
#define HW5_SYMBOL_HPP

#include "BaseObj.hpp"
#include "Exp_t.hpp"

class Symbol : public BaseObj {
public:
    IDtype id;
    Exp_t exp;

    explicit Symbol(IDtype id);
    Symbol(IDtype id, Exp_t exp);
    Symbol(IDtype id, Type t);

    Symbol(const Symbol& _sym);

    Type getType();
    string getReg();
    string getId();
};

class SymList : public BaseObj {
public:
    vector<Symbol> symList;
    SymList(const vector<Symbol> &symList);
    SymList();

    void insert(Symbol sym);

    typedef vector<Symbol>::iterator iterator;
};

class FuncSymbol : public BaseObj {
public:
    Type retType;
    IDtype id;
    SymList symList;

    FuncSymbol(Type _retType, IDtype _id, SymList _symList);
};

class FuncList : public BaseObj {
public:
    vector<FuncSymbol> funcList;

    FuncList(vector<FuncSymbol> & _funcList);
    FuncList();

    void insert(FuncSymbol func);

    typedef vector<FuncSymbol>::iterator iterator;
};


#endif