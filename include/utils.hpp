#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <set>

template <typename T>
void printVector(const std::vector<T>& vec, bool new_line) 
{
    std::cout << "[ ";
    for (const auto& item : vec) 
    {
        std::cout << item << " ";
    }
    std::cout << "]";
    if (new_line)
        std::cout << '\n';
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

template <typename T>
void printSet(const std::set<T>& s, const std::string& label = "") {
    if (!label.empty()) {
        std::cout << label << ": ";
    }
    std::cout << "{ ";
    for (const auto& elem : s) {
        std::cout << elem << " ";
    }
    std::cout << "}\n";
}

template<typename K, typename V>
void print_hashmap(const std::unordered_map<K, V>& m) {
    for (const auto& [key, value] : m) {
        std::cout << "  [" << key << "] = " << value << "\n";
    }
    std::cout << std::endl;
}

template<typename K, typename V>
void print_map_of_sets(const std::map<K, std::set<V>>& m) {
    for (const auto& [key, value_set] : m) {
        std::cout << "  [" << key << "] = { ";
        for (const auto& val : value_set) {
            std::cout << val << " ";
        }
        std::cout << "}\n";
    }
    std::cout << std::endl;
}

template<typename T1, typename T2>
void printStackOfPairsSafe(const std::stack<std::pair<T1, T2>>& original) {
    std::stack<std::pair<T1, T2>> copy = original;
    std::vector<std::pair<T1, T2>> elements;

    while (!copy.empty()) {
        elements.push_back(copy.top());
        copy.pop();
    }

    // std::cout << "Stack contents (bottom to top):\n";
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        std::cout << "(" << it->first << ", " << it->second << ")\n";
    }
}

template<typename K, typename T1, typename T2, typename T3>
void printTseitinMap(const std::unordered_map<K, std::tuple<T1, T2, T3>>& tseitinMap) {
    for (const auto& [key, values] : tseitinMap) {
        const auto& [v1, v2, v3] = values;
        std::cout << key << " : " << v1 << " " << v2 << " " << v3 << '\n';
    }
}



#endif // UTILS_HPP