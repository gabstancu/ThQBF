#include "Solver.hpp"

Solver::Solver (SolverData& data): data(data), state(qbf::UNDEFINED), level(qbf::UNDEFINED) 
{
    VStack = {};
    SStack = {};
    Search_Stack = {};
    implied_variables = {};
    unit_clauses = {};
    conflict_clause = qbf::UNDEFINED;
    // std::cout << "Initialized stacks...\n";
}


void Solver::assign(int varID, int value, int searchLevel)
{    
    std::cout << "assigning variable " << varID << " to " << value << " at level " << searchLevel << '\n';
    if (data.Variables.at(varID).is_available() == qbf::UNAVAILABLE)
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
    
    if (value == 1) /* assign positive */
    {
        /* 
            clauses where varID appears positive are satisfied 
            and are marked as unavailable (or inactive)
        */
        if (data.Variables.at(varID).get_numPosAppear())
        {   
            for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_positiveOccurrences())
            {   
                // printf("clauseID %d position in clause %d pos\n", clauseID, positionInClause);
                // printVector(data.Clauses.at(clauseID).get_literals());
                if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE) continue;

                data.Clauses.at(clauseID).increase_assigned();
                data.Clauses.at(clauseID).decrease_unassigned();
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
                // printf("clauseID %d position in clause %d neg\n", clauseID, positionInClause);
                if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE) continue;

                remove_literal_from_clause(varID, clauseID, positionInClause, searchLevel);
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
                // printf("clauseID %d position in clause %d pos\n", clauseID, positionInClause);
                if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE) continue;

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
                // printf("clauseID %d position in clause %d neg\n", clauseID, positionInClause);
                if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE) continue;

                data.Clauses.at(clauseID).increase_assigned();
                data.Clauses.at(clauseID).decrease_unassigned();
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

    check_affected_vars(searchLevel);
}


void Solver::propagate_in_tseitin(int varID, int value, int searchLevel)
{
    std::cout << "propagating " << varID << " = " << value << " at " << searchLevel << " in tseitin clauses\n";

    for (;;)
    {

    }
}


bool Solver::any_tseitin_true()
{   
    for (const auto& [varID, variable] : data.Tseitin_variables)
    {
        if (variable.get_assignment() == 1)
        {
            std::cout << "found winning strategy\nsaving winning moves\n";
            std::cout << "winning strategy (corresponding tseitin variable): " << varID << '\n';
            return true;
        }
    }
    return false;
}


