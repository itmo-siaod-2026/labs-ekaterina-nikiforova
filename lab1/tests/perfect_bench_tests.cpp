#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

#include "perfect_hash.h"

using namespace itmo_algo;
namespace fs = std::filesystem;

const std::string kCsvPath = std::string(DATA_DIR) + "perfect_hash_latency_raw.csv";
constexpr int kIterations = 7;

using Dictionary = std::unordered_map<int64_t, int64_t>;

struct BenchResult
{
    uint64_t n;
    int iter;
    double build_val;
    double get_val;
};

int main()
{
    std::mt19937_64 gen(42);

    std::vector<uint64_t> n_values = {
        100'000, 200'000, 300'000, 400'000, 500'000, 600'000, 700'000,
        800'000, 900'000, 1'000'000
    };

    std::vector<BenchResult> all_results;

    for (uint64_t n : n_values)
    {
        for (int i = 0; i < kIterations; ++i)
        {
            Dictionary test_data;
            test_data.reserve(n);
            std::vector<int64_t> keys;
            keys.reserve(n);

            while (test_data.size() < n)
            {
                int64_t k = gen();
                int64_t v = gen();
                if (test_data.try_emplace(k, v).second)
                    keys.push_back(k);
            }
            std::shuffle(keys.begin(), keys.end(), gen);

            auto t1 = std::chrono::high_resolution_clock::now();
            PerfectHash ph(test_data);
            auto t2 = std::chrono::high_resolution_clock::now();

            double build_time_total = std::chrono::duration<double, std::nano>(t2 - t1).count();
            double t_build = build_time_total / n;

            volatile int64_t sink;
            auto t3 = std::chrono::high_resolution_clock::now();
            for (const auto& k : keys)
            {
                auto res = ph.get(k);
                if (res)
                    sink = *res;
            }
            auto t4 = std::chrono::high_resolution_clock::now();

            double get_time_total = std::chrono::duration<double, std::nano>(t4 - t3).count();
            double t_get = get_time_total / n;

            all_results.push_back({n, i, t_build, t_get});
        }
        std::cout << "[OK] Данные для N = " << n << " собраны." << std::endl;
    }

    std::ofstream file(kCsvPath);
    if (file.is_open())
    {
        file << "N,Iteration,BUILD,GET\n";
        for (const auto& r : all_results)
        {
            file << r.n << ","
                << r.iter << ","
                << std::fixed << std::setprecision(4) << r.build_val << ","
                << r.get_val << "\n";
        }
        std::cout << "\nРезультаты сохранены в: " << kCsvPath << std::endl;
    }
    else
        std::cerr << "\nОшибка: Не удалось открыть файл для записи!" << std::endl;

    return 0;
}
