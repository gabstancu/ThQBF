#include "ThQBF.hpp"
#include <assert.h>

ThQBF::ThQBF (const QDimacsParser& parser) : level(UNDEFINED), solver_status(qbf::SolverStatus::PRESEARCH)
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

    this->S                    = parser.S;
    this->P                    = parser.P;

    this->last_clause_idx      = parser.numClauses;
    
    // construct prefix
    for (QuantifierBlock b : Blocks)
    {
        for (int variable : b.variables)
        {   
            PREFIX[b.blockID].insert(variable - 1); // start indexing in prefix from 0
        }
    }

    for (int i = 0; i < this->numClauses; i++)
    {
        ClauseHashes.insert(Clauses[i].hash);
    }
}


void ThQBF::assign (int variable, int value)
{   
    int varID = variable - 1;

    if (Variables[varID].status != qbf::VariableStatus::ACTIVE)
    {
        return;
    }

    std::cout << "--------------------- ASSIGNMENT " << varID + 1 << ": " << value << " (LEVEL " << level << ") ---------------------\n"; 
    if (solver_status == qbf::SolverStatus::PRESEARCH)
    {
        Variables[varID].status = qbf::VariableStatus::ELIMINATED;
    }
    else if (solver_status == qbf::SolverStatus::SEARCH)
    {
        if (Variables[varID].antecedent_clause == UNDEFINED)
        {
            Variables[varID].status  = qbf::VariableStatus::ASSIGNED;
            decision_variable_at[level] = varID;
        }
        else
        {
            Variables[varID].status = qbf::VariableStatus::IMPLIED;
            std::cout << "implying...\n";
        }
    }

    Variables[varID].level      = level;
    Variables[varID].assignment = value;
    Variables[varID].available_values--;

    int literal;

    if (value == 1)
    {
        // remove clauses where varID appears positive
        if (Variables[varID].numPosAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].positiveOccurrences)
            {   
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;

                remove_clause(clauseID, varID);
                if (solver_status == qbf::SolverStatus::SAT)
                {
                    /* IN SEARCH LOOP */
                    /* ??? analyze_SAT ??? */
                    /* ??? restore ??? */
                    return;
                }
                
            }
        }


        // remove negative appearances of varID from everywhere
        if (Variables[varID].numNegAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].negativeOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;
                
                literal = -(variable);
                remove_literal_from_clause(literal, clauseID, positionInClause);
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

                remove_clause(clauseID, varID);
                if (solver_status == qbf::SolverStatus::SAT)
                {
                    /* IN SEARCH LOOP */
                    /* ??? analyze_SAT ??? */
                    /* ??? restore ??? */
                    return;
                }
                
            }
        }

        // remove positive appearances of varID from everywhere
        if (Variables[varID].numPosAppear)
        {
            for (const auto& [clauseID, positionInClause] : Variables[varID].positiveOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                    continue;
                
                literal = variable;    
                remove_literal_from_clause(literal, clauseID, positionInClause);
            }
        }
    }

    for (const auto& varID : Variables_trail.at(level))
    {   
        if (Variables[varID].numNegAppear == 0 && Variables[varID].numPosAppear == 0)
        {    
            // std::cout << "zero appear: " << varID + 1 << '\n';
            remove_variable(varID + 1);
        }
    }
    Variables_trail.erase(level);
}


