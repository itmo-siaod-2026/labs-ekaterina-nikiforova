#include "../include/perfect_hash.h"
#include <random>
#include <algorithm>

using namespace itmo_algo;

PerfectHash::PerfectHash(const std::vector<int>& keys)
{
    if (keys.empty())
    {
        _n = 0;
        return;
    }
    std::vector<int> unique_keys = keys;

    std::ranges::sort(unique_keys);
    unique_keys.erase(std::ranges::unique(unique_keys).begin(), unique_keys.end());

    _n = unique_keys.size();
    build(unique_keys);
}

size_t PerfectHash::hash(int key, const HashParams& p) const
{
    if (p.m == 0) return 0;
    unsigned __int128 val = static_cast<unsigned __int128>(p.a) * static_cast<unsigned int>(key);
    val += p.b;
    return static_cast<size_t>((val % kPrimeP) % p.m);
}

void PerfectHash::build(const std::vector<int>& keys)
{
    _first_level_params.m = _n;
    _second_level_tables.resize(_n);

    std::mt19937 gen(1337);
    std::uniform_int_distribution<size_t> dist_a(1, kPrimeP - 1);
    std::uniform_int_distribution<size_t> dist_b(0, kPrimeP - 1);

    std::vector<std::vector<int>> buckets(_n);
    bool success_level1 = false;

    while (!success_level1)
    {
        _first_level_params.a = dist_a(gen);
        _first_level_params.b = dist_b(gen);

        for (auto& bucket : buckets)
            bucket.clear();

        for (int k : keys)
            buckets[hash(k, _first_level_params)].push_back(k);

        size_t sum_n_sq = 0;
        for (const auto& bucket : buckets)
            sum_n_sq += bucket.size() * bucket.size();

        if (sum_n_sq < 4 * _n)
            success_level1 = true;
    }

    for (size_t i = 0; i < _n; ++i)
    {
        if (buckets[i].empty()) continue;

        const size_t keys_in_first_bucker = buckets[i].size();
        if (keys_in_first_bucker == 1)
        {
            _second_level_tables[i].params = {0, 0, 1};
            _second_level_tables[i].cells.push_back(buckets[i][0]);
            _second_level_tables[i].initialized = true;
            continue;
        }

        const size_t m_j = keys_in_first_bucker * keys_in_first_bucker;
        _second_level_tables[i].params.m = m_j;
        _second_level_tables[i].cells.assign(m_j, SubTable::kEmpty);
        _second_level_tables[i].initialized = true;

        bool collision = true;
        while (collision)
        {
            collision = false;
            _second_level_tables[i].params.a = dist_a(gen);
            _second_level_tables[i].params.b = dist_b(gen);
            std::fill(_second_level_tables[i].cells.begin(), _second_level_tables[i].cells.end(), SubTable::kEmpty);

            for (const int k : buckets[i])
            {
                size_t h2 = hash(k, _second_level_tables[i].params);
                if (_second_level_tables[i].cells[h2] != SubTable::kEmpty)
                {
                    collision = true;
                    break;
                }
                _second_level_tables[i].cells[h2] = k;
            }
        }
    }
}

bool PerfectHash::find(int key) const
{
    if (_n == 0) return false;

    size_t h1 = hash(key, _first_level_params);
    const SubTable& st = _second_level_tables[h1];

    if (!st.initialized) return false;

    size_t h2 = hash(key, st.params);
    return st.cells[h2] == key;
}
