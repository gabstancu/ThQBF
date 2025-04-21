#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <vector>
#include "Constants.hpp"

class Block
{
    private:
        char type;
        int blockID;
        size_t size;
        bool available;
        int availableVars;

        std::vector<int> variables;
        std::vector<int> state;
        std::vector<int> level;
        
    public:
        Block (char type, 
               int blockID, 
               std::vector<int> variables)
            :  type(type), 
               blockID(blockID),
               size(variables.size()),
               available(true),
               availableVars(variables.size()),
               variables(variables)
        {
            /* ... */
        }
};

#endif // BLOCK_HPP