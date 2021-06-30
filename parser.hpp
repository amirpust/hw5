#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <string>
#include "hw3_output.hpp"
#include "Enums.hpp"
#include "RelopAUX.hpp"
#include "Exp_t.hpp"
#include "Statement.hpp"
#include "Table.hpp"
#include "bp.hpp"
#include "CaseList.hpp"
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

    void ruleCloseFunc(Type t){
        symbolTable->closeCurrentScope();

        codeBuffer.emitCloseFunc(t);
    }

    void ruleAddSymbol(Type t, IDtype id, Exp_t exp){
        output::printLog("Add Symbol Rule | id: " + id.id + " | exp: " + exp.regName);
        Exp_t newExp(t);
        symbolTable->addSymbol(id, &newExp);
        ruleAssign(id, exp);
    }

    void ruleAssign(IDtype id, Exp_t exp){
        Exp_t dst = symbolTable->getExpByID(id);
        symbolTable->assign(id, exp);

        if (exp.t == E_bool){
            Exp_t* newSrc = boolToExp(exp);
            codeBuffer.emitAssign(&dst, newSrc, symbolTable->getCurrentRbp());
            delete newSrc;
            return;
        }

        codeBuffer.emitAssign(&dst, &exp, symbolTable->getCurrentRbp());
    }

    Exp_t* boolToExp(Exp_t exp){
        string tl, fl, nl;
        NextList nextList;
        int n1, n2;
        Exp_t* newSrc = new Exp_t(E_bool);

        tl = getNewLabel("PHI_TRUE_LABEL");
        codeBuffer.emitUnconditinalJump(tl);
        codeBuffer.emit(tl+":");
        n1 = codeBuffer.emitUnconditinalJump("@");

        fl = getNewLabel("PHI_FALSE_LABEL");
        codeBuffer.emit(fl+":");
        n2 = codeBuffer.emitUnconditinalJump("@");

        nl = getNewLabel("PHI_NEXT_LABEL");
        codeBuffer.emit(nl+":");

        nextList = codeBuffer.merge(
                codeBuffer.makelist(pair<int, BranchLabelIndex >(n1, FIRST)),
                codeBuffer.makelist(pair<int, BranchLabelIndex >(n2, FIRST))
        );

        codeBuffer.bpatch(exp.trueList, tl);
        codeBuffer.bpatch(exp.falseList, fl);
        codeBuffer.bpatch(nextList, nl);

        codeBuffer.emitPhi(newSrc, tl, fl);

        return newSrc;
    }

    Exp_t* ruleLoadExpById(IDtype id){
        Exp_t* exp = new Exp_t(symbolTable->getExpByID(id));
        if (exp->offset < 0){
            exp->regName = "%" + to_string(((-1)*exp->offset) - 1);
        }else{
            codeBuffer.emitLoad(exp, symbolTable->getCurrentRbp());
        }

        if(exp->t == E_bool){
             string cmp = getNewRegister("cmp_res");
             codeBuffer.emit(cmp + " = icmp ne i32 0, " + exp->regName);
             int address = codeBuffer.emitConditinalJump(cmp, "@", "@");
             exp->trueList = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, FIRST));
             exp->falseList = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, SECOND));
        }
        return exp;
    }

    Exp_t* ruleCallFunc(IDtype funcName, ExpList arguments){
        Exp_t* retVal = new Exp_t(symbolTable->callFunc(funcName, arguments));
        output::printLog("ruleCallFunc " + funcName.id);
        ExpList reversedArgs;
        for (int i = arguments.expList.size() - 1; i >= 0 ; i--){
            if (arguments.expList[i].t.t == E_bool){
                Exp_t* newExp = boolToExp(arguments.expList[i]);
                arguments.expList[i] = Exp_t(*newExp);
                delete newExp;
            }
            reversedArgs.insert(arguments.expList[i]);

        }

        for (auto arg : reversedArgs.expList){
            output::printLog(arg.regName + ": " + to_string(arg.offset));

        }

        codeBuffer.emitCallFunc(retVal, funcName, reversedArgs);
        if(retVal->t == E_bool){
           Exp_t* newRetVal = boolToExp(*retVal);
           delete retVal;
           return newRetVal;
        }
        return retVal;
    }

    void ruleReturn(Exp_t exp){
        symbolTable->checkReturnType(exp);
        if (exp.t == E_bool){
            Exp_t* newExp = boolToExp(exp);
            codeBuffer.emitReturn(newExp);
            delete newExp;
            return;
        }
        codeBuffer.emitReturn(&exp);
    }

    Exp_t* ruleInitString(String newStr){
        output::printLog(newStr.val);
        Exp_t* exp = new Exp_t(E_string);
        codeBuffer.emitSaveString(exp, newStr);
        return exp;
    }

    // IF statments
    String* ruleGenLabel(string prefix = "IF_LABEL"){
        String* newLabel = new String(getNewLabel(prefix));
        codeBuffer.emitUnconditinalJump(newLabel->val);
        newLabel->address = codeBuffer.emit(newLabel->val+":");
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
        String* nextLabel = ruleGenLabel("NEXT_LABEL");
        codeBuffer.bpatch(exp.falseList, nextLabel->val);
        codeBuffer.bpatch(exp.nextList, nextLabel->val);
        delete nextLabel;
    }

    void ruleIf(Exp_t exp, String trueS, String falseS){
        codeBuffer.bpatch(exp.trueList, trueS.val);
        codeBuffer.bpatch(exp.falseList, falseS.val);

        String* nextLabel = ruleGenLabel("NEXT_LABEL");
        codeBuffer.bpatch(exp.nextList, nextLabel->val);
        delete nextLabel;
    }

    void ruleWhile(Exp_t exp, String expS, String trueS, Statement statement){
        codeBuffer.emitUnconditinalJump(expS.val);
        String* nextLabel = ruleGenLabel("NEXT_LABEL");

        codeBuffer.bpatch(exp.trueList, trueS.val);
        codeBuffer.bpatch(exp.falseList, nextLabel->val);
        codeBuffer.bpatch(exp.nextList, nextLabel->val);
        codeBuffer.bpatch(statement.breakList, nextLabel->val);
        codeBuffer.bpatch(statement.contList, expS.val);
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


    //Statements
    void mergeStatement(Statement* parent, Statement s){
        parent->contList = codeBuffer.merge(parent->contList, s.contList);
        parent->breakList = codeBuffer.merge(parent->breakList, s.breakList);
    }

    void placeBreak(Statement* parent){
        int address = codeBuffer.emitUnconditinalJump("@");
        parent->breakList = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, FIRST));
    }

    void placeCont(Statement* parent){
        int address = codeBuffer.emitUnconditinalJump("@");
        parent->contList = codeBuffer.makelist(pair<int, BranchLabelIndex>(address, FIRST));
    }

    //CaseList
    void ruleAddCase(CaseList* caseList, Num num, Statement statement, String caseLabel){
        output::printLog("--------START ruleAddCase-------- ");
        caseList->caseList.emplace_back(caseLabel.val, num.val);
        caseList->contList = codeBuffer.merge(caseList->contList, statement.contList);
        caseList->breakList = codeBuffer.merge(caseList->breakList, statement.breakList);
        output::printLog("--------START ruleAddCase-------- ");
    }
    void ruleSeenDefault(CaseList* caseList, Statement statement,String defaultLabel){
        output::printLog("--------START ruleSeenDefault-------- ");
        caseList->seenDefault = true;
        caseList->DefaultLabel = defaultLabel.val;
        caseList->contList = codeBuffer.merge(caseList->contList, statement.contList);
        caseList->breakList = codeBuffer.merge(caseList->breakList, statement.breakList);
        output::printLog("--------END ruleSeenDefault-------- ");
    }
    CaseList* ruleMergeCaseLists(CaseList caseList1, CaseList caseList2){
        output::printLog("--------START ruleSeenDefault-------- ");
        CaseList* newCaseList = new CaseList();
        newCaseList->seenDefault = caseList1.seenDefault || caseList2.seenDefault;
        newCaseList->DefaultLabel = caseList1.DefaultLabel + caseList2.DefaultLabel;
        newCaseList->contList = codeBuffer.merge(caseList1.contList, caseList2.contList);
        newCaseList->breakList = codeBuffer.merge(caseList1.breakList, caseList2.breakList);
        newCaseList->caseList.insert(newCaseList->caseList.end(), caseList1.caseList.begin(), caseList1.caseList.end());
        newCaseList->caseList.insert(newCaseList->caseList.end(), caseList2.caseList.begin(), caseList2.caseList.end());
        output::printLog("--------END ruleMergeCaseLists-------- ");
        return newCaseList;
    }
    void ruleSwitch(Statement* switchStatement, Exp_t exp, CaseList caseList){
        //switch <intty> <value>, label <defaultdest> [ <intty> <val>, label <dest> ... ]

        output::printLog("--------START ruleSwitch-------- ");
        placeBreak(switchStatement);
        String* startLabel = ruleGenLabel("START_SWITCH");
        output::printLog("--------before BP ruleSwitch-------- ");
        codeBuffer.bpatch(switchStatement->switchLabel, startLabel->val);
        output::printLog("--------after BP ruleSwitch-------- ");
        string nextLabel = getNewLabel("NEXT_SWITCH");
        if(!caseList.seenDefault){
            caseList.DefaultLabel = nextLabel;
        }
        string toEmit = "switch i32 " + exp.regName + ", label %" + caseList.DefaultLabel + " [ ";
        for(auto Case : caseList.caseList){

            toEmit += "\n";

            toEmit += "i32 " + to_string(Case.second) + ", label %" + Case.first;
        }
        toEmit += "]";
        codeBuffer.emit(toEmit);
        codeBuffer.emitUnconditinalJump(nextLabel);
        codeBuffer.emit(nextLabel + ":");
        codeBuffer.bpatch(caseList.breakList, nextLabel);
        codeBuffer.bpatch(switchStatement->breakList, nextLabel);
        switchStatement->breakList = BreakList();
        switchStatement->contList = codeBuffer.merge(switchStatement->contList, caseList.contList);
        delete startLabel;
        output::printLog("--------END ruleSwitch-------- ");
    }

    void ruleInitSwitch(Statement* statement){
        int switchAddr = codeBuffer.emitUnconditinalJump("@");
        statement->switchLabel = codeBuffer.makelist(pair<int, BranchLabelIndex>(switchAddr, FIRST));
    }

    Exp_t* ruleDefaultInit(Type t){
        if(t.t == E_bool){
            return ruleInitBool(false);
        }
        return ruleInitNum(t.t, Num(0));
    }
};

#endif
