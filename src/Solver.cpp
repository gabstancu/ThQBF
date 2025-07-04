#include "Solver.hpp"

Solver::Solver (SolverData& data): data(data), state(qbf::UNDEFINED), level(qbf::UNDEFINED) 
{
    PStack = {};
    SStack = {};
    Search_Stack = {};
    implied_variables = {};
    unit_clauses = {};
    conflicting_clause = qbf::UNDEFINED;
    GAME_FLAG = qbf::UNDEFINED;
    level = qbf::PRESEARCH;
    std::cout << "Initialized stacks...\n";
}


void Solver::assign(int varID, int value, int searchLevel)
{    
    // std::cout << "assigning variable " << varID << " to " << value << " at level " << searchLevel << '\n';
    if (data.Variables.at(varID).is_available() == qbf::UNAVAILABLE)
    {   
        std::cout << "variable " << varID << " is not available (level " << searchLevel << ")\n"; 
        return;
    }
    std::cout << "assigning variable " << varID << " to " << value << " at level " << searchLevel << '\n';
    if (!data.Variables.at(varID).is_implied())
        decision_variable_at[searchLevel] = varID;

    data.Variables.at(varID).set_level(searchLevel);
    data.Variables.at(varID).assign(value);
    data.Variables.at(varID).decrease_num_of_values();
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
    
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
                    /* IN SEARCH LOOP */
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
                // if (state == qbf::UNSAT)
                // {   
                //     /* IN SEARCH LOOP */
                //     /* ??? analyze_conflict ??? */
                //     /* ??? restore ??? */
                //     return;
                // }
            }
            // if (state == qbf::UNSAT)
            // {   
            //     /* IN SEARCH LOOP */
            //     /* ??? analyze_conflict ??? */
            //     /* ??? restore ??? */
            //     return;
            // }

            
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
                // if (state == qbf::UNSAT)
                // {   
                //     /* IN SEARCH LOOP */
                //     /* ??? analyze_conflict ??? */
                //     /* ??? restore ??? */
                //     return;
                // }
            }
            // if (state == qbf::UNSAT)
            // {   
            //     /* IN SEARCH LOOP */
            //     /* ??? analyze_conflict ??? */
            //     /* ??? restore ??? */
            //     return;
            // }
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
                    /* IN SEARCH LOOP */
                    /* ??? analyze_SAT ??? */
                    /* ??? restore ??? */
                    return;
                }
            }
        }
    }

    // if (data.Variables_trail.count(searchLevel))
    check_affected_vars(searchLevel);
}


void Solver::remove_literal_from_clause(int varID, int clauseID, int positionInClause, int searchLevel)
{      

    int literal = data.Clauses.at(clauseID).get_literals()[positionInClause];
    /* check if the literal has been previously removed from the clause  */
    if (data.Clauses.at(clauseID).get_state()[positionInClause] != qbf::AVAILABLE)
    {   
        // std::cout << "literal " << literal << " has been previously removed from clause " << clauseID << '\n';
        return;
    }

    // std::cout << "removing literal " << literal << " from clause " << clauseID << " at level " << searchLevel << " position: " << positionInClause <<'\n';
    // data.Clauses.at(clauseID).print();

    data.Clauses_trail[searchLevel].insert(clauseID);
    data.Variables_trail[searchLevel].insert(varID);
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
    

    /* add flag for resolution ... */
    data.Clauses.at(clauseID).decrease_unassigned();
    data.Clauses.at(clauseID).increase_assigned();

    /* UNSAT checks */
    // if (data.Clauses.at(clauseID).get_a_num() != 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    // {   
    //     std::cout << "all universal clause " << clauseID << " " << "at level " << searchLevel << '\n';
    //     conflicting_clause = clauseID;
    //     // conflicting_clauses.insert(conflicting_clause);
    //     state = qbf::UNSAT;
    //     return;
    // }

    if (data.Clauses.at(clauseID).get_a_num() == 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    {
        std::cout << "empty clause " << clauseID << " " << "at level " << searchLevel << '\n';
        conflicting_clause = clauseID;
        // conflicting_clauses.insert(conflicting_clause);
        state = qbf::UNSAT;
        // return;
    }

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
                    // printStackOfPairsSafe(unit_clauses);
                    std::cout << clauseID << " is unit\n";
                    data.Clauses.at(clauseID).set_unit_position(candidate_unit_literal_position);
                    break;
                }
            }
        }
    }
}