void Solver::remove_literal_from_clause(int varID, int clauseID, int positionInClause, int searchLevel)
{      

    int literal = data.Clauses.at(clauseID).get_literals()[positionInClause];
    /* check if the literal has been previously removed from the clause  */
    if (data.Clauses.at(clauseID).get_state()[positionInClause] != qbf::AVAILABLE)
    {   
        std::cout << "literal " << literal << " has been previously removed from clause " << clauseID << '\n';
        return;
    }

    // std::cout << "removing literal " << literal << " from clause " << clauseID << " at level " << searchLevel << " position: " << positionInClause <<'\n';
    // data.Clauses.at(clauseID).print();

    data.Clauses_trail[searchLevel].insert(clauseID);
    // data.Clauses.at(clauseID).decrease_size();
    data.Clauses.at(clauseID).get_state()[positionInClause] = searchLevel;


    if (literal > 0) 
        data.Variables.at(varID).decrease_posNum();
    else 
        data.Variables.at(varID).decrease_negNum();


    if (data.Variables.at(varID).is_existential())
        data.Clauses.at(clauseID).decrease_e_num();
    else
        data.Clauses.at(clauseID).decrease_a_num();
    
    
    // if (data.Clauses.at(clauseID).get_a_num() != 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    // {   
    //     std::cout << "all universal clause " << clauseID << " " << "at level " << searchLevel << '\n';
    //     conflicting_clause = clauseID;
    //     state = qbf::UNSAT;
    //     return;
    // }

    if (data.Clauses.at(clauseID).get_a_num() == 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    {
        std::cout << "empty clause " << clauseID << " " << "at level " << searchLevel << '\n';
        conflict_clause = clauseID;
        state = qbf::UNSAT;
        return;
    }
    
    /* add flag for resolution ... */
    data.Clauses.at(clauseID).decrease_unassigned();
    data.Clauses.at(clauseID).increase_assigned();

    /* if e_num == 1 find position of the only existential in the clause */
    if (data.Clauses.at(clauseID).get_e_num() == 1)
    {
        for (size_t i = 0; i < data.Clauses.at(clauseID).get_size(); i++)
        {
            if (data.Clauses.at(clauseID).get_state()[i] != qbf::AVAILABLE)
                continue;
            
            int var = std::abs(data.Clauses.at(clauseID).get_literals()[i]);

            if (data.Variables.at(var).is_existential())
            {   
                int candidate_unit_literal_position = i;

                if (clause_is_unit(clauseID, var) == 1)
                {
                    unit_clauses.push({clauseID, searchLevel});
                    data.Clauses.at(clauseID).set_unit_position(candidate_unit_literal_position);
                }
            }
            break;
        }
    }

    
    /* ??? Maybe call call check_affected_vars to check all? ??? */
    // if (data.Variables.at(std::abs(literal)).get_numNegAppear() == 0 && data.Variables.at(std::abs(literal)).get_numPosAppear() == 0)
    // {
    //     remove_variable(std::abs(literal), searchLevel);
    //     return;
    // }
}


void Solver::remove_clause(int clauseID, int searchLevel)
{
    if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE) return;

    // std::cout << "removing clause " << clauseID << " at level " << searchLevel << '\n';

    data.numClauses--;
    if (!data.numClauses) /* check for empty matrix -> return SAT */
    {
        std::cout << "empty matrix at searchLevel " << searchLevel << " (clauseID " << clauseID << ")\n";
        state = qbf::SAT;
        return;
    }

    data.Clauses.at(clauseID).set_availability(qbf::UNAVAILABLE);
    // std::cout << data.Clauses.at(clauseID).is_available() << "\n\n";
    data.Clauses.at(clauseID).set_level(searchLevel);
    data.Clauses_trail[searchLevel].insert(clauseID);

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

        data.Variables_trail[searchLevel].insert(std::abs(literal));

        data.Clauses.at(clauseID).get_state()[i] = searchLevel;
    }
}


void Solver::remove_variable(int varID, int searchLevel)
{
    /* set as unavailable to Block and do the rest */
    std::cout << "removing var " << varID << " at level " << searchLevel << '\n';
    int blockID = data.Variables.at(varID).get_blockID();
    int positionInBlock = data.Variables.at(varID).get_block_position();

    /* remove from prefix */
    data.Blocks.at(blockID).decrease_size();
    data.Blocks.at(blockID).decrease_available_vars();
    data.Blocks.at(blockID).get_state()[positionInBlock] = qbf::UNAVAILABLE;
    data.Blocks.at(blockID).get_decision_level()[positionInBlock] = searchLevel;
    data.prefix.at(blockID).erase(varID);

    if (data.Blocks.at(blockID).get_available_variables() == 0)
    {
        data.Blocks.at(blockID).set_availability(qbf::UNAVAILABLE);
        data.numBlocks--;
        
        std::cout << "no variables left in block; removing block " << blockID << " from prefix\n";
        data.prefix.erase(blockID);
        print_Prefix();
    }

    data.Variables.at(varID).set_level(searchLevel);
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
    data.numVars--;
}


