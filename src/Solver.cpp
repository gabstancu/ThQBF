#include "Solver.hpp"

Solver::Solver (SolverData& data): data(data), state(0), level(qbf::UNDEFINED) { }


void Solver::assign(int varID, int value, int searchLevel)
{   
    if (!data.Variables.at(varID).is_available())
    {   
        std::cout << "variable " << varID << " is not available (level " << searchLevel << ")\n"; 
        return;
    }

    data.Variables.at(varID).set_level(searchLevel);
    data.Variables.at(varID).assign(value);
    data.Variables.at(varID).decrease_num_of_values();
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
    // data.numVars--;
    // data.Variables_trail.at(searchLevel).emplace(varID); /* add to variables trail | is being added in remove_clause */
    
    if (value) /* assign positive */
    {
        /* 
            clauses where varID appears positive are satisfied 
            and are marked as unavailable (or inactive)
        */
        if (data.Variables.at(varID).get_numPosAppear())
        {   
            for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_positiveOccurrences())
            {   
                if (!data.Clauses.at(clauseID).is_available()) continue;

                // data.Clauses.at(clauseID).increase_assigned();
                // data.Clauses.at(clauseID).decrease_unassigned();
                remove_clause(clauseID, searchLevel);
                if (state == qbf::SAT)
                {   
                    /* ??? analyze_SAT ??? */
                    /* ??? restore ??? */
                    return;
                }
            }
        }
        /*
            remove varID from clauses where it appears negative
        */
        if (data.Variables.at(varID).get_numNegAppear())
        {
            for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_negativeOccurrences())
            {
                if (!data.Clauses.at(clauseID).is_available()) continue;

                remove_literal_from_clause(-varID, clauseID, positionInClause, searchLevel);
                if (state == qbf::UNSAT)
                {   
                    /* ??? analyze_conflict ??? */
                    /* ??? restore ??? */
                    return;
                }
            }
        } 
    }
    else /* assign negative */
    {
        /*
            remove varID from clauses where it appears negative
        */
        if (data.Variables.at(varID).get_numPosAppear())
        {
            for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_positiveOccurrences())
            {   
                if (!data.Clauses.at(clauseID).is_available()) continue;

                remove_literal_from_clause(varID, clauseID, positionInClause, searchLevel);
                if (state == qbf::UNSAT)
                {   
                    /* ??? analyze_conflict ??? */
                    /* ??? restore ??? */
                    return;
                }
            }
        }
        

        if (data.Variables.at(varID).get_numNegAppear())
        {
            for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_negativeOccurrences())
            {
                if (!data.Clauses.at(clauseID).is_available()) continue;

                // data.Clauses.at(clauseID).increase_assigned();
                // data.Clauses.at(clauseID).decrease_unassigned();
                remove_clause(clauseID, searchLevel);
                if (state == qbf::SAT)
                {   
                    /* ??? analyze_SAT ??? */
                    /* ??? restore ??? */
                    return;
                }
            }
        }
    }
}


