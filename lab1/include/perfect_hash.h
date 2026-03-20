#pragma once
#include <cstdint>
#include <limits>
#include <vector>
#include <optional>

namespace itmo_algo
{
    struct Entry
    {
        int64_t key;
        int64_t value;
        static constexpr int64_t kEmpty = std::numeric_limits<int64_t>::min();
    };

    struct HashParams
    {
        size_t a;
        size_t b;
        size_t m;
    };

    struct SubTable
    {
        HashParams params;
        std::vector<Entry> cells;
        bool initialized = false;

        SubTable() : params{0, 0, 0}, initialized(false)
        {
        }
    };

    const size_t kPrimeP = (1ULL << 31) - 1;

    class PerfectHash
    {
    public:
        explicit PerfectHash(const std::vector<Entry>& data);
        std::optional<int64_t> get(int64_t key) const;

    private:
        size_t _n;
        HashParams _first_level_params;
        std::vector<SubTable> _second_level_tables;

        size_t hash(int64_t key, const HashParams& p) const;
        void build(const std::vector<Entry>& data);
    };
}
