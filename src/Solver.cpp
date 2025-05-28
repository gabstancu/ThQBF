#include "Solver.hpp"

Solver::Solver (SolverData& data): data(data), state(qbf::UNDEFINED), level(qbf::UNDEFINED) 
{
    PStack = {};
    SStack = {};
    Search_Stack = {};
    implied_variables = {};
    unit_clauses = {};
    conflicting_clause = qbf::UNDEFINED;
    conflicting_clauses = {};
    GAME_FLAG = qbf::UNDEFINED;
    level = qbf::PRESEARCH;
    std::cout << "Initialized stacks...\n";
}


void Solver::assign(int varID, int value, int searchLevel)
{    
    // std::cout << "assigning variable " << varID << " to " << value << " at level " << searchLevel << '\n';
    if (data.Variables.at(varID).is_available() == qbf::UNAVAILABLE)
    {   
        // std::cout << "variable " << varID << " is not available (level " << searchLevel << ")\n"; 
        return;
    }
    std::cout << "assigning variable " << varID << " to " << value << " at level " << searchLevel << '\n';
    if (!data.Variables.at(varID).is_implied())
        decision_variable_at[searchLevel] = varID;

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
                    // return;
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
                    // return;
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
                    // return;
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
    if (data.Clauses.at(clauseID).get_a_num() != 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    {   
        std::cout << "all universal clause " << clauseID << " " << "at level " << searchLevel << '\n';
        conflicting_clause = clauseID;
        conflicting_clauses.insert(conflicting_clause);
        state = qbf::UNSAT;
        // return;
    }

    if (data.Clauses.at(clauseID).get_a_num() == 0 && data.Clauses.at(clauseID).get_e_num() == 0)
    {
        std::cout << "empty clause " << clauseID << " " << "at level " << searchLevel << '\n';
        conflicting_clause = clauseID;
        conflicting_clauses.insert(conflicting_clause);
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
                    data.Clauses.at(clauseID).set_unit_position(candidate_unit_literal_position);
                }
            }
            break;
        }
    }

    /* detect unit universal clauses if GAME_FLAG is 1 */
    if (data.Clauses.at(clauseID).get_a_num() == 1 && GAME_FLAG == qbf::GAME_ON)
    {
        for (size_t i = 0; i < data.Clauses.at(clauseID).get_size(); i++)
        {
            if (data.Clauses.at(clauseID).get_state()[i] != qbf::AVAILABLE)
                continue;
            
            int var = std::abs(data.Clauses.at(clauseID).get_literals()[i]);

            if (data.Variables.at(var).is_universal() && (data.Clauses.at(clauseID).is_rule() || data.Clauses.at(clauseID).is_tseitin()))
            {   
                // std::cout << "alaniiiii... " << clauseID << '\n';
                universal_unit_clauses.push({clauseID, searchLevel});
                data.Clauses.at(clauseID).set_universal_position(i);
                break;
            }   
        }
    }
    
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
    for (const auto& varID : data.Variables_trail.at(searchLevel))
    {
        if (data.Variables.at(varID).get_numNegAppear() == 0 && data.Variables.at(varID).get_numPosAppear() == 0)
        {   
            if (data.Variables.at(varID).is_tseitin())
            {   
                // std::cout << "removing tseitin variable " << varID << '\n';
                data.Variables.at(varID).set_level(searchLevel);
                data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);
                data.numVars--;
                continue;
            }
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

        if (unit_literal > 0)
        {   
            if (GAME_FLAG == qbf::GAME_ON && data.Variables.at(var).is_tseitin())
            {
                std::cout << "implying " << var << " to be true.\n";
                std::cout << "e has found a winning strategy... SAT\n";
                state = qbf::SAT;
            }
            assign(unit_literal, 1, searchLevel);
            implied_variables.push({unit_literal, searchLevel});
        }
        else
        {   
            assign(std::abs(unit_literal), 0, searchLevel);
            implied_variables.push({std::abs(unit_literal), searchLevel});
        }
        
        unit_clauses.pop();
    }

    std::cout << "DONE implying...\n";
    printStackOfPairsSafe(implied_variables);
}


