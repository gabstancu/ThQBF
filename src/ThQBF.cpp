#include "ThQBF.hpp"

ThQBF::ThQBF (const QDimacsParser& parser) : level(UNDEFINED)
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

    this->last_clause_idx      = parser.numClauses;
    
    // construct prefix
    for (QuantifierBlock b : this->Blocks)
    {
        for (int variable : b.variables)
        {
            this->PREFIX[b.blockID].insert(variable);
        }
    }
}

// TODO
void ThQBF::assign ()
{

}

// TODO
void ThQBF::remove_literal_from_clause ()
{

}

// TODO
void ThQBF::restore_level ()
{

}

// TODO
void ThQBF::remove_variable ()
{

}

