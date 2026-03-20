#include "perfect_hash.h"
#include <algorithm>
#include <random>

using namespace itmo_algo;

PerfectHash::PerfectHash(const std::vector<Entry>& data)
{
    if (data.empty())
    {
        _n = 0;
        return;
    }

    std::vector<Entry> unique_data = data;
    std::ranges::sort(unique_data, {}, &Entry::key);
    auto [first, last] = std::ranges::unique(unique_data, [](const Entry& a, const Entry& b)
    {
        return a.key == b.key;
    });
    unique_data.erase(first, last);

    _n = unique_data.size();
    build(unique_data);
}

size_t PerfectHash::hash(int64_t key, const HashParams& p) const
{
    if (p.m == 0) return 0;
    unsigned __int128 val = static_cast<unsigned __int128>(p.a) * static_cast<uint64_t>(key);
    val += p.b;
    return static_cast<size_t>((val % kPrimeP) % p.m);
}

void PerfectHash::build(const std::vector<Entry>& data)
{
    _first_level_params.m = _n;
    _second_level_tables.assign(_n, SubTable());

    std::mt19937 gen(1337);
    std::uniform_int_distribution<size_t> dist_a(1, kPrimeP - 1);
    std::uniform_int_distribution<size_t> dist_b(0, kPrimeP - 1);

    std::vector<std::vector<Entry>> buckets(_n);
    bool success_level1 = false;

    while (!success_level1)
    {
        _first_level_params.a = dist_a(gen);
        _first_level_params.b = dist_b(gen);

        for (auto& bucket : buckets)
            bucket.clear();

        for (const auto& e : data)
            buckets[hash(e.key, _first_level_params)].push_back(e);

        size_t sum_n_sq = 0;
        for (const auto& bucket : buckets)
            sum_n_sq += bucket.size() * bucket.size();

        if (sum_n_sq < 4 * _n)
            success_level1 = true;
    }

    for (size_t i = 0; i < _n; ++i)
    {
        if (buckets[i].empty()) continue;
        size_t m_j = (buckets[i].size() == 1) ? 1 : buckets[i].size() * buckets[i].size();

        _second_level_tables[i].params.m = m_j;
        _second_level_tables[i].cells.assign(m_j, {Entry::kEmpty, 0});
        _second_level_tables[i].initialized = true;

        bool collision = true;
        while (collision)
        {
            collision = false;
            _second_level_tables[i].params.a = dist_a(gen);
            _second_level_tables[i].params.b = dist_b(gen);
            std::fill(_second_level_tables[i].cells.begin(), _second_level_tables[i].cells.end(),
                      Entry{Entry::kEmpty, 0});

            for (const auto& e : buckets[i])
            {
                size_t h2 = hash(e.key, _second_level_tables[i].params);
                if (_second_level_tables[i].cells[h2].key != Entry::kEmpty)
                {
                    collision = true;
                    break;
                }
                _second_level_tables[i].cells[h2] = e;
            }
        }
    }
}

std::optional<int64_t> PerfectHash::get(int64_t key) const
{
    if (_n == 0)
        return std::nullopt;

    size_t h1 = hash(key, _first_level_params);
    const auto& st = _second_level_tables[h1];
    if (!st.initialized)
        return std::nullopt;

    size_t h2 = hash(key, st.params);
    if (st.cells[h2].key == key)
        return st.cells[h2].value;

    return std::nullopt;
}
