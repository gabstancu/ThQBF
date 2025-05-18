#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Data.hpp"

class Solver
{
    private:
        SolverData& data;
        
        int state;
        int level;
        int conflict_clause;
        bool strategy_found = false;

        int GAME_FLAG;

        std::stack<std::pair<int, int>> VStack;
        std::stack<std::pair<int, int>> SStack;
        std::stack<std::pair<int, int>> Search_Stack;

        /* existential variables (use for learning) */
        std::stack<std::pair<int, int>> implied_variables; // (var, level)
        std::stack<std::pair<int, int>> unit_clauses; // (unit_clause, level)

        /* universal variables (do not use for learning) */
        std::stack<std::pair<int, int>> implied_universals;
        std::stack<std::pair<int, int>> universal_unit_clauses;

        void assign (int varID, int value, int searchLevel);
        void remove_literal_from_clause(int varID, int clauseID, int positionInClause, int searchLevel);
        // void retract_assignment (int varID, int value, int searchLevel);
        void restore_level (int searchLevel);
        void remove_variable (int varID, int searchLevel);
        void restore_variable (int varID, int searchLevel);
        void check_affected_vars(int searchLevel);

        void remove_clause(int clauseID, int searchLevel);
        void restore_clause(int clauseID, int searchLevel);
        int clause_is_unit(int clauseID, int reference_varID);

        void analyze_conflict();
        void analyze_SAT();
        void matrix_is_empty();
        void imply(int searchLevel);

        void imply_universal_move(int searchLevel);
        // void universal_reduction();
        // void pure_literal();

        int any_a_tseitin_true();
        int any_e_tseitin_true();

        
    public:
        Solver(SolverData& d);

        bool solve();
        void print();

        void print_Clauses();
        void print_Variables();
        void print_Blocks();
        void print_Prefix();

        /* 
            mode -> 0: for a random generated QBF instance
            mode -> 1: for tic-tac-toe instance
        */
        void set_mode(int mode) { GAME_FLAG = mode; };

};


#endif // SOLVER_HPP