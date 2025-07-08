#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "qbf/qbf.hpp"
#include "utils/Logger.hpp"
#include "utils/QDimacsParser.hpp"

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
        std::map<int, std::set<int>>    PREFIX;
        std::unordered_set<std::size_t> ClauseHashes; /* avoid duplicate clauses */
        std::vector<int>                P;            /* set of universal variables */
        std::vector<int>                S;            /* set of existential variables */
        int numVars;
        int numClauses;
        int numBlocks;
        int numOfExistentialVars;
        int numOfUniversalVars;
        int last_clause_idx;


        /* ====================== Search data ====================== */
        int level;
        int remainingVars;
        int remainingClauses;
        int remainingBlocks;
        std::stack<std::pair<int, int>> implied_variables = {}; // { variableid, level }
        std::stack<std::pair<int, int>> unit_clauses      = {}; // { clauseID,   level }
        

        /* ====================== Trailing ====================== */
        std::unordered_map<int, std::set<int>> Clauses_trail   = {};  /* level: clauses changed/ removed */
        std::unordered_map<int, std::set<int>> Variables_trail = {};
        std::set<int>                          varsAffected    = {};  /* variables affected during a clause removal */

        /* ====================== Assignments ====================== */
        std::unordered_map<int, int>              Path                  = {};  /* branch assignments */
        std::vector<std::unordered_map<int, int>> TPaths                = {};  /* T paths */
        std::map<int, int>                        deduceAssignments     = {};  /* deduce assignments */
        std::map<int, int>                        preprocessAssignments = {};



        /* ====================== Implication rules ====================== */
        void UnitPropagation    ();
        void UniversalReduction ();
        void PureLiteral        ();


        /* ====================== Clause learning ====================== */




        /* ====================== Cube Learning ====================== */



        void build_prefix ()
        {
            for (QuantifierBlock b : Blocks)
            {
                for (int variable : b.variables)
                {
                    this->PREFIX[b.blockID].insert(variable);
                }
            }
        }


    public:
        QBFSolver(QDimacsParser parser)
        {
            this->Clauses              = parser.matrix;
            this->Blocks               = parser.quantifier_prefix;
            this->Variables            = parser.variables; 
            this->numVars              = parser.numVars;
            this->numClauses           = parser.numClauses;
            this->numBlocks            = parser.numBlocks;
            this->numOfExistentialVars = parser.numOfExistentialVars;
            this->numOfUniversalVars   = parser.numOfUniversalVars;

            this->remainingVars        = parser.numVars;
            this->remainingClauses     = parser.numClauses;
            this->remainingBlocks      = parser.numBlocks;
        }

        void solve ();

        void print ();

};


#endif // SOLVER_HPP