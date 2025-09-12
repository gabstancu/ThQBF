#ifndef QDIMACSPARSER_HPP
#define QDIMACSPARSER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include "qbf/qbf.hpp"
class QDimacsParser
{
    private:
        std::ifstream file;

        void parse_header          (const std::string line);
        void parse_clause_line     (const std::string line, int clauseID);
        void parse_quantifier_line (const std::string line, int blockID);

    public:
        QDimacsParser(const std::string filename);

        int numVars              = 0;
        int numClauses           = 0;
        int numBlocks            = 0;
        int numOfExistentialVars = 0;
        int numOfUniversalVars   = 0;

        std::vector<Clause>          matrix;
        std::vector<QuantifierBlock> quantifier_prefix;
        std::vector<Variable>        variables;

        std::vector<int>             P;
        std::vector<int>             S;

        void parse();
};


#endif // QDIMACSPARSER_HPP