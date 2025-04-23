#include "QDimacsParser.hpp"

QDimacsParser::QDimacsParser(const std::string filename)
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
    int clauseID = 0;
    int blockID = 0;

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
        else{ /* clause line */
            parse_clause_line(line, clauseID++);
        }
        
    }
}


void QDimacsParser::parse_header(const std::string line)
{   
    std::istringstream iss(line);
    std::string t;
    iss >> t >> t >> numVars >> numClauses;

    // std::cout << numVars << " " << numClauses << '\n'; 
    // std::cout << line << '\n';
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
    }

    blocks.insert({blockID, Block(quantifier, blockID, vars)});
    // std::cout << blockID << "   " << line << '\n';
}


SolverData QDimacsParser::to_solver_data() const
{
    SolverData data;

    data.numVars = numVars;
    data.numClauses = numClauses;
    data.Variables = variables;
    data.Clauses = clauses;
    data.Blocks = blocks;
    data.prefix = prefix;

    return data;
}