void ThQBF::remove_literal_from_clause (int literal, int clauseID, int positionInClause)
{   
    int varID = std::abs(literal) - 1;

    if (Clauses[clauseID].state[positionInClause] != qbf::LiteralStatus::AVAILABLE)
    {
        return;
    }

    Clauses[clauseID].state[positionInClause] = level;

    Clauses_trail[level].insert(clauseID);
    Variables_trail[level].insert(varID);

    if (literal > 0)
    {   
        Variables[varID].numPosAppear--;
    }
    else
    {   
        Variables[varID].numNegAppear--;
    }

    if (Variables[varID].is_existential())
    {   
        Clauses[clauseID].e_num--;
    }
    else
    {
        Clauses[clauseID].a_num--;
    }

    // check for all universal clause or empty clause
    if (Clauses[clauseID].e_num == 0)
    {   
        conflict_clause = clauseID;
        solver_status   = qbf::SolverStatus::UNSAT;
        // conficting_clauses.push_back(clauseID);
        return;
    }

    // if e_num == 1 find the position of the only existential in the clause
    if (Clauses[clauseID].e_num == 1)
    {   
        for (int i = 0; i < Clauses[clauseID].size; i++)
        {
            if (Clauses[clauseID].state[i] != qbf::LiteralStatus::AVAILABLE)
            {
                continue;
            }
            
            varID = std::abs(Clauses[clauseID].literals[i]) - 1;

            if (Variables[varID].is_existential())
            {
                int literal_position = i;

                if (clause_is_unit(clauseID, varID + 1))
                {   
                    unit_clauses.push({clauseID, level});
                    Clauses[clauseID].unit_literal_position = literal_position;
                    break;
                }
            }
        }
    }
}


void ThQBF::restore_level (int search_level)
{   
    std::cout << "restoring level " << search_level << "\n";
    // restore clauses that we affected at search_level
    for (const auto& clauseID : Clauses_trail.at(search_level))
    {   
        // std::cout << "clause " << clauseID << "\n";
        Clauses[clauseID].level = UNDEFINED;

        if (Clauses[clauseID].e_num == 1)
        {
            Clauses[clauseID].unit_literal_position = UNDEFINED;
        }

        if (Clauses[clauseID].status == qbf::ClauseStatus::SATISFIED)
        {   
            remainingClauses++;
            // std::cout << "Restoring clause " << clauseID << " remaining: " << remainingClauses << '\n';
        }
        Clauses[clauseID].status = qbf::ClauseStatus::ACTIVE;

        // restore literals
        for (int i = 0; i < Clauses[clauseID].size; i++)
        {   
            // std::cout << "looking at literal " << Clauses[clauseID].literals[i] << " state: " << Clauses[clauseID].state[i] << "\n";
            
            if (Clauses[clauseID].state[i] == search_level)
            {   
                int literal = Clauses[clauseID].literals[i];
                // std::cout << "looking at literal " << literal << " state: " << Clauses[clauseID].state[i] << "\n";
                int var     = std::abs(literal) - 1; 
                Variables_trail[search_level].insert(std::abs(literal) - 1);

                Clauses[clauseID].state[i] = qbf::LiteralStatus::AVAILABLE;

                if (literal > 0)
                {
                    Variables[var].numPosAppear++;
                }
                else
                {
                    Variables[var].numNegAppear++;
                }

                if (Variables[var].is_existential())
                {
                    Clauses[clauseID].e_num++;
                }
                else
                {
                    Clauses[clauseID].a_num++;
                }
            }
        }
    }
    Clauses_trail.erase(search_level);

    for (const auto& varID : Variables_trail[search_level])
    {   
        // std::cout << "varID " << varID << " search_level: " << search_level << '\n';
        // if (Variables[varID].status == qbf::VariableStatus::ACTIVE)
        // {   
        //     continue;
        // }
        restore_variable(varID + 1);
    }

    solver_status = qbf::SolverStatus::SEARCH;
    Variables_trail.erase(search_level);
    implied_e_variables.erase(search_level);
    decision_variable_at.erase(search_level);
}


void ThQBF::remove_variable (int variable)
{   
    std::cout << "removing variable " << variable << "\n";
    int varID           = variable - 1; 
    int blockID         = Variables[varID].blockID;
    int positionInBlock = Variables[varID].positionInBlock;

    Blocks[blockID].size--;
    Blocks[blockID].available_variables--;
    remainingVars--;
    // std::cout << "available vars in block " << Blocks[blockID].available_variables << '\n';
    PREFIX[blockID].erase(varID);

    if (Blocks[blockID].size == 0)
    {   
        Blocks[blockID].status = qbf::QuantifierBlockStatus::UNAVAILABLE;
        remainingBlocks--;
        PREFIX.erase(blockID);
    }


    Variables[varID].level  = level;
    // Variables[varID].status = qbf::VariableStatus::ASSIGNED;
}


