#ifndef HW3_RELOPAUX_HPP
#define HW3_RELOPAUX_HPP

#include "Enums.hpp"
#include "Exp_t.hpp"

string llvmRelop[] = {"sgt", "slt" ,"sge" , "sle", "eq", "ne"};

class RelopAUX : public BaseObj{
public:
    RELOP_ENUM op;

    RelopAUX() : op(E_L) {};
    RelopAUX(const string& _op){
        if(_op == ">"){
            op = E_G;
        }
        if(_op == "<"){
            op = E_L;
        }
        if(_op == ">="){
            op = E_GOE;
        }
        if(_op == "<="){
            op = E_LOE;
        }
        if(_op == "=="){
            op = E_EQ;
        }
        if(_op == "!="){
            op = E_NE;
        }
    };
    string getLLVMRelop(){
        return llvmRelop[(int)(op)];
    }
};

#endif //HW3_RELOPAUX_HPP
