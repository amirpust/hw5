//
// Created by jonat on 27/06/2021.
//
#include "Enums.hpp"

string getNewRegister(string str){
    static int nextRegister = 0;
    return "%" + str + to_string(nextRegister++);
}

string getNewGlobalRegister(string str){
    static int nextRegister = 0;
    return "@." + str + to_string(nextRegister++);
}

string getNewLabel(string label){
    static int nextLabel = 0;
    return label + to_string(nextLabel++);
}