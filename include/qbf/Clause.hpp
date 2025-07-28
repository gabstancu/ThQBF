#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <iostream>
#include "utils/helper.hpp"

namespace qbf::ClauseStatus
{
    constexpr int ACTIVE    =  1;
    constexpr int DELETED   = -1;
    constexpr int SATISFIED = -2;

    inline const char* to_string(int s) 
    {
        switch (s) 
        {
            case ClauseStatus::ACTIVE:    return "ACTIVE";
            case ClauseStatus::DELETED:   return "DELETED";
            case ClauseStatus::SATISFIED: return "SATISFIED";
        }
        return "INVALID.";
    }
}

namespace qbf::LiteralStatus
{
    constexpr int AVAILABLE           = -2;
    constexpr int UNIVERSAL_REDUCTION =  0;
    constexpr int PURE_LITERAL        =  1;
    constexpr int ASSIGNMENT          =  2;
}


struct Clause
{
    int size;
    int status;
    int level;
    int clauseID;

    int num_of_unassigned, num_of_assigned;

    std::vector<int> literals, state, reason;
    int              unit_literal_position;
    int              e_num = 0, a_num = 0;
    bool             learned;

    size_t hash;

    std::size_t compute_hash() const 
    {
        std::size_t h = 0;
        for (int lit : literals) 
        {
            h ^= std::hash<int>{}(lit) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }

    bool is_empty ()
    {
        return (e_num == 0 && a_num == 0);
    }

    bool is_unit ()
    {
        return e_num == 1 && a_num == 0;
    }

    bool is_literal_available (int position)
    {
        return state[position] == qbf::LiteralStatus::AVAILABLE;
    }

    void print ()
    {
        printVector(literals, false);
        std::cout << "  " ;
        printVector(state, false);
        std::cout << "  level: " << level << "  ";
        std::cout << "availability: " << qbf::ClauseStatus::to_string(status) << "  ";
        std::cout << "candidate unit literal position: " << unit_literal_position << "   ";
        std::cout << "learned: " << learned << "  ";
    }
};


#endif // CLAUSE_HPP