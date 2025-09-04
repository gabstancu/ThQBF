#ifndef CUBE_HPP
#define CUBE_HPP

#include <iostream>
#include <unordered_map>
#include "Literal.hpp"
#include "utils/helper.hpp"

namespace qbf::CubeStatus
{
    constexpr int ACTIVE      =  1;  // participates in propagation
    constexpr int DELETED     = -1;  // removed by garbage collection
    constexpr int SATISFYING  =  2;  // currently satisfied by the assignment (all universals = 1, no existential = 0)

    inline const char* to_string(int s)
    {
        switch (s)
        {
            case ACTIVE:     return "ACTIVE";
            case DELETED:    return "DELETED";
            case SATISFYING: return "SATISFYING";
        }
        return "INVALID.";
    }
}



struct Cube
{
    int size;
    int status;
    int cubeID;

    std::vector<int>     literals;
    std::vector<int>     state;
    std::vector<Literal> lits;

    // bool learned = false;

    size_t hash = 0;

    int e_num = 0;
    int a_num = 0;

    int unit_literal_position = UNDEFINED;


    std::size_t compute_hash() const
    {
        std::size_t h = 0;
        for (int lit : literals)
            h ^= std::hash<int>{}(lit) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }

    bool is_empty ()
    {
        return size == 0;

    }

    std::unordered_map<int, int> map_representation()
    {
        std::unordered_map<int, int> map;
        for (int i = 0; i < size; ++i) 
            map.insert({literals[i], state[i]});
        return map;
    }
};






#endif // CUBE_HPP