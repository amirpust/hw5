#ifndef HW3_TABLE_HPP
#define HW3_TABLE_HPP

#include "Enums.hpp"
#include "Exp_t.hpp"
#include "BaseObj.hpp"
#include "Symbol.hpp"
#include <string>
#include <vector>
#include "hw3_output.hpp"
#include <stack>
#include <algorithm>
#include "bp.hpp"



using namespace std;

class Scope{
public:
    SymList symList;
    int offest;
    SCOPE_REASON type;
    string rbp;

    Scope(int _offest, SCOPE_REASON _type) :
            symList(),
            offest(_offest),
            type(_type) {};
};

typedef vector<Scope> ScopeList;

class SymbolTable{
public:
    ScopeList scopeList;
    FuncList funcList;
    bool seenMainFunc;
    stack<int> offsets;
    int cases;

    SymbolTable() : scopeList(), funcList(), seenMainFunc(false) , offsets() , cases(0){
        scopeList.emplace_back(0, GLOBAL_SCOPE);

        SymList pSymList = SymList(vector<Symbol>(1, Symbol(IDtype(""), Type(E_string))));
        FuncSymbol p = FuncSymbol(Type(E_void),IDtype("print"), pSymList);

        SymList piSymList = SymList(vector<Symbol>(1, Symbol(IDtype(""), Type(E_int))));
        FuncSymbol pi = FuncSymbol(Type(E_void),IDtype("printi"), piSymList);

        funcList.insert(p);
        funcList.insert(pi);

        offsets.push(0);
    };

    void gg() {
        checkMain();
        closeCurrentScope();
    }

    void checkMain(){
        if(!seenMainFunc){
            output::errorMainMissing();
            exit(1);
        }
    }

    // open scopes
    void openNewScope(SCOPE_REASON type = REGULAR_SCOPE){
        output::printLog("Start openNewScope | size " + to_string(scopeList.size()));
        string rbp = scopeList.back().rbp;
        scopeList.emplace_back(offsets.top(), type);
        scopeList.back().rbp = rbp;
        offsets.push(offsets.top());
        output::printLog("End openNewScope | size " + to_string(scopeList.size()));
    }
    void openLoopScope(){
        openNewScope(LOOP_SCOPE);
    }
    void openSwitchScope(Exp_t e) {
        if (e.t != E_byte && e.t != E_int) {
            output::errorMismatch(yylineno);
            output::printLog("openSwitchScope");
            exit(1);
        }

        openNewScope( SWITCH_SCOPE);
    }
    void openFuncScope(IDtype id, SymList args, Type retType, string rbp) {
        reverse(args.symList.begin(),args.symList.end());

        output::printLog("Flag " + id.id);
        if ((retType == E_void) && (id.id == "main") && args.symList.empty()){
            seenMainFunc = true;
        }

        if (findFunc(id) != funcList.funcList.end()){
            output::errorDef(yylineno, id.id);
            exit(1);
        }
        for(SymList::iterator sym = args.symList.begin(); sym != args.symList.end(); sym++){
            (*sym).exp.offset = -2;
        }

        funcList.insert(FuncSymbol(retType, id, args));

        for(SymList::iterator sym = args.symList.begin(); sym != args.symList.end(); sym++){
            if(findFunc((*sym).id) != funcList.funcList.end()){
                output::printLog("isId:" + (*sym).getId());
                output::errorDef(yylineno, (*sym).getId());
                exit(444);
            }
            for(SymList::iterator sym2 = sym; sym2 != args.symList.end(); sym2++){
                if (sym == sym2)
                    continue;
                if ((*sym).getId() == (*sym2).getId()){
                    output::printLog("isId2:" + (*sym).getId());
                    output::errorDef(yylineno, (*sym).getId());
                    exit(445);
                }
            }
        }


        offsets.push(0);
        scopeList.emplace_back(offsets.top(), FUNC_SCOPE);
        scopeList.back().rbp = rbp;
    }

    // triggers
    void triggerCase(){
        cases++ ;
    }
    void triggerBreak(){
        for(ScopeList::iterator i = scopeList.begin(); i != scopeList.end(); i++){
            if ((*i).type == LOOP_SCOPE || (*i).type == SWITCH_SCOPE){
                cases = 0;
                return;
            }
        }

        output::errorUnexpectedBreak(yylineno);
        exit(-1);
    }

    void triggerContinue(){
        for(ScopeList::iterator i = scopeList.begin(); i != scopeList.end(); i++){
            if ((*i).type == LOOP_SCOPE){
                return;
            }
        }
        output::errorUnexpectedContinue(yylineno);
        exit(-1);
    }

