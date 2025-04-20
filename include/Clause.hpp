#ifndef CLAUSE_HPP
#define CLAUSE_HPP

#include <vector>

class Clause
{
    private:
        int size;
        int in;
        int level;

        int unassigned;
        int assigned;

        std::vector<int> literals;
        std::vector<int> state;

        int e_num;
        int a_num;

        bool learned = false;

    public:
        Clause (const std::vector<int>& literals, 
                const std::vector<int>& state, 
                bool learned = false) : literals(literals), state(state), learned(learned) {};
        
        /* To mutate during search */
        std::vector<int>& get_literals() { return this->literals; }
        std::vector<int>& get_state() { return this->state; }

        /* Read only access */
        const std::vector<int>& get_literals() const { return this->literals; }
        const std::vector<int>& get_state() const { return this->state; }

        /* extend for unit clause detection and resolution */
};

#endif // CLAUSE_HPP