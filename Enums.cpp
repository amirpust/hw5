//
// Created by jonat on 27/06/2021.
//
#include "Enums.hpp"

string getNewRegister(){
    static int nextRegister = 0;
    return "%reg" + to_string(nextRegister++);
}