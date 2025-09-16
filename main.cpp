#include <iostream>
#include <vector>
#include <string_view>
#include <filesystem>  
#include "utils/helper.hpp"
#include "utils/Logger.hpp"
#include "ThQBF.hpp"
#include "internal.h"

static void print_usage (const char* prog)
{
    std::cerr <<
                "Usage:\n"
                "  " << prog << " <file.qdimacs> [flags]\n\n"
                "Flags (enable/disable):\n"
                "  --debug\n"
                "  --qcdcl      | --no-qcdcl\n"
                "  --cube       | --no-cube\n"
                "  --ur         | --no-ur\n"
                "  --up         | --no-up\n"
                "  --pl         | --no-pl\n"
                "\nLogging (optional):\n"
                "  --log                      (enable logging)\n"
                "  --log-dir=<dir>            (default: logs)\n"
                "  --log-file=<name.csv>      (default: runs.csv)\n";
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

        if (on("--debug"))               { opt.debug         = true; continue; }
        if (on("--qcdcl"))               { opt.qcdcl         = true; continue; }
        if (f == "--no-qcdcl")           { opt.qcdcl         = false; continue; }
        if (on("--cube"))                { opt.cube_learning = true; continue; }
        if (f == "--no-cube")            { opt.cube_learning = false; continue; }
        if (on("--ur"))                  { opt.ur            = true; continue; }
        if (f == "--no-ur")              { opt.ur            = false; continue; }
        if (on("--up"))                  { opt.up            = true; continue; }
        if (f == "--no-up")              { opt.up            = false; continue; }
        if (on("--pl"))                  { opt.pl            = true; continue; }
        if (f == "--no-pl")              { opt.pl            = false; continue; }

        // logging flags
        if (on("--log"))                 { opt.log           = true; continue; }

        // prefixes: --log-dir=..., --log-file=...
        // (use std::string to parse easily)
        std::string fs = std::string(f);
        if (fs.rfind("--log-dir=", 0) == 0)  
        { 
            opt.log_dir  = fs.substr(10); 
            continue; 
        }
        if (fs.rfind("--log-file=", 0) == 0) 
        { 
            opt.log_file = fs.substr(11); 
            continue; 
        }

        std::cerr << "Unknown flag: " << f << "\n";
        print_usage(argv[0]);
        return false;
    }
    return true;
}


int main (int argc, char* argv[])
{
    Options      opts;
    std::string  QDIMACS;

    if (!parse_flags(argc, argv, opts, QDIMACS)) 
        return 64; // EX_USAGE

    // Prepare logger only if requested
    std::unique_ptr<tqbf::Logger> logger;
    if (opts.log)
    {
        try 
        {
            std::filesystem::create_directories(opts.log_dir);
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Failed to create log directory '" << opts.log_dir
                      << "': " << e.what() << "\n";
            return 73; // EX_CANTCREAT
        }

        const std::filesystem::path log_path = std::filesystem::path(opts.log_dir) / opts.log_file;
        logger = std::make_unique<tqbf::Logger>(log_path.string());
        std::cout << "[log] writing to " << log_path << "\n";
    }

    std::cout << "Reading QDIMACS " << QDIMACS << "\n";
    QDimacsParser parser(QDIMACS);
    parser.parse();
    std::cout << "Done parsing " << QDIMACS << '\n';

    // Pass logger pointer (nullptr when logging is disabled)
    ThQBF solver(parser, opts, logger.get());
    solver.print_options();

    int status = solver.test();
    // or:
    // int status = opts.qcdcl ? solver.solve_BJ() : solver.solve_BT();

    if (logger) 
    {
        logger->finish(SolverStatus::to_string(status));
        logger->append_row(); 
    }

    // Optional pretty print if you have it
    solver.pprint();

    return (status == SolverStatus::SAT) ? 10 : (status == SolverStatus::UNSAT) ? 20 : 30;
}
