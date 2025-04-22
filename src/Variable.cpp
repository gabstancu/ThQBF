#include "Variable.hpp"

Variable::Variable (int varID, char quantifier, int blockID, int positionInBlock)  
            : varID(varID), 
              quantifier(quantifier),
              available(qbf::AVAILABLE), 
              assignment(qbf::UNASSIGNED), 
              level(qbf::PRESEARCH), 
              positionInBlock(positionInBlock),
              blockID(blockID),
              numNegAppear(0),
              numPosAppear(0)
{

}