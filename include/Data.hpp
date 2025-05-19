#ifndef DATA_HPP
#define DATA_HPP

#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Variable.hpp"
#include "Clause.hpp"
#include "Block.hpp"

struct SolverData
{
    int numVars, numClauses, numBlocks, numOfExistentialVars, numOfUniversalVars;

    int numTseitinClauses;
    int numTseitinVariables;

    int last_clause_idx;
    std::set<int> P, S; /* sets of universal and existential variables respectively */

    std::unordered_map<int, Variable> Variables;
    std::unordered_map<int, Clause> Clauses;
    std::unordered_set<std::size_t> ClauseHashes; // checking for duplicate clauses
    std::unordered_map<int, Block> Blocks;
    std::map<int, std::set<int>> prefix;

    // std::unordered_map<int, Variable> Tseitin_variables;
    // std::unordered_map<int, Clause> Tseitin_clauses;
    // std::unordered_map<int, Block> Tseitin_block;
    std::unordered_map<int, std::tuple<int, int, int>> e_tseitin;
    std::unordered_map<int, std::tuple<int, int, int>> a_tseitin;

    std::unordered_map<int, std::set<int>> Clauses_trail = {};
    std::unordered_map<int, std::set<int>> Variables_trail = {};

    // TODO: add assignments data structures

};


struct QCDCL
{

};

struct CubeLearning
{

};



#endif // DATA_HPP