#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <vector>
#include <string>

#include "extendible_hash.h"

using namespace itmo_algo;
namespace fs = std::filesystem;

const std::string kDbPath = std::string(DATA_DIR) + "benchmark_db.bin";
const uint32_t kBucketCapacity = 64;

void execute_benchmark(ExtendibleHashing& db, const std::vector<int64_t>& keys, const std::string& op)
{
    volatile int64_t tmp_val;
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
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <N> <PREPARE|GET|UPDATE|DELETE>" << std::endl;
        return 1;
    }

    uint64_t n = std::stoull(argv[1]);
    std::string op = argv[2];

    std::mt19937_64 gen(42);
    std::uniform_int_distribution<int64_t> dist(1, 1e9);
    std::vector<int64_t> keys(n);
    for (uint64_t i = 0; i < n; ++i) keys[i] = dist(gen);

    ExtendibleHashing db;

    if (op == "INSERT")
    {
        if (fs::exists(kDbPath)) fs::remove(kDbPath);
        db.openTable(kDbPath, kBucketCapacity);
        execute_benchmark(db, keys, "INSERT");
        db.closeTable();
    }
    else
    {
        if (!fs::exists(kDbPath))
        {
            std::cerr << "ERR: Run INSERT first!" << std::endl;
            return 1;
        }
        db.openTable(kDbPath, kBucketCapacity);
        execute_benchmark(db, keys, op);
        db.closeTable();
    }
    return 0;
}
