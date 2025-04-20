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
        bool available;
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

        Variable (int varID, char quantifier, int blockID, int positionInBlock)  
            : varID(varID), 
              quantifier(quantifier),
              available(qbf::AVAILABLE), 
              assignment(qbf::UNASSIGNED), 
              level(qbf::PRESEARCH), 
              positionInBlock(positionInBlock),
              blockID(blockID),
              numNegAppear(0),
              numPosAppear(0)
        {
            /* ... */
        }

        /* Read only access */
        int get_varID () const { return varID; }
        char get_variable_type() const { return quantifier; }
        bool is_available() const { return available; } 
        int get_assignment () const { return assignment; }
        int get_decision_level () const { return level; }
        int get_block_position () const { return positionInBlock; }
        int get_blockID () const { return blockID; }
        int get_numNegAppear () const { return numNegAppear; }
        int get_numPosAppear () const { return numPosAppear; }

        const std::unordered_map<int, int>& get_negativeOccurrences () const 
        { 
            return negativeOccurrences;
        }
        const std::unordered_map<int, int>& get_positiveOccurrences () const 
        { 
            return positiveOccurrences;
        }

        /* Mutators (during search) */
        void set_varID(int ID) { varID = ID; }
        void set_quantifier(char q) { quantifier = q; }
        void set_availability(bool status) { available = status; }
        void assign(int value) { assignment = value; }
        void set_level(int lvl) { level = lvl; }
        void set_block_position(int position) { positionInBlock = position; }
        void set_blockID(int block) { blockID = block; }
        void set_numNegAppear(int n) { numNegAppear = n; }
        void set_numPosAppear(int p) { numPosAppear = p; }
        std::unordered_map<int, int>& get_negativeOccurrences () { return negativeOccurrences;}
        std::unordered_map<int, int>& get_positiveOccurrences ()  { return positiveOccurrences;}

        /* add in .cpp later */
        void addOccurrence (int clauseID, int position, bool positive);
        bool appears_in_clause ();
        int get_position_in_clause ();
        // void assign();
        void unassign();
        void simplify_under_assignment();

};

#endif // VARIABLE_HPP