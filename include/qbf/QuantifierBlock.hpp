#ifndef QUANTIFIER_BLOCK_HPP
#define QUANTIFIER_BLOCK_HPP

#include <iostream>
namespace qbf
{
    enum class QuantifierType
    {
        EXISTENTIAL, UNIVERSAL
    };

    enum class QuantifierBlockStatus
    {
        AVAILABLE, UNAVAILABLE
    };

    inline const char* to_string(QuantifierType qt) 
    {
        switch (qt) 
        {
            case QuantifierType::EXISTENTIAL: return "∃";
            case QuantifierType::UNIVERSAL: return "∀";
        }
        return "???";
    }

    inline const char* to_string(QuantifierBlockStatus s) 
    {
        switch (s) 
        {
            case QuantifierBlockStatus::AVAILABLE: return "AVAILABLE";
            case QuantifierBlockStatus::UNAVAILABLE: return "UNAVAILABLE";
        }
        return "???";
    }
}
struct QuantifierBlock
{
    qbf::QuantifierType quantifier;
    qbf::QuantifierBlockStatus available;
    int size;
    int blockID;
    int available_variables;

    std::vector<int> variables;

    bool is_existential ()
    {
        return quantifier == qbf::QuantifierType::EXISTENTIAL;
    }

    bool is_universal ()
    {
        return quantifier == qbf::QuantifierType::UNIVERSAL;
    }
};


#endif // QUANTIFIER_BLOCK_HPP