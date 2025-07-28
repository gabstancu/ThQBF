#include "utils/QDimacsParser.hpp"

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
    int         clauseID = 0;
    int         blockID  = 0;

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


void QDimacsParser::parse_header (const std::string line)
{   
    std::istringstream iss(line);
    std::string t;
    iss >> t >> t >> numVars >> numClauses;
    // std::cout << numVars << " " << numClauses << '\n';
}


void QDimacsParser::parse_quantifier_line (const std::string line, int blockID)
{   
    std::istringstream iss(line);
    char quantifier;
    iss >> quantifier;

    std::vector<int> block_variables;
    int              var, positionInBlock = 0;

    while (iss >> var && var != 0)
    {
        Variable variable;
        variable.status          = qbf::VariableStatus::ACTIVE;
        variable.varID           = var - 1;
        variable.variable        = var;
        variable.blockID         = blockID;
        variable.positionInBlock = positionInBlock;

        if (quantifier == 'e') 
        {
            S.push_back(var);
            numOfExistentialVars++;
            variable.quantifier = qbf::VariableType::EXISTENTIAL;
        }
        else
        {
            P.push_back(var);
            numOfUniversalVars++;
            variable.quantifier = qbf::VariableType::UNIVERSAL;
        }

        variables.push_back(variable);
        block_variables.push_back(var);
        positionInBlock++;
    }

    QuantifierBlock block;
    block.status              = qbf::QuantifierBlockStatus::AVAILABLE;
    block.variables           = block_variables;
    block.size                = block_variables.size();
    block.blockID             = blockID;
    block.available_variables = block_variables.size();

    if (quantifier == 'e')
        block.quantifier = qbf::QuantifierType::EXISTENTIAL;
    else
        block.quantifier = qbf::QuantifierType::UNIVERSAL;

    quantifier_prefix.push_back(block);
    numBlocks++;
    // std::cout << blockID << "   " << line << '\n';
}


void QDimacsParser::parse_clause_line (const std::string line, int clauseID)
{   
    std::vector<int> literals;
    int lit;
    std::istringstream iss(line);
    // std::cout << clauseID << "   " << line << '\n';

    while (iss >> lit && lit != 0)
    {
        literals.push_back(lit);
    }

    std::vector<Literal> lits;
    std::vector<int> state (literals.size(), qbf::LiteralStatus::AVAILABLE);
    std::sort(literals.begin(), literals.end());

    Clause clause;
    clause.clauseID              = clauseID;
    clause.size                  = literals.size();
    clause.status                = qbf::ClauseStatus::ACTIVE;
    clause.num_of_unassigned     = literals.size();
    clause.num_of_assigned       = 0;
    clause.literals              = literals;
    clause.state                 = state;
    clause.learned               = false;
    clause.hash                  = clause.compute_hash();
    clause.unit_literal_position = UNDEFINED;
    clause.level                 = UNDEFINED;
    
    int index = 0, var;
    // std::cout << "----------------- clauseID: " << clauseID << "\n";
    for (int literal : literals)
    {   
        Literal lit;
        lit.state    = qbf::LiteralStatus::AVAILABLE;
        lit.clauseID = clauseID;

        var = std::abs(literal) - 1;
        // std::cout << "variable " << var << " literal " << literal << "\n";
        if (literal > 0)
        {
            variables[var].addOccurence(clauseID, index, 1);
            lit.value = literal;
            // variables[var].numPosAppear++;
        }
        else
        {
            variables[var].addOccurence(clauseID, index, 0);
            lit.value = literal;
            // variables[var].numNegAppear++;
        }

        if (variables[var].is_existential())
            clause.e_num++;
        else
            clause.a_num++;

        index++;
        lits.push_back(lit);
    }
    clause.lits = lits;
    matrix.push_back(clause);
}