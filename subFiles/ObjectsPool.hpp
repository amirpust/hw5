#ifndef U_HPP
#define U_HPP

#include <iostream>
#include <string>
#include "hw3_output.hpp"
#include "Enums.hpp"
#include "RelopAUX.hpp"
#include "Exp_t.hpp"
#include "Table.hpp"

using namespace std;
extern int lineno;

typedef struct objects_pool_t{
    string idVal;
    ExpList expList;
    TYPE typeVal;
    Symbol symbol;
    SymList symList;

}objects_pool;


#endif
