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
    constexpr int INACTIVE    =  3;

    inline const char* to_string(int s)
    {
        switch (s)
        {
            case ACTIVE:     return "ACTIVE";
            case DELETED:    return "DELETED";
            case SATISFYING: return "SATISFYING";
            case INACTIVE:   return "INACTIVE"; 
        }
        return "INVALID.";
    }
}

struct Cube
{
    int size;
    int status;
    int level;
    int cubeID;

    std::vector<int>     literals;
    std::vector<int>     state;
    // std::vector<Literal> lits;

    // bool learned = false;

    size_t hash = 0;

    int e_num = 0;
    int a_num = 0;

    int unit_literal_position = UNDEFINED;

    Cube (int cubeID, std::unordered_map<int, int> cube, int level): cubeID(cubeID), level(level)
    {   
        std::map<int, int> ordered_cube(cube.begin(), cube.end());
        for (const auto& [literal, state] : ordered_cube)
        {
            this->literals.push_back(literal);
            this->state.push_back(state);
        }

        this->size                  = this->literals.size();
        this->status                = qbf::CubeStatus::ACTIVE; 
        this->hash                  = this->compute_hash(); 
        this->unit_literal_position = UNDEFINED;
        // std::cout << "cubeID: " << this->cubeID << '\n';
    }

    std::size_t compute_hash() const
    {
        std::size_t h = 0;
        for (int lit : literals)
            h ^= std::hash<int>{}(lit) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }

    bool is_literal_available (int position)
    {
        return literals[position] == qbf::LiteralStatus::AVAILABLE;
    }

    bool is_active ()
    {
        return status == qbf::CubeStatus::ACTIVE;
    }

    std::unordered_map<int, int> map_representation()
    {
        std::unordered_map<int, int> map;
        for (int i = 0; i < size; ++i) 
            map.insert({literals[i], state[i]});
        return map;
    }

    void print ()
    {
        
    }
};






#endif // CUBE_HPP