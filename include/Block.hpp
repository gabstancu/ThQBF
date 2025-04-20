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
        Block (char type, int blockID);
};

#endif // BLOCK_HPP