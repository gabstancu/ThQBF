#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <vector>

class Block
{
    private:
        std::vector<int> variables;
        
    public:
        Block (char quantifier, int blockID);
};

#endif // BLOCK_HPP