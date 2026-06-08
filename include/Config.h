#pragma once

#include <string>

/**
 * @brief Holds all user-configurable simulation parameters.
 *
 * Loaded from config.txt at startup.  Users edit config.txt
 * instead of modifying C++ source code.
 */
struct SimConfig {
    // ---- Network ----
    int          numRouters      = 20;
    std::string  topologyStr     = "RANDOM";   // RANDOM, RING, MESH, TREE

    // ---- Simulation ----
    int          numPackets      = 100;
    int          sourceRouter    = 0;           // -1 = first router (0)
    int          destRouter      = -1;          // -1 = last router

    // ---- Simulated Link Latency ----
    bool         simulateLatency = false;
    double       latencyMinMs    = 1.0;
    double       latencyMaxMs    = 10.0;

    // ---- Output ----
    std::string  logLevelStr     = "INFO";      // SILENT, INFO, DEBUG
    unsigned int seed            = 0;           // 0 = non-deterministic
};

/**
 * @brief Load configuration from a text file.
 *
 * Format:  key = value   (lines starting with '#' are comments)
 * Missing keys keep their default values.
 *
 * @param filename  Path to the config file (default: "config.txt")
 * @return Parsed SimConfig with user values applied.
 */
SimConfig loadConfig(const std::string& filename = "config.txt");

/**
 * @brief Print the loaded configuration to stdout.
 */
void printConfig(const SimConfig& cfg);
