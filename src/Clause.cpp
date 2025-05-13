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
          unique_existential_position(qbf::UNDEFINED),
          e_num(0), 
          a_num(0),
          learned(learned), 
          tseitin(false)
{ 
  std::sort(this->literals.begin(), this->literals.end());
}


void Clause::print()
{
    // print literals
    printVector(literals);
    // print state
    std::cout << "  ";
    printVector(state);
    std::cout << "  ";
    std::cout << "level: " << level << "   ";
    std::cout << "availability: " << available << "   ";
    std::cout << "candidate unit literal position: " << unique_existential_position << "   ";
    std::cout << "learned: " << learned << "   ";
    std::cout << "tseitin: " << tseitin << "   ";
    std::cout << '\n';
}

bool Clause::is_unit()
{
  return e_num == 1;
}

void Clause::resolve(Clause& c, int referenceVar)
{
  
}