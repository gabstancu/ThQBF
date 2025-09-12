#ifndef INTERNAL_HPP
#define INTERNAL_HPP

#include <string>

struct Options {
    bool debug         = false;
    // solver controls
    bool qcdcl         = true;   // master switch for conflict/cube driven reasoning
    bool cube_learning = false;  // disabled by default if you want plain QCDCL(CLA)
    bool ur            = true;   // universal reduction
    bool up            = true;   // unit propagation on clauses/cubes
    bool pl            = false;  // pure literal
};

// OTHER
constexpr int PRESEARCH = -1; // status in clause (removed during preprocessing)
constexpr int UNDEFINED = -666;


#endif // INTERNAL_HPP