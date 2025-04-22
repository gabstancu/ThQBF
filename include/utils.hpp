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

template<typename K, typename V>
void print_hashmap(const std::unordered_map<K, V>& map) {
    for (const auto& [key, value] : map) {
        std::cout << "  [" << key << "] = " << value << "\n";
    }
    std::cout << std::endl;
}

template<typename K, typename V>
void print_map_of_sets(const std::map<K, std::set<V>>& map) {
    for (const auto& [key, value_set] : map) {
        std::cout << "  [" << key << "] = { ";
        for (const auto& val : value_set) {
            std::cout << val << " ";
        }
        std::cout << "}\n";
    }
    std::cout << std::endl;
}