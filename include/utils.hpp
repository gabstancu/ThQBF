#include <iostream>
#include <vector>

template <typename T>
void printVector(const std::vector<T>& vec) 
{
    std::cout << "[ ";
    for (const auto& item : vec) 
    {
        std::cout << item << " ";
    }
    std::cout << "]\n";
}

template <typename T>
void print2DVector(const std::vector<std::vector<T>>& matrix) 
{
    std::cout << "[\n";
    for (const auto& row : matrix) 
    {
        std::cout << "  [ ";
        for (const auto& item : row) 
        {
            std::cout << item << " ";
        }
        std::cout << "]\n";
    }
    std::cout << "]\n";
}