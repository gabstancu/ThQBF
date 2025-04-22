#include <iostream>
#include "include/QDimacsParser.hpp"

int main (int argc, char* argv[])
{

    std::cout << "testing main in solver..." << '\n';
    if (argc != 2)
    {
        std::cerr << "Error: Expected exactly one parameter.\n";
        std::cerr << "Usage: ./solve <file name> (.qdimacs or .txt)\n";
        return 1;
    }

    std::string filename = argv[1];

    std::cout << "Reading QDIMACS " << filename << "\n";
    QDimacsParser parser(filename);
    parser.parse();
    std::cout << "Done." << "\n";

    // print_map_of_sets(parser.get_prefix());
    

    /*  
        TODO: learned clauses data base (possibly a struct) for QCDCL
        TODO: cubes data base for Cube Learning
    */

    return 0;
}