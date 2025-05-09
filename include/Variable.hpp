#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <map>
#include "Constants.hpp"
// #include "Clause.hpp"
// #include "Block.hpp"
#include "utils.hpp"

class Variable
{
    private:
        int varID;
        char quantifier;
        bool tseitin = false;

        /* search information */
        int available;
        int assignment;
        int antecedent;
        int level;
        int num_of_values;

        /* block information */
        int positionInBlock;
        int blockID;

        int numNegAppear;
        // { clause : position_in_clause }
        std::unordered_map<int, int> negativeOccurrences;
        std::unordered_map<int, int> negativeTseitinOccurrences;

        int numPosAppear;
        std::unordered_map<int, int> positiveOccurrences;
        std::unordered_map<int, int> positiveTseitinOccurrences;

    public:

        Variable (int varID, char quantifier, int blockID, int positionInBlock);

        /* Read only access */
        int get_varID () const { return varID; }
        bool is_existential() const { return quantifier == qbf::EXISTENTIAL; }
        bool is_universal() const { return quantifier == qbf::UNIVERSAL; }
        int is_available() const { return available; } 
        int get_assignment () const { return assignment; }
        int get_decision_level () const { return level; }
        int get_block_position () const { return positionInBlock; }
        int get_blockID () const { return blockID; }
        int get_numNegAppear () const { return numNegAppear; }
        int get_numPosAppear () const { return numPosAppear; }
        int get_antecedent_clause () { return antecedent; }

        const std::unordered_map<int, int>& get_negativeOccurrences () const 
        { 
            return negativeOccurrences;
        }
        const std::unordered_map<int, int>& get_positiveOccurrences () const 
        { 
            return positiveOccurrences;
        }

        bool is_tseitin() const { return tseitin; }

        /* Mutators (during search) */
        void set_varID (int ID) { varID = ID; }
        void set_availability (int status) { available = status; }
        void assign (int value) { assignment = value; }
        void set_level(int lvl) { level = lvl; }
        void set_numNegAppear (int n) { numNegAppear = n; }
        void set_numPosAppear (int p) { numPosAppear = p; }
        std::unordered_map<int, int>& get_negativeOccurrences () { return negativeOccurrences;}
        std::unordered_map<int, int>& get_positiveOccurrences ()  { return positiveOccurrences;}
        void set_antecedent_clause (int antecedent_clause) { antecedent = antecedent_clause; }
        
        void increase_posNum() { numPosAppear++; }
        void decrease_posNum() { numPosAppear--; }
        void increase_negNum() { numNegAppear++; }
        void decrease_negNum() { numNegAppear--; }
        void increase_num_of_values() { num_of_values++; }
        void decrease_num_of_values() { num_of_values--; }

        void print (std::vector<int> vec) { printVector(vec); }
        void print (std::unordered_map<int, int> m) { print_hashmap(m); };
        
        void addOccurrence (int clauseID, int position, bool positive);
        bool appears_in_clause (int clauseID);
        int get_position_in_clause (int clauseID, bool positive);
        void set_tseitin(bool ts) { tseitin = ts; }
        void print ();
        
       

};

#endif // VARIABLE_HPP