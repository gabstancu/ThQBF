#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <iostream>
#include "internal.h"

namespace qbf::VariableType
{
    constexpr char EXISTENTIAL = 'e';
    constexpr char UNIVERSAL   = 'a';


    inline const char* to_string(char t) 
    {
        switch (t) 
        {
            case VariableType::EXISTENTIAL: return "∃";
            case VariableType::UNIVERSAL:   return "∀";
        }
        return "INVALID";
    }
}

namespace qbf::VariableStatus
{   

    constexpr int ACTIVE     =  1; 
    constexpr int ASSIGNED   =  3; 
    constexpr int ELIMINATED =  0; 
    constexpr int IMPLIED    =  2;
    constexpr int REMOVED    =  4;

    inline const char* to_string(int s) 
    {
        switch (s) 
        {
            case VariableStatus::ACTIVE:     return "ACTIVE";
            case VariableStatus::ASSIGNED:   return "ASSIGNED";
            case VariableStatus::ELIMINATED: return "ELIMINATED";
            case VariableStatus::IMPLIED:    return "IMPLIED";
            case VariableStatus::REMOVED:    return "REMOVED";
        }
        return "INVALID";
    }
}

struct Variable
{
    int  varID;
    int  variable;
    int  status;
    char quantifier;

    int  assignment               = UNDEFINED;
    int  level                    = UNDEFINED;
    int  available_values         = 2;


    int  assignment_trail_index   = UNDEFINED;
    int  implication_trail_index  = UNDEFINED;
    
    int  antecedent_clause        = UNDEFINED;
    int  pos_in_antecedent_clause = UNDEFINED;
    
    int  antecedent_cube          = UNDEFINED;
    int  pos_in_antecedent_cube   = UNDEFINED;


    int  blockID;
    int  positionInBlock;

    /* appearances in clauses */
    int  numNegAppear = 0, numPosAppear = 0;
    // { clauseID : position_in_clause }
    std::unordered_map<int, int> negativeOccurrences;
    std::unordered_map<int, int> positiveOccurrences;

    /* appearances in cubes */
    // { cubeID : positon_in_cube }
    int numNegAppearCubes = 0, numPosAppearCubes = 0;
    std::unordered_map<int, int> negativeOccurrencesCubes;
    std::unordered_map<int, int> positiveOccurrencesCubes;


    bool is_existential ()
    {
        return quantifier == qbf::VariableType::EXISTENTIAL;
    }

    bool is_universal ()
    {
        return quantifier == qbf::VariableType::UNIVERSAL;
    }

    bool is_impied ()
    {
        return status == qbf::VariableStatus::IMPLIED;
    }
    
    bool is_decision ()
    {
        return status == qbf::VariableStatus::ASSIGNED;
    }

    
    bool appears_in_clause (int clauseID)
    {
        return positiveOccurrences.count(clauseID) || negativeOccurrences.count(clauseID);
    }


    bool appears_in_cube (int cubeID)
    {
        return positiveOccurrencesCubes.count(cubeID) || negativeOccurrencesCubes.count(cubeID);
    }


    void addOccurence (int targetID, int position, bool polarity, const std::string target)
    {   
        if (target == "CLAUSE")
        {
            if (polarity)
            {
                positiveOccurrences.insert({targetID, position});
                numPosAppear++;
            }
            else
            {
                negativeOccurrences.insert({targetID, position});
                numNegAppear++;
            }
        }
        if (target == "CUBE")
        {
            if (polarity)
            {
                positiveOccurrencesCubes.insert({targetID, position});
                numPosAppearCubes++;
            }
            else
            {
                negativeOccurrencesCubes.insert({targetID, position});
                numNegAppearCubes++;
            }
        }  
    }

    
    int get_position_in_clause (int clauseID, bool polarity)
    {
        if (polarity) return positiveOccurrences[clauseID];
        return negativeOccurrences[clauseID];
    }


    int get_position_in_cube (int cubeID, bool polarity)
    {
        if (polarity) return positiveOccurrencesCubes[cubeID];
        return negativeOccurrencesCubes[cubeID];
    }

    void print ()
    {
        std::cout << "Status: " << qbf::VariableStatus::to_string(status) << '\n';
        std::cout << "Level: " << level << '\n';
        std::cout << "Assignment: " << assignment << '\n';
        std::cout << "Variable: " << variable << '\n';
        std::cout << "Antecedent clause: " << antecedent_clause << '\n';
        std::cout << "Position in antecedent: " << pos_in_antecedent_clause << '\n';
        std::cout << "Block: " << blockID << '\n';
        std::cout << "Quantifier: " << qbf::VariableType::to_string(quantifier) << '\n';
        std::cout << "+: " << numPosAppear << " -: " << numNegAppear << '\n';
    }
};


#endif // VARIABLE_HPP