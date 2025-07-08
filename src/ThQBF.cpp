#include "ThQBF.hpp"

ThQBF::ThQBF (QDimacsParser parser) : level(UNDEFINED)
{
    this->Clauses              = parser.matrix;
    this->Blocks               = parser.quantifier_prefix;
    this->Variables            = parser.variables; 
    this->numVars              = parser.numVars;
    this->numClauses           = parser.numClauses;
    this->numBlocks            = parser.numBlocks;
    this->numOfExistentialVars = parser.numOfExistentialVars;
    this->numOfUniversalVars   = parser.numOfUniversalVars;

    this->remainingVars        = parser.numVars;
    this->remainingClauses     = parser.numClauses;
    this->remainingBlocks      = parser.numBlocks;

    build_prefix();
}