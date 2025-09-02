#ifndef LITERAL_HPP
#define LITERAL_HPP

#include <iostream>
#include "utils/helper.hpp"
#include "internal.h"

namespace qbf::LiteralStatus
{
    constexpr int AVAILABLE           = -2;
    constexpr int UNIVERSAL_REDUCTION = -10;
    constexpr int PURE_LITERAL        = -20;
    constexpr int ASSIGNMENT          = -30;
}

// namespace qbf::LiteralStatus
// {
//     constexpr int AVAILABLE           = -2;
//     constexpr int UNIVERSAL_REDUCTION =  0;
//     constexpr int PURE_LITERAL        =  1;
//     constexpr int ASSIGNMENT          =  2;
// }

struct Literal
{
    int value    = UNDEFINED;
    int state    = UNDEFINED; 
    int level    = UNDEFINED; // level removed
    int clauseID = UNDEFINED;


    int var ()
    {
        return std::abs(this->value);
    }

    int negation ()
    {
        return -(this->value);
    }

    bool is_positive ()
    {
        return this->value > 0;
    }

    bool is_negative ()
    {
        return this->value < 0;
    }

    bool is_available ()
    {
        return state == qbf::LiteralStatus::AVAILABLE;
    }

    bool is_reduced ()
    {
        return state == qbf::LiteralStatus::UNIVERSAL_REDUCTION;
    }
};



#endif // LITERAL_HPP
