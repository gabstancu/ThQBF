#include "Clause.hpp"

Clause::Clause (const std::vector<int>& literals, 
                const std::vector<int>& state, 
                int level,
                bool learned) 
        : size(literals.size()), 
          available(true), 
          level(level), 
          unassigned(literals.size()), 
          assigned(0),
          literals(literals),
          state(state),
          learned(learned)
{

}