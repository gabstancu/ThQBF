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
}

struct Clause
{
    /* data */
};


#endif // CLAUSE_HPP