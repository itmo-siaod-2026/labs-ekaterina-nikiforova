#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <map>
#include "extendible_hash.h"

using namespace itmo_algo;
namespace fs = std::filesystem;

const std::string kDbPath = "benchmark_db.bin";
const uint32_t kBucketCapacity = 64;
const int kIterations = 5;

struct BenchResult
{
    double insert_val, get_val, update_val, delete_val;
};

double runOp(ExtendibleHashing& db, std::vector<int64_t>& keys, const std::string& op, std::mt19937_64& gen)
{
    std::shuffle(keys.begin(), keys.end(), gen);

    auto start = std::chrono::high_resolution_clock::now();

    if (op == "INSERT")
    {
        for (const auto& k : keys) db.insertRecord(k, k * 10);
    }
    else if (op == "GET")
    {
        int64_t val;
        for (const auto& k : keys) db.getRecord(k, val);
    }
    else if (op == "UPDATE")
    {
        for (const auto& k : keys) db.updateRecord(k, k * 20);
    }
    else if (op == "DELETE")
    {
        for (const auto& k : keys) db.removeRecord(k);
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void printTable(const std::string& title, const std::string& unit, const std::map<uint64_t, BenchResult>& data)
{
    std::cout << "\n=== " << title << " (" << unit << ") ===\n";
    std::cout << std::left << std::setw(10) << "N" << " | "
        << std::setw(12) << "INSERT" << " | "
        << std::setw(12) << "UPDATE" << " | "
        << std::setw(12) << "DELETE" << " | "
        << std::setw(12) << "GET" << std::endl;
    std::cout << std::string(75, '-') << std::endl;

    for (auto const& [n, r] : data)
    {
        std::cout << std::left << std::setw(10) << n << " | "
            << std::right << std::fixed << std::setprecision(2)
            << std::setw(12) << r.insert_val << " | "
            << std::setw(12) << r.update_val << " | "
            << std::setw(12) << r.delete_val << " | "
            << std::setw(12) << r.get_val << std::endl;
    }
}

int main()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());

    std::vector<uint64_t> n_values = {1'000, 5'000, 10'000, 50'000, 100'000, 500'000, 1'000'000};
    std::map<uint64_t, BenchResult> latency_avg, throughput_avg;

    std::cout << "Запуск бенчмарка (" << kIterations << " итераций на каждый N)..." << std::endl;

    for (uint64_t n : n_values)
    {
        BenchResult lat_sum = {0, 0, 0, 0};
        BenchResult thr_sum = {0, 0, 0, 0};

        for (int i = 0; i < kIterations; ++i)
        {
            std::vector<int64_t> keys(n);
            std::uniform_int_distribution<int64_t> dist(1, 1000000000);
            for (uint64_t j = 0; j < n; ++j) keys[j] = dist(gen);

            if (fs::exists(kDbPath)) fs::remove(kDbPath);
            ExtendibleHashing db;
            db.openTable(kDbPath, kBucketCapacity);

            double t_ins = runOp(db, keys, "INSERT", gen);
            double t_get = runOp(db, keys, "GET", gen);
            double t_upd = runOp(db, keys, "UPDATE", gen);
            double t_del = runOp(db, keys, "DELETE", gen);

            lat_sum.insert_val += (t_ins * 1000000.0) / n;
            lat_sum.get_val += (t_get * 1000000.0) / n;
            lat_sum.update_val += (t_upd * 1000000.0) / n;
            lat_sum.delete_val += (t_del * 1000000.0) / n;

            thr_sum.insert_val += (n / (t_ins / 1000.0)) / 1000000.0;
            thr_sum.get_val += (n / (t_get / 1000.0)) / 1000000.0;
            thr_sum.update_val += (n / (t_upd / 1000.0)) / 1000000.0;
            thr_sum.delete_val += (n / (t_del / 1000.0)) / 1000000.0;
        }

        latency_avg[n] = {
            lat_sum.insert_val / kIterations, lat_sum.get_val / kIterations,
            lat_sum.update_val / kIterations, lat_sum.delete_val / kIterations
        };
        throughput_avg[n] = {
            thr_sum.insert_val / kIterations, thr_sum.get_val / kIterations,
            thr_sum.update_val / kIterations, thr_sum.delete_val / kIterations
        };

        std::cout << "[OK] Данные для N = " << n << " собраны." << std::endl;
    }

    printTable("РЕЗУЛЬТАТЫ ЗАДЕРЖКИ", "среднее, ns/op", latency_avg);
    printTable("РЕЗУЛЬТАТЫ ПРОПУСКНОЙ СПОСОБНОСТИ", "среднее, млн оп/с", throughput_avg);

    return 0;
}
