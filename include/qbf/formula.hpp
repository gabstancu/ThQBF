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

    inline const char* to_string(FormulaStatus s) 
    {
        switch (s) 
        {
            case FormulaStatus::SAT: return "SAT";
            case FormulaStatus::UNSAT: return "UNSAT";
            case FormulaStatus::SIMPLIFIED: return "SIMPLIFIED";
        }
        return "INVALID";
    }
}

struct formula
{
    /* data */
};


#endif // FORMULA_HPP