void Solver::remove_clause(int clauseID, int searchLevel)
{
    if (data.Clauses.at(clauseID).is_available() == qbf::UNAVAILABLE) return;

    // std::cout << "removing clause " << clauseID << " at level " << searchLevel << '\n';

    data.numClauses--;
    // if (!data.numClauses) /* check for empty matrix -> return SAT */
    // {
    //     std::cout << "empty matrix at searchLevel " << searchLevel << " (clauseID " << clauseID << ")\n";
    //     state = qbf::SAT;
    //     return;
    // }

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

    if (!data.numClauses) /* check for empty matrix -> return SAT */
    {
        std::cout << "empty matrix at searchLevel " << searchLevel << " (clauseID " << clauseID << ")\n";
        state = qbf::SAT;
        return;
    }
}


void Solver::remove_variable(int varID, int searchLevel)
{
    /* set as unavailable to Block and do the rest */
    // std::cout << "removing var " << varID << " at level " << searchLevel << '\n';
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
        
        // std::cout << "no variables left in block; removing block " << blockID << " from prefix\n";
        data.prefix.erase(blockID);
        // print_Prefix();
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

        if (data.Variables.at(varID).is_tseitin())
        {
            data.Variables.at(varID).assign(qbf::UNASSIGNED);
            data.Variables.at(varID).set_level(qbf::UNDEFINED);
            data.Variables.at(varID).set_availability(qbf::AVAILABLE);
            data.numVars++;
            continue;
        }
        restore_variable(varID, searchLevel);
    }

    state = qbf::UNDEFINED;
}


void Solver::check_affected_vars(int searchLevel)
{   
    // std::cout << "affected vars...\n";
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
        int var = std::abs(unit_literal);
        if (data.Variables.at(var).get_antecedent_clause() != qbf::UNDEFINED)
        {
            // std::cout << "literal implied at " << data.Variables.at(var).get_decision_level() << '\n';
            unit_clauses.pop();
            continue;
        }
        std::cout << "implying unit literal " << unit_literal << " at level " << searchLevel << " in clause " << unit_clauseID <<'\n';
        
        data.Variables.at(std::abs(unit_literal)).set_implied(true);
        data.Variables.at(std::abs(unit_literal)).set_antecedent_clause(unit_clauseID);
        data.Variables.at(std::abs(unit_literal)).set_pos_in_antecedent(unit_literal_position);

        if (unit_literal > 0)
        {   
            unit_clauses.pop();
            assign(unit_literal, 1, searchLevel);
            implied_variables.push({unit_literal, searchLevel});
            // if (state == qbf::UNSAT)
            // {
            //     std::cout << "concluded UNSAT\n";
            //     std::cout << "solver state: " << state << '\n';
            //     std::cout << "conflict clause: " << conflicting_clause << '\n';
            //     // print_Clauses();
            //     return;
            // }

            // if (state == qbf::SAT)
            // {
            //     std::cout << "concluded SAT\n";
            //     std::cout << "solver state: " << state << '\n';
            //     // print_Clauses();
            //     return;
            // }
        }
        else
        {   
            unit_clauses.pop();
            assign(std::abs(unit_literal), 0, searchLevel);
            // implied_variables.push({std::abs(unit_literal), searchLevel});
            implied_variables.push({unit_literal, searchLevel});
            // if (state == qbf::UNSAT)
            // {
            //     std::cout << "concluded UNSAT\n";
            //     std::cout << "solver state: " << state << '\n';
            //     std::cout << "conflict clause: " << conflicting_clause << '\n';
            //     // print_Clauses();
            //     return;
            // }

            // if (state == qbf::SAT)
            // {
            //     std::cout << "concluded SAT\n";
            //     std::cout << "solver state: " << state << '\n';
            //     // print_Clauses();
            //     return;
            // }
        }
        // if (state == qbf::UNSAT)
        // {
        //     std::cout << "concluded UNSAT\n";
        //     std::cout << "solver state: " << state << '\n';
        //     std::cout << "conflict clause: " << conflicting_clause << '\n';
        //     // print_Clauses();
        //     return;
        // }
        // if (state == qbf::SAT)
        // {
        //     std::cout << "concluded SAT\n";
        //     std::cout << "solver state: " << state << '\n';
        //     // print_Clauses();
        //     return;
        // }
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

    return unit_flag;
}


