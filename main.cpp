#include <iostream>
#include "include/QDimacsParser.hpp"
#include "include/Data.hpp"
#include "include/Solver.hpp"

int main (int argc, char* argv[])
{

    std::cout << "testing main in solver..." << '\n';
    if (argc != 2 && argc != 4) {
        std::cerr << "Error: Expected one or three parameters.\n";
        std::cerr << "Usage:\n";
        std::cerr << "  ./solve <file>                (for default mode)\n";
        std::cerr << "  ./solve <file1> <file2> <file3> (for mode 1 with Tseitin variables)\n";
        return 1;
    }

    SolverData data;
    int mode;
    if (argc == 2) /* one input file -> regular QBF instance */
    {
        std::string QDIMACS = argv[1];
        std::cout << "Reading QDIMACS " << QDIMACS << "\n";
        QDimacsParser parser(QDIMACS, qbf::GAME_OFF);
        parser.parse();
        data = parser.to_solver_data();
        mode = qbf::GAME_OFF;
        std::cout << "Done." << "\n";
    }
    else if (argc == 4)
    {
        std::string QDIMACS = argv[1];
        std::string E_TSEITIN_VARS = argv[2];
        std::string A_TSEITIN_VARS = argv[3];
        QDimacsParser parser(QDIMACS, E_TSEITIN_VARS, A_TSEITIN_VARS, qbf::GAME_ON);
        parser.parse();
        data = parser.to_solver_data();
        mode = qbf::GAME_ON;
        std::cout << "Done." << "\n";
    }


    Solver solver(data);
    solver.set_mode(mode);
    solver.solve();
    solver.print_Prefix();

    return 0;
}