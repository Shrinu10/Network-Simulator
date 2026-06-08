#include "Config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

// ═══════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}

// ═══════════════════════════════════════════════════════════════════
// Config loader
// ═══════════════════════════════════════════════════════════════════

SimConfig loadConfig(const std::string& filename) {
    SimConfig cfg;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[CONFIG] Could not open '" << filename
                  << "' -- using default values.\n";
        return cfg;
    }

    std::cout << "[CONFIG] Loading configuration from '" << filename << "'...\n";

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Parse "key = value"
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            std::cout << "[CONFIG] Warning: ignoring malformed line "
                      << lineNum << ": " << line << "\n";
            continue;
        }

        std::string key   = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        std::string keyUp = toUpper(key);

        try {
            if      (keyUp == "ROUTERS")           cfg.numRouters      = std::stoi(value);
            else if (keyUp == "TOPOLOGY")          cfg.topologyStr     = toUpper(value);
            else if (keyUp == "PACKETS")           cfg.numPackets      = std::stoi(value);
            else if (keyUp == "SOURCE")            cfg.sourceRouter    = std::stoi(value);
            else if (keyUp == "DESTINATION")       cfg.destRouter      = std::stoi(value);
            else if (keyUp == "SIMULATE_LATENCY")  cfg.simulateLatency = (toUpper(value) == "ON");
            else if (keyUp == "LATENCY_MIN_MS")    cfg.latencyMinMs    = std::stod(value);
            else if (keyUp == "LATENCY_MAX_MS")    cfg.latencyMaxMs    = std::stod(value);
            else if (keyUp == "LOG_LEVEL")         cfg.logLevelStr     = toUpper(value);
            else if (keyUp == "SEED")              cfg.seed            = static_cast<unsigned int>(std::stoul(value));
            else {
                std::cout << "[CONFIG] Warning: unknown key '"
                          << key << "' on line " << lineNum << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "[CONFIG] Warning: invalid value for '"
                      << key << "' on line " << lineNum
                      << ": " << e.what() << "\n";
        }
    }

    // ── Apply defaults for special values ────────────────────
    if (cfg.sourceRouter < 0)  cfg.sourceRouter = 0;
    if (cfg.destRouter < 0)    cfg.destRouter   = cfg.numRouters - 1;

    // ── Clamp destination to valid range ─────────────────────
    if (cfg.destRouter >= cfg.numRouters)
        cfg.destRouter = cfg.numRouters - 1;

    // ── Ensure latency range is valid ────────────────────────
    if (cfg.latencyMinMs > cfg.latencyMaxMs)
        std::swap(cfg.latencyMinMs, cfg.latencyMaxMs);

    return cfg;
}

// ═══════════════════════════════════════════════════════════════════
// Pretty-print config
// ═══════════════════════════════════════════════════════════════════

void printConfig(const SimConfig& cfg) {
    std::cout << "\n";
    std::cout << "+----------------------------------------------+\n";
    std::cout << "|         Simulation Configuration             |\n";
    std::cout << "+----------------------------------------------+\n";
    std::cout << "|  Routers:           " << cfg.numRouters << "\n";
    std::cout << "|  Topology:          " << cfg.topologyStr << "\n";
    std::cout << "|  Batch Packets:     " << cfg.numPackets << "\n";
    std::cout << "|  Source Router:     " << cfg.sourceRouter << "\n";
    std::cout << "|  Dest Router:       " << cfg.destRouter << "\n";
    std::cout << "|  Simulate Latency:  " << (cfg.simulateLatency ? "ON" : "OFF") << "\n";
    if (cfg.simulateLatency) {
        std::cout << "|  Latency Range:     "
                  << cfg.latencyMinMs << " ms - "
                  << cfg.latencyMaxMs << " ms\n";
    }
    std::cout << "|  Log Level:         " << cfg.logLevelStr << "\n";
    std::cout << "|  Seed:              "
              << (cfg.seed == 0 ? "random" : std::to_string(cfg.seed)) << "\n";
    std::cout << "+----------------------------------------------+\n\n";
}