int Solver::choose_literal(std::vector<int> cl)
{   
    int most_recently_implied = qbf::UNDEFINED;
    int implication_level = qbf::UNDEFINED;

    for (int literal:cl)
    {
        int var = std::abs(literal);

        if (data.Variables.at(var).get_antecedent_clause() == qbf::UNDEFINED)
            continue;

        if (data.Variables.at(var).get_decision_level()>=implication_level)
        {
            // std::cout << "found...\n" << literal << '\n';
            most_recently_implied = literal;
            implication_level = data.Variables.at(var).get_decision_level();
        }
    }

    return most_recently_implied;
}


bool Solver::stop_criterion_met(std::unordered_map<int, int> c1)
{   
    /*  
                        1st condition
        Among all its existential variables, one and only one of
        them has the highest decision level (which may not be
        the current decision level). Suppose this variable is V.
    */

    int v = -1, highest_decision_level = 0;
    int V = v;
    std::pair<int, int> p;
    std::unordered_map<int, std::pair<int, int>> levels = {}; /* { descision_level: (appearances, V) } */
    
    for (const auto& [literal, sign] : c1)
    {   
        if (data.Variables.at(std::abs(literal)).is_universal())
            continue;

        v = std::abs(literal);
        int decision_level = data.Variables.at(std::abs(literal)).get_decision_level();
        
        if (decision_level > highest_decision_level) /* keep MAX and check at the end */
        {
             highest_decision_level = decision_level;
             V = v;
        } 
           
        if (levels.find(decision_level) == levels.end()) /* level not found */
        { 
            p = std::make_pair(1, v);
            levels.insert({decision_level, p});
        }
        else
            levels[decision_level].first++;
    }

    if (levels[highest_decision_level].first > 1)
        return false;
    
    
    /*  
                        2nd condition
        V is in a decision level with an existential variable as the decision variable.
    */
    
    int decision_var = decision_variable_at[highest_decision_level];
    if (!data.Variables.at(decision_var).is_existential())
        return false;

    /*
                         3rd condition
        All universal literals with quantification level smaller than V’s are 
        assigned to 0 before the decision level of V.
    */    
    int V_quant_level = data.Variables.at(V).get_blockID();
    for (const auto& [literal, sign] : c1)
    {
        if (data.Variables.at(std::abs(literal)).is_existential())
            continue;
        
        int var = std::abs(literal);
        int var_quant_level = data.Variables.at(var).get_blockID();

        if (var_quant_level > V_quant_level) continue;

        /* 
            only checking universal variables with quantification 
            level smaller than V's
        */
        if (!data.Variables.at(var).is_available())
            continue;
        else
            return false;
    }
    return true;
}


std::unordered_map<int, int> Solver::resolve(std::vector<int> c1, std::vector<int> c2, int pivot_variable)
{
    std::unordered_map<int, int> new_clause = {}; // literal, state
    int state_in_clause = qbf::UNDEFINED;

    for (int literal : c1)
    {   
        if (std::abs(literal) == pivot_variable) continue;

        if (data.Variables.at(std::abs(literal)).get_decision_level() == qbf::UNDEFINED)
            state_in_clause = qbf::AVAILABLE;
        else
            state_in_clause = data.Variables.at(std::abs(literal)).get_decision_level();

        new_clause.insert({literal, state_in_clause});
    }

    for (int literal : c2)
    {
        if (std::abs(literal) == pivot_variable) continue;
        if (new_clause.find(literal) != new_clause.end()) continue; /* already in clause */
        
        /* check for opposite existential */
        if (data.Variables.at(std::abs(literal)).is_existential())
        {
            if (new_clause.find(-literal) != new_clause.end()) /* opposite of literal is in, eliminate */
            {
                new_clause.erase(literal);
                continue;
            }
        }

        /* add literal to clause */
        if (data.Variables.at(std::abs(literal)).get_decision_level() == qbf::UNDEFINED)
            state_in_clause = qbf::AVAILABLE;
        else
            state_in_clause = data.Variables.at(std::abs(literal)).get_decision_level();

        new_clause.insert({literal, state_in_clause});
    }

    return new_clause;
}


