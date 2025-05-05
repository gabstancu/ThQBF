#ifndef QDIMACSPARSER_HPP
#define QDIMACSPARSER_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "Variable.hpp"
#include "Block.hpp"
#include "Clause.hpp"
#include "Data.hpp"

class QDimacsParser
{
    private:
        std::ifstream file;

        int numVars = 0;
        int numClauses = 0;
        int numBlocks = 0;

        int first_tseitin_varID = 0;
        int first_tseitin_clauseID = 0;
        int tseitin_blockID = 0;

        bool tseitin_clause_flag = false;
        std::set<int> P, S;

        std::unordered_map<int, Variable> variables;
        std::unordered_map<int, Variable> tseitin_variables;
        std::unordered_map<int, Clause> clauses;
        std::unordered_map<int, Clause> tseitin_clauses;
        std::unordered_map<int, Block> blocks;
        std::unordered_map<int, Block> tseitin_block;

        std::map<int, std::set<int>> prefix;

        void parse_header(const std::string line);
        void parse_clause_line(const std::string line, int clauseID);
        void parse_quantifier_line(const std::string line, int blockID);
        void separate_rules_from_tseitin();

    public:
        QDimacsParser(const std::string filename);

        void parse();
        SolverData to_solver_data() const; // pass parsed data to solver data

        std::unordered_map<int, Variable>& get_variables() { return variables; }
        std::unordered_map<int, Clause>& get_clauses() { return clauses; }
        std::unordered_map<int, Block>& get_blocks() { return blocks; }
        std::map<int, std::set<int>>& get_prefix() { return prefix; }
};


#endif // QDIMACSPARSER_HPP