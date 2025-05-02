#include "Clause.hpp"

Clause::Clause (const std::vector<int>& literals, 
                const std::vector<int>& state, 
                int level,
                bool learned) 
        : size(literals.size()), 
          available(qbf::AVAILABLE), 
          level(level), 
          unassigned(literals.size()), 
          assigned(0),
          literals(literals),
          state(state),
          e_num(0), 
          a_num(0),
          learned(learned)
{ 
  std::sort(this->literals.begin(), this->literals.end());
}


bool Clause::is_unit()
{
  return false;
}

void Clause::resolve(Clause& c, int referenceVar)
{

}