void ThQBF::restore_variable (int variable)
{   
    // std::cout << "restoring variable: " << variable << "\n";
    int varID = variable - 1;

    if (Variables[varID].antecedent_clause != UNDEFINED)
    {   
        Variables[varID].antecedent_clause        = UNDEFINED;
        Variables[varID].pos_in_antecedent_clause = UNDEFINED;
    }
    
    Variables[varID].status     = qbf::VariableStatus::ACTIVE;
    Variables[varID].level      = UNDEFINED;
    Variables[varID].assignment = UNDEFINED;
    remainingVars++;

    int blockID         = Variables[varID].blockID;
    int positionInBlock = Variables[varID].positionInBlock;
    
    PREFIX[blockID].insert(varID);
    Blocks[blockID].size++;
    Blocks[blockID].available_variables++;

    if (Blocks[blockID].size == 1)
    {
        remainingBlocks++;
        Blocks[blockID].status = qbf::QuantifierBlockStatus::AVAILABLE;
    }
}


void ThQBF::remove_clause (int clauseID, int referenceVarID)
{
    if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
    {
        return;
    }
    
    remainingClauses--;
    if (solver_status == qbf::SolverStatus::PRESEARCH)
    {   
        Clauses[clauseID].status = qbf::ClauseStatus::DELETED;
        // std::cout << "remove_clause -> if clauseID: " << clauseID << " " << qbf::ClauseStatus::to_string(Clauses[clauseID].status) <<"\n";
    }
    else if (solver_status == qbf::SolverStatus::SEARCH)
    {
        Clauses[clauseID].status = qbf::ClauseStatus::SATISFIED;
    }
        
    Clauses[clauseID].level = level; 
    Clauses_trail[level].insert(clauseID);

    int literal, varID;
    for (int i = 0; i < Clauses[clauseID].size; i++)
    {
        if (Clauses[clauseID].state[i] != qbf::LiteralStatus::AVAILABLE)
        {   
            continue;
        }

        literal = Clauses[clauseID].literals[i];
        varID   = std::abs(literal) - 1;

        if (literal > 0)
        {   
            Variables[varID].numPosAppear--;
        }
        else
        {   
            Variables[varID].numNegAppear--;
        }

        if (Variables[varID].is_existential())
        {
            Clauses[clauseID].e_num--;
        }
        else
        {
            Clauses[clauseID].a_num--;
        }

        Variables_trail[level].insert(varID);
        Clauses[clauseID].state[i] = level;
    }

    if (!remainingClauses)
    {
        std::cout << "Empty matrix at level " << level << '\n';
        solver_status = qbf::SolverStatus::SAT;
        return;
    }
}


int ThQBF::clause_is_unit (int clauseID, int referenceVariable)
{   
    int referenceVarID  =  referenceVariable - 1;
    int reference_level =  Variables[referenceVarID].blockID;
    int unit_flag       =  1;

    for (int i = 0; i < Clauses[clauseID].size; i++)
    {
        if (Clauses[clauseID].state[i] != qbf::LiteralStatus::AVAILABLE)
        {
            continue;
        }

        int literal = Clauses[clauseID].literals[i];
        int var     = std::abs(literal) - 1;

        if (var == referenceVarID)
            continue;
        
        int universal_level = Variables[var].blockID;

        if (universal_level < reference_level)
        {
            unit_flag = 0;
            break;
        }
    }

    if (!unit_flag)
        return 0;

    return unit_flag;
}


