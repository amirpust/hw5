#ifndef HW3_EXP_T_HPP
#define HW3_EXP_T_HPP

#include <iostream>
#include <string>
#include "hw3_output.hpp"
#include "Enums.hpp"
#include "BaseObj.hpp"

class Exp_t : public BaseObj{
public:
    Type t;
    int offset;
    Exp_t(): t(E_def) {};
    explicit Exp_t(Type t) : t(t), offset(-1) {};
    Exp_t(Type t, int i) : t(t), offset(-1)  {
        if ( t == E_byte && i >= (1 << 8)){
            output::errorByteTooLarge(yylineno, i);
            exit(5456);
        }
    };

    Exp_t(Type t, int i, int expOffset) : t(t), offset(expOffset)  {
        if ( t == E_byte && i >= (1 << 8)){
            output::errorByteTooLarge(yylineno, i);
            exit(5456);
        }
    };

    Exp_t(const Exp_t& c) = default;

    bool isBool() const {
        return t == E_bool;
    }
    bool isNumerical() const{
        return t == E_int || t == E_byte;
    }
    Type getDualType(const Exp_t& e) const {
        if (t == E_byte && e.t == E_byte)
            return E_byte;

        return E_int;
    }
    bool castType(Type newT){
        if (t == newT){
            return true;
        }

        if(newT == E_void || t == E_void){
            output::errorMismatch(yylineno);
            output::printLog("Casting void");
            exit(1);
        }

        if (newT == E_int && t == E_byte){
            t = E_int;
            return true;
        }

        if (t == E_def){
            t = newT;
            return true;
        }

        return false;
    }

    /*
    Exp_t& operator=(const Exp_t& e){
        if (!Exp_t(e).castType(t)){
            output::errorMismatch(yylineno);
            output::printLog("Flag casting from " + e.t.getStr() + " to " + t.getStr());
            exit(-1);
        }
        return (*this);
    };*/
    /*
    Exp_t operator+(const Exp_t& e) const {
        if(!isNumerical() || !e.isNumerical()){
            exit(621);
        }

        return Exp_t(getDualType(e)) ;
    }
    Exp_t operator-(const Exp_t& e) const {
        if(!isNumerical() || !e.isNumerical()){
        }

        return Exp_t(getDualType(e)) ;
    }
    */
    /*
    bool operator==(const Exp_t& e){
        if ( isNumerical() && e.isNumerical()){
            return i == e.i;
        }
        if (t == E_bool && e.t == E_bool){
            return i == e.i;
        }
        if ( t == E_string && e.t == E_string){
            return s == e.s;
        }
        return false;
    }
    bool operator<=(const Exp_t& e){
        if (!isNumerical() || !e.isNumerical()){
        }

        return t <= e.t;
    }
    bool operator<(const Exp_t& e){
        if (!isNumerical() || !e.isNumerical()){
        }

        return t < e.t;
    } */
};


class ExpList : public BaseObj{
public:
    vector<Exp_t> expList;
    ExpList(const vector<Exp_t>& _expList) : expList(_expList) {}
    ExpList() : expList() {};

    void insert(Exp_t _e){
        expList.push_back(_e);
    }

    typedef vector<Exp_t>::iterator iterator;
};

#endif //HW3_EXP_T_HPP
