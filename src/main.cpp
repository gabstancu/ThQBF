#include <iostream>
#include "reader.hpp"

int main (int argc, char* argv[])
{

    std::cout << "testing main in solver..." << '\n';
    if (argc != 2)
    {
        std::cerr << "Error: Expected exactly one parameter.\n";
        std::cerr << "Usage: ./solve <file name>\n";
        return 1;
    }
    std::string filename = argv[1];
    std::cout << "reading QDIMACS " << filename << '\n';

    return 0;
}