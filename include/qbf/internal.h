#ifndef INTERNAL_HPP
#define INTERNAL_HPP

// RUNTIME FLAGS
constexpr const char* DEBUG         = "DEBUG";

// SOLVER FLAGS 
constexpr const char* QCDCL         = "QCDCL";
constexpr const char* CUBE_LEARNING = "CUBE_LEARNING";
constexpr const char* UR            = "UR";
constexpr const char* UP            = "UP";
constexpr const char* PL            = "PL";

// OTHER
constexpr int PRESEARCH = -1; // status in clause (removed during preprocessing)
constexpr int UNDEFINED = -666;

#endif // INTERNAL_HPP