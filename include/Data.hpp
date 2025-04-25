#ifndef DATA_HPP
#define DATA_HPP

#include <map>
#include <set>
#include <vector>
#include <unordered_map>

#include "Variable.hpp"
#include "Clause.hpp"
#include "Block.hpp"

struct SolverData
{
    int numVars, numClauses, numBlocks;

    std::unordered_map<int, Variable> Variables;
    std::unordered_map<int, Clause> Clauses;
    std::unordered_map<int, Block> Blocks;
    std::map<int, std::set<int>> prefix;

    /* ---------------- state --------------- */
    int depth = 0; // seach level
    // define winning clause (last clause)
    // define winning block (last block)


    /* ------------- add necessary trails and stacks ------------- */

    /* 
        clauses that were removed or mutated.
        Whenever a clause is removed the variables'
        appearances 
    */
    std::unordered_map<int, std::vector<int>> Clauses_trail;
    
    std::unordered_map<int, std::unordered_map<int, int>> Variables_trail;
};


/* TODO: 'add' struct for QCDCL */

struct QCDCL
{

};


/* TODO: 'add' struct for Cube Learning */
struct CubeLearning
{

};



#endif // DATA_HPP