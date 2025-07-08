#ifndef FORMULA_HPP
#define FORMULA_HPP

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
}

struct formula
{
    /* data */
};


#endif // FORMULA_HPP