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

    int last_clause_idx;
    std::set<int> P, S; /* sets of universal and existential variables respectively */

    std::unordered_map<int, Variable> Variables;
    std::unordered_map<int, Clause> Clauses;
    std::unordered_set<std::size_t> ClauseHashes; // checking for duplicate clauses
    std::unordered_map<int, Block> Blocks;
    std::map<int, std::set<int>> prefix;
    
    /* search related */
    std::unordered_map<int, std::set<int>> Clauses_trail = {};
    std::unordered_map<int, std::set<int>> Variables_trail = {};
    std::map<int, int> A; /* final branch assignments */
    std::map<int, int> A_; /* in loop assignments */
    std::vector<std::map<int, int>> T;


};


struct QCDCL
{
    std::unordered_map<int, int> learned_clauses; // {clauseID, level}
};

struct Cubes
{
    
};



#endif // DATA_HPP