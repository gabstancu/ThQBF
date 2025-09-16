#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <chrono>
#include <cstdint>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace tqbf {

    // ========================== What we record per run ==========================
    struct RunLog 
    {
        // Identifiers
        std::string instance_path;   // relative or absolute path to .qdimacs
        std::string setting;         // e.g. "CB", "CB+INF", "CDL", "CDL+CUBE"
        std::string build_id;        // e.g. "dev" or git hash
        int         threads = 1;

        int n_vars    = 0;
        int n_clauses = 0;
        int n_blocks  = 0;
        int L         = 0;           // if fixed
        int k         = 0;           // alternation depth if available
        std::string density_label;   // optional: dE2p250

        // Outcome & timing
        std::string status;          // "SAT"/"UNSAT"/"UNDETERMINED"/...
        double wall_ms = 0.0;        // wall time in milliseconds

        // Counters (increment these from the solver)
        uint64_t decisions_E      = 0;
        uint64_t decisions_A      = 0;
        uint64_t decisions_total  = 0;

        uint64_t conflicts        = 0;   // clause-side conflicts
        uint64_t backtracks       = 0;   // chronological
        uint64_t backjumps        = 0;   // non-chronological

        uint64_t unit_propagations = 0;  // total literals/unit pushes
        uint64_t ur_reductions     = 0;  // total literals removed by UR
        uint64_t pure_literal_sets = 0;  // number of PL assignments

        uint64_t learned_clauses   = 0;
        uint64_t learned_cubes     = 0;

        uint64_t leaves_sat        = 0;  // number of SAT leaves explored
        uint64_t leaves_unsat      = 0;  // number of UNSAT leaves explored
        uint64_t nodes_visited     = 0;  // optional aggregate search nodes

        // OPTIONAL: instance seed parsed from file name (for reproducibility)
        uint64_t instance_seed     = 0;

        // (Optional) proof bookkeeping if you later add it
        bool     proof_generated   = false;
        size_t   proof_size_bytes  = 0;
        bool     proof_verified    = false;
    };

    // ========================== CSV logger ==========================
    class Logger 
    {
        public:
            explicit Logger(std::string csv_path)
                : csv_path_(std::move(csv_path)) {}

            // Start a run (timestamped). No seed parameter.
            void start( const std::string& instance_path,
                        const std::string& setting,
                        const std::string& build_id = "dev",
                        int threads = 1) 
            {
                log = RunLog{};
                log.instance_path = instance_path;
                log.setting       = setting;
                log.build_id      = build_id;
                log.threads       = threads;
                t0_               = Clock::now();
                // OPTIONAL: instance seed — comment out to remove completely
                // log.instance_seed = extract_instance_seed_from_path(instance_path);
            }

            // Finish the run: set status string and compute wall time.
            void finish(const std::string& status) 
            {
                log.status = status;
                auto t1 = Clock::now();
                log.wall_ms = std::chrono::duration<double, std::milli>(t1 - t0_).count();
            }

            // Append to CSV; creates file with header if it does not exist.
            void append_row() 
            {
                std::lock_guard<std::mutex> lk(mu_);

                const bool need_header = !file_exists(csv_path_);
                std::ofstream out(csv_path_, std::ios::app);
                out.setf(std::ios::fixed);
                out << std::setprecision(3);

                if (need_header) 
                    write_csv_header(out);
                write_csv_row(out, log);
            }

            
            void inc_decision_E()              { ++log.decisions_E; ++log.decisions_total; }
            void inc_decision_A()              { ++log.decisions_A; ++log.decisions_total; }
            void inc_conflict()                { ++log.conflicts; }
            void inc_backtrack()               { ++log.backtracks; }
            void inc_backjump()                { ++log.backjumps; }
            void inc_unit_prop(uint64_t k = 1) { log.unit_propagations += k; }
            void inc_ur(uint64_t k = 1)        { log.ur_reductions += k; }
            void inc_pl(uint64_t k = 1)        { log.pure_literal_sets += k; }
            void inc_lclause(uint64_t k = 1)   { log.learned_clauses += k; }
            void inc_lcube(uint64_t k = 1)     { log.learned_cubes += k; }
            void inc_leaf_sat()        { ++log.leaves_sat; }
            void inc_leaf_unsat()      { ++log.leaves_unsat; }
            void inc_node()            { ++log.nodes_visited; }

            RunLog log;  // public so you can set parsed fields (n, m, k, L, ...)

            // Utility: extract seed from "..._seed####.qdimacs" → #### (OPTIONAL)
            static uint64_t extract_instance_seed_from_path(const std::string& path) 
            {
                auto pos = path.rfind("seed");
                if (pos == std::string::npos) 
                    return 0;
                pos += 4;
                uint64_t v = 0; 
                bool any = false;
                while (pos < path.size() && std::isdigit(static_cast<unsigned char>(path[pos]))) 
                {
                    any = true; v = v * 10 + (path[pos] - '0'); ++pos;
                }
                return any ? v : 0;
            }

        private:
            using Clock = std::chrono::steady_clock;

            std::string csv_path_;
            std::mutex  mu_;
            Clock::time_point t0_{};

            static bool file_exists(const std::string& p) 
            {
                std::ifstream in(p);
                return in.good();
            }

            static void write_csv_header(std::ofstream& out) 
            {
                out << "instance_path,setting,build_id,threads,"
                    "n_vars,n_clauses,n_blocks,L,k,density_label,"
                    "status,wall_ms,"
                    "decisions_E,decisions_A,decisions_total,"
                    "conflicts,backtracks,backjumps,"
                    "unit_propagations,ur_reductions,pure_literal_sets,"
                    "learned_clauses,learned_cubes,"
                    "leaves_sat,leaves_unsat,nodes_visited,"
                    "proof_generated,proof_size_bytes,proof_verified,"
                    "instance_seed\n"; // ← remove this column if you don’t want it
            }

            static void write_csv_row(std::ofstream& out, const RunLog& r) 
            {
                out << escape(r.instance_path) << ','
                    << escape(r.setting)       << ','
                    << escape(r.build_id)      << ','
                    << r.threads               << ','
                    << r.n_vars    << ','
                    << r.n_clauses << ','
                    << r.n_blocks  << ','
                    << r.L         << ','
                    << r.k         << ','
                    << escape(r.density_label) << ','
                    << r.status    << ','
                    << r.wall_ms   << ','
                    << r.decisions_E     << ','
                    << r.decisions_A     << ','
                    << r.decisions_total << ','
                    << r.conflicts       << ','
                    << r.backtracks      << ','
                    << r.backjumps       << ','
                    << r.unit_propagations << ','
                    << r.ur_reductions     << ','
                    << r.pure_literal_sets << ','
                    << r.learned_clauses   << ','
                    << r.learned_cubes     << ','
                    << r.leaves_sat        << ','
                    << r.leaves_unsat      << ','
                    << r.nodes_visited     << ','
                    << (r.proof_generated ? 1 : 0) << ','
                    << r.proof_size_bytes          << ','
                    << (r.proof_verified ? 1 : 0)  << ','
                    << r.instance_seed              // ← delete this if you don’t want it
                    << "\n";
            }

            // naive CSV escaping (wraps in quotes if comma present)
            static std::string escape(const std::string& s) 
            {
                if (s.find(',') == std::string::npos) 
                    return s;
                std::ostringstream oss;
                oss << '"' ;
                for (char c : s) 
                {
                    if (c == '"') oss << "\"\""; else oss << c;
                }
                oss << '"';
                return oss.str();
            }
    };

} // namespace tqbf

#endif // LOGGER_HPP