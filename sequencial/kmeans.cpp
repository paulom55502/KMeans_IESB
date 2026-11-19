#include "../common/kmeans_common.hpp"
#include <chrono>

int main(int argc, char** argv) {
    try {
        Config cfg = parse_args(argc, argv);
        auto t0 = std::chrono::steady_clock::now();

        std::vector<double> points = make_points(cfg.n, cfg.d);
        std::vector<double> centroids = initial_centroids(points, cfg.k, cfg.d);
        std::vector<double> sums(static_cast<std::size_t>(cfg.k) * cfg.d);
        std::vector<int> counts(cfg.k);
        int iterations = 0;

        for (int iter = 0; iter < cfg.max_iter; ++iter) {
            std::fill(sums.begin(), sums.end(), 0.0);
            std::fill(counts.begin(), counts.end(), 0);
            for (int i = 0; i < cfg.n; ++i) {
                const double* point = &points[static_cast<std::size_t>(i) * cfg.d];
                int best = 0;
                double best_dist = squared_distance(point, &centroids[0], cfg.d);
                for (int c = 1; c < cfg.k; ++c) {
                    double dist = squared_distance(point, &centroids[static_cast<std::size_t>(c) * cfg.d], cfg.d);
                    if (dist < best_dist) { best_dist = dist; best = c; }
                }
                counts[best]++;
                for (int j = 0; j < cfg.d; ++j)
                    sums[static_cast<std::size_t>(best) * cfg.d + j] += point[j];
            }

            double max_move = 0.0;
            for (int c = 0; c < cfg.k; ++c) {
                if (counts[c] == 0) continue;
                for (int j = 0; j < cfg.d; ++j) {
                    std::size_t idx = static_cast<std::size_t>(c) * cfg.d + j;
                    double next = sums[idx] / counts[c];
                    double diff = next - centroids[idx];
                    max_move = std::max(max_move, diff * diff);
                    centroids[idx] = next;
                }
            }
            iterations = iter + 1;
            if (max_move < cfg.tolerance * cfg.tolerance) break;
        }

        auto t1 = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration<double>(t1 - t0).count();
        print_result(cfg, iterations, centroids, seconds, "SEQUENCIAL");
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
