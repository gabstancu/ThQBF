#ifndef CLAUSE_HPP
#define CLAUSE_HPP

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

    inline const char* to_string(ClauseStatus s) {
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
    /* data */
};


#endif // CLAUSE_HPP