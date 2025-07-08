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
void ThQBF::assign (int varID, int value)
{
    if (Variables[varID].status != qbf::VariableStatus::ACTIVE)
    {
        return;
    }

    if (Variables[varID].status != qbf::VariableStatus::IMPLIED)
    {
        decision_variable_at[level] = varID;
    }

    Variables[varID].level      = level;
    Variables[varID].assignment = value;
    Variables[varID].status     = qbf::VariableStatus::ASSIGNED;
    Variables[varID].available_values--;

    if (value == 1)
    {
        // remove clauses where varID appears positive
        if (Variables[varID].numPosAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].positiveOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;

                remove_clause();
                
            }
        }


        // remove negative appearances of varID from everywhere
        if (Variables[varID].numNegAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].negativeOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;
                
                remove_literal_from_clause();
            }
        } 
    }
    else
    {
        // remove clauses where varID appears negative
        if (Variables[varID].numNegAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].negativeOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;

                remove_clause();
                
            }
        }

        // remove positive appearances of varID from everywhere
        if (Variables[varID].numPosAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].positiveOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;
                
                remove_literal_from_clause();
            }
        }
    }

    check_affected_vars();
}

// TODO: remove_literal_from_clause ()
void ThQBF::remove_literal_from_clause ()
{

}

// TODO: restore_level ()
void ThQBF::restore_level ()
{

}

// TODO: remove_variable ()
void ThQBF::remove_variable ()
{

}

// TODO: restore_variable ()
void ThQBF::restore_variable ()
{

}

// TODO: check_affected_vars ()
void ThQBF::check_affected_vars ()
{

}

// TODO: remove_clause ()
void ThQBF::remove_clause ()
{

}

// TODO: restore_clause ()
void ThQBF::restore_clause ()
{

}

// TODO: UnitPropagation ()
void ThQBF::UnitPropagation ()
{

}

// TODO: UniversalReduction ()
void ThQBF::UniversalReduction ()
{

}

// TODO: PureLiteral ()
void ThQBF::PureLiteral ()
{

}

// TODO: deduce ()
void ThQBF::deduce ()
{

}
