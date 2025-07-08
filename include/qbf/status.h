// #ifndef STATUS_HPP
// #define STATUS_HPP

// namespace qbf
// {
//     enum class SolverStatus { SAT, UNSAT, UNKNOWN };

//     enum class ClauseStatus { ACTIVE,  DELETED, SATISFIED };

//     enum class LiteralStatus { ASSIGNED, UNASSIGNED, TRUE, FALSE };

//     enum class VariableStatus { ASSIGNED, UNASSIGNED, TRUE, FALSE };

//     // variables related: active, assigned, eliminated
//     // literals: available, unavailable
//     // clauses: active, deleted, satisfied
//     // solver status: SAT, UNSAT, SIMPLIFIED
    
// } // namespace qbf


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

// #endif // STATUS_HPP