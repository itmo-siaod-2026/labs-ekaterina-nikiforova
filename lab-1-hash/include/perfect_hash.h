#pragma once
#include <cstdint>
#include <limits>
#include <vector>
#include <optional>
#include <unordered_map>

namespace itmo_algo
{
    static constexpr int64_t kEmpty = std::numeric_limits<int64_t>::min();

    struct Entry
    {
        int64_t key;
        int64_t value;
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
    };

    class PerfectHash
    {
    public:
        explicit PerfectHash(const std::unordered_map<int64_t, int64_t>& data);

        std::optional<int64_t> get(int64_t key) const;

    private:
        size_t _n;
        HashParams _first_level_params{};
        std::vector<SubTable> _second_level_tables;
        static constexpr size_t kPrimeP = (1ULL << 61) - 1;

        size_t hash(int64_t key, const HashParams& p) const;
        void build(const std::unordered_map<int64_t, int64_t>& data);
    };
}