int Solver::clause_asserting_level(std::vector<int> cl_vec)
{
    int asserting_literal = qbf::UNDEFINED;
    int clause_asserting_lvl = qbf::UNDEFINED;

    /* 
        conlict occured due to a universal assignment: 
        backtrack to the last existential decision 
    */
    if (data.Variables.at(Search_Stack.top().first).is_universal()) /* maybe not... */
    {
        clause_asserting_lvl = SStack.top().second;
        std::cout << "backtracking to " << clause_asserting_lvl << '\n';
        return clause_asserting_lvl;
    }

    int existential_literals_at_current_level = 0;
    int diff = INT_MAX;
    int conflict_level  = level;

    for (int literal : cl_vec)
    {
        if (data.Variables.at(std::abs(literal)).is_universal())
            continue;
        
        int e_var = std::abs(literal);
        int existential_level = data.Variables.at(e_var).get_decision_level();

        if (diff >= conflict_level - existential_level)
        {
            diff = conflict_level - existential_level;
            asserting_literal = literal;
            clause_asserting_lvl = existential_level;
        }
    }
    return clause_asserting_lvl;
}


void Solver::add_clause_to_db(std::vector<int> cl_vec, std::unordered_map<int, int> cl_hash)
{
    std::sort(cl_vec.begin(), cl_vec.end()); /* sort literals in clause */
    std::vector<int> state(cl_vec.size(), qbf::AVAILABLE);
    print_hashmap(cl_hash);

    int index = 0;
    
    Clause new_clause = Clause(cl_vec, state, level, true);
    std::size_t h = new_clause.compute_hash();

    /* adding learned clause to database */
    if (data.ClauseHashes.find(h) == data.ClauseHashes.end())
    {   
        data.last_clause_idx++; data.numClauses++;
        data.ClauseHashes.insert(h);

        /* ------------------- init. new clause ------------------- */

        for (int literal : cl_vec)
        {
            int literal_state = cl_hash[literal];
            new_clause.get_state()[index] = literal_state;

            if (literal>0)
                data.Variables.at(literal).addOccurrence(data.last_clause_idx, index, true);
            else
                data.Variables.at(std::abs(literal)).addOccurrence(data.last_clause_idx, index, false);

            index++;

            if (literal_state == qbf::AVAILABLE)
            {
                if (data.Variables.at(std::abs(literal)).is_existential())
                    new_clause.increase_e_num();
                else 
                    new_clause.increase_a_num();
                continue;
            }
            new_clause.increase_assigned();
            new_clause.decrease_unassigned();
        }

        new_clause.set_size(cl_vec.size());
        new_clause.set_availability(qbf::AVAILABLE);

        data.Clauses.insert({data.last_clause_idx, new_clause});
        data.Clauses_trail.at(level).insert(data.last_clause_idx);

        return;
    }
    
}


