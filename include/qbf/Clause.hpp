#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <iostream>
#include "utils/helper.hpp"
namespace qbf
{
    enum class ClauseStatus
    {
        ACTIVE, DELETED, SATISFIED, EMPTY
        /*
            ACTIVE: still in
            DELETED: removed/satisfied during preprocessing
            SATISFIED: removed/satisfied during search
            EMPTY: no literals or only universals (set solver state to unsat)
        */
    };

    enum class LiteralStatus: int32_t
    {
        AVAILABLE = -2
    };

    inline const char* to_string(ClauseStatus s) 
    {
        switch (s) 
        {
            case ClauseStatus::ACTIVE: return "ACTIVE";
            case ClauseStatus::DELETED: return "DELETED";
            case ClauseStatus::SATISFIED: return "SATISFIED";
            case ClauseStatus::EMPTY: return "EMPTY";
        }
        return "INVALID.";
    }
}

struct Clause
{
    int size;
    qbf::ClauseStatus available;
    int level;

    int num_of_unassigned, num_of_assigned;

    std::vector<int> literals, state;
    int unique_existential_position;

    int e_num, a_num;

    bool learned;

    std::size_t compute_hash() const 
    {
        std::size_t h = 0;
        for (int lit : literals) {
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
        return e_num == 1;
    }

    void print ()
    {
        printVector(literals, false);
        std::cout << "  " ;
        printVector(state, false);
        std::cout << "  level: " << level << "  ";
        std::cout << "availability: " << to_string(available) << "  ";
        std::cout << "candidate unit literal position: " << unique_existential_position << "   ";
        std::cout << "learned: " << learned << "  ";
    }
};


#endif // CLAUSE_HPP