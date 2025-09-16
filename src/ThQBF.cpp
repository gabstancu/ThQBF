#include "ThQBF.hpp"
#include "internal.h"
#include <assert.h>
#include <climits>

ThQBF::ThQBF (const QDimacsParser& parser, const Options& options, tqbf::Logger* lg) : opts(options), logger(lg),  level(UNDEFINED), solver_status(SolverStatus::PRESEARCH)
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

    if (logger)
    {
        logger->log.n_vars    = this->numVars;
        logger->log.n_clauses = this->numClauses;
        logger->log.n_blocks  = this->numBlocks;
        logger->log.k         =  0;
        logger->log.L         = -1;
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

    // std::cout << "============================ ASSIGNMENT " << varID + 1 << ": " << value << " (LEVEL " << level << ") ============================\n"; 
    if (solver_status == SolverStatus::PRESEARCH)
    {
        Variables[varID].status = qbf::VariableStatus::ELIMINATED;
        Variables[varID].available_values = 0;
    }
    else if (solver_status == SolverStatus::SEARCH)
    {   
        if (Variables[varID].antecedent_clause == UNDEFINED && Variables[varID].antecedent_cube == UNDEFINED)
        {
            Variables[varID].status     = qbf::VariableStatus::ASSIGNED;
            decision_variable_at[level] = varID;
            Variables[varID].available_values--;
        }
        else
        {   
            Variables[varID].status = qbf::VariableStatus::IMPLIED;
        }
    }

    Variables[varID].level      = level;
    Variables[varID].assignment = value;

    if (value == 1)
    {
        // Path.push_back(variable);
        Path[variable] = level;
    }
    else
    {
        // Path.push_back(-variable);
        Path[-variable] = level;
    }
    // std::cout << "Current assignment: \n";
    // // printVector(Path, true);
    // print_hashmap(Path);

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
                    check_affectedVars();
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
                // printf("removing %d from %d at level %d\n", literal, clauseID, level);
                remove_literal_from_clause(literal, clauseID, positionInClause);
                if (solver_status == SolverStatus::UNSAT)
                {
                    // std::cout << "empty clause " << clauseID << " at level " << level << '\n';
                    // Clauses[clauseID].print();
                    // return;
                }
            }
        } 

        
        if (opts.cube_learning /* && level != SolverStatus::PRESEARCH */) /* if cube learning is enabled */
        {
            if (Cubes.size() != 0)
            {
                // remove positive occurences of varID from everywhere
                for (const auto& [cubeID, positionInCube] : Variables[varID].positiveOccurrencesCubes)
                {   
                    if (!Cubes[cubeID].is_active())
                        continue;
                    
                    literal = variable;
                    remove_literal_from_cube(literal, cubeID, positionInCube);
                    if (solver_status == SolverStatus::SAT)
                    {   
                        check_affectedVars();
                        return;
                    }
                }

                // remove cubes where varID appears negative
                for (const auto& [cubeID, positionInCube] : Variables[varID].negativeOccurrencesCubes)
                {
                    if (!Cubes[cubeID].is_active())
                        continue;
                    
                    // literal = -(variable);
                    remove_cube(cubeID, varID);
                }
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
                if (solver_status == SolverStatus::SAT)
                {   
                    check_affectedVars();
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
                // printf("removing %d from %d at level %d\n", literal, clauseID, level);
                remove_literal_from_clause(literal, clauseID, positionInClause);
                if (solver_status == SolverStatus::UNSAT)
                {
                    // std::cout << "empty clause " << clauseID << " at level " << level << '\n';
                    // Clauses[clauseID].print();
                    // return;
                }
            }
        }

        if (opts.cube_learning /* && level != SolverStatus::PRESEARCH */) /* if cube learning is enabled */
        {
            if (Cubes.size() != 0)
            {   
                // remove cubes where varID appears positive
                if (Variables[varID].numPosAppearCubes)
                {
                    for (const auto& [cubeID, positionInCube] : Variables[varID].positiveOccurrencesCubes)
                    {
                        if (!Cubes[cubeID].is_active())
                            continue;
                        
                        // literal = variable;
                        remove_cube(cubeID, varID);
                    }
                }
                
                // remove negative occurences of varID from everywhere
                if (Variables[varID].numNegAppearCubes)
                {
                    for (const auto& [cubeID, positionInCube] : Variables[varID].negativeOccurrencesCubes)
                    {
                        if (!Cubes[cubeID].is_active())
                            continue;
                        
                        literal = -(variable);
                        remove_literal_from_cube(literal, cubeID, positionInCube);
                        if (solver_status == SolverStatus::SAT)
                        {
                            check_affectedVars();
                            return;
                        }
                    }   
                }  
            }
        }
    }
    check_affectedVars();
}