void Solver::imply_universal_move(int searchLevel)
{
    while (!universal_unit_clauses.empty())
    {
        int unit_clauseID = universal_unit_clauses.top().first;
        int unit_literal_position = data.Clauses.at(unit_clauseID).get_universal_position();
        int unit_literal = data.Clauses.at(unit_clauseID).get_literals()[unit_literal_position];
        int var = std::abs(unit_literal);
        // printf("unit clauseID: %d | unit position: %d | literal: %d\n", unit_clauseID, unit_literal_position, unit_literal);
        
        if (unit_literal > 0)
        {   
            if (data.Variables.at(var).is_tseitin())
            {
                std::cout << "implying " << var << " to be true.\n";
                std::cout << "a has found a winning strategy... UNSAT\n";
                state = qbf::UNSAT;
            }
            assign(unit_literal, 1, searchLevel);
            implied_universals.push({unit_literal, searchLevel});
        }
        else
        {
            assign(std::abs(unit_literal), 0, searchLevel);
            implied_universals.push({std::abs(unit_literal), searchLevel});
        }

        universal_unit_clauses.pop();
    }

    std::cout << "DONE implying universal moves...\n";
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
    // int literal_position = data.Clauses.at(clauseID).get_unit_position();
    // int literal = data.Clauses.at(clauseID).get_literals()[literal_position];
    // std::cout << "unit literal in clause: " << literal << '\n';
    // data.Clauses.at(clauseID).print();

    return unit_flag;
}


