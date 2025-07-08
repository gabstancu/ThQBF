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

// TODO: assign ()
void ThQBF::assign ()
{

}

// TODO: remove_literal_from_clause ()
void ThQBF::remove_literal_from_clause ()
{

}

// TODO: restore_level()
void ThQBF::restore_level ()
{

}

// TODO: remove_variable()
void ThQBF::remove_variable ()
{

}

// TODO: restore_variable()
void ThQBF::restore_variable ()
{

}

// TODO: check_affected_vars()
void ThQBF::check_affected_vars ()
{

}

// TODO: remove_clause()
void ThQBF::remove_clause ()
{

}

// TODO: restore_clause()
void ThQBF::restore_clause ()
{

}

// TODO: UnitPropagation()
void ThQBF::UnitPropagation ()
{

}

// TODO: UniversalReduction()
void ThQBF::UniversalReduction ()
{

}

// TODO: PureLiteral()
void ThQBF::PureLiteral ()
{

}

// TODO: deduce()
void ThQBF::deduce ()
{

}
