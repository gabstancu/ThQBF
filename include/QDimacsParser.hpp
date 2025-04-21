#ifndef QDIMACSPARSER_HPP
#define QDIMACSPARSER_HPP

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "Variable.hpp"
#include "Block.hpp"
#include "Clause.hpp"

class QDimacsParser
{
    private:
        std::ifstream file;

        int numVars;
        int numClauses;

        std::unordered_map<int, Variable> Variables;
        std::unordered_map<int, Clause> Clauses;
        std::unordered_map<int, Block> Blocks;

        std::map<int, std::set<int>> prefix;
        int currentBlockID = 0;

        void parse_header();
        void parse_clause_line();
        void parse_quantifier_line();

    public:
        QDimacsParser(const std::string filename)
        {

        }

        void parse();
};


#endif // QDIMACSPARSER_HPP