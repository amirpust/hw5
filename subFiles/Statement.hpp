//
// Created by jonat on 29/06/2021.
//

#ifndef HW5_STATEMENT_HPP
#define HW5_STATEMENT_HPP

#include <iostream>
#include <string>
#include "hw3_output.hpp"
#include "Enums.hpp"
#include "BaseObj.hpp"

class Statement : public BaseObj {
public:
    ContList contList;
    BreakList breakList;
    SwitchLabelList switchLabel;
};

#endif //HW5_STATEMENT_HPP
