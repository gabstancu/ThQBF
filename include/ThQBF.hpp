#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "qbf/qbf.hpp"
#include "utils/Logger.hpp"
#include "utils/QDimacsParser.hpp"

namespace SolverStatus
{
    constexpr int SAT               =  1;
    constexpr int UNSAT             =  0;
    constexpr int PRESEARCH         = -1;
    constexpr int SEARCH            =  2;
    constexpr int ROOT              = -2;
    constexpr int UNDETERMINED      = -3;

    inline const char* to_string(int s) 
    {
        switch (s) 
        {
            case SolverStatus::SAT:          return "SAT";
            case SolverStatus::UNSAT:        return "UNSAT";
            case SolverStatus::PRESEARCH:    return "PRESEARCH";
            case SolverStatus::SEARCH:       return "SEARCH";
            case SolverStatus::ROOT:         return "ROOT";
            case SolverStatus::UNDETERMINED: return "UNDETERMINED";
        }
        return "INVALID.";
    }
}

class ThQBF
{   
    private:

        int solver_status;

        /* ================================ Formula data ================================ */
        std::vector<Clause>             Clauses;
        std::vector<QuantifierBlock>    Blocks;
        std::vector<Variable>           Variables;
        std::map<int, std::set<int>>    PREFIX;
        std::unordered_set<std::size_t> ClauseHashes; /* avoid duplicate clauses */
        std::vector<int>                P;            /* set of universal variables */
        std::vector<int>                S;            /* set of existential variables */

        int numVars;
        int numClauses;
        int numCubes;
        int numBlocks;
        int numOfExistentialVars;
        int numOfUniversalVars;
        int last_clause_idx;
        int cubeID;


        /* ================================ Search data ================================ */
        int level;
        int remainingVars;
        int remainingClauses;
        int remainingCubes;
        int remainingBlocks;

        std::stack<std::pair<int, int>>          PStack                     = {}; // { decision_a_var: decision_level }
        std::stack<std::pair<int, int>>          SStack                     = {}; // { decision_e_var: decision_level }
        std::stack<std::pair<int, int>>          Search_Stack               = {}; // { decision_var: decision_level }

        std::unordered_map<int, std::stack<int>> assignment_trail           = {}; // { level, assigned_variables}: variables are inserted chronologically
        std::unordered_map<int, int>             decision_variable_at       = {}; // { level: decision_variable }
        
        std::unordered_map<int, std::stack<int>> implied_e_variables        = {}; // { level, implied e variables at level}
        std::unordered_map<int, std::stack<int>> implied_a_variables        = {}; // { level, implied a variables at level}
        
        std::stack<std::pair<int, int>>          unit_clauses               = {}; // { clauseID,   level }
        std::stack<std::pair<int, int>>          unit_cubes                 = {}; // { cubeID,     level }

        /* ================================ Trailing ================================ */
        std::unordered_map<int, std::set<int>> Clauses_trail   = {};  /* level: clauses changed/ removed */
        std::unordered_map<int, std::set<int>> Cubes_trail     = {}; 
        std::set<int>                          varsAffected    = {};
        std::unordered_map<int, int>           PureLiterals    = {}; 


        /* ================================ Assignments ================================ */
        std::unordered_map<int, int>              Path                  = {};  /* branch assignments */
        std::vector<std::unordered_map<int, int>> TPaths                = {};  /* T paths */

        std::map<int, int>                        deduceAssignments     = {};  /* deduce assignments */
        std::map<int, int>                        preprocessAssignments = {};  /* forced assignments during preprocessing */


        /* ================================ General ================================ */
        void assign                     (int variable,   int value); 
        
        void remove_literal_from_clause (int literal, int clauseID, int positionInClause);
        void remove_literal_from_cube   (int literal, int cubeID,   int positionInCube);
        
        void remove_clause              (int clauseID, int referenceVarID);
        void remove_cube                (int cubeID,   int referenceVarID);
        
        int  clause_is_unit             (int clauseID, int referenceVariable);
        int  cube_is_unit               (int cubeID,   int referenceVariable); // TODO: (cube_is_unit) write body

        void restore_level              (int search_level);
        void remove_variable            (int variable);
        void restore_variable           (int variable);


        /* ================================ Inference ================================ */
        void UnitPropagation    ();
        void UniversalReduction (int clauseID);
        void PureLiteral        ();
        void deduce             ();
        void imply              (); // TODO: (imply) both unit clauses and cubes


        /* ================================ Clause learning ================================ */
        int              conflict_clause;
        std::vector<int> conficting_clauses = {};

        int                          choose_e_literal       (std::unordered_map<int, int> cc);
        std::pair<int, int>          clause_asserting_level (const std::unordered_map<int, int>& learned_clause);
        std::unordered_map<int, int> resolve                (const std::unordered_map<int, int>& c1, 
                                                             const std::unordered_map<int, int>& c2, 
                                                             int pivot_variable);
        bool                         stop_criteria_met      (const std::unordered_map<int, int>& resolvent);
        void                         add_clause_to_db       (const std::unordered_map<int, int>& learned_clause, int asserting_literal);
        std::pair<int, int>          analyse_conflict       ();


        /* ================================ Cube Learning ================================ */
        std::vector<Cube>               Cubes;
        std::unordered_set<std::size_t> CubeHashes; /* avoid duplicate cubes */
        
        std::unordered_map<int, int> find_SAT_cube              (); // TODO: (find_SAT_cube)
        std::unordered_map<int, int> construct_SAT_induced_cube (const std::unordered_map<int, int>& Path); // TODO: (construct_SAT_induced_cube)
        bool                         cube_stop_criteria_met     (const std::unordered_map<int, int>& cube); // TODO: (cube_stop_criteria_met)
        std::pair<int, int>          cube_asserting_level       (const std::unordered_map<int, int>& learned_cube); // TODO: (cube_asserting_level)
        
        std::unordered_map<int, int> consensus_gen_cube         (const std::unordered_map<int, int>& cube); // TODO: (consensus_gen_cube)
        int                          choose_a_literal           (const std::unordered_map<int, int>& sc); // TODO: (choose_a_literal)
        void                         add_cube_to_db             (const std::unordered_map<int, int>& learned_cube, int asserting_literal);
        std::pair<int, int>          analyse_SAT                (); // TODO: (analyse_SAT)


        /* ================================ Solver utils ================================ */
        void print_Clauses   ();
        void print_Cubes     ();
        void print_Variables ();
        void print_Blocks    ();
        void print_Prefix    ();



    public:
        ThQBF(const QDimacsParser& parser);

        void solve ();
        void test  ();

        void print ();

};


#endif // SOLVER_HPP