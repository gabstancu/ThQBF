#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "qbf/qbf.hpp"

namespace qbf
{
    enum class SolverStatus
    {
        SAT, UNSAT, PRESEARCH, SEARCH, ROOT = -1
        /* 
            SAT: empty matrix
            UNSAT: empty clause detected
            PRESEARCH: preprocessing
            SEARCH: during search
        */
    };

    inline const char* to_string(SolverStatus s) 
    {
        switch (s) 
        {
            case SolverStatus::SAT: return "SAT";
            case SolverStatus::UNSAT: return "UNSAT";
            case SolverStatus::PRESEARCH: return "PRESEARCH";
            case SolverStatus::SEARCH: return "SEARCH";
        }
        return "INVALID";
    }
}

class QBFSolver
{
    public:

    private:
};


#endif // SOLVER_HPP