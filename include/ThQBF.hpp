#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "qbf/qbf.hpp"
#include "utils/Logger.hpp"
#include "utils/QDimacsParser.hpp"

// namespace qbf
// {
//     enum class SolverStatus
//     {
//         SAT, UNSAT, PRESEARCH, SEARCH, ROOT = -1
//         /* 
//             SAT: empty matrix
//             UNSAT: empty clause detected
//             PRESEARCH: preprocessing
//             SEARCH: during search
//         */
//     };

//     inline const char* to_string(SolverStatus s) 
//     {
//         switch (s) 
//         {
//             case SolverStatus::SAT:       return "SAT";
//             case SolverStatus::UNSAT:     return "UNSAT";
//             case SolverStatus::PRESEARCH: return "PRESEARCH";
//             case SolverStatus::SEARCH:    return "SEARCH";
//             case SolverStatus::ROOT:      return "ROOT";
//         }
//         return "INVALID";
//     }
// }

namespace qbf::SolverStatus
{
    constexpr int SAT       =  1;
    constexpr int UNSAT     =  0;
    constexpr int PRESEARCH = -2;
    constexpr int SEARCH    =  2;
    constexpr int ROOT      = -1;
}

class ThQBF
{   
    private:

        int solver_status;

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
        std::stack<std::pair<int, int>> PStack               = {}; // { decision_a_var: decision_level }
        std::stack<std::pair<int, int>> SStack               = {}; // { decision_e_var: decision_level }
        std::stack<std::pair<int, int>> Search_Stack         = {}; // { decision_var: decision_level }
        std::stack<std::pair<int, int>> implied_variables    = {}; // { variableid, level }
        std::stack<std::pair<int, int>> unit_clauses         = {}; // { clauseID,   level }
        std::unordered_map<int, int>    decision_variable_at = {}; // { level: decision_variable }
        

        /* ====================== Trailing ====================== */
        std::unordered_map<int, std::set<int>> Clauses_trail   = {};  /* level: clauses changed/ removed */
        std::unordered_map<int, std::set<int>> Variables_trail = {};  /* variables affected due to a clause removal */ 


        /* ====================== Assignments ====================== */
        std::unordered_map<int, int>              Path                  = {};  /* branch assignments */
        std::vector<std::unordered_map<int, int>> TPaths                = {};  /* T paths */
        std::map<int, int>                        deduceAssignments     = {};  /* deduce assignments */
        std::map<int, int>                        preprocessAssignments = {};  /* forced assignments during preprocessin*/



        /* ====================== -------- ====================== */
        void assign                     (int varID,   int value);
        void remove_literal_from_clause (int literal, int clauseID, int positionInClause);
        void restore_level              (int search_level);
        void remove_variable            (int varID);
        void restore_variable           (int varID);
        void check_affected_vars        ();

        void remove_clause              (int clauseID);

        int clause_is_unit              (int clauseID, int referenceVarID);


        /* ====================== Implication rules ====================== */
        void UnitPropagation    ();
        void imply              ();
        void UniversalReduction ();
        void PureLiteral        ();
        void deduce             ();


        /* ====================== Clause learning ====================== */
        int conflict_clause;



        /* ====================== Cube Learning ====================== */



    public:
        ThQBF(const QDimacsParser& parser);

        void solve ();

        void print ();

};


#endif // SOLVER_HPP