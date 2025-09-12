#include <iostream>
#include <vector>
#include <string_view>
#include "utils/helper.hpp"
#include "ThQBF.hpp"
#include "internal.h"

static void print_usage (const char* prog) 
{
    std::cerr <<
    "Usage:\n  " << prog << " <file.qdimacs> [flags]\n\n"
    "Flags (enable/disable):\n"
    "  --debug\n"
    "  --qcdcl      | --no-qcdcl\n"
    "  --cube       | --no-cube\n"
    "  --ur         | --no-ur\n"
    "  --up         | --no-up\n"
    "  --pl         | --no-pl\n";
}


static bool parse_flags (int argc, char* argv[], Options& opt, std::string& qdimacs) 
{
    if (argc < 2) 
    { 
        print_usage(argv[0]); return false; 
    }
    qdimacs = argv[1];

    for (int i = 2; i < argc; ++i) 
    {
        std::string_view f(argv[i]);
        auto on  = [&](std::string_view s){ return f == s; };
        auto off = [&](std::string_view s){ return f == std::string("--no-") + std::string(s.substr(2)); };

        if (on("--debug"))            opt.debug         = true;
        else if (on("--qcdcl"))       opt.qcdcl         = true;
        else if (f == "--no-qcdcl")   opt.qcdcl         = false;
        else if (on("--cube"))        opt.cube_learning = true;
        else if (f == "--no-cube")    opt.cube_learning = false;
        else if (on("--ur"))          opt.ur            = true;
        else if (f == "--no-ur")      opt.ur            = false;
        else if (on("--up"))          opt.up            = true;
        else if (f == "--no-up")      opt.up            = false;
        else if (on("--pl"))          opt.pl            = true;
        else if (f == "--no-pl")      opt.pl            = false;
        else 
        {
            std::cerr << "Unknown flag: " << f << "\n";
            print_usage(argv[0]);
            return false;
        }
    }
    return true;
}


int main (int argc, char* argv[])
{   
    Options opts;
    std::string QDIMACS;

    if (!parse_flags(argc, argv, opts, QDIMACS))
    {
        return 1;
    }

    std::cout << "Reading QDIMACS " << QDIMACS << "\n";
    QDimacsParser parser(QDIMACS);
    parser.parse();
    std::cout << "Done." << "\n";


    ThQBF solver(parser, opts);
    solver.print_options();
    solver.test();
    // solver.print_status();

    return 0;
}