int Solver::analyze_conflict(int conflict_level)
{
    if (conflict_level == 0)
    {   
        std::cout << "analyze_conflict() led to root (" << conflict_level << ")" << '\n';
        state = qbf::UNSAT;
        return -1;
    }
    int currentDecisionLevel = conflict_level; /* conflict level */

    std::vector<int> cl_vec = data.Clauses.at(conflicting_clause).get_literals();
    std::unordered_map<int, int> cl_hash = vector_to_hashmap(cl_vec);

    while (!stop_criterion_met(cl_hash))
    {
        int most_recently_implied_literal = choose_literal(cl_vec);
        int variable = std::abs(most_recently_implied_literal);

        if (most_recently_implied_literal == qbf::UNDEFINED)
            break;

        int antecedentID = data.Variables.at(variable).get_antecedent_clause();
        std::vector<int> antecedent_vec = data.Clauses.at(antecedentID).get_literals();

        cl_hash = resolve(cl_vec, antecedent_vec, variable);
        cl_vec = hashmap_to_vec(cl_hash);
    }
    
    int back_dl = clause_asserting_level(cl_vec);

    add_clause_to_db(cl_vec, cl_hash);

    return back_dl;
}


void Solver::print_Clauses()
{
    for (const auto& [clauseID, clause] : data.Clauses)
    {
        if (clause.is_available() == qbf::AVAILABLE)
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
    std::cout << '\n';
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
    std::cout << '\n';
}


void Solver::print()
{
    if (GAME_FLAG)
        std::cout << "Mode: GAME INSTANCE\n";
    if (!GAME_FLAG)
        std::cout << "Mode: RANDOM INSTANCE\n";
    
    std::cout << "State: " << state << '\n';
    std::cout << "numClauses: " << data.numClauses << '\n';
    std::cout << "numVars: " << data.numVars << '\n';
}

/* ================ main loop ================ */
bool Solver::solve()
{   
    std::cout << "numClauses: " << data.numClauses << '\n';
    std::cout << "state: " << state << '\n';
    std::cout << "numVars: " << data.numVars << '\n';
    std::cout << "conflicting clause: " << conflicting_clause << '\n';
    print_Clauses();

    /*  ================ testing ================   */
    level = 1;
    assign(1, 1, level);
    print_Clauses();
    imply(level);
    print_Clauses();
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Prefix();

    level++;
    assign(2, 1, level);
    PStack.push({2, level});
    Search_Stack.push({2, level});
    // print_Clauses();
    imply(level);
    print_Clauses();
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Prefix();

    level++;
    assign(3, 1, level);
    SStack.push({3, level});
    Search_Stack.push({3, level});
    // print_Clauses();
    imply(level);
    print_Clauses();
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Prefix();

    // level++;
    // assign(4, 1, level);
    // PStack.push({4, level});
    // Search_Stack.push({4, level});
    // // print_Clauses();
    // imply(level);
    // print_Clauses();
    // std::cout << "solver state: " << state << '\n';
    // std::cout << "conflict clause: " << conflicting_clause << '\n';
    // print_Prefix();

    // level++;
    // assign(5, 1, level);
    // PStack.push({5, level});
    // Search_Stack.push({5, level});
    // // print_Clauses();
    // imply(level);
    // print_Clauses();
    // std::cout << "solver state: " << state << '\n';
    // std::cout << "conflict clause: " << conflicting_clause << '\n';
    // print_Prefix();

    // level++;
    // assign(6, 1, level);
    // SStack.push({6, level});
    // Search_Stack.push({6, level});
    // // print_Clauses();
    // imply(level);
    // print_Clauses();
    // std::cout << "solver state: " << state << '\n';
    // std::cout << "conflict clause: " << conflicting_clause << '\n';
    // print_Prefix();


    // int result = analyze_conflict(level);
    // std::cout << result << '\n';

    // // print_Clauses();

    // data.Clauses.at(data.last_clause_idx).print();
    
    // level++;
    // assign(5, 1, level);
    // PStack.push({5, level});
    // Search_Stack.push({5, level});
    // print_Clauses();
    // imply(level);
    // print_Clauses();
    // std::cout << "solver state: " << state << '\n';
    // std::cout << "conflict clause: " << conflicting_clause << '\n';

    // print_Prefix();


    // std::cout << "conflict level: " << level << '\n';

    // int result = analyze_conflict(level);
    // std::cout << result << '\n';

    // print_Clauses();

    // std::cout << data.last_clause_idx << '\n';
    // data.Clauses.at(data.last_clause_idx).print();
    

    return state;
}