void ThQBF::imply ()
{
    if (1) /* if QCDCL flag is on (maybe not exclusively) imply unit clauses */
    {
         while (!unit_clauses.empty())
        {   
            // std::cout << "num of unit clauses: " << unit_clauses.size() << '\n';
            int unit_clauseID         = unit_clauses.top().first;
            int unit_literal_position = Clauses[unit_clauseID].unit_literal_position;
            int unit_literal          = Clauses[unit_clauseID].literals[unit_literal_position];
            int varID                 = std::abs(unit_literal) - 1; 
            int variable              = std::abs(unit_literal); 
            
            if (Variables[varID].antecedent_clause != UNDEFINED) /* literal has already been implied */
            {   
                unit_clauses.pop();
                continue;
            }

            Variables[varID].antecedent_clause        = unit_clauseID;
            Variables[varID].pos_in_antecedent_clause = unit_literal_position;

            if (unit_literal > 0)
            {  
                // std::cout << "unit clause: " << unit_clauseID << " unit literal: " << unit_literal << "\n";
                unit_clauses.pop();
                assign(variable, 1);
                implied_e_variables[level].push(varID);
                Variables[varID].trail_index = implied_e_variables[level].size();
                print_Clauses();
            }
            else
            {   
                // std::cout << "unit clause: " << unit_clauseID << " unit literal: " << unit_literal << "\n";
                unit_clauses.pop();
                assign(variable, 0);
                implied_e_variables[level].push(varID);
                Variables[varID].trail_index = implied_e_variables[level].size();
                print_Clauses();
            }
            if (solver_status == qbf::SolverStatus::UNSAT)
            {   
                std::cout << "Empty (all universal clause) detected at level " << level << "\n";
                std::cout << "Conflicting clause: " << conflict_clause << '\n';
                return;
            }
        }
    }

    if (1) /* if Cube Learning flag is on imply unit cubes */
    {

    }
       
    // print_Clauses();
}


void ThQBF::UnitPropagation ()
{   
    int lit_is_unit = 0;

    for (int i = 0; i < S.size(); i++)
    {   
        // std::cout << "Checking " << S[i] << '\n';
        if (Variables[S[i] - 1].status != qbf::VariableStatus::ACTIVE)
        {
            continue;
        }
        
        // check positive appearances
        for (const auto& [clauseID, position] : Variables[S[i] - 1].positiveOccurrences)
        {   
            if (Clauses[clauseID].e_num > 1 && Clauses[clauseID].a_num == 0)
            {
                continue;
            }
            if (clause_is_unit(clauseID, S[i]) || Clauses[clauseID].is_unit())
            {
                unit_clauses.push({clauseID, PRESEARCH});
                Clauses[clauseID].unit_literal_position = position;
                lit_is_unit = 1;
                break;
            }
        }

        if (lit_is_unit) /* don't check negative appearances */
        {   
            lit_is_unit = 0;
            continue;
        }

        // check negative appearances
        for (const auto& [clauseID, position] : Variables[S[i] - 1].negativeOccurrences)
        {   
            if (Clauses[clauseID].e_num > 1 && Clauses[clauseID].a_num == 0)
            {
                continue;
            }

            if (clause_is_unit(clauseID, S[i]) || Clauses[clauseID].is_unit())
            {   
                unit_clauses.push({clauseID, PRESEARCH});
                Clauses[clauseID].unit_literal_position = position;
                break;
            }
        }
    }

    imply();
}


void ThQBF::UniversalReduction ()
{
    if (solver_status == qbf::SolverStatus::PRESEARCH)
    {
        std::cout << "Solver stage: PRESEARCH\n\n"; 
    }
    else
    {
        std::cout << "Solver stage: SEARCH\n\n";
    }
}

// TODO: PureLiteral () -> add pure literals 'trail'
void ThQBF::PureLiteral ()
{
    if (solver_status == qbf::SolverStatus::PRESEARCH)
    {
        std::cout << "Solver stage: PRESEARCH\n\n"; 
    }
    else
    {
        std::cout << "Solver stage: SEARCH\n\n";
    }
}

