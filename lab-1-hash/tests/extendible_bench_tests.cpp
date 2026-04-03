#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "extendible_hash.h"

using namespace itmo_algo;
namespace fs = std::filesystem;

const std::string kDbPath = std::string(DATA_DIR) + "benchmark_db.bin";
const uint32_t kBucketCapacity = 64;
const int kIterations = 5;

struct BenchResult
{
    uint64_t n;
    int iter;
    double insert_val, get_val, update_val, delete_val;
};

double runOp(ExtendibleHashing& db, const std::vector<int64_t>& keys, const std::string& op)
{
    volatile int64_t tmp_val;
    auto start = std::chrono::high_resolution_clock::now();

    if (op == "INSERT")
    {
        for (const auto& k : keys)
            db.insertRecord(k, k * 10);
    }
    else if (op == "GET")
    {
        int64_t val;
        for (const auto& k : keys)
        {
            db.getRecord(k, val);
            tmp_val = val;
        }
    }
    else if (op == "UPDATE")
    {
        for (const auto& k : keys)
            db.updateRecord(k, k * 20);
    }
    else if (op == "DELETE")
    {
        for (const auto& k : keys)
            db.removeRecord(k);
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());

    uint64_t warmup_n = 50'000;
    std::vector<int64_t> warmup_keys(warmup_n);
    std::uniform_int_distribution<int64_t> dist(1, 1e9);
    for (auto& k : warmup_keys)
        k = dist(gen);

    {
        if (fs::exists(kDbPath))
            fs::remove(kDbPath);
        ExtendibleHashing warmup_db;
        warmup_db.openTable(kDbPath, kBucketCapacity);

        runOp(warmup_db, warmup_keys, "INSERT");
        runOp(warmup_db, warmup_keys, "GET");
        runOp(warmup_db, warmup_keys, "UPDATE");
        runOp(warmup_db, warmup_keys, "DELETE");
    }

    std::vector<uint64_t> n_values = {1'000, 5'000, 10'000, 50'000, 100'000, 500'000, 1'000'000};
    std::vector<BenchResult> all_results;

    for (uint64_t n : n_values)
    {
        std::vector<int64_t> keys(n);
        std::uniform_int_distribution<int64_t> dist(1, 1000000000);

        for (int i = 0; i < kIterations; ++i)
        {
            for (uint64_t j = 0; j < n; ++j)
                keys[j] = dist(gen);

            if (fs::exists(kDbPath))
                fs::remove(kDbPath);

            ExtendibleHashing db;
            db.openTable(kDbPath, kBucketCapacity);

            double t_ins = (runOp(db, keys, "INSERT") * 1e6) / n;
            double t_get = (runOp(db, keys, "GET") * 1e6) / n;
            double t_upd = (runOp(db, keys, "UPDATE") * 1e6) / n;
            double t_del = (runOp(db, keys, "DELETE") * 1e6) / n;

            all_results.push_back({n, i, t_ins, t_get, t_upd, t_del});
            std::cout << "[OK] Данные для N = " << n << " iteration = " << i << " собраны." << std::endl;
        }
    }

    std::ofstream file(std::string(DATA_DIR) + "extendible_latency_raw.csv");
    file << "N,Iteration,INSERT,UPDATE,DELETE,GET\n";
    for (const auto& r : all_results)
    {
        file << r.n << "," << r.iter << "," << r.insert_val << "," << r.update_val << "," << r.delete_val << "," << r.
            get_val << "\n";
    }

    return 0;
}
