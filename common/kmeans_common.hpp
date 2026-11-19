#pragma once
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stdexcept>

struct Config {
    int n = 100000;
    int d = 4;
    int k = 5;
    int max_iter = 100;
    double tolerance = 1e-6;
};

inline Config parse_args(int argc, char** argv) {
    Config c;
    if (argc > 1) c.n = std::stoi(argv[1]);
    if (argc > 2) c.d = std::stoi(argv[2]);
    if (argc > 3) c.k = std::stoi(argv[3]);
    if (argc > 4) c.max_iter = std::stoi(argv[4]);
    if (c.n <= 0 || c.d <= 0 || c.k <= 0 || c.k > c.n || c.max_iter <= 0)
        throw std::runtime_error("Use: programa [N] [D] [K] [MAX_ITER], com N>=K>0, D>0.");
    return c;
}

// Gerador determinístico: o mesmo ponto é produzido em qualquer versão/processo.
inline double point_value(int i, int j) {
    std::uint64_t x = static_cast<std::uint64_t>(i + 1) * 0x9E3779B185EBCA87ULL;
    x ^= static_cast<std::uint64_t>(j + 11) * 0xC2B2AE3D27D4EB4FULL;
    x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27; x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;
    return static_cast<double>(x % 1000000ULL) / 1000000.0;
}

inline std::vector<double> make_points(int n, int d) {
    std::vector<double> p(static_cast<std::size_t>(n) * d);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j)
            p[static_cast<std::size_t>(i) * d + j] = point_value(i, j);
    return p;
}

inline double squared_distance(const double* a, const double* b, int d) {
    double sum = 0.0;
    for (int j = 0; j < d; ++j) {
        const double diff = a[j] - b[j];
        sum += diff * diff;
    }
    return sum;
}

inline std::vector<double> initial_centroids(const std::vector<double>& points, int k, int d) {
    std::vector<double> c(static_cast<std::size_t>(k) * d);
    for (int cluster = 0; cluster < k; ++cluster)
        for (int j = 0; j < d; ++j)
            c[static_cast<std::size_t>(cluster) * d + j] =
                points[static_cast<std::size_t>(cluster) * d + j];
    return c;
}

inline void print_result(const Config& cfg, int iterations,
                         const std::vector<double>& centroids, double seconds,
                         const char* version) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "VERSAO=" << version << "\nN=" << cfg.n << ";D=" << cfg.d
              << ";K=" << cfg.k << ";ITER=" << iterations
              << ";TEMPO=" << seconds << "\nCENTROIDES\n";
    for (int k = 0; k < cfg.k; ++k) {
        std::cout << k;
        for (int j = 0; j < cfg.d; ++j)
            std::cout << ";" << centroids[static_cast<std::size_t>(k) * cfg.d + j];
        std::cout << "\n";
    }
}
