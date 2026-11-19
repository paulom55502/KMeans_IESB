#include "../common/kmeans_common.hpp"
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank = 0, size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int exit_code = 0;
    try {
        Config cfg;
        if (rank == 0) {
            try { cfg = parse_args(argc, argv); }
            catch (const std::exception& e) {
                std::cerr << "Erro: " << e.what() << "\n";
                exit_code = 1;
            }
        }
        MPI_Bcast(&exit_code, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (exit_code) { MPI_Finalize(); return exit_code; }

        int params[4] = {cfg.n, cfg.d, cfg.k, cfg.max_iter};
        MPI_Bcast(params, 4, MPI_INT, 0, MPI_COMM_WORLD);
        cfg.n = params[0]; cfg.d = params[1]; cfg.k = params[2]; cfg.max_iter = params[3];

        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        std::vector<int> counts(size), displs(size);
        for (int r = 0; r < size; ++r) {
            int begin = static_cast<int>((static_cast<long long>(r) * cfg.n) / size);
            int end = static_cast<int>((static_cast<long long>(r + 1) * cfg.n) / size);
            counts[r] = (end - begin) * cfg.d;
            displs[r] = begin * cfg.d;
        }
        int local_n = counts[rank] / cfg.d;
        std::vector<double> all_points;
        if (rank == 0) all_points = make_points(cfg.n, cfg.d);
        std::vector<double> local_points(static_cast<std::size_t>(local_n) * cfg.d);

        std::vector<double> centroids;
        if (rank == 0) centroids = initial_centroids(all_points, cfg.k, cfg.d);
        else centroids.resize(static_cast<std::size_t>(cfg.k) * cfg.d);
        MPI_Bcast(centroids.data(), cfg.k * cfg.d, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        std::vector<double> local_sums(static_cast<std::size_t>(cfg.k) * cfg.d);
        std::vector<double> global_sums(static_cast<std::size_t>(cfg.k) * cfg.d);
        std::vector<int> local_counts(cfg.k), global_counts(cfg.k);

        MPI_Scatterv(rank == 0 ? all_points.data() : nullptr, counts.data(), displs.data(), MPI_DOUBLE,
                     local_points.data(), counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

        int iterations = 0;
        for (int iter = 0; iter < cfg.max_iter; ++iter) {
            std::fill(local_sums.begin(), local_sums.end(), 0.0);
            std::fill(local_counts.begin(), local_counts.end(), 0);

            for (int i = 0; i < local_n; ++i) {
                const double* point = &local_points[static_cast<std::size_t>(i) * cfg.d];
                int best = 0;
                double best_dist = squared_distance(point, &centroids[0], cfg.d);
                for (int c = 1; c < cfg.k; ++c) {
                    double dist = squared_distance(point, &centroids[static_cast<std::size_t>(c) * cfg.d], cfg.d);
                    if (dist < best_dist) { best_dist = dist; best = c; }
                }
                local_counts[best]++;
                for (int j = 0; j < cfg.d; ++j)
                    local_sums[static_cast<std::size_t>(best) * cfg.d + j] += point[j];
            }

            MPI_Allreduce(local_sums.data(), global_sums.data(), cfg.k * cfg.d, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            MPI_Allreduce(local_counts.data(), global_counts.data(), cfg.k, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

            double max_move = 0.0;
            for (int c = 0; c < cfg.k; ++c) {
                if (global_counts[c] == 0) continue;
                for (int j = 0; j < cfg.d; ++j) {
                    std::size_t idx = static_cast<std::size_t>(c) * cfg.d + j;
                    double next = global_sums[idx] / global_counts[c];
                    double diff = next - centroids[idx];
                    max_move = std::max(max_move, diff * diff);
                    centroids[idx] = next;
                }
            }
            iterations = iter + 1;
            if (max_move < cfg.tolerance * cfg.tolerance) break;
        }

        double local_elapsed = MPI_Wtime() - start, elapsed = 0.0;
        MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            print_result(cfg, iterations, centroids, elapsed, "MPI");
            std::cout << "PROCESSOS=" << size << "\n";
        }
    } catch (const std::exception& e) {
        if (rank == 0) std::cerr << "Erro: " << e.what() << "\n";
        exit_code = 1;
    }
    MPI_Finalize();
    return exit_code;
}
