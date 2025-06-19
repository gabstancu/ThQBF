#include "Variable.hpp"

Variable::Variable (int varID, char quantifier, int blockID, int positionInBlock)  
            : varID(varID), 
              quantifier(quantifier),
              available(qbf::AVAILABLE), 
              assignment(qbf::UNASSIGNED), 
              antecedent(qbf::UNDEFINED),
              pos_in_antecedent(qbf::UNDEFINED),
              level(qbf::UNDEFINED), 
              num_of_values(2),
              positionInBlock(positionInBlock),
              blockID(blockID),
              numNegAppear(0),
              numPosAppear(0)
{
    negativeOccurrences = {};
    positiveOccurrences = {};
}


void Variable::addOccurrence(int clauseID, int position, bool positive)
{
  if (positive)
  {
    positiveOccurrences.insert({clauseID, position});
    numPosAppear++;
  }
  else
  {
    negativeOccurrences.insert({clauseID, position});
    numNegAppear++;
  }
}


bool Variable::appears_in_clause (int clauseID)
{
  return positiveOccurrences.count(clauseID) || negativeOccurrences.count(clauseID);
}


int Variable::get_position_in_clause (int clauseID, bool positive)
{
  if (positive) return positiveOccurrences[clauseID];
  return negativeOccurrences[clauseID];
}


void Variable::print()
{
  
}