#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <iostream>
#include "internal.h"
namespace qbf
{
    enum class VariableStatus
    {
        ACTIVE, ASSIGNED, ELIMINATED, IMPLIED, UNDEFINED
        /*
            ACTIVE: still available
            ASSIGNED: has been assigned a value due to a decision
            ELIMINATED: eliminated during preprocessing
            IMPLIED: has been assigned due to implication
        */
    };

    enum class VariableType
    {
        EXISTENTIAL, UNIVERSAL
    };

    inline const char* to_string(VariableStatus s) 
    {
        switch (s) 
        {
            case VariableStatus::ACTIVE: return "ACTIVE";
            case VariableStatus::ASSIGNED: return "ASSIGNED";
            case VariableStatus::ELIMINATED: return "ELIMINATED";
            case VariableStatus::IMPLIED: return "IMPLIED";
            case VariableStatus::UNDEFINED: return "UNDEFINED";
        }
        return "INVALID";
    }

    inline const char* to_string(VariableType t) 
    {
        switch (t) 
        {
            case VariableType::EXISTENTIAL: return "∃";
            case VariableType::UNIVERSAL: return "∀";
        }
        return "INVALID";
    }
}

struct Variable
{
    int varID;
    int variable;
    qbf::VariableStatus status;
    qbf::VariableType quantifier;

    int assignment = UNDEFINED;
    int antecedent = UNDEFINED, pos_in_antecedent = UNDEFINED;
    int level = UNDEFINED;
    int available_values = 2;

    int blockID;
    int positionInBlock;

    int numNegAppear = 0, numPosAppear = 0;
    // { clause : position_in_clause }
    std::unordered_map<int, int> negativeOccurrences;
    std::unordered_map<int, int> positiveOccurrences;


    bool is_existential ()
    {
        return quantifier == qbf::VariableType::EXISTENTIAL;
    }

    bool is_universal ()
    {
        return quantifier == qbf::VariableType::UNIVERSAL;
    }

    bool appears_in_clause (int clauseID)
    {
        return positiveOccurrences.count(clauseID) || negativeOccurrences.count(clauseID);
    }

    void addOccurence (int clauseID, int position, bool polarity)
    {
        if (polarity)
        {
            positiveOccurrences.insert({clauseID, position});
            numPosAppear++;
        }
        else
        {
            negativeOccurrences.insert({clauseID, position});
            numNegAppear++;
        }
    }

    int get_position_in_clause (int clauseID, bool polarity)
    {
        if (polarity) return positiveOccurrences[clauseID];
        return negativeOccurrences[clauseID];
    }

    void print ()
    {
        std::cout << "Status: " << to_string(status) << '\n';
        std::cout << "Level: " << level << '\n';
        std::cout << "Assignment: " << assignment << '\n';
        std::cout << "Variable: " << variable << '\n';
        std::cout << "Antecedent clause: " << antecedent << '\n';
        std::cout << "Position in antecedent: " << pos_in_antecedent << '\n';
        std::cout << "Block: " << blockID << '\n';
        std::cout << "Quantifier: " << to_string(quantifier) << '\n';
        std::cout << "+: " << numPosAppear << " -: " << numNegAppear << '\n';
    }
};


#endif // VARIABLE_HPP