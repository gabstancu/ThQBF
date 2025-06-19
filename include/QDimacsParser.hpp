#ifndef QDIMACSPARSER_HPP
#define QDIMACSPARSER_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "Constants.hpp"
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
        int numTseitinClauses = 0;
        int numTseitinVariables = 0;
        int numOfExistentialVars = 0;
        int numOfUniversalVars = 0;

        int mode;

        // int first_tseitin_varID = 0;
        // int first_tseitin_clauseID = 0;
        // int tseitin_blockID = 0;

        // bool tseitin_clause_flag = false;
        std::set<int> P, S;

        std::unordered_map<int, Variable> variables;
        std::unordered_map<int, Clause> clauses;
        std::unordered_map<int, Block> blocks;
        std::map<int, std::set<int>> prefix;

        // std::unordered_map<int, std::tuple<int, int, int>> e_tseitin;
        // std::unordered_map<int, std::tuple<int, int, int>> a_tseitin;

        void parse_header(const std::string line);
        void parse_clause_line(const std::string line, int clauseID);
        void parse_quantifier_line(const std::string line, int blockID);
        // void separate_rules_from_tseitin();
        // void read_tseitin_variables(std::ifstream& filestream, char var_type);

    public:
        QDimacsParser(const std::string filename, int mode);

        void parse();
        SolverData to_solver_data() const; // pass parsed data to solver data

        std::unordered_map<int, Variable>& get_variables() { return variables; }
        std::unordered_map<int, Clause>& get_clauses() { return clauses; }
        std::unordered_map<int, Block>& get_blocks() { return blocks; }
        std::map<int, std::set<int>>& get_prefix() { return prefix; }
};


#endif // QDIMACSPARSER_HPP