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

        void parse_header(const std::string line);
        void parse_clause_line(const std::string line, int clauseID);
        void parse_quantifier_line(const std::string line, int blockID);

    public:
        QDimacsParser(const std::string filename);

        void parse();

        std::unordered_map<int, Variable>& get_variables() { return Variables; }
        std::unordered_map<int, Clause>& get_clauses() { return Clauses; }
        std::unordered_map<int, Block>& get_blocks() { return Blocks; }
        std::map<int, std::set<int>>& get_prefix() { return prefix; }
};


#endif // QDIMACSPARSER_HPP