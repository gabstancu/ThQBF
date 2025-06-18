#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Data.hpp"

class Solver
{
    private:
        SolverData& data;
        
        int state;
        int level;
        int conflicting_clause;
        bool strategy_found = false;

        int GAME_FLAG;

        std::stack<std::pair<int, int>> PStack; // { decision_a_var: decision_level }
        std::stack<std::pair<int, int>> SStack; // { decision_e_var: decision_level }
        std::stack<std::pair<int, int>> Search_Stack; // {decision_var: decision_level}
        std::unordered_map<int, int> decision_variable_at; // { level: decision_variable}

        /* existential variables (use for learning) */
        std::stack<std::pair<int, int>> implied_variables; // (var, level)
        std::stack<std::pair<int, int>> unit_clauses; // (unit_clause, level)

        std::set<int> conflicting_clauses; // (conlicting_clause, level)

        /* universal variables (do not use for learning): game mode is on */
        std::stack<std::pair<int, int>> implied_universals; // (var, level)
        std::stack<std::pair<int, int>> universal_unit_clauses; // (unit_clause, level)

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

        /* clause learning: game mode off */
        void analyze_conflict(); // entry point
        int choose_literal(std::vector<int> cl); // pick most recently implied literal
        std::unordered_map<int, int> resolve(std::vector<int> c1, std::vector<int> c2, int pivot_literal); // resolve with respect to the pivot variable
        bool stop_criterion_met(std::unordered_map<int, int> c1, int currentSearchLevel); // 
        int clause_asserting_level(std::vector<int> cl_vec); // returns the backtracking level (the clause becomes unit at that level)

        /* cube learning */
        void analyze_SAT();
        void matrix_is_empty();
        void imply(int searchLevel);

        // void universal_reduction();
        // void pure_literal();
        void unit_propagation();

        std::unordered_map<int, int> vector_to_hashmap(std::vector<int> literals){
            std::unordered_map<int, int> clause = {};
            
            for (int literal : literals)
                (literal > 0) ? clause.insert({literal, 1}) : clause.insert({literal, 0});

            return clause;
        };

        std::vector<int> hashmap_to_vec(std::unordered_map<int, int> literals){
            std::vector<int> clause = {};
            
            for (const auto& [literal, sign] : literals)
            {
                clause.push_back(literal);
            }
            return clause;
        };

        
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
            mode -> 1: for a tic-tac-toe instance
        */
        void set_mode(int mode) { GAME_FLAG = mode; };

};


#endif // SOLVER_HPP