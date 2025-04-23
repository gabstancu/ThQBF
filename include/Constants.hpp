#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace qbf
{
    /* literals/ variables */
    constexpr bool AVAILABLE = true;
    constexpr int PRESEARCH = -1;

    /* assignments */
    constexpr int UNASSIGNED = 0;
    constexpr int ASSIGNED = 1;

    /* quantifiers */
    constexpr char EXISTENTIAL = 'e';
    constexpr char UNIVERSAL = 'a';

    /* other */
    constexpr int UNIDENTIFIED = -100;
    constexpr int UNDEFINED = -666;
}


#endif // CONSTANTS_HPP