#ifndef VARIABLE_HPP
#define VARIABLE_HPP

namespace qbf
{
    enum class VariableStatus
    {
        ACTIVE, ASSIGNED, ELIMINATED, IMPLIED
        /*
            ACTIVE: still available
            ASSIGNED: has been assigned a value due to a decision
            ELIMINATED: eliminated during preprocessing
            IMPLIED: has been assigned due to implication
        */
    };
}

struct Variable
{
    /* data */
};


#endif // VARIABLE_HPP