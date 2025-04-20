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
        int available;
        int assignment;
        int level;

        /* block information */
        int positionInBlock;
        int blockID;

        int numNegAppear;
        // { clause : position_in_clause }
        std::unordered_map<int, int> negativeOccurrences;

        int numPosAppear;
        std::unordered_map<int, int> positiveOccurrences;

    public:

        Variable (int varID, char quantifier);

        /* Read only access */
        int get_varID () const { return varID; }
        int get_assignment () const { return assignment; }
        int get_decision_level () const { return level; }
        int get_block_position () const { return positionInBlock; }
        int get_blockID () const { return blockID; }
        int get_numNegAppear () const { return numNegAppear; }
        int get_numPosAppear () const { return numPosAppear; }

        /* Mutators (during search) */
        std::unordered_map<int, int>& getPositiveOccurrences ()
        {
            return positiveOccurrences;
        }

        std::unordered_map<int, int>& getNegativeOccurrences ()
        {
            return negativeOccurrences;
        }


        /* add in .cpp later */
        void addOccurrence (int clauseID, int position, bool positive);




};

#endif // VARIABLE_HPP