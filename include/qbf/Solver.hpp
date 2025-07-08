#ifndef SOLVER_HPP
#define SOLVER_HPP

namespace qbf
{
    enum class SolverStatus
    {
        SAT, UNSAT, PRESEARCH, SEARCH
        /* 
            SAT: empty matrix
            UNSAT: empty clause detected
            PRESEARCH: preprocessing
            SEARCH: during search
        */
    };
}

class QBFSolver
{
    public:

    private:
};


#endif // SOLVER_HPP