    Type callFunc(IDtype funcName, ExpList arguments) {
        output::printLog("callFunc: start");

        if(findFunc(funcName) == funcList.funcList.end()){
            output::errorUndefFunc(yylineno, funcName.id);
            exit(1);
        }

        //TODO: pass arguments to function
        SymList sArgs = SymList();
        for (ExpList::iterator a = arguments.expList.begin(); a != arguments.expList.end(); a++){
            sArgs.insert(Symbol(IDtype(""),(*a).t));
        }

        reverse(sArgs.symList.begin(),sArgs.symList.end());

        output::printLog("callFunc: Flag 1");

        FuncSymbol func = *findFunc(funcName);
        output::printLog("func size: " + to_string(func.symList.symList.size()));
        output::printLog("sArgs size: " + to_string(sArgs.symList.size()));

        std::vector<string> strTypes = std::vector<string>();
        for (SymList::iterator i = func.symList.symList.begin(); i != func.symList.symList.end(); i++){
            strTypes.push_back((*i).getType().getStr());
        }

        if(sArgs.symList.size() != func.symList.symList.size()){
            output::errorPrototypeMismatch(yylineno, funcName.id, strTypes);
            exit(-1);
        }


        for (unsigned int i = 0; i < sArgs.symList.size(); ++i) {
            if(
                    (sArgs.symList[i].getType() != func.symList.symList[i].getType()) &&
                    !(sArgs.symList[i].getType() == E_byte && func.symList.symList[i].getType() == E_int)){
                output::printLog("Got: " + sArgs.symList[i].getType().getStr());
                output::printLog("Expected: " + func.symList.symList[i].getType().getStr());
                output::errorPrototypeMismatch(yylineno, funcName.id, strTypes);
                exit(-1);
            }
        }

        return func.retType;
    }
    void closeCurrentScope(){

        output::endScope();

        if (scopeList.empty()){
            output::printLog("ASSERT: closeCurrentScope - empty");
            return;
        }


        if (scopeList.size() == 1){
            //Close global
            for (FuncList::iterator func = funcList.funcList.begin(); func != funcList.funcList.end(); ++func){
                std::vector<string> argTypes;
                string funcType = (*func).retType.getStr();
                for(SymList::iterator sym = (*func).symList.symList.begin(); sym != (*func).symList.symList.end(); ++sym){
                    argTypes.push_back((*sym).getType().getStr());
                }
                output::printID((*func).id.id, 0, output::makeFunctionType(funcType, argTypes));
            }
        } else {
            if (scopeList.size() == 2) {
                //Func scope
                FuncSymbol func = funcList.funcList.back();

                for (unsigned int i = 0; i < func.symList.symList.size(); ++i) {
                    string typeForPrinting = func.symList.symList[i].getType().getStr();
                    output::printID(func.symList.symList[i].getId(), -1 - i,
                                    typeForPrinting);
                }
            }

            Scope closingScope = scopeList.back();
            offsets.pop();
            for (unsigned int i = 0; i < closingScope.symList.symList.size(); ++i) {
                string typeForPrinting = closingScope.symList.symList[i].getType().getStr();
                output::printID(closingScope.symList.symList[i].getId(),
                                offsets.top() + i, typeForPrinting);
            }
        }


        scopeList.pop_back();
        output::printLog("closeCurrentScope - end flag");
    }

    void checkReturnType(Exp_t exp){
        if(!exp.castType(funcList.funcList.back().retType)){
            output::errorMismatch(yylineno);
            output::printLog("checkReturnType");
            exit(1);
        }
    }

    void checkReturnType(){
        if(funcList.funcList.back().retType != E_void){
            output::errorMismatch(yylineno);
            output::printLog("checkReturnType");
            exit(1);
        }
    }
    void addSymbol(IDtype id, Exp_t* exp){
        if(isId(id)) {
            output::errorDef(yylineno, id.id);
            exit(-1);
        }
        output::printLog("add symbol"+ id.id + " " + exp->t.getStr());

        exp->offset = offsets.top()++;
        scopeList.back().symList.insert(Symbol(id, *exp));
        output::printLog("add symbol after insert "+ id.id + " " + scopeList.back().symList.symList.back().exp.t.getStr());
    }

    Type getTypeByID(IDtype _id){
        Symbol* sym = findSym(_id);
        if(!sym){
            output::errorUndef(yylineno, _id.id);
            exit(-46);
        }
        return sym->getType();
    }

    Exp_t getExpByID(IDtype _id){
        Symbol* sym = findSym(_id);
        if(!sym){
            output::errorUndef(yylineno, _id.id);
            exit(-46);
        }
        output::printLog("getExpByID: " + _id.id + " " + sym->exp.t.getStr());
        return sym->exp;
    }

    //TODO: assign properly
    void assign(IDtype _id, Exp_t e){
        Symbol* sym = findSym(_id);
        if(!sym){
            output::errorUndef(yylineno, _id.id);
            exit(-463);
        }
        Exp_t newE = Exp_t(sym->exp);
        newE = e;
    }

    string getCurrentRbp(){
        output::printLog("getCurrentRbp: " + scopeList.back().rbp);
        return scopeList.back().rbp;
    }

private:
    FuncList::iterator findFunc(IDtype _id){
        for (FuncList::iterator f = funcList.funcList.begin(); f != funcList.funcList.end(); f++){
            if (f->id.id == _id.id){
                return f;
            }
        }
        return funcList.funcList.end();
    }
    Symbol* findSym(IDtype _id){
        for(ScopeList::iterator scope = scopeList.begin(); scope != scopeList.end(); scope++){
            for(SymList::iterator sym = (*scope).symList.symList.begin(); sym != (*scope).symList.symList.end() ; sym++){
                if ((*sym).getId() == _id.id){
                    return &(*sym);
                }
            }
        }

        for(SymList::iterator sym = funcList.funcList.back().symList.symList.begin(); sym != funcList.funcList.back().symList.symList.end() ; sym++){
            if ((*sym).getId() == _id.id){
                return &(*sym);
            }
        }

        return NULL;
    }
    bool isId(IDtype _id){
        return findSym(_id) || findFunc(_id) != funcList.funcList.end();
    }

};

#endif //HW3_TABLE_HPP
