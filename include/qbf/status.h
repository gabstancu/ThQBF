// #ifndef STATUS_HPP
// #define STATUS_HPP

// #include <cstdint>

// namespace qbf
// {

//     enum class VariableStatus
//     {
//         ACTIVE, ASSIGNED, ELIMINATED, IMPLIED
//         /*
//             ACTIVE: still available
//             ASSIGNED: has been assigned a value due to a decision
//             ELIMINATED: eliminated during preprocessing
//             IMPLIED: has been assigned due to implication
//         */
//     };


//     enum class SolverStatus
//     {
//         SAT, UNSAT, PRESEARCH, SEARCH
//         /* 
//             SAT: empty matrix
//             UNSAT: empty clause detected
//             PRESEARCH: preprocessing
//             SEARCH: during search
//         */
//     };


//     enum class QuantifierType
//     {
//         EXISTENTIAL, UNIVERSAL
//     };


//     enum class LiteralState: int32_t
//     {
//         AVAILABLE = -2
//     };


//     enum class FormulaStatus 
//     {
//         SAT, UNSAT, SIMPLIFIED
//         /*
//             SAT: empty
//             UNSAT: empty clause
//             SIMPLIFIED: simplified 
//         */
//     };


//     enum class ClauseStatus
//     {
//         ACTIVE, DELETED, SATISFIED, EMPTY
//         /*
//             ACTIVE: still in
//             DELETED: removed/satisfied during preprocessing
//             SATISFIED: removed/satisfied during search
//             EMPTY: no literals or only universals (set solver state to unsat)
//         */
//     };


//     inline const char* to_string(SolverStatus s) 
//     {
//         switch (s) 
//         {
//             case SolverStatus::SAT: return "SAT";
//             case SolverStatus::UNSAT: return "UNSAT";
//             case SolverStatus::PRESEARCH: return "PRESEARCH";
//             case SolverStatus::SEARCH: return "SEARCH";
//         }
//         return "INVALID";
//     }


//     inline const char* to_string(QuantifierType qt) 
//     {
//         switch (qt) 
//         {
//             case QuantifierType::EXISTENTIAL: return "∃";
//             case QuantifierType::UNIVERSAL: return "∀";
//         }
//         return "???";
//     }


//     inline const char* to_string(FormulaStatus s) 
//     {
//         switch (s) 
//         {
//             case FormulaStatus::SAT: return "SAT";
//             case FormulaStatus::UNSAT: return "UNSAT";
//             case FormulaStatus::SIMPLIFIED: return "SIMPLIFIED";
//         }
//         return "INVALID";
//     }

    
//     inline const char* to_string(VariableStatus s) 
//     {
//         switch (s) 
//         {
//             case VariableStatus::ACTIVE: return "ACTIVE";
//             case VariableStatus::ASSIGNED: return "ASSIGNED";
//             case VariableStatus::ELIMINATED: return "ELIMINATED";
//             case VariableStatus::IMPLIED: return "IMPLIED";
//         }
//         return "INVALID";
//     }


//     inline const char* to_string(ClauseStatus s) {
//         switch (s) 
//         {
//             case ClauseStatus::ACTIVE: return "ACTIVE";
//             case ClauseStatus::DELETED: return "DELETED";
//             case ClauseStatus::SATISFIED: return "SATISFIED";
//             case ClauseStatus::EMPTY: return "EMPTY";
//         }
//         return "INVALID.";
//     }    
// }

// #endif // STATUS_HPP