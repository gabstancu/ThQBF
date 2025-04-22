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
    std::cout << line;
}

void QDimacsParser::parse_clause_line(const std::string line, int clauseID)
{   
    int varID = 1;
    std::cout << clauseID << "   " << line;
}

void QDimacsParser::parse_quantifier_line(const std::string line, int blockID)
{
    std::cout << blockID << "   " << line;
}
