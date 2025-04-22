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
    int clauseID;
    int blockID;

    while (std::getline(file, line))
    {
        std::cout << line;

        if (line[0] == 'c' || line.empty()) /* comment line or empty */
        {
            continue;
        }

        if (line[0] == 'p') /* header line */
        {

        }
        else if (line[0] == 'e' || line[0] == 'a') /* block line */
        {

        }
        else{ /* clause line */

        }
        
    }
}


void QDimacsParser::parse_header(const std::string line)
{

}

void QDimacsParser::parse_clause_line(const std::string line, int clauseID)
{
    
}

void QDimacsParser::parse_quantifier_line(const std::string line, int blockID)
{
    
}
