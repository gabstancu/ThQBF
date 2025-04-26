#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Data.hpp"

class Solver
{
    private:
        SolverData& data;
        int state;
        int level;
        std::stack<std::pair<int, int>> VStack;
        std::stack<std::pair<int, int>> SStack;
        std::stack<std::pair<int, int>> Search_Stack;

        /* variable related */
        void assign (int varID, int value, int searchLevel);
        void remove_literal_from_clause(int literal, int clauseID, int positionInClause, int searchLevel);
        void retract_assignment (int varID, int value, int searchLevel);
        void restore_level (int searchLevel);
        void remove_variable (int varID, int searchLevel);
        void restore_variable (int varID, int searchLevel);
        void check_affected_vars(int searchLevel);

        /* clause related ??? */
        void remove_clause(int clauseID, int searchLevel);
        void restore_clause(int clauseID, int searchLevel);

        /* search related ("abstract" functions) */
        void analyze_conflict();
        void analyze_SAT();
        void matrix_is_empty();
        void unit_propagation();
        void restore_level();

    public:
        Solver(SolverData& d);

        bool solve();
        void print();

};


#endif // SOLVER_HPP