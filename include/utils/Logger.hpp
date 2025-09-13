#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#if defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
  #include <sys/resource.h>
  #include <sys/time.h>
#endif

namespace tqbf {

    // ---------- tiny time/mem helpers ----------
    inline uint64_t now_ms() 
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    inline uint64_t cpu_now_ms() 
    {
        return static_cast<uint64_t>((1000.0 * std::clock()) / CLOCKS_PER_SEC);
    }

    inline uint64_t peak_rss_mb() 
    {
        #if defined(__APPLE__) || defined(__MACH__)
            struct rusage ru{};
            if (getrusage(RUSAGE_SELF, &ru) == 0) 
                return static_cast<uint64_t>(ru.ru_maxrss) / (1024ULL * 1024ULL); // bytes → MB
            return 0;
        #elif defined(__linux__)
            struct rusage ru{};
            if (getrusage(RUSAGE_SELF, &ru) == 0) return static_cast<uint64_t>(ru.ru_maxrss) / 1024ULL; // kB → MB
            return 0;
        #else
            return 0; // unsupported platform
        #endif
    }
    
    inline std::string csv_escape(const std::string& s) 
    {
        if (s.find_first_of(",\"\n") == std::string::npos) 
            return s;
        std::string out; out.reserve(s.size() + 2);
        out.push_back('"');
        for (char c : s) { 
            if (c == '"') out.push_back('"'); out.push_back(c); 
        }
        out.push_back('"');
        return out;
    }

    // ---------- plain data container you will bump ----------
    struct RunLog 
    {
        // identity / config
        std::string instance_relpath;   // e.g. "Sigma/k05/n050/dE2p000/....qdimacs"
        std::string setting_id;         // "CB", "CB+INF", "CDL", "CDL+CUBE", etc.
        std::string solver_version;     // git hash or tag
        int         threads        = 1;
        uint64_t    solver_seed    = 0;
        int         timeout_s      = 0;

        // outcome
        std::string outcome;            // "TRUE" | "FALSE" | "TIMEOUT" | "UNDEF"
        int         exit_code      = 0;

        // timing & memory (filled by Logger::stop)
        uint64_t    time_ms        = 0; // wall
        uint64_t    cpu_time_ms    = 0; // CPU
        uint64_t    mem_peak_mb    = 0;

        // plain counters — you just ++ these
        uint64_t decisions         = 0;
        uint64_t propagations      = 0;
        uint64_t conflicts         = 0;
        uint64_t backtracks        = 0;
        uint64_t restarts          = 0;

        uint64_t learned_clauses   = 0;
        uint64_t learned_cubes     = 0;

        uint64_t sat_leaves        = 0;
        uint64_t unsat_leaves      = 0;

        // inference / simplifications
        uint64_t unit_props            = 0;
        uint64_t universal_reductions  = 0;
        uint64_t pure_lits_removed     = 0;
        uint64_t cube_props            = 0;

        // db maintenance
        uint64_t clause_db_peak    = 0;
        uint64_t db_reductions     = 0;

        // proofs (if you have them; else ignore)
        bool     proof_generated   = false;
        uint64_t proof_size_bytes  = 0;
        bool     proof_verified    = false;

        void clear() 
        { 
            *this = RunLog(); 
        }
    };

    // ---------- minimal logger ----------
    class Logger 
    {
        public:
            RunLog log;

            void start (const std::string& instance_relpath,
                    const std::string& setting_id,
                    const std::string& solver_version,
                    int threads,
                    uint64_t solver_seed,
                    int timeout_s)
            {
                log.clear();
                log.instance_relpath = instance_relpath;
                log.setting_id       = setting_id;
                log.solver_version   = solver_version;
                log.threads          = threads;
                log.solver_seed      = solver_seed;
                log.timeout_s        = timeout_s;

                t0_wall_ms = now_ms();
                t0_cpu_ms  = cpu_now_ms();
            }

            void stop(const std::string& outcome, int exit_code = 0) 
            {
                log.time_ms     = now_ms()   - t0_wall_ms;
                log.cpu_time_ms = cpu_now_ms()- t0_cpu_ms;
                log.mem_peak_mb = peak_rss_mb();
                log.outcome     = outcome;
                log.exit_code   = exit_code;
            }

            static void write_csv_header(std::ostream& os) {
                os << "instance_relpath,setting_id,solver_version,threads,solver_seed,timeout_s,"
                << "outcome,exit_code,time_ms,cpu_time_ms,mem_peak_mb,"
                << "decisions,propagations,conflicts,backtracks,restarts,"
                << "learned_clauses,learned_cubes,sat_leaves,unsat_leaves,"
                << "unit_props,universal_reductions,pure_lits_removed,cube_props,"
                << "clause_db_peak,db_reductions,proof_generated,proof_size_bytes,proof_verified"
                << "\n";
            }

            void write_csv_row(std::ostream& os) const {
                os << csv_escape(log.instance_relpath) << ','
                << csv_escape(log.setting_id)       << ','
                << csv_escape(log.solver_version)   << ','
                << log.threads                      << ','
                << log.solver_seed                  << ','
                << log.timeout_s                    << ','
                << csv_escape(log.outcome)          << ','
                << log.exit_code                    << ','
                << log.time_ms                      << ','
                << log.cpu_time_ms                  << ','
                << log.mem_peak_mb                  << ','
                << log.decisions                    << ','
                << log.propagations                 << ','
                << log.conflicts                    << ','
                << log.backtracks                   << ','
                << log.restarts                     << ','
                << log.learned_clauses              << ','
                << log.learned_cubes                << ','
                << log.sat_leaves                   << ','
                << log.unsat_leaves                 << ','
                << log.unit_props                   << ','
                << log.universal_reductions         << ','
                << log.pure_lits_removed            << ','
                << log.cube_props                   << ','
                << log.clause_db_peak               << ','
                << log.db_reductions                << ','
                << (log.proof_generated ? 1 : 0)    << ','
                << log.proof_size_bytes             << ','
                << (log.proof_verified ? 1 : 0)
                << "\n";
            }

            // append a row; create file+header if missing
            void append_row_to_file(const std::string& csv_path) const 
            {
                bool need_header = false;

                { 
                    std::ifstream test(csv_path); 
                    if (!test.good()) 
                        need_header = true; 
                }

                std::ofstream out(csv_path, std::ios::app);

                if (!out.good()) 
                { 
                    std::cerr << "[logger] cannot open " << csv_path << "\n"; 
                    return; 
                }

                if (need_header) 
                    write_csv_header(out);

                write_csv_row(out);
            }

        private:
            uint64_t t0_wall_ms = 0;
            uint64_t t0_cpu_ms  = 0;
    };

    // Optional: quick mapper to a compact setting name
    inline std::string setting_id_from_options(bool qcdcl, bool cube_learning, bool inference_enabled) 
    {
        if (!qcdcl && !cube_learning &&  inference_enabled) return "CB+INF";
        if (!qcdcl && !cube_learning && !inference_enabled) return "CB";
        if ( qcdcl && !cube_learning)                       return "CDL";
        if ( qcdcl &&  cube_learning)                       return "CDL+CUBE";
        return "UNKNOWN";
    }

} // namespace tqbf


#endif // LOGGER_HPP