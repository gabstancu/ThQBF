#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "Data.hpp"

class Solver
{
    private:
        SolverData& data;


        /* variable related */
        void assign (int varID, int value, int searchLevel);
        void substitute_literal(int varID, int value, int searchLevel);
        void retract_assignment (int varID, int value, int searchLevel);

        /* clause related ??? */

        /* search related ("abstract" functions) */
        void analyze_conflict();
        void analyze_SAT();

    public:
        Solver(SolverData& d);

        bool solve();

};


#endif // SOLVER_HPP