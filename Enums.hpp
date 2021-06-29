#ifndef HW3_ENUMS_HPP
#define HW3_ENUMS_HPP

#include <string>
#include <vector>

extern int yylineno;
using namespace std;

enum TYPE {
    E_def,
    E_bool,
    E_byte,
    E_int,
    E_string,
    E_void
};

const std::string typeStr[] = {
        "default",
        "BOOL",
        "BYTE",
        "INT",
        "STRING",
        "VOID"
};

enum RELOP_ENUM{
    E_G,
    E_L,
    E_GOE,
    E_LOE,
    E_EQ,
    E_NE
};

enum SCOPE_REASON{
    GLOBAL_SCOPE,
    REGULAR_SCOPE,
    IF_SCOPE,
    CASE_SCOPE,
    SWITCH_SCOPE,
    LOOP_SCOPE,
    FUNC_SCOPE
};


string getNewRegister(string reg = "reg");
string getNewGlobalRegister(string str = "glob");
string getNewLabel(string label = "Label");

enum BranchLabelIndex {FIRST, SECOND};
typedef vector<pair<int,BranchLabelIndex>> TrueList;
typedef vector<pair<int,BranchLabelIndex>> FalseList;
typedef vector<pair<int,BranchLabelIndex>> NextList;
typedef vector<pair<int,BranchLabelIndex>> ContList;
typedef vector<pair<int,BranchLabelIndex>> BreakList;
#endif //HW3_ENUMS_HPP
