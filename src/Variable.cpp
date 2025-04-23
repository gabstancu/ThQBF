#include "Variable.hpp"

Variable::Variable (int varID, char quantifier, int blockID, int positionInBlock)  
            : varID(varID), 
              quantifier(quantifier),
              available(qbf::AVAILABLE), 
              assignment(qbf::UNASSIGNED), 
              level(qbf::UNDEFINED), 
              positionInBlock(positionInBlock),
              blockID(blockID),
              numNegAppear(0),
              numPosAppear(0)
{

}


void Variable::addOccurrence(int clauseID, int position, int sign)
{
  if (sign)
  {
    positiveOccurrences.insert({clauseID, position});
    numPosAppear++;
  }
  else
  {
    negativeOccurrences.insert(clauseID, position);
    numNegAppear++;
  }
}


bool Variable::appears_in_clause (int clauseID)
{

}


int Variable::get_position_in_clause (int clauseID, int sign)
{

}


void Variable::assign(int value, int searchLevel)
{

}


void Variable::retract_assignment(int value, int searchLevel)
{

}