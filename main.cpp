#include <iostream>
#include "utils/helper.hpp"
#include "ThQBF.hpp"

int main (int argc, char* argv[])
{

    // if (argc != 2) 
    // {
    //     std::cerr << "Error: Expected one parameter.\n";
    //     std::cerr << "Usage:\n";
    //     std::cerr << "          ./solve <file>\n";
    //     return 1;
    // }

    std::string QDIMACS = argv[1];
    // std::string QDIMACS = "test-SAT.qdimacs";
    std::cout << "Reading QDIMACS " << QDIMACS << "\n";
    QDimacsParser parser(QDIMACS);
    parser.parse();
    std::cout << "Done." << "\n";


    ThQBF solver(parser);
    solver.test();
    // solver.print();

    return 0;
}