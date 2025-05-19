#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace qbf
{
    /* literals/ variables */
    constexpr int AVAILABLE = -2;
    constexpr int UNAVAILABLE = -3;
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
    constexpr int GAME_ON = 1;
    constexpr int GAME_OFF = 0;

    /* search */
    constexpr bool SAT = true;
    constexpr bool UNSAT = false;
}


#endif // CONSTANTS_HPP