void Solver::restore_variable(int varID, int searchLevel)
{   
    // std::cout << "restoring variable " << varID << " at level " << searchLevel << '\n';
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


void Solver::restore_level(int searchLevel)
{
    std::cout << "restoring level " << searchLevel << '\n';
    int literal;

    /* restore all clauses that were affected at searchLevel */
    for (const auto& clauseID : data.Clauses_trail.at(searchLevel))
    {
        // std::cout << "restoring clause " << clauseID << '\n';
        // std::cout << data.Clauses.at(clauseID).is_available() << "\n\n";

        data.Clauses.at(clauseID).set_level(qbf::UNDEFINED);
        /* check if clause was candidate unit clause */
        if (data.Clauses.at(clauseID).is_unit())
        {
            data.Clauses.at(clauseID).set_unit_position(qbf::UNDEFINED);
        }
        
        if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE)
            data.numClauses++;

        data.Clauses.at(clauseID).set_availability(qbf::AVAILABLE);

        /* restoring literals */
        for (size_t i = 0 ; i < data.Clauses.at(clauseID).get_size(); i++)
        {   
            // std::cout << "--\n";
            if (data.Clauses.at(clauseID).get_state()[i] != searchLevel)
                continue;
            
            literal = data.Clauses.at(clauseID).get_literals()[i];
            data.Variables_trail[searchLevel].insert(std::abs(literal));

            data.Clauses.at(clauseID).get_state()[i] = qbf::AVAILABLE;

            if (literal > 0) 
                data.Variables.at(std::abs(literal)).increase_posNum();
            else 
                data.Variables.at(std::abs(literal)).increase_negNum();


            if (data.Variables.at(std::abs(literal)).is_existential())
                data.Clauses.at(clauseID).increase_e_num();
            else 
                data.Clauses.at(clauseID).increase_a_num();
            

            data.Clauses.at(clauseID).increase_unassigned();
            data.Clauses.at(clauseID).decrease_assigned();

        }
    }

    data.Clauses_trail.erase(searchLevel);

    /* restore vars that were affected (??? by the assignment ???) */
    for (const auto& varID : data.Variables_trail[searchLevel])
    {   
        if (data.Variables.at(varID).is_available() == qbf::AVAILABLE)
            continue;

        restore_variable(varID, searchLevel);
    }

    state = qbf::UNDEFINED;
}


void Solver::check_affected_vars(int searchLevel)
{
    for (const auto& varID : data.Variables_trail.at(searchLevel))
    {
        if (data.Variables.at(varID).get_numNegAppear() == 0 && data.Variables.at(varID).get_numPosAppear() == 0)
        {
            remove_variable(varID, searchLevel);
        }
    }
    data.Variables_trail.erase(searchLevel);
}


void Solver::imply(int searchLevel)
{
    while (!unit_clauses.empty())
    // while (unit_clauses.top().second == searchLevel)
    {
        // std::cout << "num of unit clauses: " << unit_clauses.size() << '\n';
        int unit_clauseID = unit_clauses.top().first;
        int unit_literal_position = data.Clauses.at(unit_clauseID).get_unit_position();
        int unit_literal = data.Clauses.at(unit_clauseID).get_literals()[unit_literal_position];

        if (unit_literal > 0)
        {
            assign(unit_literal, 1, searchLevel);
            implied_variables.push({unit_literal, searchLevel});
        }
        else
        {
            assign(std::abs(unit_literal), 0, searchLevel);
            implied_variables.push({std::abs(unit_literal), searchLevel});
        }

        /* set antecedent */
        data.Variables.at(std::abs(unit_literal)).set_antecedent_clause(unit_clauseID);
        
        unit_clauses.pop();
    }

    std::cout << "DONE implying...\n";
}


int Solver::clause_is_unit(int clauseID, int reference_varID)
{   
    int reference_level = data.Variables.at(reference_varID).get_blockID();
    int unit_flag = 1;
    int sign_flag = -1;
    size_t i;

    for (i = 0; i < data.Clauses.at(clauseID).get_size(); i++)
    {   
        /* we only check free (unassigned) literals */
        if (data.Clauses.at(clauseID).get_state()[i] != qbf::AVAILABLE) 
            continue;

        int literal = data.Clauses.at(clauseID).get_literals()[i];
        int var = std::abs(literal);

        if (literal > 0) 
            sign_flag = 1;
        else
            sign_flag = 0;


        if (var == reference_varID) continue;

        int universal_level = data.Variables.at(var).get_blockID();

        if (universal_level < reference_level)
        {
            unit_flag = 0;
            break;
        }
    }

    if (!unit_flag) return 0;

    // std::cout << clauseID << " is unit\n";

    return unit_flag;
}


void Solver::analyze_conflict()
{
    if (level == 0)
    {   
        std::cout << "analyze_conflict() led to searchLevel " << level << '\n';
        state = qbf::UNSAT;
        return;
    }
    // Clause c1 = find_conflicting_clause();
    // while (!stop_criterion_met(c1))
    // {
    //     int literal = choose_literal(c1);
    //     int var = std::abs(literal);
    //     Clause ante = antecedent(var); // the clause that implied a value for var
    //     c1 = resolve(c1, ante, var);
    // }

    // std::size_t h = c1.compute_hash();
    // if (ClauseHashes.find(h) == ClauseHashes.end())
    // {
        // std::cout << "New learned clause: \n";
        // c1.print();
        // data.ClauseHashes.insert(h);
        // data.Clauses[++data.last_clause_idx] = c1;
        // add_clause_to_database(c1); /* ??? if ??? */
    // }
    // else
    // {
    //     std::cout << "Skipping learned clause (duplicate):\n";
    //     c1.print();
    // }

    // back_dl = clause_asserting_level(c1);
    // return back_dl;
}


