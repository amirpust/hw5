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
#define codeBuffer (CodeBuffer::instance())


class Parser{
public:
    SymbolTable* symbolTable;

    Parser(SymbolTable* symbolTable): symbolTable(symbolTable){
        codeBuffer.firstEmit();

        //codeBuffer.emit("define i32 @main() {");
    };
    ~Parser(){
        //codeBuffer.emit("ret i32 0  ");
        //codeBuffer.emit("}");

    }

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

    Exp_t* ruleInitBool(bool val){
        Exp_t* exp =  new Exp_t(E_bool);
        codeBuffer.emit(exp->regName + " = add i32 " + to_string(val) + ", 0");
        return exp;
    }

    void ruleOpenFunctionScope(IDtype id, SymList args, Type retType){
        codeBuffer.emitFuncDefenition(id, args, retType);
        string newRbp = codeBuffer.emitAlloca();
        symbolTable->openFuncScope(id, args, retType, newRbp);
    }

    void ruleCloseFunc(){
        symbolTable->closeCurrentScope();
        codeBuffer.emitCloseFunc();
    }
};

#endif
