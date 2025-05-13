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

        std::stack<std::pair<int, int>> VStack;
        std::stack<std::pair<int, int>> SStack;
        std::stack<std::pair<int, int>> Search_Stack;
        std::stack<std::pair<int, int>> implied_variables; // (var, level)
        std::stack<std::pair<int, int>> unit_clauses; // (unit_clause, level)

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
        // void universal_reduction();
        // void pure_literal();

        void propagate_in_tseitin(int varID, int value, int searchLevel);
        bool any_tseitin_true();
        
    public:
        Solver(SolverData& d);

        bool solve();
        void print();

        void print_Clauses();
        void print_Variables();
        void print_Blocks();
        void print_Prefix();

};


#endif // SOLVER_HPP