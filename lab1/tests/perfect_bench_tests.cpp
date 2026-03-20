#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <map>
#include "perfect_hash.h"

using namespace itmo_algo;

const int kIterations = 20;

struct BenchResult
{
    double build_val, get_val;
};

void printTable(const std::string& title, const std::string& unit, const std::map<uint64_t, BenchResult>& data)
{
    std::cout << "\n=== " << title << " (" << unit << ") ===\n";
    std::cout << std::left << std::setw(10) << "N" << " | "
        << std::setw(15) << "BUILD" << " | "
        << std::setw(15) << "GET" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    for (auto const& [n, r] : data)
    {
        std::cout << std::left << std::setw(10) << n << " | "
            << std::right << std::fixed << std::setprecision(2)
            << std::setw(15) << r.build_val << " | "
            << std::setw(15) << r.get_val << std::endl;
    }
}

int main()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());

    std::vector<uint64_t> n_values = {1'000, 5'000, 10'000, 50'000, 100'000, 500'000, 1'000'000};
    std::map<uint64_t, BenchResult> latency_avg, throughput_avg;

    std::cout << "Запуск бенчмарка Perfect Hashing (" << kIterations << " итераций)..." << std::endl;

    for (uint64_t n : n_values)
    {
        double build_time_total = 0;
        double get_time_total = 0;

        for (int i = 0; i < kIterations; ++i)
        {
            std::vector<Entry> entries(n);
            std::uniform_int_distribution<int64_t> dist(1, 2000000000);
            for (uint64_t j = 0; j < n; ++j)
                entries[j] = {dist(gen), static_cast<int64_t>(j)};

            auto t1 = std::chrono::high_resolution_clock::now();
            PerfectHash ph(entries);
            auto t2 = std::chrono::high_resolution_clock::now();
            build_time_total += std::chrono::duration<double, std::milli>(t2 - t1).count();

            std::vector<int64_t> query_keys;
            for (const auto& e : entries)
                query_keys.push_back(e.key);

            std::shuffle(query_keys.begin(), query_keys.end(), gen);

            volatile int64_t sink;
            auto t3 = std::chrono::high_resolution_clock::now();
            for (const auto& k : query_keys)
            {
                auto res = ph.get(k);
                if (res) sink = *res;
            }
            auto t4 = std::chrono::high_resolution_clock::now();
            get_time_total += std::chrono::duration<double, std::milli>(t4 - t3).count();
        }

        latency_avg[n] = {
            (build_time_total * 1000000.0) / (n * kIterations),
            (get_time_total * 1000000.0) / (n * kIterations)
        };

        double avg_build_sec = (build_time_total / kIterations) / 1000.0;
        double avg_get_sec = (get_time_total / kIterations) / 1000.0;

        throughput_avg[n] = {
            (n / avg_build_sec) / 1000000.0,
            (n / avg_get_sec) / 1000000.0
        };

        std::cout << "[OK] Данные для N = " << n << " собраны." << std::endl;
    }

    printTable("РЕЗУЛЬТАТЫ ЗАДЕРЖКИ", "среднее, ns/op", latency_avg);
    printTable("РЕЗУЛЬТАТЫ ПРОПУСКНОЙ СПОСОБНОСТИ", "среднее, млн оп/с", throughput_avg);

    return 0;
}
