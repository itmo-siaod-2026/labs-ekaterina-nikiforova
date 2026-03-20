#pragma once

#include <limits>
#include <vector>

namespace itmo_algo
{
    struct HashParams
    {
        size_t a;
        size_t b;
        size_t m;
    };

    struct SubTable
    {
        HashParams params;
        std::vector<int> cells;
        bool initialized = false;

        static constexpr int kEmpty = std::numeric_limits<int>::min();

        SubTable() : params{0, 0, 0}, initialized(false)
        {
        }
    };

    const size_t kPrimeP = (1ULL << 31) - 1;

    class PerfectHash
    {
    public:
        explicit PerfectHash(const std::vector<int>& keys);
        bool find(int key) const;

    private:
        size_t _n;
        HashParams _first_level_params;
        std::vector<SubTable> _second_level_tables;

        size_t hash(int key, const HashParams& p) const;
        void build(const std::vector<int>& keys);
    };
}
