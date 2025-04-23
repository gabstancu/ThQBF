#ifndef DATA_HPP
#define DATA_HPP

#include <map>
#include <set>
#include <vector>
#include <unordered_map>

#include "Variable.hpp"
#include "Clause.hpp"
#include "Block.hpp"
#include "Constants.hpp"

struct SolverData
{
    int numVars, numClauses;

    std::unordered_map<int, Variable> Variables;
    std::unordered_map<int, Clause> Clauses;
    std::unordered_map<int, Block> Blocks;
    std::map<int, std::set<int>> prefix;

    /* state */
    int depth = 0; // seach level
    /* add necessary trails and stacks */
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