// TODO: deduce ()
void ThQBF::deduce ()
{   
    if (solver_status == qbf::SolverStatus::PRESEARCH)
    {
        std::cout << "Solver stage: PRESEARCH\n\n"; 
        UnitPropagation();
    }
}


std::pair<int, int> ThQBF::analyse_conflict ()
{   
    int back_dl;
    std::pair<int, int> p;

    if (level == qbf::SolverStatus::ROOT)
    {   
        p.first  = qbf::SolverStatus::ROOT;
        p.second = qbf::SolverStatus::ROOT;
        return p;
    }

    std::unordered_map<int, int> cl = Clauses[conflict_clause].map_representation();

    while (!stop_criteria_met(cl))
    {   
        int literal                       = choose_e_literal(cl);
        int variable                      = std::abs(literal);
        int varID                         = variable - 1;
        int antecedent_clauseID           = Variables[varID].antecedent_clause;
        std::unordered_map<int, int> ante = Clauses[antecedent_clauseID].map_representation();
        cl                                = resolve(cl, ante, variable);
    }
    // std::cout << "learned clause:\n";
    print_hashmap(cl);
    
    p = clause_asserting_level(cl);
    // maybe add learned clause to Clauses_trail[level] to mark it as changed at this level
    add_clause_to_db(cl, p.second);
    std::cout << "clause asserting level: " << p.first << " asserting literal: " << p.second << '\n';

    return p;
}


void ThQBF::add_clause_to_db (std::unordered_map<int, int> learned_clause, int asserting_literal)
{
    Clause           new_clause;
    std::vector<int> literals;

    for (const auto& [literal, ct] : learned_clause)
    {
        literals.push_back(literal);
    }
    std::sort(literals.begin(), literals.end());
    std::map<int, int> learned_cl_ordered(learned_clause.begin(), learned_clause.end());

    new_clause.literals = literals;
    new_clause.size     = literals.size();
    size_t h            = new_clause.compute_hash();

    if (ClauseHashes.find(h) == ClauseHashes.end())
    {
        std::cout << "adding clause to db...\n";
        ClauseHashes.insert(h);
        new_clause.hash                  = h;
        new_clause.status                = qbf::ClauseStatus::ACTIVE;
        new_clause.learned               = true;
        new_clause.level                 = UNDEFINED;
        new_clause.clauseID              = last_clause_idx;
        
        int index = 0;
        /* set state */
        for (const auto& [literal, ct] : learned_cl_ordered)
        {   
            int varID = std::abs(literal) - 1;
            Variables_trail[ct].insert(varID);
            new_clause.state.push_back(ct);
            // std::cout << "literal: " << literal << " state: " << ct << '\n';

            if (literal > 0)
            {   
                // std::cout << "[POSITIVE] variable " << varID+1 << " clause " << last_clause_idx << " index " << index << '\n';
                Variables[varID].addOccurence(last_clause_idx, index, 1);
                if (ct != qbf::LiteralStatus::AVAILABLE)
                {
                    Variables[varID].numPosAppear--;
                }
            }
            else
            {   
                // std::cout << "[NEGATIVE] variable " << varID+1 << " clause " << last_clause_idx << " index " << index << '\n';
                Variables[varID].addOccurence(last_clause_idx, index, 0);
                if (ct != qbf::LiteralStatus::AVAILABLE)
                {
                    Variables[varID].numNegAppear--;
                }
            }

            if (ct == qbf::LiteralStatus::AVAILABLE)
            {
                if (Variables[varID].is_existential())
                {
                    new_clause.e_num++;
                }
                else
                {
                    new_clause.a_num++;
                }
            }

            if (literal == asserting_literal)
            {   
                new_clause.unit_literal_position = index;
            }

            index++;
        }

        numClauses++;
        remainingClauses++;
        last_clause_idx++;
        Clauses.push_back(new_clause);
        // new_clause.print();
        Clauses_trail.at(level).insert(new_clause.clauseID);
        // new_clause.print();
        // print_Clauses();
    }
}


