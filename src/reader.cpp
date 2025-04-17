#include "reader.hpp"
#include "utils.hpp"

void read_QDIMACS (const std::string filepath)
{
    std::ifstream file(filepath);
    if(!file.is_open())
    {
        std::cerr << "Could not open file!\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::cout << "Read line: " << line << "\n";
    }

    file.close();
}