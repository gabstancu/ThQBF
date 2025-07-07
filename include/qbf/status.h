#ifndef STATUS_HPP
#define STATUS_HPP

namespace qbf
{
    enum class SolverStatus { SAT, UNSAT, UNKNOWN };

    enum class ClauseStatus { ACTIVE,  DELETED, SATISFIED };

    enum class LiteralStatus { UNASSIGNED, TRUE, FALSE };


    inline const char* to_string(SolverStatus status) {
    switch (status) 
    {
        case SolverStatus::SAT: return "SAT";
        case SolverStatus::UNSAT: return "UNSAT";
        case SolverStatus::UNKNOWN: return "UNKNOWN";
    }
        return "INVALID";
    }

    inline const char* to_string(ClauseStatus status) {
        switch (status) 
        {
            case ClauseStatus::ACTIVE: return "ACTIVE";
            case ClauseStatus::DELETED: return "DELETED";
            case ClauseStatus::SATISFIED: return "SATISFIED";
        }
        return "INVALID";
    }

    inline const char* to_string(LiteralStatus status) {
        switch (status) 
        {
            case LiteralStatus::UNASSIGNED: return "UNASSIGNED";
            case LiteralStatus::TRUE: return "TRUE";
            case LiteralStatus::FALSE: return "FALSE";
        }
        return "INVALID";
    }
} // namespace qbf


// namespace Status
// {
//     /* literals, variables, level */
//     constexpr int AVAILABLE = -2;
//     constexpr int UNAVAILABLE = -3;

//     /* assignments */
//     constexpr int UNASSIGNED = 0;
//     constexpr int ASSIGNED = 1;

//     /* quantifiers */
//     constexpr char EXISTENTIAL = 'e';
//     constexpr char UNIVERSAL = 'a';

//     /* other */
//     constexpr int UNIDENTIFIED = -100;
//     constexpr int UNDEFINED = -666;
//     constexpr int PRESEARCH = -1;

//     /* search */
//     constexpr int SAT = 1;
//     constexpr int UNSAT = 0;
//     constexpr int SIMPLIFIED = -1;
//     constexpr int ROOT = 0;
// }

#endif // STATUS_HPP