int ThQBF::choose_e_literal (std::unordered_map<int, int> cc)
{
    int most_recently_implied = UNDEFINED;
    int best_trail_index      = UNDEFINED;
    int top_level             = UNDEFINED;

    // find highest decision level among implied existential variables
    for (const auto& [literal, ct] : cc)
    {
        int variable = std::abs(literal); 
        int varID    = variable - 1;

        if (Variables[varID].is_universal())
        {
            continue;
        }

        if (Variables[varID].antecedent_clause == UNDEFINED)
        {
            continue;
        }

        top_level = std::max(top_level, Variables[varID].level);
    }

    assert(top_level>=0 && "No implied ∃ in clause (should be asserting or UNSAT-root).");

    // pick latest implied existential literal at top level by trail index
    for (const auto& [literal, ct] : cc)
    {
        int variable = std::abs(literal); 
        int varID    = variable - 1;

        if (Variables[varID].is_universal())
        {
            continue;
        }

        if (Variables[varID].antecedent_clause == UNDEFINED)
        {
            continue;
        }

        if (Variables[varID].level != top_level)
        {
            continue;
        }

        int trail_index = Variables[varID].trail_index;
        if (trail_index > best_trail_index)
        {
            most_recently_implied = literal;
            best_trail_index      = trail_index;
        }
    }

    assert(most_recently_implied != UNDEFINED && "No implied ∃ at top level (reason: bookkeeping bug?)");

    // assert that antecedent contains -most_recently_implied
    int var = std::abs(most_recently_implied) - 1;
    int antecedent = Variables[var].antecedent_clause;

    assert(Variables[var].appears_in_clause(antecedent) && "neg(literal) does not appear in antecedent!");
    
    bool polarity = (-most_recently_implied > 0);
    int position_in_ante = Variables[var].get_position_in_clause(antecedent, polarity);

    assert(Clauses[antecedent].literals[position_in_ante] == -most_recently_implied && "neg(literal) does not appear in antecedent!");
    
    return most_recently_implied;
}



std::unordered_map<int, int> ThQBF::resolve (std::unordered_map<int, int> c1, 
                                             std::unordered_map<int, int> c2, 
                                             int pivot_variable)
{
    std::unordered_map<int, int> new_clause;

    /* add all literals of c1 to new_clause  */
    for (const auto& [literal, ct] : c1)
    {
        if (std::abs(literal) == pivot_variable)
        {
            continue;
        }
        new_clause.insert({literal, ct});
    }

    /* add all literals of c2 to new_clause (omit opposite existentials) */
    for (const auto& [literal, ct] : c2)
    {
        if (std::abs(literal) == pivot_variable)
        {
            continue;
        }

        int variable = std::abs(literal);

        if (new_clause.find(literal) != new_clause.end())
        {
            continue;
        }

        /* opposite polarity is found */
        if (new_clause.find(-literal) != new_clause.end())
        {
            if (Variables[variable-1].is_existential())
            {
                new_clause.erase(literal);
                continue;
            }
            else /* if universal */
            {
                if (Variables[variable-1].blockID > Variables[pivot_variable-1].blockID)
                {
                    new_clause.insert({literal, ct});
                    continue;
                }
            }
        }

        /* add literal to clause */
        new_clause.insert({literal, ct});
    }


    return new_clause;
}


