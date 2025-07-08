#ifndef QUANTIFIER_BLOCK_HPP
#define QUANTIFIER_BLOCK_HPP

namespace qbf
{
    enum class QuantifierType
    {
        EXISTENTIAL, UNIVERSAL
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
}
struct QuantifierBlock
{
    /* data */
};


#endif // QUANTIFIER_BLOCK_HPP