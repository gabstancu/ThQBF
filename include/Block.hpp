#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <vector>
#include "Constants.hpp"
#include "utils.hpp"

class Block
{
    private:
        char type;
        int blockID;
        size_t size;
        bool available;
        int available_variables;

        std::vector<int> variables;
        std::vector<int> state;
        std::vector<int> decision_level;
        
    public:
        Block (char type, 
               int blockID, 
               std::vector<int> variables)
            :  type(type), 
               blockID(blockID),
               size(variables.size()),
               available(true),
               available_variables(variables.size()),
               variables(variables)
        {
            /* ... */
        }

        /* Read only access */
        char get_type() const { return type; }
        int get_blockID() const { return blockID; }
        size_t get_size() const { return size; }
        bool is_available() const { return available; }
        int get_available_variables() const { return available_variables; }
        // const std::vector<int>& get_variables() const { return variables; }
        // const std::vector<int>& get_state() const { return state; }
        // const std::vector<int>& get_decision_level() const { return decision_level; }


        /* mutators (during search) */
        void set_type(char t) { type = t; }
        void set_blockID(int ID) { blockID = ID; }
        void set_size(size_t s) { size = s; }
        void set_availability(bool status) { available = status; }
        void set_available_variables(int available_vars) { available_variables = available_vars; }

        std::vector<int>& get_variables() { return variables; }
        std::vector<int>& get_state() { return state; }
        std::vector<int>& get_decision_level() { return decision_level; }

        /* other */
        bool is_existential() const { return type == qbf::EXISTENTIAL; }
        bool is_universal() const { return type == qbf::UNIVERSAL; }
        void print(std::vector<int> vec) { printVector(vec); }

        /* extend... */
        void remove_variable();
};

#endif // BLOCK_HPP