#ifndef FORMULA_HPP
#define FORMULA_HPP

#include <vector>
#include "Variable.hpp"
#include "Clause.hpp"
#include "QuantifierBlock.hpp"
#include "utils/helper.hpp"

namespace qbf
{
    enum class FormulaStatus 
    {
        SAT, UNSAT, SIMPLIFIED
        /*
            SAT: empty
            UNSAT: empty clause
            SIMPLIFIED: simplified 
        */
    };

    inline const char* to_string(FormulaStatus s) 
    {
        switch (s) 
        {
            case FormulaStatus::SAT:        return "SAT";
            case FormulaStatus::UNSAT:      return "UNSAT";
            case FormulaStatus::SIMPLIFIED: return "SIMPLIFIED";
        }
        return "INVALID";
    }
}

struct Formula
{
    std::vector<QuantifierBlock>    QuantifierPrefix;
    std::vector<Clause>             Matrix;           
    std::vector<Variable>           Variables; 
    std::unordered_set<std::size_t> ClauseHashes;

    Formula (std::vector<Clause> clauses, 
             std::vector<Variable> variables, 
             std::vector<QuantifierBlock> prefix)
    {
        this->Matrix           = clauses;
        this->QuantifierPrefix = prefix;
        this->Variables        = variables;

        for (Clause c : Matrix)
        {
            ClauseHashes.insert(c.hash);
        }
    }
};


#endif // FORMULA_HPP