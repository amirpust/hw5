#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <string>
#include "hw3_output.hpp"
#include "Enums.hpp"
#include "RelopAUX.hpp"
#include "Exp_t.hpp"
#include "Table.hpp"
#include "bp.hpp"
using namespace std;
#define codeBuffer (CodeBuffer::instance())


class Parser{
public:
    SymbolTable* symbolTable;

    Parser(SymbolTable* symbolTable): symbolTable(symbolTable){
        codeBuffer.firstEmit();

        //codeBuffer.emit("define i32 @main() {");
    };

    Exp_t* ruleOP(Exp_t* exp1, Exp_t* exp2, string op){

        if(!exp1->isNumerical() || !exp2->isNumerical()){
            output::errorMismatch(yylineno);
            output::printLog("isNumerical");
            exit(12);
        };
        Exp_t* res = new Exp_t(Type(exp1->getDualType(*exp2)));

        if(op != "div"){
            codeBuffer.emitOp(res, exp1, op ,exp2);
        }else{
            codeBuffer.emit("call void @checkDivideByZero(i32 " + exp2->regName + ")");
            if(exp1->getDualType((*exp2)) == E_int){
                codeBuffer.emitOp(res, exp1, "sdiv" ,exp2);
            }else{
                codeBuffer.emitOp(res, exp1, "udiv" ,exp2);
            }
        }

        delete exp1;
        delete exp2;
        return res;
    }

    Exp_t* ruleInitNum(TYPE t, const Num& n){
        if (t == E_byte && n.val >= 1 << 8){
            output::errorByteTooLarge(yylineno, n.val);
            exit(1);
        }

        Exp_t* exp =  new Exp_t(Type(t));

        codeBuffer.emit(exp->regName + " = add i32 " + to_string(n.val) + ", 0");
        return exp;
    }

    void ruleOpenFunctionScope(IDtype id, SymList args, Type retType){
        codeBuffer.emitFuncDefenition(id, args, retType);
        string newRbp = codeBuffer.emitAlloca();
        output::printLog("ruleOpenFunctionScope - new rbp: " + newRbp);
        symbolTable->openFuncScope(id, args, retType, newRbp);
        output::printLog("ruleOpenFunctionScope - end");
    }

    void ruleCloseFunc(){
        symbolTable->closeCurrentScope();
        codeBuffer.emitCloseFunc();
    }

    void ruleAddSymbol(Type t, IDtype id, Exp_t exp){
        output::printLog("Add Symbol Rule | id: " + id.id + " | exp: " + exp.regName);
        Exp_t newExp(t);
        symbolTable->addSymbol(id, &newExp);
        symbolTable->assign(id, exp);
        codeBuffer.emitAssign(&newExp, &exp, symbolTable->getCurrentRbp());
    }

    void ruleAssign(IDtype id, Exp_t exp){
        Exp_t dst = symbolTable->getExpByID(id);
        codeBuffer.emitAssign(&dst, &exp, symbolTable->getCurrentRbp());
    }

    Exp_t* ruleLoadExpById(IDtype id){
        Exp_t* exp = new Exp_t(symbolTable->getExpByID(id));
        if (exp->offset < 0){
            exp->regName = "%" + to_string(((-1)*exp->offset) - 1);
            return exp;
        }
        codeBuffer.emitLoad(exp, symbolTable->getCurrentRbp());
        return exp;
    }

    Exp_t* ruleCallFunc(IDtype funcName, ExpList arguments){
        Exp_t* retVal = new Exp_t(symbolTable->callFunc(funcName, arguments));
        output::printLog("ruleCallFunc " + funcName.id);
        ExpList reversedArgs;
        for (int i = arguments.expList.size() - 1; i >= 0 ; i--){
            reversedArgs.insert(arguments.expList[i]);
        }

        for (auto arg : reversedArgs.expList){
            output::printLog(arg.regName + ": " + to_string(arg.offset));

        }

        codeBuffer.emitCallFunc(retVal, funcName, reversedArgs);
        return retVal;
    }

    void ruleReturn(Exp_t exp){
        symbolTable->checkReturnType(exp);
        codeBuffer.emitReturn(&exp);
    }

    Exp_t* ruleInitString(String newStr){
        output::printLog(newStr.val);
        Exp_t* exp = new Exp_t(E_string);
        codeBuffer.emitSaveString(exp, newStr);
        return exp;
    }

    // IF statments
    String* ruleGenLabel(){
        String* newLabel = new String(getNewLabel("IF_LABEL"));
        codeBuffer.emitUnconditinalJump(newLabel->val);
        codeBuffer.emit(newLabel->val+":");
        return newLabel;
    }

    Exp_t* ruleInitBool(bool val){
        Exp_t* exp =  new Exp_t(E_bool);
        int address = codeBuffer.emit("br label @");
        if (val){
            exp->trueList = codeBuffer.makelist(pair<int, BranchLabelIndex >(address, FIRST));
        }else{
            exp->falseList = codeBuffer.makelist(pair<int, BranchLabelIndex >(address, SECOND));
        }

        return exp;
    }

    void ruleIf(Exp_t exp, String trueS){
        codeBuffer.bpatch(exp.trueList, trueS.val);
        String* nextLabel = ruleGenLabel();
        codeBuffer.bpatch(exp.falseList, nextLabel->val);
        codeBuffer.bpatch(exp.nextList, nextLabel->val);
        delete nextLabel;
    }

    void ruleIf(Exp_t exp, String trueS, String falseS){
        codeBuffer.bpatch(exp.trueList, trueS.val);
        codeBuffer.bpatch(exp.falseList, falseS.val);

        String* nextLabel = ruleGenLabel();
        codeBuffer.bpatch(exp.nextList, nextLabel->val);
        delete nextLabel;
    }

    void ruleGenNextLabel(Exp_t* parent){
        int address = codeBuffer.emitUnconditinalJump("@");
        NextList nl = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, FIRST));
        parent->nextList = codeBuffer.merge(nl, parent->nextList);
    }

    void ruleLogicalAnd(Exp_t* parent, Exp_t E1, Exp_t E2, String label){
        codeBuffer.bpatch(E1.trueList, label.val);
        parent->trueList = TrueList(E2.trueList);
        parent->falseList = codeBuffer.merge(E1.falseList, E2.falseList);
    }

    void ruleLogicalOr(Exp_t* parent, Exp_t E1, Exp_t E2, String label){
        codeBuffer.bpatch(E1.falseList, label.val);
        parent->falseList = FalseList(E2.falseList);
        parent->trueList = codeBuffer.merge(E1.trueList, E2.trueList);
    }
    void ruleLogicalNot(Exp_t* parent, Exp_t E){
        parent->falseList = FalseList(E.trueList);
        parent->trueList = TrueList(E.falseList);
    }

    void ruleRelop(Exp_t* parent, Exp_t exp1, Exp_t exp2, RelopAUX op){
        codeBuffer.emitRelop(parent, exp1, exp2, op);
        int address = codeBuffer.emitConditinalJump(parent->regName, "@", "@");
        parent->trueList = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, FIRST));
        parent->falseList = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, SECOND));
    }
};

#endif
