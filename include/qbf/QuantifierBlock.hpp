#ifndef QUANTIFIER_BLOCK_HPP
#define QUANTIFIER_BLOCK_HPP

#include <iostream>

namespace qbf::QuantifierBlockStatus
{

    constexpr int AVAILABLE   = 1;
    constexpr int UNAVAILABLE = 0;

    inline const char* to_string(int s) 
    {
        switch (s) 
        {
            case QuantifierBlockStatus::AVAILABLE:   return "AVAILABLE";
            case QuantifierBlockStatus::UNAVAILABLE: return "UNAVAILABLE";
        }
        return "???";
    }
}

namespace qbf::QuantifierType
{

    constexpr char EXISTENTIAL = 'e';
    constexpr char UNIVERSAL   = 'a';

    inline const char* to_string(char qt) 
    {
        switch (qt) 
        {
            case QuantifierType::EXISTENTIAL: return "∃";
            case QuantifierType::UNIVERSAL:   return "∀";
        }
        return "???";
    }

}
struct QuantifierBlock
{
    char quantifier;
    int  status;
    int  size;
    int  blockID;
    int  available_variables;

    std::vector<int> variables;

    bool is_existential ()
    {
        return quantifier == qbf::QuantifierType::EXISTENTIAL;
    }

    bool is_universal ()
    {
        return quantifier == qbf::QuantifierType::UNIVERSAL;
    }

    void print ()
    {
        std::cout << "BlockID: " << blockID << ' ';
        std::cout << "Quantifier: " << quantifier << ' ';
        std::cout << "Size: " << size << ' ';
        std::cout << "Available variables: " << available_variables<< '\n';
    }
};


#endif // QUANTIFIER_BLOCK_HPP