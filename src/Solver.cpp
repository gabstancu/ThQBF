#include "Solver.hpp"

Solver::Solver (SolverData& data): data(data) { }


void Solver::assign(int varID, int value, int searchLevel)
{
    data.Variables.at(varID).set_level(searchLevel);
    data.Variables.at(varID).assign(value);
    data.Variables.at(varID).set_availability(qbf::UNAVAILABLE);

    /* searchLevel specific */
    std::vector<int> clauses_removed;
    std::unordered_map<int, std::vector<std::pair<int, int>>> appearances_removed;
    
    if (value) /* assign positive */
    {
        /* 
            clauses where varID appears positive are satisfied 
            and are marked as unavailable (or inactive)
        */
        for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_positiveOccurrences())
        {
            data.numClauses--;
            data.Clauses.at(clauseID).set_availability(false);
            remove_clause(clauseID, searchLevel);
            clauses_removed.push_back(clauseID);

            /* 
                remove clause from varID's appearances and add to trail. 
                remove it from the appearances of the rest of the vars as well 
            */
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
            
            
            /* remove clauseID from appearances and add to trail */
           
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
            
            
            /* remove clauseID from appearances and add to trail */
        }

        /* 
            clauses where varID appears negative are satisfied 
            and are marked as unavailable (or inactive)
        */
        for (const auto& [clauseID, positionInClause] : data.Variables.at(varID).get_negativeOccurrences())
        {
            data.numClauses--;
            data.Clauses.at(clauseID).set_availability(false);
            remove_clause(clauseID, searchLevel);
            clauses_removed.push_back(clauseID);

            /* 
                remove clause from varID's appearances and add to trail. 
                remove it from the appearances of the rest of the vars as well 
            */
        }
    }

    data.numVars--;
}


void Solver::remove_literal_from_clause(int varID, int value, int searchLevel)
{

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


void Solver::remove_variable(int varID, int clauseID)
{

}


bool Solver::solve()
{   
    std::cout << "in solve()" << '\n';
    return qbf::SAT;
}