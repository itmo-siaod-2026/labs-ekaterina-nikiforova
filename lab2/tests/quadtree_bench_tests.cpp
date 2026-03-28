#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "quadtree.h"

namespace fs = std::filesystem;

const std::string kCsvPath = std::string(DATA_DIR) + "quadtree_latency_raw.csv";
const int kIterations = 5;
const Rect kWorldBoundary(-180.0, -90.0, 180.0, 90.0);
const int kQueriesPerIter = 1000;

struct BenchResult
{
    uint64_t n;
    int iter;
    double insert_val, get_val, update_val, delete_val;
};

std::vector<Rect> generateRandomQueries(int count, std::mt19937_64& gen)
{
    std::uniform_real_distribution<double> dist_x(kWorldBoundary.min_x, kWorldBoundary.max_x - 1.0);
    std::uniform_real_distribution<double> dist_y(kWorldBoundary.min_y, kWorldBoundary.max_y - 1.0);

    std::vector<Rect> queries;
    queries.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        double x = dist_x(gen);
        double y = dist_y(gen);
        queries.emplace_back(x, y, x + 1.0, y + 1.0);
    }
    return queries;
}

double runOp(QuadtreeNode& qt, const std::vector<Point>& points, const std::string& op,
             const std::vector<Rect>& queries)
{
    auto start = std::chrono::high_resolution_clock::now();

    if (op == "INSERT")
    {
        for (const auto& p : points)
            qt.insertPoint(p);
    }
    else if (op == "GET")
    {
        std::vector<Point> results;
        results.reserve(points.size() / 100); // Примерный буфер
        for (const auto& q : queries)
        {
            qt.queryRange(q, results);
            results.clear();
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<double> dist_x(kWorldBoundary.min_x, kWorldBoundary.max_x);
    std::uniform_real_distribution<double> dist_y(kWorldBoundary.min_y, kWorldBoundary.max_y);

    uint64_t warmup_n = 50'000;
    std::vector<Point> warmup_points;
    for (uint64_t i = 0; i < warmup_n; ++i)
        warmup_points.emplace_back(dist_x(gen), dist_y(gen), i);

    auto warmup_queries = generateRandomQueries(100, gen);

    {
        QuadtreeNode warmup_qt(kWorldBoundary);
        runOp(warmup_qt, warmup_points, "INSERT", warmup_queries);
        runOp(warmup_qt, warmup_points, "GET", warmup_queries);
    }

    std::vector<uint64_t> n_values = {1'000, 5'000, 10'000, 50'000, 100'000, 500'000, 1'000'000};
    std::vector<BenchResult> all_results;

    for (uint64_t n : n_values)
    {
        for (int i = 0; i < kIterations; ++i)
        {
            std::vector<Point> points;
            points.reserve(n);
            for (uint64_t j = 0; j < n; ++j)
                points.emplace_back(dist_x(gen), dist_y(gen), j);

            auto queries = generateRandomQueries(kQueriesPerIter, gen);

            QuadtreeNode qt(kWorldBoundary);

            double t_ins = (runOp(qt, points, "INSERT", queries) * 1e6) / n;
            double t_get = (runOp(qt, points, "GET", queries) * 1e6) / kQueriesPerIter;
            double t_upd = 0.0;
            double t_del = 0.0;

            all_results.push_back({n, i, t_ins, t_get, t_upd, t_del});
            std::cout << "[OK] Данные для N = " << n << " iteration = " << i << " собраны." << std::endl;
        }
    }

    std::ofstream file(kCsvPath);
    file << "N,Iteration,INSERT,UPDATE,DELETE,GET\n";
    for (const auto& r : all_results)
    {
        file << r.n << "," << r.iter << "," << r.insert_val << ","
            << r.update_val << "," << r.delete_val << "," << r.get_val << "\n";
    }

    return 0;
}
