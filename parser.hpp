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
        codeBuffer.emit("@.intFormat = internal constant [4 x i8] c\"%d\\0A\\00\"");
        codeBuffer.emit("%format_ptr = getelementptr [4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0");

        codeBuffer.emit("declare i32 @printf(i8*, …)");
        codeBuffer.emit("declare void @exit(i32)");
        codeBuffer.emit("DIVIDE_BY_ZERO:");
        //codeBuffer.emit("%printf_retval = call i32 (i8*, ...) @printf(i8* %format_ptr, i32 ");
        codeBuffer.emit("%printf_retval = call i32 (i8*, ...) @printf(\"Error division by zero\n\")");
        codeBuffer.emit("@exit(0)");


        codeBuffer.emit("define i32 @main() {");
    };
    ~Parser(){
        codeBuffer.emit("ret i32 0  ");
        codeBuffer.emit("}");

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
            string nextLabel = getNewLabel("GOOD_DIV");
            Exp_t ifZero(E_bool);
            codeBuffer.emit(ifZero.regName + " = icmp eq i32 " + exp2->regName + ", 0");
            codeBuffer.emit("br i1 " + ifZero.regName + ", label %DIVIDE_BY_ZERO, label %" + nextLabel);
            codeBuffer.emit(nextLabel + ":");
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
};

#endif