bool Solver::stop_criterion_met(std::unordered_map<int, int> c1, int currentSearchLevel)
{   
    /*  
                        1st condition
        Among all its existential variables, one and only one of
        them has the highest decision level (which may not be
        the current decision level). Suppose this variable is V.
    */

    int V = -1, highest_decision_level = 0;
    std::pair<int, int> p;
    std::unordered_map<int, std::pair<int, int>> levels = {}; /* { descision_level: (appearances, V) } */
    
    for (const auto& [literal, sign] : c1)
    {   
        if (data.Variables.at(std::abs(literal)).is_universal())
            continue;

        V = std::abs(literal);
        int decision_level = data.Variables.at(std::abs(literal)).get_decision_level();
        
        if (decision_level > highest_decision_level) /* keep MAX and check at the end */
            highest_decision_level = decision_level;

        if (levels.find(decision_level) == levels.end()) /* level not found */
        { 
            p = std::make_pair(1, V);
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
        assigned 0 before decision level of V’s.
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
    std::unordered_map<int, int> new_clause = {}; // literal, sign (1: positive, 0: negative)

    for (int literal : c1)
    {   
        if (std::abs(literal) == pivot_variable) continue;
        (literal > 0) ? new_clause.insert({literal, 1}) : new_clause.insert({literal, 0});
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

        (literal > 0) ? new_clause.insert({literal, 1}) : new_clause.insert({literal, 0}); /* add literal to clause */
    }


    return new_clause;
}

/* TODO: write body */
void Solver::analyze_conflict()
{
    if (level == 0)
    {   
        std::cout << "analyze_conflict() led to root (" << level << ")" << '\n';
        state = qbf::UNSAT;
        return;
    }
    int currentDecisionLevel = level;
    Clause c1 = data.Clauses.at(conflicting_clause);
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
    if (GAME_FLAG)
        std::cout << "Mode: GAME INSTANCE\n";
    if (!GAME_FLAG)
        std::cout << "Mode: RANDOM INSTANCE\n";
    
    std::cout << "State: " << state << '\n';
    std::cout << "numClauses: " << data.numClauses << '\n';
    std::cout << "numVars: " << data.numVars << '\n';
    std::cout << "numTseitinVars: " << data.numTseitinVariables << '\n';
    std::cout << "numTseitinClauses: " << data.numTseitinClauses << '\n';
}


bool Solver::solve()
{   
    std::cout << "numClauses: " << data.numClauses << '\n';
    std::cout << "state: " << state << '\n';
    std::cout << "numVars: " << data.numVars << '\n';
    print_Clauses();
    // std::cout << "numTseitinVars: " << data.numTseitinVariables << '\n';
    // std::cout << "numTseitinClauses: " << data.numTseitinClauses << '\n';

    /* e plays {0, 0} */
    // level = 1;
    // int varID = 1;
    // int value = 1;
    // assign(varID, value, level);
    // imply(level);
    // if (GAME_FLAG == qbf::GAME_ON) imply_universal_move(level);

    // // print_Clauses();
    // /* a plays {1, 3} */
    // level++;
    // varID = 14;ß
    // value = 1;
    // assign(varID, value, level);
    // imply(level);
    // if (GAME_FLAG == qbf::GAME_ON) imply_universal_move(level);

    // /* e plays {2, 1} */
    // level++;
    // varID = 20;
    // value = 1;
    // assign(varID, value, level);
    // imply(level);
    // if (GAME_FLAG == qbf::GAME_ON) imply_universal_move(level);

    // /* a plays {3, 4} */
    // level++;
    // varID = 32;
    // value = 1;
    // assign(varID, value, level);
    // imply(level);
    // if (GAME_FLAG == qbf::GAME_ON) imply_universal_move(level);

    // /* e plays {4, 2} */
    // level++;
    // varID = 39;
    // value = 1;
    // assign(varID, value, level);
    // imply(level);
    // if (GAME_FLAG == qbf::GAME_ON) imply_universal_move(level);

    // level++;
    // varID = 11;
    // value = 1;
    
    // assign(varID, value, level);
    // imply(level);
    // imply_universal_move(level);

    // print_Clauses();

    // printStackOfPairsSafe(implied_variables);

    // level++;
    // varID = 10;
    // value = 1;
    // assign(varID, value, level);

    // if (state == qbf::UNSAT)
    //     restore_level(level);

    std::cout << conflicting_clause << '\n';

    level = 1;
    assign(1, 0, level);
    imply(level);
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Clauses();

    level++;
    assign(3, 0, level);
    imply(level);
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Clauses();

    level++;
    assign(4, 0, level);
    imply(level);
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Clauses();

    level++;
    assign(6, 0, level);
    imply(level);
    std::cout << "solver state: " << state << '\n';
    std::cout << "conflict clause: " << conflicting_clause << '\n';
    print_Clauses();

    
    for (int literal : data.Clauses.at(conflicting_clause).get_literals())
    {
        std::cout << literal << ": ";
        int variable = std::abs(literal);
        if (data.Variables.at(variable).get_antecedent_clause() != qbf::UNDEFINED)
        {
            std::cout << "implied due to " << data.Variables.at(variable).get_antecedent_clause() << " at level " << data.Variables.at(variable).get_decision_level() << '\n';
        }
        else
            std::cout << '\n';
    }

    // top of implication stack
    std::cout << implied_variables.top().first << " " << implied_variables.top().second << '\n';
    
    
    /* find conflict clause */
    int clause_falsidied = conflicting_clause; 
    std::vector<int> cl = data.Clauses.at(clause_falsidied).get_literals();

    /* find most recently implied literal */
    printVector(data.Clauses.at(clause_falsidied).get_literals(), true);
    int pivot_literal = implied_variables.top().first;
    int pivot_var = (pivot_literal > 0) ? pivot_literal : std::abs(pivot_literal);
    int pivot_literal_position;
    if (pivot_literal > 0)
        pivot_literal_position = data.Variables.at(pivot_var).get_position_in_clause(clause_falsidied, true);
    else
        pivot_literal_position = data.Variables.at(pivot_var).get_position_in_clause(clause_falsidied, false); 
    
    /* get antecedent */
    int antecedent_clause = data.Variables.at(pivot_var).get_antecedent_clause();
    std::vector<int> antecedent = data.Clauses.at(antecedent_clause).get_literals();
    printVector(antecedent, true);

    /* perform resolution */
    std::unordered_map<int, int> new_clause = resolve(cl, antecedent, pivot_var);

    bool criteria_met = stop_criterion_met(new_clause, level);
    



    printSet(conflicting_clauses);

    return state;
}