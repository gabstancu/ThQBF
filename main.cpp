#include <iostream>
#include "utils/helper.hpp"
#include "ThQBF.hpp"

int main (int argc, char* argv[])
{

    if (argc != 2) 
    {
        std::cerr << "Error: Expected one parameter.\n";
        std::cerr << "Usage:\n";
        std::cerr << "          ./solve <file>\n";
        return 1;
    }

    std::string QDIMACS = argv[1];
    std::cout << "Reading QDIMACS " << QDIMACS << "\n";
    QDimacsParser parser(QDIMACS);
    parser.parse();
    std::cout << "Done." << "\n";


    // for (Variable v : parser.variables)
    // {   
    //     std::cout << "---------------------------\n";
    //     v.print();
    // }

    // ThQBF solver(parser);
    // solver.solve();
    // solver.print();

    // std::cout << "=====================================\n";
    // for (Clause c : parser.matrix)
    // {
    //     c.print();
    //     std::cout << "\n=====================================\n";
    // }
    // std::cout << "\n=====================================\n";
    // for (Variable v : parser.variables)
    // {
    //     v.print();
    //     std::cout << "\n=====================================\n";
    // }

    return 0;
}