void Solver::print_Clauses()
{
    for (const auto& [clauseID, clause] : data.Clauses)
    {
        if (clause.is_available() == qbf::AVAILABLE && clause.is_tseitin() == false)
        {
            std::cout << "clauseID: " << clauseID << " | size: " << clause.get_size() << " | e_num: " << clause.get_e_num() << " a_num: " << clause.get_a_num() << " | ";
            for (size_t i = 0; i < clause.get_size(); i++)
            {   
                if (clause.get_state()[i] == qbf::AVAILABLE)
                    std::cout << clause.get_literals()[i] << " ";
            }
            std::cout << "unassigned " << clause.get_unassigned() << '\n';
        }
        // std::cout << '\n';
    }
}


void Solver::print_Variables()
{
    for (const auto& [varID, variable] : data.Variables)
    {
        if (variable.is_available() == qbf::UNAVAILABLE)
            continue;
        
        std::cout << "varID: " << varID << " pos.: " << variable.get_numPosAppear() << " neg.: " << variable.get_numNegAppear() << '\n';
    }
}


void Solver::print_Blocks()
{
    for (const auto& [blockID, block] : data.Blocks)
    {
        if (block.is_available() == qbf::UNAVAILABLE)
            continue;
        
        std::cout << "block " << blockID << ": ";
        for (int variable : block.get_variables())
        {
            if (data.Variables.at(variable).is_available() == qbf::UNAVAILABLE)
                continue;
            std::cout << variable << " ";
        }
        std::cout << '\n';
    }
}


void Solver::print_Prefix()
{
    for (const auto& [blockID, variables] : data.prefix)
    {
        std::cout << blockID << ": ";
        for (int variable : variables)
        {
            std::cout << variable << " ";
        }
        std::cout << '\n';
    }
}


void Solver::print()
{

}


bool Solver::solve()
{   
    // std::cout << "numClauses: " << data.numClauses << '\n';
    // std::cout << "state: " << state << '\n';
    // std::cout << "numVars: " << data.numVars << '\n';

    level = 1;
    int varID = 1;
    int value = 1;
    
    // // std::cout << "numClauses: " << data.numClauses << '\n';
    // std::cout << data.numBlocks << '\n';
    std::cout << data.numVars << " " << data.numClauses << '\n';
    // print_hashmap(data.Variables.at(varID).get_negativeOccurrences());
    assign(varID, value, level);
    // std::cout << data.numBlocks << '\n';
    std::cout << data.numVars << " " << data.numClauses << '\n';
    
    // // print_Blocks();

    // restore_level(level);
    // print_Clauses();
    // std::cout << data.numBlocks << '\n';
    // std::cout << data.numVars << " " << data.numClauses << '\n';

    // std::stack<std::pair<int, int>> cp;
    // std::cout << "unit clauses:\n";
    // while(!unit_clauses.empty())
    // {   
    //     cp.push(unit_clauses.top());
    //     unit_clauses.pop();
    // }

    // while(!cp.empty())
    // {   
    //     int unit_clauseID = cp.top().first;
    //     std::cout << "unit clause " << unit_clauseID << " level " << cp.top().second << '\n';
    //     int unit_literal_position = data.Clauses.at(cp.top().first).get_unit_position();
    //     int unit_literal = data.Clauses.at(cp.top().first).get_literals()[unit_literal_position];
    //     std::cout << "unit literal " << unit_literal << '\n';
        
    //     cp.pop();
    // }

    imply(level);

    // print_Clauses();
    std::cout << data.numVars << " " << data.numClauses << '\n';

    printStackOfPairsSafe(implied_variables);

    // level++;
    // varID = 10;
    // value = 1;
    // assign(varID, value, level);

    // if (state == qbf::UNSAT)
    //     restore_level(level);


    return state;
}