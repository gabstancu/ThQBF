#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <map>
#include "Constants.hpp"

class Variable
{
    private:
        int varID;
        char quantifier;

        /* search information */
        int in;
        int assignment;
        int level;

        /* block information */
        int positionInBlock;
        int blockID;

        int numNeg;
        // { clause : position_in_clause }
        std::unordered_map<int, int> negativeOccurences;

        int numPos;
        std::unordered_map<int, int> positiveOccurences;

        void addOccurrence (int clauseID, int position, bool positive);

    public:

        Variable (int varID, char quantifier);

        /* accessors */
        int get_varID () { return this->varID; }
        int get_assignment () { return this-> assignment; }
        int get_decision_level () { return this->level; }
        int get_block_position () { return this->positionInBlock; }
        int get_blockID () { return this->blockID; }
        int get_numNeg () { return this->numNeg; }
        int get_numPos () { return this->numPos; }

        std::unordered_map<int, int>& getPositiveOccurrences ()
        {
            return this->positiveOccurences;
        }

        std::unordered_map<int, int>& getNegativeOccurrences ()
        {
            return this->negativeOccurences;
        }

        /* mutators */




};

#endif // VARIABLE_HPP