#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <vector>
#include "Constants.hpp"
#include "utils.hpp"

class Clause
{
    private:
        size_t size; // number of literals in the clause
        bool available; 
        int level; // level at which the clause was removed

        int unassigned; // number of literals that have not been assigned
        int assigned;

        std::vector<int> literals;
        std::vector<int> state;

        int e_num; // number of existential literals active
        int a_num; // number of universal literals active

        bool learned = false;

    public:
        Clause (const std::vector<int>& literals, 
                const std::vector<int>& state, 
                int level,
                bool learned);
        
        /* Mutators (during search) */
        void set_size(size_t s) { size = s; }
        void set_availability(bool status) { available = status; }
        void set_level(int lvl) { level = lvl; }
        void set_unassigned(int u) { unassigned = u; }
        void set_assigned(int a) { assigned = a; }
        std::vector<int>& get_literals() { return literals; }
        std::vector<int>& get_state() { return state; }

        /* Read only access */
        size_t get_size() const { return size; }
        bool is_available() const { return available; }
        int get_level() const { return level; }
        int get_unassigned() const { return unassigned; }
        int get_assigned() const { return assigned; }
        const std::vector<int>& get_literals() const { return literals; }
        const std::vector<int>& get_state() const { return state; }
        int get_e_num() const { return e_num; }
        int get_a_num() const { return a_num; }
        bool is_learned() const { return learned; }

        /* other */
        // void print(std::vector<int> vec) { printVector(vec); }
        void print();
        void set_literal_state(int positionInClause, int newState) { state[positionInClause] = newState; }
        void increase_e_num() { e_num++; }
        void decrease_e_num() { e_num--; }
        void increase_a_num() { a_num++; }
        void decrease_a_num() { a_num--; }
        void increase_size() { size++; }
        void decrease_size() { size--; }

        /* extend for unit clause detection and resolution and add to .cpp */
        bool is_empty() { return (e_num == 0 && a_num == 0); }
        void resolve(Clause& c, int referenceVarID);
};

#endif // CLAUSE_HPP