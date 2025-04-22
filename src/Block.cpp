#include "Block.hpp"

Block::Block (char type, int blockID, std::vector<int> variables)
            :  type(type), 
               blockID(blockID),
               size(variables.size()),
               available(true),
               available_variables(variables.size()),
               variables(variables)
{

}