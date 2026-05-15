#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "min_lsh_hash.h"

const std::string kOutPath = std::string(DATA_DIR) + "min_hash_bench_raw.csv";
constexpr int kIterations = 5;
constexpr int kWordsInDoc = 30;

struct BenchResult
{
    uint64_t n;
    int iter;
    double add_val, lsh_search_val, full_scan_val;
};

std::string generateRandomText(size_t num_words, std::mt19937_64& gen)
{
    static const std::vector<std::string> dict = {
        "apple", "banana", "cherry", "data", "science", "algorithm", "hashing",
        "minhash", "performance", "itmo", "university", "distributed", "system",
        "cloud", "computing", "database", "index", "search", "engine", "vector"
    };
    std::uniform_int_distribution<size_t> dist(0, dict.size() - 1);
    std::string res;
    for (size_t i = 0; i < num_words; ++i)
        res += dict[dist(gen)] + (i == num_words - 1 ? "" : " ");
    return res;
}

double runOp(MinHashLSH& lsh, const std::vector<std::string>& docs, const std::string& op)
{
    auto start = std::chrono::high_resolution_clock::now();

    if (op == "ADD")
    {
        for (const auto& doc : docs)
        {
            lsh.addDocument(doc);
        }
    }
    else if (op == "LSH_SEARCH")
    {
        for (const auto& doc : docs)
        {
            auto res = lsh.findDuplicates(doc, 0.5);
            volatile size_t s = res.size();
        }
    }
    else if (op == "FULL_SCAN")
    {
        for (const auto& doc : docs)
        {
            auto res = lsh.fullScanDuplicates(doc, 0.5);
            volatile size_t s = res.size();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());

    MinHashLSH lsh(256, 8, 4);
    std::vector<std::string> warmup_docs;
    for (int i = 0; i < 1000; ++i)
        warmup_docs.push_back(generateRandomText(kWordsInDoc, gen));

    runOp(lsh, warmup_docs, "ADD");
    runOp(lsh, warmup_docs, "LSH_SEARCH");

    std::vector<uint64_t> n_values = {300, 500, 700, 1000, 1500, 2000, 2500};
    std::vector<BenchResult> all_results;

    for (uint64_t n : n_values)
    {
        for (int i = 0; i < kIterations; ++i)
        {
            std::vector<std::string> docs_to_add;
            std::vector<std::string> docs_to_query;
            for (uint64_t j = 0; j < n; ++j)
            {
                docs_to_add.push_back(generateRandomText(kWordsInDoc, gen));
                docs_to_query.push_back(generateRandomText(kWordsInDoc, gen));
            }

            double t_add = (runOp(lsh, docs_to_add, "ADD") * 1e6) / n;
            double t_lsh = (runOp(lsh, docs_to_query, "LSH_SEARCH") * 1e6) / n;
            double t_fs = (runOp(lsh, docs_to_query, "FULL_SCAN") * 1e6);

            all_results.push_back({n, i, t_add, t_lsh, t_fs});
            std::cout << "[OK] N = " << n << " Iter = " << i << std::endl;
        }
    }

    std::ofstream file(kOutPath);
    file << "N,Iteration,ADD_ns,LSH_SEARCH_ns,FULL_SCAN_ns\n";
    for (const auto& r : all_results)
    {
        file << r.n << "," << r.iter << "," << r.add_val << "," << r.lsh_search_val << "," << r.full_scan_val << "\n";
    }

    std::cout << "Бенчмарк завершен. Результаты сохранены в: " << kOutPath << std::endl;

    return 0;
}