void ThQBF::check_affectedVars ()
{
    for (auto it=varsAffected.begin(); it!=varsAffected.end();)
    {   
        if (Variables[*it].numNegAppear == 0 && 
            Variables[*it].numPosAppear == 0 &&
            Variables[*it].numPosAppearCubes == 0 &&
            Variables[*it].numNegAppearCubes == 0)
        {   
            // std::cout << "zero appear: " << *it + 1 << '\n';
            remove_variable(*it + 1);
            if (Variables[*it].status == qbf::VariableStatus::ACTIVE)
            {
                Variables[*it].status = qbf::VariableStatus::REMOVED;
            }
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

        if (!Variables[varID].numPosAppear)
        {
            PureLiterals[varID] = 1;
        }
    }
    else
    {   
        Variables[varID].numNegAppear--;
        if (!Variables[varID].numNegAppear)
        {
            PureLiterals[varID] = 0;
        }
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


void ThQBF::remove_literal_from_cube (int literal, int cubeID, int positionInCube)
{
    int varID = std::abs(literal) - 1;

    if (Cubes[cubeID].state[positionInCube] != qbf::LiteralStatus::AVAILABLE)
    {   
        // std::cout << "---------- in ---------\n";
        // printf("cube: %d | literal: %d | position in cube: %d\n", cubeID, literal, positionInCube);
        return;
    }


    Cubes[cubeID].state[positionInCube] = level;

    Cubes_trail[level].insert(cubeID);
    varsAffected.insert(varID);

    if (literal > 0)
    {
        Variables[varID].numPosAppearCubes--;
    }
    else
    {
        Variables[varID].numNegAppearCubes--;
    }

    if (Variables[varID].is_existential())
    {
        Cubes[cubeID].e_num--;
    }
    else
    {
        Cubes[cubeID].a_num--;
    }

    if (Cubes[cubeID].a_num == 0 && Cubes[cubeID].e_num == 0)
    {
        solver_status   = SolverStatus::SAT;
        satisfying_cube = cubeID;
        return;
    }

    // if a_num == 1 find the position of the only universal in the clause
    if (Cubes[cubeID].a_num == 1)
    {
        for (int i = 0; i < Cubes[cubeID].size; i++)
        {
            if (Cubes[cubeID].state[i] != qbf::LiteralStatus::AVAILABLE)
            {
                continue;
            }

            varID = std::abs(Cubes[cubeID].literals[i]) - 1;

            if (Variables[varID].is_universal())
            {
                int literal_position = i;

                if (cube_is_unit(cubeID, varID + 1))
                {
                    unit_cubes.push({cubeID, level});
                    Cubes[cubeID].unit_literal_position = literal_position;
                    break;
                }
            }
        }
    }
}


void ThQBF::restore_level (int search_level)
{   
    // std::cout << "restoring level " << search_level << "\n";
    // restore clauses that we affected at search_level
    for (const auto& clauseID : Clauses_trail.at(search_level))
    {   
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

    if (opts.cube_learning && !Cubes.empty()) /* if cube learning is enabled */
    {
        for (const auto& cubeID : Cubes_trail.at(search_level))
        {   
            Cubes[cubeID].level = UNDEFINED;

            if (Cubes[cubeID].a_num == 1)
            {
                Cubes[cubeID].unit_literal_position = UNDEFINED;
            }

            if (Cubes[cubeID].status == qbf::CubeStatus::INACTIVE)
            {
                remainingCubes++;
            }

            Cubes[cubeID].status = qbf::CubeStatus::ACTIVE;

            // restore literals in cube
            for (int i = 0; i < Cubes[cubeID].size; i++)
            {
                if (Cubes[cubeID].state[i] == search_level)
                {
                    int literal = Cubes[cubeID].literals[i];
                    int var     = std::abs(literal) - 1;

                    Cubes[cubeID].state[i] = qbf::LiteralStatus::AVAILABLE;

                    if (literal > 0)
                    {
                        Variables[var].numPosAppearCubes++;
                    }
                    else
                    {
                        Variables[var].numNegAppearCubes++;
                    }

                    if (Variables[var].is_existential())
                    {
                        Cubes[cubeID].e_num++;
                    }
                    else
                    {
                        Cubes[cubeID].a_num++;
                    }
                }
            }
        }

        Cubes_trail.erase(search_level);
    }

    // printSet(varsAffected);
    for (auto it=varsAffected.begin(); it!=varsAffected.end();)
    {   
        if (Variables[*it].status == qbf::VariableStatus::ACTIVE)
        {   
            ++it;
            continue;
        }
        if (Variables[*it].numNegAppear      + Variables[*it].numPosAppear      != 0 ||
            Variables[*it].numNegAppearCubes + Variables[*it].numPosAppearCubes != 0)
        {   
            restore_variable(*it + 1);
            // print_Prefix();
            // print_Blocks();
            // print_Clauses();
        }
        ++it;
    }

    // solver_status = SolverStatus::SEARCH;
    varsAffected  = {};

    assignment_trail.erase(search_level);
    implied_e_variables.erase(search_level);
    implied_a_variables.erase(search_level);
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
    // std::cout << "block size " << Blocks[blockID].size << '\n';
    PREFIX[blockID].erase(varID);
    // print_Prefix();


    if (Blocks[blockID].size == 0)
    {   
        // std::cout << "block " << blockID << " removed!\n";
        Blocks[blockID].status = qbf::QuantifierBlockStatus::UNAVAILABLE;
        remainingBlocks--;
        PREFIX.erase(blockID);
        // print_Prefix();
    }


    Variables[varID].level  = level;
    // Variables[varID].status = qbf::VariableStatus::ASSIGNED;
}


void ThQBF::restore_variable (int variable)
{   
    // std::cout << "restoring variable: " << variable << "\n";
    // std::cout << "variable status: " << qbf::VariableStatus::to_string(Variables[variable-1].status) << '\n';
    int varID = variable - 1;

    if (Variables[varID].antecedent_clause != UNDEFINED)
    {   
        Variables[varID].antecedent_clause        = UNDEFINED;
        Variables[varID].pos_in_antecedent_clause = UNDEFINED;
        // Variables[varID].available_values         = 2;
    }

    if (Variables[varID].antecedent_cube != UNDEFINED)
    {
        Variables[varID].antecedent_cube        = UNDEFINED;
        Variables[varID].pos_in_antecedent_cube = UNDEFINED;
        // Variables[varID].available_values         = 2;
    }

    if (Variables[varID].assignment != UNDEFINED)
    {
        if (Variables[varID].assignment == 1)
        {
            Path.erase(variable);
        }
        else
        {
            Path.erase(-variable);
        }
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
    Clauses[clauseID].status = qbf::ClauseStatus::SATISFIED; // TODO: keep temporary
        
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

            if (!Variables[varID].numPosAppear)
            {
                PureLiterals[varID] = 1;
            }
        }
        else
        {   
            Variables[varID].numNegAppear--;
            if (!Variables[varID].numNegAppear)
            {
                PureLiterals[varID] = 0;
            }
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
        // std::cout << "Empty matrix at level " << level << '\n';
        solver_status = SolverStatus::SAT;
        return;
    }
}


void ThQBF::remove_cube (int cubeID, int referenceVarID)
{
    if (Cubes[cubeID].status != qbf::CubeStatus::ACTIVE)
    {
        return;
    }

    remainingCubes--;

    Cubes[cubeID].status = qbf::CubeStatus::INACTIVE;
    Cubes[cubeID].level  = level;

    Cubes_trail[level].insert(cubeID);

    int literal, varID;
    for (int i = 0; i < Cubes[cubeID].size; i++)
    {
        if (Cubes[cubeID].state[i] != qbf::LiteralStatus::AVAILABLE)
        {
            continue;
        }

        literal = Cubes[cubeID].literals[i];
        varID   = std::abs(literal) - 1;

        if (literal > 0)
        {
            Variables[varID].numPosAppearCubes--;
        }
        else
        {
            Variables[varID].numNegAppearCubes--;
        }

        if (Variables[varID].is_existential())
        {
            Cubes[cubeID].e_num--;
        }
        else
        {
            Cubes[cubeID].a_num--;
        }

        varsAffected.insert(varID);
        Cubes[cubeID].state[i] = level;
    }

    if (!remainingCubes)
    {
        std::cout << "No cubes left in cubes db\n";
    }
}


int ThQBF::clause_is_unit (int clauseID, int referenceVariable)
{   
    int referenceVarID  =  referenceVariable - 1;
    int reference_level =  Variables[referenceVarID].blockID;
    int unit_flag       =  1;

    if (Clauses[clauseID].e_num > 1)
    {
        return 0;
    }

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


int ThQBF::infer ()
{  
    std::unordered_map<int, int> assignments;
    while (1)
    {   
        assignments = Path;

        if (opts.ur)
        {   
            // std::cout << "===================== UR =====================\n";
            UniversalReduction();
            if (solver_status == SolverStatus::SAT || solver_status == SolverStatus::UNSAT)
            {
                return solver_status;
            }
        }
        
        if (opts.up)
        {
            // std::cout << "===================== UP =====================\n";
            UnitPropagation();
            if (solver_status == SolverStatus::SAT || solver_status == SolverStatus::UNSAT)
            {
                return solver_status;
            }
        }

        if (opts.pl)
        {
            // std::cout << "===================== PL =====================\n";
            PureLiteral();
            if (solver_status == SolverStatus::SAT || solver_status == SolverStatus::UNSAT)
            {
                return solver_status;
            }
        }
        
        if (assignments == Path)
        {
            return SolverStatus::SEARCH;
        }
    }
}


void ThQBF::UnitPropagation ()
{   
    int lit_is_unit = 0;

    for (int i = 0; i < S.size(); i++)
    {   
        if (Variables[S[i] - 1].status != qbf::VariableStatus::ACTIVE)
        {
            continue;
        }
    
        // check positive appearances
        for (const auto& [clauseID, position] : Variables[S[i] - 1].positiveOccurrences)
        {   
            if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
            {
                continue;
            }

            if (clause_is_unit(clauseID, S[i]))
            {
                unit_clauses.push({clauseID, level});
                Clauses[clauseID].unit_literal_position = position;
                lit_is_unit = 1;
                break;
            }
        }

        if (lit_is_unit) /* ignore check negative appearances if lit is unit */
        {   
            lit_is_unit = 0;
            continue;
        }

        // check negative appearances
        for (const auto& [clauseID, position] : Variables[S[i] - 1].negativeOccurrences)
        {   
            if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
            {
                continue;
            }

            if (clause_is_unit(clauseID, S[i]))
            {   
                unit_clauses.push({clauseID, level});
                Clauses[clauseID].unit_literal_position = position;
                break;
            }
        }
    }

    lit_is_unit = 0;

    for (int i = 0; i < P.size(); i++)
    {   
        if (Variables[P[i] - 1].status != qbf::VariableStatus::ACTIVE)
        {
            continue;
        }
    
        // check positive appearances
        for (const auto& [cubeID, position] : Variables[P[i] - 1].positiveOccurrencesCubes)
        {   
            if (Cubes[cubeID].status != qbf::CubeStatus::ACTIVE)
            {
                continue;
            }

            if (cube_is_unit(cubeID, P[i]))
            {
                unit_cubes.push({cubeID, level});
                Cubes[cubeID].unit_literal_position = position;
                lit_is_unit = 1;
                break;
            }
        }

        if (lit_is_unit) /* ignore check negative appearances if lit is unit */
        {   
            lit_is_unit = 0;
            continue;
        }

        // check negative appearances
        for (const auto& [cubeID, position] : Variables[P[i] - 1].negativeOccurrencesCubes)
        {   
            if (Cubes[cubeID].status != qbf::CubeStatus::ACTIVE)
            {
                continue;
            }

            if (cube_is_unit(cubeID, P[i]))
            {   
                unit_cubes.push({cubeID, level});
                Cubes[cubeID].unit_literal_position = position;
                break;
            }
        }
    }

    deduce();
    return;
}


bool ThQBF::can_perform_UR (int u, int clauseID)
{
    int varID    = u - 1;
    int qb_u     = Variables[varID].blockID;
    int e_left   = 0;

    for (int i = 0; i < Clauses[clauseID].size; i++)
    {
        if (Clauses[clauseID].state[i] != qbf::LiteralStatus::AVAILABLE)
        {
            continue;
        }

        int literal = Clauses[clauseID].literals[i];
        int var     = std::abs(literal) - 1;

        if (Variables[var].is_existential() && Variables[var].blockID < qb_u)
        {
            e_left++;
        }
    }

    if (e_left == Clauses[clauseID].e_num)
    {
        return true;
    }

    return false;
}


void ThQBF::UniversalReduction ()
{
    for (int u : P)
    {
        if (Variables[u-1].blockID == 0)
        {
            continue;
        }

        if (Variables[u-1].status != qbf::VariableStatus::ACTIVE)
        {
            continue;
        }

        /* check clauses that u appears in */
        if (Variables[u-1].numPosAppear)
        {
            for (const auto& [clauseID, position] : Variables[u-1].positiveOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                {
                    continue;
                }

                if (can_perform_UR(u, clauseID))
                {
                    remove_literal_from_clause(u, clauseID, position);
                }
            }
        }

        if (Variables[u-1].numNegAppear)
        {
            for (const auto& [clauseID, position] : Variables[u-1].negativeOccurrences)
            {
                if (Clauses[clauseID].status != qbf::ClauseStatus::ACTIVE)
                {
                    continue;
                }

                if (can_perform_UR(u, clauseID))
                {
                    remove_literal_from_clause(-u, clauseID, position);
                }
            }
        }
    }

    check_affectedVars();
}


void ThQBF::PureLiteral ()
{
    if (PureLiterals.size() == 0)
    {
        for (Variable v : Variables)
        {
            int varID = v.varID;

            /* pure positive */
            if (Variables[varID].numPosAppear && !Variables[varID].numNegAppear)
            {
                if (Variables[varID].is_existential())
                {
                    assign(v.variable, 1);
                }
                else
                {
                    assign(v.variable, 0);
                }
            }

            /* pure negative */
            if (!Variables[varID].numPosAppear && Variables[varID].numNegAppear)
            {
                if (Variables[varID].is_existential())
                {
                    assign(v.variable, 0);
                }
                else
                {
                    assign(v.variable, 1);
                }
            }
        }
    }
    else
    {
        for (const auto& [literal, polarity] : PureLiterals)
        {
            int varID = std::abs(literal) - 1;

            if (polarity == 1) /* pure positive */
            {
                if (Variables[varID].is_existential())
                {
                    assign(varID+1, 1);
                }
                else
                {
                    assign(varID+1, 0);
                }
            }
            else if (polarity == 0) /* pure negative */
            {
                if (Variables[varID].is_existential())
                {
                    assign(varID+1, 0);
                }
                else
                {
                    assign(varID+1, 1);
                }
            }
        }
        PureLiterals.clear();
    }
}


int ThQBF::deduce ()
{   
    if (!opts.up)
    {
        std::cout << "Deducing is not enabled\n";
        return solver_status;
    }

    while (solver_status != SolverStatus::SAT && solver_status != SolverStatus::UNSAT)
    {    
        if (opts.up || opts.qcdcl) /* if UP or CDCL enabled */
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
                    // std::cout << "unit clause: " << unit_clauseID << " unit literal: " << unit_literal << "\n";
                    unit_clauses.pop();
                    assign(variable, 1);
                    implied_e_variables[level].push(varID);
                    Variables[varID].implication_trail_index = implied_e_variables[level].size();
                    // print_Clauses();
                    // print_Cubes();
                    // std::cout << "prefix:\n";
                    // print_Prefix();
                    // print_Blocks();
                    // print_Variables();
                }
                else
                {   
                    // std::cout << "implying...\n";
                    // std::cout << "unit clause: " << unit_clauseID << " unit literal: " << unit_literal << "\n";
                    unit_clauses.pop();
                    assign(variable, 0);
                    implied_e_variables[level].push(varID);
                    Variables[varID].implication_trail_index = implied_e_variables[level].size();
                    // print_Clauses();
                    // print_Cubes();
                    // std::cout << "prefix:\n";
                    // print_Prefix();
                    // print_Blocks();
                }
            }
        }

        if (opts.cube_learning /* && level != PRESEARCH */) /* if Cube Learning flag is on imply unit cubes */
        {
            while(!unit_cubes.empty())
            {
                int unit_cubeID           = unit_cubes.top().first;
                int unit_literal_position = Cubes[unit_cubeID].unit_literal_position;
                int unit_literal          = Cubes[unit_cubeID].literals[unit_literal_position];
                int varID                 = std::abs(unit_literal) - 1;
                int variable              = std::abs(unit_literal);

                if (Variables[varID].antecedent_cube != UNDEFINED) /* literal has already been implied */
                {
                    unit_cubes.pop();
                    continue;
                }

                Variables[varID].antecedent_cube        = unit_cubeID;
                Variables[varID].pos_in_antecedent_cube = unit_literal_position;

                if (unit_literal > 0)
                {
                    // std::cout << "unit cube: " << unit_cubeID << " unit literal: " << unit_literal << '\n';
                    unit_cubes.pop();
                    assign(variable, 0);
                    implied_a_variables[level].push(varID);
                    Variables[varID].implication_trail_index = implied_a_variables[level].size();
                    // print_Clauses();
                    // print_Cubes();
                    // std::cout << "prefix:\n";
                    // print_Prefix();
                }
                else
                {
                    // std::cout << "unit cube: " << unit_cubeID << " unit literal: " << unit_literal << '\n';
                    unit_cubes.pop();
                    assign(variable, 1);
                    implied_a_variables[level].push(varID);
                    Variables[varID].implication_trail_index = implied_a_variables[level].size();
                    // print_Clauses();
                    // print_Cubes();
                    // std::cout << "prefix:\n";
                    // print_Prefix();
                }
            }
        }

        if (unit_clauses.empty()) /* no new unit clauses */
        {   
            if (solver_status != SolverStatus::SAT && solver_status != SolverStatus::UNSAT)
            {   
                std::cout << "Solver status after deducing at level " << level << ": " << SolverStatus::to_string(solver_status) << '\n';
                return SolverStatus::SEARCH;
            }
            else
            {
                break;
            }
        }
    }

    if (solver_status == SolverStatus::SAT)
    {   
        std::cout << "SAT after deducing at level " << level << '\n';
        return SolverStatus::SAT;
    }
    else
    {   
        std::cout << "UNSAT after deducing at level " << level << '\n';
        return SolverStatus::UNSAT;
    }
}


void ThQBF::decide_next_branch (int& blevel, int& varID)
{
    blevel = PREFIX.begin()->first;
    varID  = *PREFIX[blevel].begin();
}

/* ================================ Conflict-driven Learning ================================ */
std::pair<int, int> ThQBF::analyse_conflict ()
{   
    int back_dl;
    std::pair<int, int> p;

    if (level == SolverStatus::PRESEARCH)
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
    
    if (solver_status == SolverStatus::UNSAT_EXCEPTION)
    {   
        p.first  = SolverStatus::ROOT;
        p.second = SolverStatus::ROOT;
        return p;
    }
    std::cout << "learned clause:\n";
    print_hashmap(cl);
    
    p = clause_asserting_level(cl);
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


int ThQBF::choose_e_literal (const std::unordered_map<int, int>& cc)
{
    int most_recently_implied = UNDEFINED;
    int best_trail_index      = UNDEFINED;
    int top_level             = UNDEFINED;

    // find highest decision level among implied existential variables
    for (const auto& [literal, _] : cc)
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
    for (const auto& [literal, _] : cc)
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
    
    bool polarity        = (-most_recently_implied > 0);
    int position_in_ante = Variables[var].get_position_in_clause(antecedent, polarity);

    assert(Clauses[antecedent].literals[position_in_ante] == -most_recently_implied && "neg(literal) does not appear in antecedent clause!");
    
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
            assert(Variables[varID].is_existential() && "Tautological existential pair: invalid reason or wrong pivot.");
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
        solver_status = SolverStatus::UNSAT_EXCEPTION;
        return true;
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
    for (const auto& [literal, _] : learned_clause)
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
    for (const auto& [literal, _] : learned_clause)
    {
        int varID    = std::abs(literal);
        int level    = Variables[varID].level; 

        if (Variables[varID].is_universal())
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
                // std::cout << "fdfdfdfdfdf\n" << i << '\n';
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


int ThQBF::cube_is_unit (int cubeID, int referenceVariable)
{   
    int referenceVarID  = referenceVariable - 1;
    int reference_level = Variables[referenceVarID].level;
    int unit_flag       = 1;

    for (int i = 0;  i < Cubes[cubeID].size; i++)
    {
        if (Cubes[cubeID].state[i] != qbf::LiteralStatus::AVAILABLE)
        {
            continue;
        }

        int literal = Cubes[cubeID].literals[i];
        int var     = std::abs(literal) - 1;

        if (var == referenceVarID)
        {
            continue;
        }

        int existential_level = Variables[var].blockID;

        if (existential_level < reference_level)
        {
            unit_flag = 0;
            break;
        }
    }

    if (!unit_flag)
    {   
        std::cout << "cube " << cubeID << " is not unit on current branch\n";
        return 0;
    }

    std::cout << "cube " << cubeID << " is unit on current branch\n";
    return unit_flag;
}


int ThQBF::choose_a_literal (const std::unordered_map<int, int>& sc)
{
    int most_recently_implied = UNDEFINED;
    int best_trail_index      = UNDEFINED;
    int top_level             = UNDEFINED;

    for (const auto& [literal, _] : sc)
    {
        int variable = std::abs(literal); 
        int varID    = variable - 1;

        if (Variables[varID].is_existential())
        {
            continue;
        }

        if (Variables[varID].antecedent_cube == UNDEFINED)
        {
            continue;
        }

        top_level = std::max(top_level, Variables[varID].level);
    }
    std::cout << "top level: " << top_level << "\n";

    assert(top_level>=0 && "No implied FORALL in cube (should be asserting or UNSAT-root).");

    // pick most recently implied universal literal at top level by trail index
    for (const auto& [literal, _] : sc)
    {
        int variable = std::abs(literal); 
        int varID    = variable - 1;

        if (Variables[varID].is_existential())
        {
            continue;
        }

        if (Variables[varID].antecedent_cube == UNDEFINED)
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

    assert(most_recently_implied != UNDEFINED && "No implied FORALL at top level (reason: bookkeeping bug?)");

    // assert that antecedent contains -most_recently_implied
    int var = std::abs(most_recently_implied) - 1;
    int antecedent = Variables[var].antecedent_cube;

    assert(Variables[var].appears_in_cube(antecedent) && "neg(literal) does not appear in antecedent!");

    bool polarity        = (-most_recently_implied > 0);
    int position_in_ante = Variables[var].get_position_in_cube(antecedent, polarity);

    assert(Cubes[antecedent].literals[position_in_ante] == -most_recently_implied && "neg(literal) does not appear in antecedent cube!");

    return most_recently_implied;
}


std::pair<int, int> ThQBF::analyse_SAT ()
{
    int back_dl;
    std::pair<int, int> p;

    if (level == SolverStatus::PRESEARCH)
    {   
        p.first  = SolverStatus::ROOT;
        p.second = SolverStatus::ROOT;
        return p;
    }

    std::unordered_map sat_cube = find_SAT_cube();

    if (sat_cube.empty())
    {
        sat_cube = construct_SAT_induced_cube();
    }

    while (!cube_stop_criteria_met(sat_cube))
    {
        int literal                       = choose_a_literal(sat_cube);
        int variable                      = std::abs(literal);
        int varID                         = variable - 1;
        int antecedent_cubeID             = Variables[varID].antecedent_cube; 
        std::unordered_map<int, int> ante = Cubes[antecedent_cubeID].map_representation();
        sat_cube                          = consensus(sat_cube, ante, variable); 
    }

    // std::cout << "learned cube:\n";
    // print_hashmap(sat_cube);

    if (solver_status == SolverStatus::SAT_EXCEPTION) /* all existential cube case */
    {   
        p.first  = SolverStatus::ROOT;
        p.second = SolverStatus::ROOT; 
        return p;
    }

    p = cube_asserting_level(sat_cube);
    add_cube_to_db(sat_cube, p.second);
    // std::cout << "cube asserting level: " << p.first << " asserting literal: " << p.second << '\n';

    return p;
}


std::unordered_map<int, int> ThQBF::find_SAT_cube ()
{
    if (Cubes.empty())
    {   
        std::cout << "First SAT path. No cubes in the db: returning a null cube.\n";
        return {};
    }
    else if (solver_status == SolverStatus::SAT && !remainingClauses) /* clause db is satisfied by a new partial assignment */
    {   
        std::cout << "Clauses are satisfied by a new partial assignment: returning a null cube\n";
        return {};
    }
    else /* if a satisfying cube is found before all clauses are satisfied */
    {   
        std::cout << "Concluded SAT by satisfying cube " << satisfying_cube << " at level " << level << '\n';
        std::cout << "Returning cube " << satisfying_cube << '\n';
        return Cubes[satisfying_cube].map_representation();
    }
}


std::unordered_map<int, int> ThQBF::consensus (const std::unordered_map<int, int>& c1, 
                                               const std::unordered_map<int, int>& c2, 
                                               int pivot_variable)
{
    std::unordered_map<int, int> new_cube;

    /* add all literals of c1 to new_cube  */
    for (const auto& [literal, ct] : c1)
    {
        if (std::abs(literal) == pivot_variable)
        {
            continue;
        }

        new_cube.insert({literal, ct});
    }

        /* add all literals of c2 to new_cube (omit opposite universals) */
    for (const auto& [literal, ct] : c2)
    {
        if (std::abs(literal) == pivot_variable)
        {
            continue;
        }

        int varID = std::abs(literal) - 1;

        /* literal already present in cube */
        if (new_cube.find(literal) != new_cube.end())
        {
            continue;
        }

        /* opposite polarity is found */
        if (new_cube.find(-literal) != new_cube.end())
        {   
            assert(Variables[varID].is_universal() && "Tautological universal pair: invalid reason or wrong pivot.");
            assert(Variables[varID].blockID > Variables[pivot_variable-1].blockID && "LD pair left of pivot: invalid reason or wrong pivot.");
            new_cube.insert({literal, ct});
            continue;
        }

        /* add literal to clause */
        new_cube.insert({literal, ct});
    }

    return new_cube;
}


std::unordered_map<int, int> ThQBF::construct_SAT_induced_cube ()
{
    // std::cout << "------------------------ Constructing SAT induced cube... ------------------------\n";
    std::unordered_map<int, int> sat_induced_cube = {};

    std::unordered_map<int, int>                     num_clauses_per_lit  = {}; // (1)   :   number of clauses each literals makes an appearance
    std::unordered_map<int, std::unordered_set<int>> clauses_per_lit      = {}; // (2)   :   clauses IDs each literal satisfies
    std::unordered_map<int, std::unordered_set<int>> EXISTS               = {}; // (3.1) :   clauses IDs existential assignments satisfy
    std::unordered_map<int, std::unordered_set<int>> FORALL               = {}; // (3.2) :   clauses IDs universal assignments satisfy                                  // (3.2) :   clauses IDs universal assignments satisfy
    std::unordered_map<int, std::unordered_set<int>> SAT_PER_CLAUSE       = {}; // (4)   :   satisfied literals per clause

    /* preprocessing... */
    for (const auto& [literal, _] : Path)
    {
        // std::cout << "literal: " << literal << " level: " << _ << '\n';
        int varID = std::abs(literal) - 1;
        if (literal > 0)
        {   
            // std::cout << "SAT clauses:\n";
            // print_hashmap(Variables[varID].positiveOccurrences);
            num_clauses_per_lit[literal] = Variables[varID].positiveOccurrences.size();
            for (const auto& [clauseID, _] : Variables[varID].positiveOccurrences)
            {
                clauses_per_lit[literal].insert(clauseID);
                SAT_PER_CLAUSE[clauseID].insert(literal);
            }
        }
        else
        {   
            // std::cout << "SAT clauses:\n";
            // print_hashmap(Variables[varID].negativeOccurrences);
            num_clauses_per_lit[literal] = Variables[varID].negativeOccurrences.size();
            for (const auto& [clauseID, _] : Variables[varID].negativeOccurrences)
            {
                clauses_per_lit[literal].insert(clauseID);
                SAT_PER_CLAUSE[clauseID].insert(literal);
            }
        }

        if (Variables[varID].is_existential())
        {
            EXISTS[literal] = clauses_per_lit[literal];
        }
        else
        {
            FORALL[literal] = clauses_per_lit[literal];
        }
    }

    /* ====================== greedy literal selection ====================== */
    
    int uncovered_clauses = SAT_PER_CLAUSE.size();
    std::vector<bool> covered(uncovered_clauses, false);

    auto adds_coverage = [&](int lit) -> int
    {
            int coverage_gain = 0;
            for (int clauseID : clauses_per_lit[lit])
            {
                if (!covered[clauseID])
                {   
                    coverage_gain++;
                }
            }
            return coverage_gain;
    };

    while (uncovered_clauses > 0)
    {
        int best_E    = 0;
        int best_A    = 0;
        int best_gain = -INT_MAX;

        /* first scan existentials that add coverage */
        for (const auto& [lit, cids] : EXISTS)
        {   
            int gain = adds_coverage(lit);
            // printf("literal: %d | coverage gain: %d\n", lit, gain);
            if (gain <= 0)
            {
                continue;
            }
            if (gain > best_gain)
            {
                best_gain = gain;
                best_E    = lit;
            }
        }
        
        int chosen = 0;
        if (best_E != 0)
        {
            chosen = best_E;
        }
        else /* if no existential helps scan universals */
        {   
            best_gain = -INT_MAX;
            for (const auto& [lit, cids] : FORALL)
            {
                int gain = adds_coverage(lit);
                // printf("literal: %d | coverage gain: %d\n", lit, gain);
                if (gain <= 0)
                {
                    continue;
                }
                if (gain > best_gain)
                {
                    best_gain = gain;
                    best_A    = lit;
                }
            }
            if (best_A != 0)
            {
                chosen = best_A;
            }
        }

        if (chosen == 0)
        {
            break;
        }

        // std::cout << "chosen: " << chosen << '\n';

        sat_induced_cube[chosen] = Path[chosen];
        
        for (int clauseID : clauses_per_lit[chosen])
        {
            if (!covered[clauseID])
            {
                covered[clauseID] = true;
                uncovered_clauses--;
            }
        }
    }

    return sat_induced_cube;
}


std::pair<int, int> ThQBF::cube_asserting_level (const std::unordered_map<int, int>& learned_cube)
{
     /* 
        asserting literal      -> universal literal at the maximum decision level
        cube asserting level   -> second highest level among the decision levels of the rest of the universals
    */
    int asserting_literal      =  UNDEFINED;
    int cube_asserting_level   =  0;
    int max_level              = -1;

    std::pair<int, int> p;

    /* find max_level */
    for (const auto& [literal, _] : learned_cube)
    {
        int varID    = std::abs(literal) - 1;
        int level    = Variables[varID].level; 

        if (Variables[varID].is_existential())
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

    assert (max_level >= 0 && "No universals at max_level (Logical error - check stop criteria).\n");

    /* find second highest level: asserting level */
    for (const auto& [literal, _] : learned_cube)
    {
        int varID    = std::abs(literal) - 1;
        int level    = Variables[varID].level; 

        if (Variables[varID].is_existential())
        {
            continue;
        }

        if (literal == asserting_literal)
        {
            continue;
        }

        cube_asserting_level = std::max(cube_asserting_level, level);
    }

    p = std::make_pair(cube_asserting_level, asserting_literal);
    // printf("cube asserting level: %d\nasserting literal: %d\n", cube_asserting_level, asserting_literal);
    return p;
}


bool ThQBF::cube_stop_criteria_met (const std::unordered_map<int, int>& resolvent)
{   

    /*                  1st criterion
        Among the universal varibles in cube, one and only one has the
        higheset decision level (which may not be the current decision 
        level). Call this variable V and its literal u*.
    */
    // print_hashmap(resolvent);
    int L_max = -1;
    int a_num = 0;
    std::unordered_map<int, std::pair<int, int>> decision_levels = {}; 
    /* { descision_level: (count, V) } */

    for (const auto& [literal, ct] : resolvent)
    {
        int varID = std::abs(literal) - 1;
        
        if (Variables[varID].is_existential())
        {
            continue;
        }
        a_num++;

        int decision_level = Variables[varID].level;
        if (decision_level > L_max)
        {
            L_max = decision_level;
        }

        if (decision_levels.find(decision_level) == decision_levels.end())
        {
            std::pair<int, int> p = std::make_pair(1, varID + 1);
            decision_levels.insert({decision_level, p});
        }
        else
        {
            decision_levels[decision_level].first++;
        }
    }

    if (!a_num) /* all existential cube (asserting at level 0)*/
    {   
        solver_status = SolverStatus::SAT_EXCEPTION; 
        return true;
    }

    if (L_max <= 0) /* all universals at the root level -> ROOT-UNSAT exception */
    {   
        return false;
    }
    // check if max level appears more than once
    if (decision_levels[L_max].first > 1)
    {
        return false;
    }

    /*                  2nd criterion
        The decision made at the highest level is a universal variable
    */
    int decision_variable_at_V = decision_variable_at[L_max];
    if (!Variables[decision_variable_at_V - 1].is_universal())
    {
        return false;
    }

    /*                  3rd criterion
        All existential literals that are quantified before V
        are already set to 1 before the decision level of V.
    */

    int V             = decision_levels[L_max].second;
    int V_quant_level = Variables[V - 1].blockID;

    for (const auto& [literal, ct] : resolvent)
    {
        int varID                 = std::abs(literal) - 1; 
        int variable_quant_level  = Variables[varID].blockID;

        if (Variables[varID].is_universal())
        {
            continue;
        }

        if (variable_quant_level >= V_quant_level)
        {
            continue;
        }

        /* only checking existentials with qb(e) < qb(V) */
        if (Variables[varID].assignment == UNDEFINED)
        {
            return false;
        }

        if (Variables[varID].level >= L_max)
        {
            return false;
        }

        if (literal > 0) // has to be 1
        {
            int assignment = Variables[varID].assignment;
            if (assignment == 0)
            {
                return false;
            }
        }
        else // has to be 0
        {
            int assignment = Variables[varID].assignment;
            if (assignment == 1)
            {
                return false;
            }
        }
    }

    return true;
}


void ThQBF::print_Clauses ()
{   
    std::cout << "Remaining clauses: " << remainingClauses << '\n';
    for (int i = 0; i < Clauses.size(); i++)
    {   
        if (Clauses[i].status == qbf::ClauseStatus::ACTIVE)
        {
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
        std::cout << " e_num.: "     << Cubes[i].e_num << " ";
        std::cout << " a_num.: "     << Cubes[i].a_num << " ";
        std::cout << " status: "     << qbf::ClauseStatus::to_string(Cubes[i].status) << '\n';
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


void ThQBF::pprint ()
{   
    std::cout << "Variables : " << numVars                << '\n';
    std::cout << "Clauses   : " << numClauses             << '\n';
    std::cout << "|S|       : " << numOfExistentialVars   << '\n';
    std::cout << "|P|       : " << numOfUniversalVars     << '\n';
    std::cout << "Status    : " << solver_status          << '\n';

    if (logger) /* print logged data if logging object has been initialised */
    {

    }
}

int ThQBF::solve_BJ ()
{

    solver_status = SolverStatus::PRESEARCH;
    level         = PRESEARCH;

    if (opts.up || opts.pl || opts.ur)
    {
        solver_status = infer();
    }

    if (solver_status == SolverStatus::SAT || solver_status == SolverStatus::UNSAT)
    {   
        std::cout << "ThQBF terminated during preprocessing.\n";
        std::cout << "Status: " << SolverStatus::to_string(solver_status) << '\n';
        return solver_status;
    }

    /* ================== search loop ================== */
    solver_status = SolverStatus::SEARCH;
    level         = 1;

    int blevel, varID;
    int value , top_level;
    std::pair<int, int> p = {};

    while (1)
    {   
        decide_next_branch(blevel, varID);
        solver_status = SolverStatus::SEARCH;

        if (Variables[varID].available_values == 0)
        {
            return solver_status;
        }

        /* choose a value for varID (first branch on 1) */
        if (Variables[varID].available_values == 2)
        {
            value = 1;
        }
        else if (Variables[varID].available_values == 1)
        {
            value = 0;
        }

        if (Variables[varID].available_values != 1)
        {
            if (Variables[varID].is_existential())
            {
                SStack.push({varID, level});
            }
            else
            {
                PStack.push({varID, level});
            }
        }

        std::cout << "***************************** LEVEL " << level << "  branching on: "<<  varID + 1 <<" value: "<< value <<" ***************************** \n";

        assign(varID+1, value);

        Search_Stack.push({varID, level});

        while (1)
        {   
            if (solver_status != SolverStatus::UNSAT && solver_status != SolverStatus::SAT)
            {   
                solver_status = deduce();
            }
            std::cout << "solver status: " << SolverStatus::to_string(solver_status) << '\n';

            if (solver_status != SolverStatus::UNSAT && solver_status != SolverStatus::SAT)
            {   
                level++;
                break;
            }
            else
            {
                if (solver_status == SolverStatus::SAT)
                {   
                    if (opts.cube_learning) /* if cube learning is enabled */
                    {   
                        p      = analyse_SAT();
                        blevel = p.first;

                        if (blevel == SolverStatus::ROOT)
                        {
                            return SolverStatus::SAT;
                        }

                        while (level > blevel)
                        {
                            restore_level(level);
                            Search_Stack.pop();
                            level--;
                        }
                        /* push unit cube to unit cubes stack */
                        int unit_pos = Cubes[Cubes.size()-1].unit_literal_position;
                        int refVar   = std::abs(Cubes[Cubes.size()-1].literals[unit_pos]);
                        unit_cubes.push({Cubes.size()-1, level});
                    }
                    else /* backtrack to the last universal */
                    {
                        if (PStack.empty())
                        {   
                            printf("SAT: PStack empty\n");
                            return solver_status;
                        }
                        varID     = PStack.top().first;
                        top_level = PStack.top().second;

                        PStack.pop();


                        while (level >= top_level)
                        {
                            restore_level(level);
                            if (level > top_level)
                            {
                                Variables[Search_Stack.top().first].available_values = 2;
                            }
                            level--;
                            // std::cout << "popping from SearchStack: " << Search_Stack.top().first + 1 << '\n';
                            Search_Stack.pop();

                            if (!SStack.empty())
                            {
                                if (SStack.top().second > top_level)
                                {   
                                    SStack.pop();
                                }
                            }
                        }
                        level++;
                        break;
                    }
                }
                else if (solver_status == SolverStatus::UNSAT)
                {   
                    if (opts.qcdcl) /* if conflict learning is enabled */
                    {   
                        
                        p      = analyse_conflict();
                        blevel = p.first;

                        if (blevel == SolverStatus::ROOT)
                        {   
                            return SolverStatus::UNSAT;
                        }
                        // printf("current level: %d\nasserting level: %d", level, blevel);
                        while (level > blevel)
                        {   
                            // std::cout << "fdfdfdfdfd\n";
                            restore_level(level);
                            Search_Stack.pop();
                            level--;
                        }

                        int unit_pos = Clauses[Clauses.size()-1].unit_literal_position;
                        int refVar   = std::abs(Clauses[Clauses.size()-1].literals[unit_pos]);
                        unit_clauses.push({Clauses.size()-1, level});
                    }
                    else /* backtrack to the last existential */
                    {
                        if (SStack.empty())
                        {   
                            printf("UNSAT: SStack empty\n");
                            return solver_status;
                        }
                        varID     = SStack.top().first;
                        top_level = SStack.top().second;
                        SStack.pop();

                        while (level >= top_level)
                        {
                            restore_level(level);
                            if (level > top_level)
                            {
                                Variables[Search_Stack.top().first].available_values = 2;
                            }
                            level--;
                            Search_Stack.pop();


                            if (!PStack.empty())
                            {
                                if (PStack.top().second > top_level)
                                {   
                                    PStack.pop();
                                }
                            }

                        }
                        level++;
                        break;
                    }
                }
            }
        }
    }
    return solver_status;
}


int ThQBF::solve_BT ()
{
    /* 
        preprocess: if any type of learning is enabled, only use UP and perhaps PL.
    */
    solver_status = SolverStatus::PRESEARCH;
    level         = PRESEARCH;


    solver_status = infer();

    if (solver_status == SolverStatus::SAT || solver_status == SolverStatus::UNSAT)
    {   
        std::cout << "ThQBF terminated during preprocessing.\n";
        std::cout << "Status: " << SolverStatus::to_string(solver_status) << '\n';
        return solver_status;
    }

    /* ===================== search loop ===================== */
    solver_status = SolverStatus::SEARCH;
    level         = 1;
    /* determine first var to assign */
    int blevel, varID;
    decide_next_branch(blevel, varID);
    logger->inc_node();

    int value;
    int top_level;

    while (1)
    {   
        if (Variables[varID].available_values != 0)
        {   
            /* assign value to variable */
            if (Variables[varID].available_values == 2)
            {
                value = 1;
            }
            else if (Variables[varID].available_values == 1)
            {
                value = 0;
            }

            if (Variables[varID].available_values != 1)
            {
                if (Variables[varID].is_existential())
                {
                    SStack.push({varID, level});
                    logger->inc_decision_E();
                }
                else
                {
                    PStack.push({varID, level});
                    logger->inc_decision_A();
                }
            }

            /* simplify and (optionally) deduce formula */
            std::cout << "***************************** LEVEL " << level << "  branching on: "<<  varID + 1 <<" value: "<< value <<" ***************************** \n";
            assign(varID+1, value);
            Search_Stack.push({varID, level});

            if (solver_status != SolverStatus::UNSAT && solver_status != SolverStatus::SAT)
            {  
                solver_status = infer();
                
            }

            std::cout << "solver status: " << SolverStatus::to_string(solver_status) << '\n';

            if (solver_status != SolverStatus::UNSAT && solver_status != SolverStatus::SAT)
            {   
                decide_next_branch(blevel, varID);
                level++;
            }
            else
            {
                if (solver_status == SolverStatus::SAT)
                {
                    if (PStack.empty())
                    {   
                        printf("SAT: PStack empty\n");
                        break;
                    }
                    varID     = PStack.top().first;
                    top_level = PStack.top().second;
                    PStack.pop(); 

                    /* restore formula up to top_level */
                    while (level >= top_level)
                    {
                        restore_level(level);
                        if (level > top_level)
                        {
                            Variables[Search_Stack.top().first].available_values = 2;
                        }
                        level--;
                        
                        Search_Stack.pop();

                        if (!SStack.empty())
                        {   
                            if (SStack.top().second > top_level)
                            {   
                                SStack.pop();
                            }
                        }
                    }
                    level++;
                    solver_status = SolverStatus::SEARCH;
                }
                else if (solver_status == SolverStatus::UNSAT)
                {
                    if (SStack.empty())
                    {   
                        printf("UNSAT: SStack empty\n");
                        break;
                    }
                    varID     = SStack.top().first;
                    top_level = SStack.top().second;
                    SStack.pop();

                    /* restore formula up to top_level */
                    while (level >= top_level)
                    {
                        restore_level(level);
                        if (level > top_level)
                        {
                            Variables[Search_Stack.top().first].available_values = 2;
                        }
                        level--;
                        
                        Search_Stack.pop();

                        if (!PStack.empty())
                        {
                            if (PStack.top().second > top_level)
                            {   
                                PStack.pop();
                            }
                        }

                    }
                    level++;
                    solver_status = SolverStatus::SEARCH;
                }
            }
        }
        else
        {
            break;
        }
    }
    return solver_status;
}


int ThQBF::test ()
{       
    // print_Clauses();
    int s = solve_BJ();
    std::cout << "return status: " << s << '\n';
    if (s == SolverStatus::SAT)
    {
        print_hashmap(Path);
    }   
    return s;


    // /* add some dummy cubes to test assignments and appearances updates */
    // std::unordered_map<int, int> cube_1 = { {-1, qbf::LiteralStatus::AVAILABLE}, 
    //                                         { 2, qbf::LiteralStatus::AVAILABLE}, 
    //                                         { 4, qbf::LiteralStatus::AVAILABLE}, 
    //                                         {-5, qbf::LiteralStatus::AVAILABLE}, 
    //                                         { 9, qbf::LiteralStatus::AVAILABLE} };

    // std::unordered_map<int, int> cube_2 = { { 1, qbf::LiteralStatus::AVAILABLE}, 
    //                                         {-5, qbf::LiteralStatus::AVAILABLE}, 
    //                                         { 7, qbf::LiteralStatus::AVAILABLE} };

    // std::unordered_map<int, int> cube_3 = { { 3, qbf::LiteralStatus::AVAILABLE}, 
    //                                         {-4, qbf::LiteralStatus::AVAILABLE}, 
    //                                         { 9, qbf::LiteralStatus::AVAILABLE}, 
    //                                         {-11,qbf::LiteralStatus::AVAILABLE} };

    // add_cube_to_db(cube_1, UNDEFINED);
    // add_cube_to_db(cube_2, UNDEFINED);
    // add_cube_to_db(cube_3, UNDEFINED);

    // print_Blocks();
    // print_Prefix();
    // print_Clauses();
    // print_Cubes();
    // print_Variables();

    // level         = PRESEARCH;
    // solver_status = SolverStatus::PRESEARCH;
    // UnitPropagation();
    // if (solver_status == SolverStatus::UNSAT || solver_status == SolverStatus::SAT)
    // {
    //     std::cout << SolverStatus::to_string(solver_status) << " at root...\n";
    //     print_hashmap(Path);
    //     return;
    // }

    // solver_status = SolverStatus::SEARCH;
    // level = 1;
    // assign(1, 1);
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(2, 1);
    // imply();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(3, 0);
    // imply();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(4, 0);
    // imply();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(5, 1);
    // imply();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();

    // level++;
    // assign(6, 0);
    // imply();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();

    // for (Variable v : Variables)
    // {
    //     std::cout << "=============== Variable "  << v.variable << " =============== \n";

    //     std::cout << "Positive occurences (clauses):\n";
    //     print_hashmap(v.positiveOccurrences);
    //     std::cout << "Negative occurences (clauses):\n";
    //     print_hashmap(v.negativeOccurrences);

    //     std::cout << "Positive occurences (cubes):\n";
    //     print_hashmap(v.positiveOccurrencesCubes);
    //     std::cout << "Negative occurences (cubes):\n";
    //     print_hashmap(v.negativeOccurrencesCubes);
    // }

    // /*  -------------------- test-SAT -------------------- */
    // level         = PRESEARCH;
    // solver_status = SolverStatus::PRESEARCH;
    // UnitPropagation();
    // if (solver_status == SolverStatus::UNSAT || solver_status == SolverStatus::SAT)
    // {
    //     std::cout << SolverStatus::to_string(solver_status) << " at root...\n";
    //     print_hashmap(Path);
    //     return;
    // }

    // /* assign variables */
    // solver_status = SolverStatus::SEARCH;
    // level = 1;
    // assign(4, 1);
    // deduce();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // printVector(Path, true);

    // level++;
    // assign(5, 1);
    // deduce();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // printVector(Path, true);


    // level++;
    // assign(7, 1);
    // deduce();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // print_hashmap(Path);

    // std::pair<int, int> p;
    // if (solver_status == SolverStatus::SAT)
    // {
    //     std::cout << "SAT at level " << level << '\n';
    //     p = analyse_SAT();
    // }

    // int back_dl = p.first;
    // while (level > back_dl)
    // {    
    //     restore_level(level);
    //     print_Clauses();
    //     print_Cubes(); 
    //     std::cout << "prefix\n";
    //     print_Prefix();
    //     print_Blocks();
    //     level--;
    // }

    // int unit_pos = Cubes[Cubes.size()-1].unit_literal_position;
    // int refVar   = std::abs(Cubes[Cubes.size()-1].literals[unit_pos]);
    // unit_cubes.push({Cubes.size()-1, level});
    // deduce();
    // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // print_Blocks();
    
    

    // for (Variable v : Variables)
    // {
    //     std::cout << "=============== Variable "  << v.variable << " =============== \n";

    //     std::cout << "Positive occurences (clauses):\n";
    //     print_hashmap(v.positiveOccurrences);
    //     std::cout << "Negative occurences (clauses):\n";
    //     print_hashmap(v.negativeOccurrences);

    //     std::cout << "Positive occurences (cubes):\n";
    //     print_hashmap(v.positiveOccurrencesCubes);
    //     std::cout << "Negative occurences (cubes):\n";
    //     print_hashmap(v.negativeOccurrencesCubes);
    // }




    // level++;
    // assign(3, 1);
    // imply();
    // // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // printVector(Path, true);

    // level++;
    // assign(4, 1);
    // imply();
    // // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // printVector(Path, true);

    // level++;
    // assign(5, 0);
    // imply();
    // // print_Clauses();
    // print_Cubes();
    // std::cout << "prefix\n";
    // print_Prefix();
    // printVector(Path, true);


    /* -------------------- test-UNSAT -------------------- */
    // level = PRESEARCH;
    // solver_status = SolverStatus::PRESEARCH;
    // UnitPropagation();
    // if (solver_status == SolverStatus::UNSAT)
    // {
    //     std::cout << "UNSAT at root...\n";
    //     return;
    // }

    // std::cout << "=============================== SEARCH ===============================\n";
    // solver_status = SolverStatus::SEARCH;
    // /* assign variables */
    // level = 1;
    // assign(3, 1);
    // print_Clauses();
    // imply();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // print_Blocks();
    // // print_Variables();
    // printVector(Path, true);
    

    // level++;
    // assign(5, 0);
    // print_Clauses();
    // imply();
    // std::cout << "prefix\n";
    // print_Prefix();
    // // print_Blocks();
    // // print_Variables();
    // printVector(Path, true);


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