void Solver::remove_literal_from_clause(int literal, int clauseID, int positionInClause, int searchLevel)
{      
    /* check if the literal has been previously removed from the clause  */
    if (data.Clauses.at(clauseID).get_state()[positionInClause] != qbf::AVAILABLE)
    {   
        std::cout << "literal " << literal << " has been previously removed from clause " << clauseID << '\n';
        return;
    }

    data.Clauses.at(clauseID).decrease_size();
    data.Clauses.at(clauseID).get_state()[positionInClause] = searchLevel;


    if (data.Variables.at(std::abs(literal)).is_existential())
        data.Clauses.at(clauseID).decrease_e_num();
    else
        data.Clauses.at(clauseID).decrease_a_num();
    
    
    if (data.Clauses.at(clauseID).get_a_num() != 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    {   
        std::cout << "all universal clause " << clauseID << " " << "at level " << searchLevel << '\n';
        state = qbf::UNSAT;
        return;
    }

    if (!data.Clauses.at(clauseID).get_size())
    {
        std::cout << "empty clause " << clauseID << " " << "at level " << searchLevel << '\n';
        state = qbf::UNSAT;
        return;
        // set solver state to UNSAT
    }

    if (literal > 0) 
        data.Variables.at(std::abs(literal)).decrease_posNum();
    else 
        data.Variables.at(std::abs(literal)).decrease_negNum();
    
    // data.Clauses.at(clauseID).decrease_unassigned();
    // data.Clauses.at(clauseID).increase_assigned();
    
    /* ??? Maybe call call check_affected_vars to check all? ??? */
    // if (data.Variables.at(std::abs(literal)).get_numNegAppear() == 0 && data.Variables.at(std::abs(literal)).get_numPosAppear() == 0)
    // {
    //     remove_variable(std::abs(literal), searchLevel);
    //     return;
    // }

}

// TODO:
void Solver::retract_assignment(int varID, int value, int searchLevel)
{
    /* 
        when unassigning a variable we must restore all its appearances 
        and restore the clauses that were removed at that particular level
    */
}


void Solver::remove_clause(int clauseID, int searchLevel)
{
    if (!data.Clauses.at(clauseID).is_available()) return;

    data.numClauses--;
    if (!data.numClauses) /* check for empty matrix -> return SAT */
    {
        std::cout << "empty matrix at searchLevel " << searchLevel << " (clauseID " << clauseID << ")\n";
        state = qbf::SAT;
        return;
    }

    data.Clauses.at(clauseID).set_availability(qbf::UNAVAILABLE);
    data.Clauses.at(clauseID).set_level(searchLevel);
    data.Clauses_trail.at(searchLevel).insert(clauseID);

    int literal;
    for (size_t i = 0; i < data.Clauses.at(clauseID).get_size(); i++)
    {
        if (data.Clauses.at(clauseID).get_state()[i] != qbf::AVAILABLE)
        {
            continue;
        }

        literal = data.Clauses.at(clauseID).get_literals()[i];

        if (literal > 0) data.Variables.at(literal).decrease_posNum();
        else data.Variables.at(std::abs(literal)).decrease_negNum();

        data.Variables_trail.at(searchLevel).insert(std::abs(literal));

        data.Clauses.at(clauseID).get_state()[i] = searchLevel;
    }
}


void Solver::remove_variable(int varID, int searchLevel)
{
    /* set as unavailable to Block and do the rest */
    int blockID = data.Variables.at(varID).get_blockID();
    int positionInBlock = data.Variables.at(varID).get_block_position();

    /* remove from prefix */
    data.Blocks.at(blockID).decrease_size();
    data.Blocks.at(blockID).decrease_available_vars();
    data.Blocks.at(blockID).get_state()[positionInBlock] = qbf::UNAVAILABLE;
    data.Blocks.at(blockID).get_decision_level()[positionInBlock] = searchLevel;

    if (data.Blocks.at(blockID).get_available_variables() == 0)
    {
        data.prefix.at(blockID).erase(varID);
        data.Blocks.at(blockID).set_availability(qbf::UNAVAILABLE);
        data.numBlocks--;
        
        std::cout << "no variables left in block; removing block " << blockID << " from prefix\n";
    }


    data.Variables.at(varID).set_level(searchLevel);
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
    data.numVars--;
}


void Solver::restore_variable(int varID, int searchLevel)
{
    data.Variables.at(varID).assign(qbf::UNASSIGNED);
    data.Variables.at(varID).set_level(qbf::UNDEFINED);
    data.Variables.at(varID).set_availability(qbf::AVAILABLE);
    data.numVars++;

    int blockID = data.Variables.at(varID).get_blockID();
    int positionInBlock = data.Variables.at(varID).get_block_position();

    data.prefix.at(blockID).insert(varID);
    data.Blocks.at(blockID).increase_size();
    data.Blocks.at(blockID).increase_available_vars();
    data.Blocks.at(blockID).get_state()[positionInBlock] = qbf::AVAILABLE;
    data.Blocks.at(blockID).get_decision_level()[positionInBlock] = qbf::UNDEFINED;

    if (data.Blocks.at(blockID).get_size() == 1)
    {
        data.numBlocks++;
        data.Blocks.at(blockID).set_availability(qbf::AVAILABLE);
    }

}

// TODO:
void Solver::restore_level(int searchLevel)
{

}


// TODO:
void Solver::check_affected_vars(int searchLevel)
{
    data.Variables_trail.clear();
}


bool Solver::solve()
{   
    std::cout << "in solve()" << '\n';
    return qbf::SAT;
}