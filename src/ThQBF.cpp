#include "ThQBF.hpp"
#include <assert.h>

ThQBF::ThQBF (const QDimacsParser& parser) : level(UNDEFINED), solver_status(SolverStatus::PRESEARCH)
{
    this->Clauses              = parser.matrix;
    this->Blocks               = parser.quantifier_prefix;
    this->Variables            = parser.variables; 
    this->numVars              = parser.numVars;
    this->numClauses           = parser.numClauses;
    this->numCubes             = 0;
    this->numBlocks            = parser.numBlocks;
    this->numOfExistentialVars = parser.numOfExistentialVars;
    this->numOfUniversalVars   = parser.numOfUniversalVars;

    this->remainingVars        = parser.numVars;
    this->remainingClauses     = parser.numClauses;
    this->remainingCubes       = this->numCubes;
    this->remainingBlocks      = parser.numBlocks;

    this->S                    = parser.S;
    this->P                    = parser.P;

    this->last_clause_idx      = parser.numClauses;
    this->cubeID               = 0;
    
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

    assignment_trail[level].push(varID);
    Variables[varID].assignment_trail_index = assignment_trail.size();

    std::cout << "--------------------- ASSIGNMENT " << varID + 1 << ": " << value << " (LEVEL " << level << ") ---------------------\n"; 
    if (solver_status == SolverStatus::PRESEARCH)
    {
        Variables[varID].status = qbf::VariableStatus::ELIMINATED;
    }
    else if (solver_status == SolverStatus::SEARCH)
    {   
        if (Variables[varID].antecedent_clause == UNDEFINED && Variables[varID].antecedent_cube == UNDEFINED)
        {
            Variables[varID].status     = qbf::VariableStatus::ASSIGNED;
            decision_variable_at[level] = varID;
        }
        else
        {   std::cout << "implying...\n";
            Variables[varID].status = qbf::VariableStatus::IMPLIED;
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
                if (solver_status == SolverStatus::SAT)
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

        // TODO: include propagation in cubes
        if (1) /* if cube learning is enabled */
        {

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
                if (solver_status == SolverStatus::SAT)
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

        // TODO: include propagation in cubes
        if (1) /* if cube learning is enabled */
        {

        }
    }

    for (auto it=varsAffected.begin(); it!=varsAffected.end();)
    {   
        if (Variables[*it].numNegAppear == 0 && Variables[*it].numPosAppear == 0)
        {    
            // std::cout << "zero appear: " << varID + 1 << '\n';
            remove_variable(*it + 1);
            Variables[*it].status = qbf::VariableStatus::REMOVED;
        }
        ++it;
    }
    varsAffected = {};
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
    varsAffected.insert(varID);

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
        solver_status   = SolverStatus::UNSAT;
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
                varsAffected.insert(var);

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

    for (auto it=varsAffected.begin(); it!=varsAffected.end();)
    {   
        if (Variables[*it].numNegAppear + Variables[*it].numPosAppear != 0)
        {   
            restore_variable(*it + 1);
        }
        ++it;
    }

    // TODO: restore cubes that were affected at search_level
    if (1) /* if cube learning is enabled */
    {

    }

    solver_status = SolverStatus::SEARCH;
    varsAffected  = {};

    assignment_trail.erase(search_level);
    implied_e_variables.erase(search_level);
    decision_variable_at.erase(search_level);
}


void ThQBF::remove_variable (int variable)
{   
    // std::cout << "removing variable " << variable << "\n";
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

    // Variables[varID].available_values = 2;

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
    if (solver_status == SolverStatus::PRESEARCH)
    {   
        Clauses[clauseID].status = qbf::ClauseStatus::DELETED;
        // std::cout << "remove_clause -> if clauseID: " << clauseID << " " << qbf::ClauseStatus::to_string(Clauses[clauseID].status) <<"\n";
    }
    else if (solver_status == SolverStatus::SEARCH)
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

        varsAffected.insert(varID);
        Clauses[clauseID].state[i] = level;
    }

    if (!remainingClauses)
    {
        std::cout << "Empty matrix at level " << level << '\n';
        solver_status = SolverStatus::SAT;
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
                // std::cout << "implying...\n";
                std::cout << "unit clause: " << unit_clauseID << " unit literal: " << unit_literal << "\n";
                unit_clauses.pop();
                assign(variable, 1);
                implied_e_variables[level].push(varID);
                Variables[varID].implication_trail_index = implied_e_variables[level].size();
                print_Clauses();
                std::cout << "prefix:\n";
                print_Prefix();
                // print_Blocks();
                // print_Variables();
            }
            else
            {   
                // std::cout << "implying...\n";
                std::cout << "unit clause: " << unit_clauseID << " unit literal: " << unit_literal << "\n";
                unit_clauses.pop();
                assign(variable, 0);
                implied_e_variables[level].push(varID);
                Variables[varID].implication_trail_index = implied_e_variables[level].size();
                print_Clauses();
                std::cout << "prefix:\n";
                print_Prefix();
                // print_Blocks();
            }
            if (solver_status == SolverStatus::UNSAT)
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


void ThQBF::UniversalReduction (int clauseID)
{
    if (solver_status == SolverStatus::PRESEARCH)
    {
        std::cout << "Solver stage: PRESEARCH\n\n"; 
    }
    else
    {
        std::cout << "Solver stage: SEARCH\n\n";
    }
}


void ThQBF::PureLiteral ()
{
    if (solver_status == SolverStatus::PRESEARCH)
    {
        std::cout << "Solver stage: PRESEARCH\n\n"; 
    }
    else
    {
        std::cout << "Solver stage: SEARCH\n\n";
    }
}

// TODO: (deduce) write body
void ThQBF::deduce ()
{   
    if (solver_status == SolverStatus::PRESEARCH)
    {
        std::cout << "Solver stage: PRESEARCH\n\n"; 
        UnitPropagation();
    }
}

/* ================================ Conflict-driven Learning ================================ */
std::pair<int, int> ThQBF::analyse_conflict ()
{   
    int back_dl;
    std::pair<int, int> p;

    if (level == SolverStatus::ROOT)
    {   
        p.first  = SolverStatus::ROOT;
        p.second = SolverStatus::ROOT;
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
    std::cout << "learned clause:\n";
    print_hashmap(cl);
    
    p = clause_asserting_level(cl);
    // maybe add learned clause to Clauses_trail[level] to mark it as changed at this level
    add_clause_to_db(cl, p.second);
    std::cout << "clause asserting level: " << p.first << " asserting literal: " << p.second << '\n';

    return p;
}


void ThQBF::add_clause_to_db (const std::unordered_map<int, int>& learned_clause, int asserting_literal)
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
            if (ct != qbf::LiteralStatus::AVAILABLE)
            {
                varsAffected.insert(varID);
            }
            
            new_clause.state.push_back(ct);
            // std::cout << "literal: " << literal << " state: " << ct << '\n';

            if (literal > 0)
            {   
                // std::cout << "[POSITIVE] variable " << varID+1 << " clause " << last_clause_idx << " index " << index << '\n';
                Variables[varID].addOccurence(last_clause_idx, index, 1, "CLAUSE");
                if (ct != qbf::LiteralStatus::AVAILABLE)
                {   
                    // std::cout << literal << " not available\n";
                    Variables[varID].numPosAppear--;
                }
            }
            else
            {   
                // std::cout << "[NEGATIVE] variable " << varID+1 << " clause " << last_clause_idx << " index " << index << '\n';
                Variables[varID].addOccurence(last_clause_idx, index, 0, "CLAUSE");
                if (ct != qbf::LiteralStatus::AVAILABLE)
                {   
                    // std::cout << literal << " not available\n";
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
        Clauses_trail.at(level).insert(new_clause.clauseID);
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

        int trail_index = Variables[varID].implication_trail_index;
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


std::unordered_map<int, int> ThQBF::resolve (const std::unordered_map<int, int>& c1, 
                                             const std::unordered_map<int, int>& c2, 
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

        int varID = std::abs(literal) - 1;

        /* literal already present in clause */
        if (new_clause.find(literal) != new_clause.end())
        {
            continue;
        }

        /* opposite polarity is found */
        if (new_clause.find(-literal) != new_clause.end())
        {   
            assert(Variables[varID].is_existential() && "Tautological existentila pair: invalid reason or wrong pivot.");
            assert(Variables[varID].blockID > Variables[pivot_variable-1].blockID && "LD pair left of pivot: invalid reason or wrong pivot.");
            new_clause.insert({literal, ct});
            continue;
        }

        /* add literal to clause */
        new_clause.insert({literal, ct});
    }

    return new_clause;
}


bool ThQBF::stop_criteria_met (const std::unordered_map<int, int>& resolvent)
{   
    /*  
                        1st condition
        Among all its existential variables, one and only one of
        them has the highest decision level (which may not be
        the current decision level). Suppose this variable is V.
    */

    int L_max = -1;
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
        if (decision_level > L_max)
        {
            L_max = Variables[variable-1].level;
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
    
    if (L_max <= 0) /* all universal clause, or all existentials at the root level -> ROOT-UNSAT exception */
    {   
        solver_status = SolverStatus::ROOT;
        return false;
    }
    // check if max level appears more than once
    if (decision_levels[L_max].first > 1)
    {
        return false;
    }

    /*  
                        2nd condition
        V is in a decision level with an existential variable as the decision variable.
    */
    int decision_variable_at_V = decision_variable_at[L_max];

    if (!Variables[decision_variable_at_V-1].is_existential())
    {
        return false;
    }

    /*
                         3rd condition
        All universal literals with quantification level smaller 
        than V’s are  assigned to 0 before the decision level of V.
    */ 
    int V             = decision_levels[L_max].second;
    int V_quant_level = Variables[V-1].blockID;

    for (const auto& [literal, ct] : resolvent)
    {   
        int varID                 = std::abs(literal) - 1; 
        int variable_quant_level  = Variables[varID].blockID;

        if (Variables[varID].is_existential())
        {
            continue;
        }

        if (variable_quant_level >= V_quant_level)
        {
            continue;
        }

        /* only checking universal with qb(a) < qb(V) */

        if (Variables[varID].assignment == UNDEFINED)
        {
            return false;
        }

        // if (ct == qbf::LiteralStatus::AVAILABLE) // literal is free
        // {
        //     return false;
        // }

        if (Variables[varID].level >= L_max)
        {
            return false;
        }

        if (literal > 0) // has to be 0
        {
            int assignment = Variables[varID].assignment;
            if (assignment == 1)
            {
                return false;
            }
        }
        else // has to be 1
        {
            int assignment = Variables[varID].assignment;
            if (assignment == 0)
            {
                return false;
            }
        }
    }

    // std::cout << "stop criteria met:" << true << "\n";
    return true;
}


std::pair<int, int> ThQBF::clause_asserting_level (const std::unordered_map<int, int>& learned_clause)
{
    /* 
        asserting literal      -> existential literal at the maximum decision level
        clause asserting level -> second highest level among the decision levels of the rest of the existentials
    */
    int asserting_literal      = UNDEFINED;
    int clause_asserting_level =  0;
    int max_level              = -1;

    std::pair<int, int> p;

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

    assert(max_level >= 0 && "No existentials at max_level (Logical error - check stop criteria).\n");
    
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

        clause_asserting_level = std::max(clause_asserting_level, level);
    }

    p = std::make_pair(clause_asserting_level, asserting_literal);
    return p;
}

/* ================================ Satisfiability-directed Learning ================================  */
std::pair<int, int> ThQBF::analyse_SAT ()
{
    int back_dl;
    std::pair<int, int> p;




    return p;
}


void ThQBF::add_cube_to_db (const std::unordered_map<int, int>& learned_cube, int asserting_literal)
{
    Cube new_cube(cubeID, learned_cube, UNDEFINED);

    if (CubeHashes.find(new_cube.hash) == CubeHashes.end())
    {
        std::cout << "adding cube to db...\n";

        for (int i = 0; i < new_cube.literals.size(); i++)
        {
            int literal = new_cube.literals[i];
            int varID   = std::abs(literal) - 1;

            /* insert cube appearance */
            if (literal > 0)
            {
                Variables[varID].addOccurence(cubeID, i, 1, "CUBE");
                if (new_cube.state[i] != qbf::LiteralStatus::AVAILABLE)
                {
                    Variables[varID].numPosAppearCubes--;
                }
            }
            else
            {
                Variables[varID].addOccurence(cubeID, i, 0, "CUBE");
                if (new_cube.state[i] != qbf::LiteralStatus::AVAILABLE)
                {
                    Variables[varID].numNegAppearCubes--;
                }
            }

            /* update existential and universal literals numbers */
            if (new_cube.state[i] == qbf::LiteralStatus::AVAILABLE)
            {
                if (Variables[varID].is_existential())
                {
                    new_cube.e_num++;
                }
                else
                {
                    new_cube.a_num++;
                }
            }

            /* unit (universal) literal position */
            if (literal == asserting_literal)
            {
                new_cube.unit_literal_position = i;
            }
        }

        Cubes.push_back(new_cube); 
        CubeHashes.insert(new_cube.hash);
        Cubes_trail[level].insert(new_cube.cubeID);
        numCubes++; 
        remainingCubes++; 
        cubeID++;
    }
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
    std::cout << '\n';
}


void ThQBF::print_Cubes ()
{
    std::cout << "Remaining/currently active cubes: " << remainingCubes << '\n';
    for (int i = 0; i < Cubes.size(); i++)
    {   
        if (Cubes[i].status != qbf::CubeStatus::ACTIVE)
        {   
            continue;
        }
        std::cout << i << ": ";
        for (int j = 0; j < Cubes[i].size; j++)
        {   
            if (Cubes[i].state[j] == qbf::LiteralStatus::AVAILABLE)
            {
                std::cout << Cubes[i].literals[j] << " ";
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
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
    std::cout << '\n';
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
    std::cout << '\n';
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

    /* 
        preprocess: if any type of learning is enabled, only use UP and perhaps PL.
    */
    solver_status = SolverStatus::PRESEARCH;
    level         = PRESEARCH;

    deduce();

    if (solver_status == SolverStatus::SAT || solver_status == SolverStatus::UNSAT)
    {   
        std::cout << "ThQBF terminated during preprocessing.\n Status: " << SolverStatus::to_string(solver_status);
        return;
    }

    /* ===================== search loop ===================== */


}


void ThQBF::test ()
{   
    /* add some dummy cubes to test assignments and appearances updates */
    std::unordered_map<int, int> cube_1 = { {-1, qbf::LiteralStatus::AVAILABLE}, 
                                            { 2, qbf::LiteralStatus::AVAILABLE}, 
                                            { 4, qbf::LiteralStatus::AVAILABLE}, 
                                            {-5, qbf::LiteralStatus::AVAILABLE}, 
                                            { 9, qbf::LiteralStatus::AVAILABLE} };
    // Cube c1(cubeID++, cube_1, SolverStatus::PRESEARCH);

    std::unordered_map<int, int> cube_2 = { { 1, qbf::LiteralStatus::AVAILABLE}, 
                                            {-5, qbf::LiteralStatus::AVAILABLE}, 
                                            { 7, qbf::LiteralStatus::AVAILABLE} };
    // Cube c2(cubeID++, cube_2, SolverStatus::PRESEARCH);

    std::unordered_map<int, int> cube_3 = { { 3, qbf::LiteralStatus::AVAILABLE}, 
                                            {-4, qbf::LiteralStatus::AVAILABLE}, 
                                            { 9, qbf::LiteralStatus::AVAILABLE}, 
                                            {-11,qbf::LiteralStatus::AVAILABLE} };
    // Cube c3(cubeID++, cube_3, SolverStatus::PRESEARCH);

    // Cubes.push_back(c1); CubeHashes.insert(c1.hash); numCubes++; remainingCubes++;
    // Cubes.push_back(c2); CubeHashes.insert(c2.hash); numCubes++; remainingCubes++;
    // Cubes.push_back(c3); CubeHashes.insert(c3.hash); numCubes++; remainingCubes++;

    add_cube_to_db(cube_1, UNDEFINED);
    add_cube_to_db(cube_2, UNDEFINED);
    add_cube_to_db(cube_3, UNDEFINED);

    print_Blocks();
    print_Prefix();
    print_Clauses();
    print_Cubes();
    print_Variables();


    // /*  -------------------- test-SAT -------------------- */
    // level = PRESEARCH;
    // solver_status = SolverStatus::PRESEARCH;
    // UnitPropagation();
    // if (solver_status == SolverStatus::UNSAT)
    // {
    //     std::cout << "UNSAT at root...\n";
    //     return;
    // }

    // /* assign variables */
    // solver_status = SolverStatus::SEARCH;
    // level = 1;
    // assign(4, 0);
    // Path[4] = 0;
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(5, 1);
    // Path[5] = 1;
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();
    // imply();

    // print_hashmap(Path);


    // /* -------------------- test-UNSAT -------------------- */
    // solver_status = SolverStatus::SEARCH;
    // /* assign variables */
    // level = 1;
    // assign(1, 1);
    // print_Clauses();
    // imply();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // print_Blocks();
    // // print_Variables();
    

    // level++;
    // assign(2, 1);
    // print_Clauses();
    // imply();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // print_Blocks();
    // // print_Variables();


    // level++;
    // assign(3, 1);
    // print_Clauses();
    // imply();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // print_Blocks();
    // // print_Variables();



    // std::cout << "================================================\n";
    // std::pair<int, int> p;
    // if (solver_status == SolverStatus::UNSAT)
    // {
    //     p = analyse_conflict();
    //     solver_status = SolverStatus::SEARCH;
    // }

    // // backtrack up to clause asserting level (exclusive)
    // int back_dl = p.first;
    // while (level > back_dl)
    // {   
    //     restore_level(level);
    //     print_Clauses();
    //     std::cout << "prefix\n";
    //     print_Prefix();
    //     // print_Blocks();
    //     level--;
    // }

    // int unit_pos = Clauses[Clauses.size()-1].unit_literal_position;
    // int refVar   = std::abs(Clauses[Clauses.size()-1].literals[unit_pos]);
    // unit_clauses.push({Clauses.size()-1, level});
    // imply();
    // print_Clauses();
    // std::cout << "prefix\n";
    // print_Prefix();

    // // print_hashmap(Variables[5].positiveOccurrences);
    // // print_hashmap(Variables[5].negativeOccurrences);




    // std::cout << "======================================================\n"



}
