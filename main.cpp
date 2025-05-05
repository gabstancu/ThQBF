#include <iostream>
#include "include/QDimacsParser.hpp"
#include "include/Data.hpp"
#include "include/Solver.hpp"

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
    SolverData data = parser.to_solver_data();
    std::cout << "Done." << "\n";

    Solver solver(data);
    solver.solve();

    return 0;
}