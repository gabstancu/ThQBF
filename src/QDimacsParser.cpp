#include "QDimacsParser.hpp"

QDimacsParser::QDimacsParser(const std::string filename, int mode) : mode(mode)
{
    file.open(filename);

    if(!file)
    {
        std::cerr << "Error: Could not open file " << filename  << '\n';
        exit(1);
    }
}


void QDimacsParser::parse()
{   
    std::string line;
    int clauseID = 1;
    int blockID = 1;

    while (std::getline(file, line))
    {   
        if (line[0] == 'c' || line.empty()) /* comment line or empty */
        {   
            continue;
        }
        if (line[0] == 'p') /* header line */
        {
            parse_header(line);
        }
        else if (line[0] == 'e' || line[0] == 'a') /* block line */
        {
            parse_quantifier_line(line, blockID++);
        }
        else /* clause line */
            parse_clause_line(line, clauseID++);
    }

}


void QDimacsParser::parse_header(const std::string line)
{   
    std::istringstream iss(line);
    std::string t;
    iss >> t >> t >> numVars >> numClauses;
    // std::cout << numVars << " " << numClauses << '\n';
}


void QDimacsParser::parse_quantifier_line(const std::string line, int blockID)
{   
    std::istringstream iss(line);
    char quantifier;
    iss >> quantifier;

    std::vector<int> vars;
    int var, positionInBlock = 0;
    while (iss >> var && var != 0)
    {
        vars.push_back(var);
        // std::cout << "Creating Variable(" << var << ", " << quantifier << ", " << blockID << ", " << positionInBlock << ")\n";
        variables.insert({var, Variable(var, quantifier, blockID, positionInBlock++)});
        prefix[blockID].insert(var);

        if (quantifier == 'e') 
        {
            S.insert(var);
            numOfExistentialVars++;
        }
        else
        {
            P.insert(var);
            numOfUniversalVars++;
        }
    }

    blocks.insert({blockID, Block(quantifier, blockID, vars)});
    numBlocks++;
    // std::cout << blockID << "   " << line << '\n';
}


void QDimacsParser::parse_clause_line(const std::string line, int clauseID)
{   
    std::vector<int> literals;
    int lit;
    std::istringstream iss(line);
    // std::cout << clauseID << "   " << line << '\n';

    while (iss >> lit && lit != 0)
    {
        literals.push_back(lit);
    }

    std::vector<int> state(literals.size(), qbf::AVAILABLE);

    clauses.insert({clauseID, Clause(literals, state, qbf::PRESEARCH, false)});
   
    int index = 0, var;
    for (int literal : clauses.at(clauseID).get_literals())
    {
        // std::cout << "literal: " << literal << " " << " variable: " << std::abs(literal) << " position: " << index << '\n';
        if (literal > 0)
        {
            // variables[literal].addOccurrence(clauseID, index, 1);
            variables.at(literal).addOccurrence(clauseID, index, 1);
        }
        else
        {   var = std::abs(literal);
            // variables[var].addOccurrence(clauseID, index, 0);
            variables.at(var).addOccurrence(clauseID, index, 0);
        }

        if (variables.at(std::abs(literal)).is_existential())
        {
            clauses.at(clauseID).increase_e_num();
        }
        else 
        {
            clauses.at(clauseID).increase_a_num();
        }

        index++;
    }
}


SolverData QDimacsParser::to_solver_data() const
{
    SolverData data;

    data.numVars = numVars;
    data.numClauses = numClauses;
    data.numBlocks = numBlocks;
    data.numOfExistentialVars = numOfExistentialVars;
    data.numOfUniversalVars = numOfUniversalVars;
    data.numTseitinClauses = numTseitinClauses;
    data.numTseitinVariables = numTseitinVariables;
    data.last_clause_idx = numClauses;
    data.S = S;
    data.P = P;
    data.Variables = variables;
    data.Clauses = clauses;
    data.Blocks = blocks;
    data.prefix = prefix;

    for (const auto& [clauseID, clause] : data.Clauses) 
    {
        std::size_t h = clause.compute_hash();
        data.ClauseHashes.insert(h);
    }


    return data;
}
