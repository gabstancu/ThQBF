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

    inline const char* to_string(VariableStatus s) 
    {
        switch (s) 
        {
            case VariableStatus::ACTIVE: return "ACTIVE";
            case VariableStatus::ASSIGNED: return "ASSIGNED";
            case VariableStatus::ELIMINATED: return "ELIMINATED";
            case VariableStatus::IMPLIED: return "IMPLIED";
        }
        return "INVALID";
    }
}

struct Variable
{
    int varID;
    qbf::VariableStatus status;
};


#endif // VARIABLE_HPP