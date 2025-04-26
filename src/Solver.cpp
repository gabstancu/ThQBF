#include "Solver.hpp"

Solver::Solver (SolverData& data): data(data), state(qbf::UNDEFINED) { }


void Solver::assign(int varID, int value, int searchLevel)
{   
    /* REMOVE AT THE END */
    /* ---------- remove_variable(varID, searchLevel); ---------- */
    data.Variables.at(varID).set_level(searchLevel);
    data.Variables.at(varID).assign(value);
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
    data.numVars--;
    data.Variables_trail.at(searchLevel).emplace(varID); /* add to variables trail */

    /* searchLevel specific */
    std::vector<int> clauses_removed;
    std::unordered_map<int, int> appearances_removed; /* of assigned variable */
    
    if (value) /* assign positive */
    {
        /* 
            clauses where varID appears positive are satisfied 
            and are marked as unavailable (or inactive)
        */
        for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_positiveOccurrences())
        {   
            remove_clause(clauseID, searchLevel);
            data.numClauses--;
            data.Clauses.at(clauseID).set_availability(false);
            clauses_removed.push_back(clauseID);
        }

        /*
            remove varID from clauses where it appears negative
        */
        for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_negativeOccurrences())
        {
            /* call remove_literal_from_clause() instead */
            data.Clauses.at(clauseID).get_state()[positionInClause] = searchLevel;

            if (data.Variables.at(varID).is_existential())
                data.Clauses.at(clauseID).decrease_e_num();
            else
                data.Clauses.at(clauseID).decrease_a_num();
            
        }
    }
    else /* assign negative */
    {
        /*
            remove varID from clauses where it appears negative
        */
        for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_positiveOccurrences())
        {
            /* call remove_literal_from_clause() instead */
            data.Clauses.at(clauseID).get_state()[positionInClause] = searchLevel;

            if (data.Variables.at(varID).is_existential())
                data.Clauses.at(clauseID).decrease_e_num();
            else
                data.Clauses.at(clauseID).decrease_a_num();
        }

        for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_negativeOccurrences())
        {
            remove_clause(clauseID, searchLevel);
            data.numClauses--;
            data.Clauses.at(clauseID).set_availability(false);
            clauses_removed.push_back(clauseID);
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

    if (!data.Clauses.at(clauseID).get_size())
    {
        std::cout << "empty clause " << clauseID << " " << "at level " << searchLevel << '\n';
        state = qbf::UNSAT;
        return;
        // set solver state to UNSAT
    }

    /* add check for all 'a' in clause and return UNSAT */

    if (data.Variables.at(std::abs(literal)).is_existential())
        data.Clauses.at(clauseID).decrease_e_num();
    else
        data.Clauses.at(clauseID).decrease_a_num();

    if (data.Variables.at(std::abs(literal)).get_numNegAppear() == 0 && data.Variables.at(std::abs(literal)).get_numPosAppear() == 0)
    {
        remove_variable(std::abs(literal), searchLevel);
        return;
    }

}


void Solver::retract_assignment(int varID, int value, int searchLevel)
{
    /* 
        when unassigning a variable we must restore all its appearances 
        and restore the clauses that were removed at that particular level
    */
}


void Solver::remove_clause(int clauseID, int searchLevel)
{
    
}


void Solver::remove_variable(int varID, int searchLevel)
{
    /* ??? set numNeg and numPos to 0 ??? */
    // data.Variables.at(varID).set_numNegAppear(0);
    // data.Variables.at(varID).set_numPosAppear(0);


    /* set as unavailable to Block and do the rest */
    int blockID = data.Variables.at(varID).get_blockID();
    int positionInBlock = data.Variables.at(varID).get_block_position();

    /* remove from prefix and do the checks */
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
    // data.Variables.at(varID).assign(value); /* doing this when assigning */
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
    data.Variables_trail.at(searchLevel).emplace(varID); /* add to variables trail */
    data.numVars--;

}


void Solver::restore_variable(int varID, int searchLevel)
{

}






bool Solver::solve()
{   
    std::cout << "in solve()" << '\n';
    return qbf::SAT;
}