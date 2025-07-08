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
            case SolverStatus::SAT:       return "SAT";
            case SolverStatus::UNSAT:     return "UNSAT";
            case SolverStatus::PRESEARCH: return "PRESEARCH";
            case SolverStatus::SEARCH:    return "SEARCH";
        }
        return "INVALID";
    }
}

class QBFSolver
{   
    private:
        /* ====================== Formula data ====================== */
        std::vector<Clause>             Clauses;
        std::vector<QuantifierBlock>    Blocks;
        std::vector<Variable>           Variables;
        std::map<int, std::set<int>>    prefix;
        std::unordered_set<std::size_t> ClauseHashes; /* avoid duplicate clauses */
        std::vector<int>                P;            /* set of universal variables */
        std::vector<int>                S;            /* set of existential variables */
        int numVars,    remainingVars;
        int numClauses, remainingClauses;
        int numBlocks,  remainingBlocks;
        int numExistentialVars;
        int numUniversalVars;
        int last_clause_idx;
        
        /* ====================== Trailing ====================== */
        std::unordered_map<int, std::set<int>> Clauses_trail   = {};  /* level: clauses changed/ removed */
        std::unordered_map<int, std::set<int>> Variables_trail = {};
        std::set<int>                          varsAffected    = {};  /* variables affected during a clause removal */

        /* ====================== Assignments ====================== */
        std::map<int, int>                        TPath = {};                  /* branch assignments */
        std::vector<std::unordered_map<int, int>> TPaths = {};                 /* all T paths */
        std::map<int, int>                        deduceAssignments     = {};  /* deduce assignments */
        std::map<int, int>                        preprocessAssignments = {};


    public:
        QBFSolver(Formula qbf)
        {
            
        }

        void solve ();

        void print ();

};


#endif // SOLVER_HPP