//
// Created by Amir on 30/06/2021.
//

#ifndef HW5_CASELIST_HPP
#define HW5_CASELIST_HPP

#include <iostream>
#include <string>
#include "hw3_output.hpp"
#include "Enums.hpp"
#include "BaseObj.hpp"

class CaseList : public BaseObj{
public:
    vector<pair<string, int>> caseList;
    BreakList breakList;
    ContList contList;
    bool seenDefault;
    string DefaultLabel;

    CaseList():seenDefault(false){};

};


#endif //HW5_CASELIST_HPP