bool ThQBF::stop_criteria_met(std::unordered_map<int, int> resolvent)
{   
    /*  
                        1st condition
        Among all its existential variables, one and only one of
        them has the highest decision level (which may not be
        the current decision level). Suppose this variable is V.
    */

    int max_decision_level = -1;
    std::unordered_map<int, std::pair<int, int>> decision_levels = {}; 
    /* { descision_level: (count, V) } */

    for (const auto& [literal, ct] : resolvent)
    {   
        int variable = std::abs(literal);

        if (Variables[variable-1].is_universal())
        {
            continue;
        }

        int decision_level = Variables[variable-1].level;
        if (decision_level > max_decision_level)
        {
            max_decision_level = Variables[variable-1].level;
        }

        if (decision_levels.find(decision_level) == decision_levels.end())
        {
            std::pair<int, int> p = std::make_pair(1, variable);
            decision_levels.insert({decision_level, p});
        }
        else
        {
            decision_levels[decision_level].first++;
        }
    }
    // check if max level appears more than once
    if (decision_levels[max_decision_level].first > 1)
        return false;

    /*  
                        2nd condition
        V is in a decision level with an existential variable as the decision variable.
    */
    int V_decision_level       = decision_levels[max_decision_level].second;
    int decision_variable_at_V = decision_variable_at[V_decision_level];

    if (!Variables[decision_variable_at_V-1].is_existential())
        return false;

    /*
                         3rd condition
        All universal literals with quantification level smaller 
        than V’s are  assigned to 0 before the decision level of V.
    */ 
    int V             = decision_levels[max_decision_level].second;
    int V_quant_level = Variables[V-1].blockID;

    for (const auto& [literal, ct] : resolvent)
    {   
        int variable              = std::abs(literal);
        int variable_quant_level  = Variables[variable-1].blockID;

        if (Variables[variable-1].is_existential())
        {
            continue;
        }

        if (variable_quant_level > V_quant_level)
        {
            continue;
        }

        /* only checking universal with qb(a) < qb(V) */
        if (ct == qbf::LiteralStatus::AVAILABLE) // literal is free
        {
            return false;
        }

        /* use Literal to determine whether the literal is tailing */
        // if (ct == qbf::LiteralStatus::UNIVERSAL_REDUCTION) 
        // {
        //     continue;
        // }

        /* if the literal is not available but its variable has not been assigned then it has been reduced */
        if (Variables[variable-1].assignment == UNDEFINED)
        {
            continue;
        }

        if (literal > 0) // has to be 0
        {
            int assignment = Variables[variable-1].assignment;
            if (assignment == 1)
            {
                return false;
            }
        }
        else // has to be 1
        {
            int assignment = Variables[variable-1].assignment;
            if (assignment == 0)
            {
                return false;
            }
        }
    }

    // std::cout << "stop criteria met:" << true << "\n";
    return true;
}


std::pair<int, int> ThQBF::clause_asserting_level (std::unordered_map<int, int> learned_clause)
{
    /* 
        asserting literal      -> existential literal at the maximum decision level
        clause asserting level -> second highest level among the decision level of the rest of the existentials
    */
    int asserting_literal      = UNDEFINED;
    int clause_asserting_level = 0;
    int max_level              = UNDEFINED;


    int conflict_level = level;
    int diff           = INT_MAX;

    /* find max_level */
    for (const auto& [literal, ct] : learned_clause)
    {
        int variable = std::abs(literal);
        int level    = Variables[variable-1].level; 

        if (Variables[variable-1].is_universal())
        {
            continue;
        }

        /* find max level */
        if (level > max_level)
        {   
            asserting_literal = literal;
            max_level         = level;
        }
    }

    /* find second highest level: asserting level */
    for (const auto& [literal, ct] : learned_clause)
    {
        int variable = std::abs(literal);
        int level    = Variables[variable-1].level; 

        if (Variables[variable-1].is_universal())
        {
            continue;
        }

        if (literal == asserting_literal)
        {
            continue;
        }

        if (diff > max_level - level)
        {   
            diff = max_level - level;
            clause_asserting_level = level;
        }
    }

    std::pair<int, int> p = std::make_pair(clause_asserting_level, asserting_literal);
    return p;
}


void ThQBF::print_Clauses ()
{   
    std::cout << "Remaining clauses: " << remainingClauses << '\n';
    for (int i = 0; i < Clauses.size(); i++)
    {   
        if (Clauses[i].status != qbf::ClauseStatus::ACTIVE)
        {
            continue;
        }
        std::cout << i << ": "; 
        for (int j = 0; j < Clauses[i].size; j++)
        {
            if (Clauses[i].state[j] == qbf::LiteralStatus::AVAILABLE)
            {
                std::cout << Clauses[i].literals[j] << " ";
            }
        }
        std::cout << " e_num.: "     << Clauses[i].e_num << " ";
        std::cout << " a_num.: "     << Clauses[i].a_num << " ";
        std::cout << " status: "     << qbf::ClauseStatus::to_string(Clauses[i].status) << '\n';
    }
}


void ThQBF::print_Variables ()
{
    for (int i = 0; i < Variables.size(); i++)
    {
        if (Variables[i].status != qbf::VariableStatus::ACTIVE)
        {
            continue;
        }
        std::cout << "variable: " << i + 1 << " pos.: " << Variables[i].numPosAppear << " neg.: " << Variables[i].numNegAppear << '\n';
    }
}


void ThQBF::print_Blocks ()
{
    for (int i = 0; i < Blocks.size(); i++)
    {
        // if (Blocks[i].status == qbf::QuantifierBlockStatus::UNAVAILABLE)
        // {
        //     continue;
        // }
        Blocks[i].print();
    }
}


void ThQBF::print_Prefix ()
{
    for (const auto& [blockID, variables] : PREFIX)
    {
        std::cout << blockID << ": ";
        for (int variable : variables)
        {
            std::cout << variable + 1 << " ";
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}


void ThQBF::solve ()
{   
    print_Blocks();
    print_Prefix();
    print_Clauses();
    print_Variables();


    level = PRESEARCH;
    solver_status = qbf::SolverStatus::PRESEARCH;
    UnitPropagation();
    if (solver_status == qbf::SolverStatus::UNSAT)
    {
        std::cout << "UNSAT at root...\n";
        return;
    }

    /* -------------------- test-UNSAT -------------------- */
    solver_status = qbf::SolverStatus::SEARCH;
    /* assign variables */
    level = 1;
    assign(1, 1);
    print_Clauses();
    imply();
    std::cout << "prefix\n";
    print_Prefix();
    

    level++;
    assign(2, 1);
    print_Clauses();
    imply();
    std::cout << "prefix\n";
    print_Prefix();


    level++;
    assign(3, 1);
    print_Clauses();
    imply();
    std::cout << "prefix\n";
    print_Prefix();
   

    std::cout << "================================================\n";
    std::pair<int, int> p;
    if (solver_status == qbf::SolverStatus::UNSAT)
    {
        p = analyse_conflict();
        solver_status = qbf::SolverStatus::SEARCH;
    }

    // // backtrack up to clause asserting level (exclusive)
    // int back_dl = p.first;
    // while (level > back_dl)
    // {   
    //     restore_level(level);
    //     print_Clauses();
    //     level--;
    // }
    // level++;

    // int unit_pos = Clauses[Clauses.size()-1].unit_literal_position;
    // int refVar = std::abs(Clauses[Clauses.size()-1].literals[unit_pos]);
    // unit_clauses.push({Clauses.size()-1, level});
    // imply();
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(4, 0);
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(5, 1);
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();
    // imply();



    // std::cout << "======================================================\n"


    /*  -------------------- test-SAT -------------------- */
    // level = PRESEARCH;
    // solver_status = qbf::SolverStatus::PRESEARCH;
    // UnitPropagation();
    // if (solver_status == qbf::SolverStatus::UNSAT)
    // {
    //     std::cout << "UNSAT at root...\n";
    //     return;
    // }

    /* assign variables */
    // level = 1;
    // solver_status = qbf::SolverStatus::SEARCH;
    // assign(4, 0);
    // imply();
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // solver_status = qbf::SolverStatus::SEARCH;
    // assign(5, 1);
    // imply();
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // solver_status = qbf::SolverStatus::SEARCH;
    // assign(7, 0);
    // imply();
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();



}
