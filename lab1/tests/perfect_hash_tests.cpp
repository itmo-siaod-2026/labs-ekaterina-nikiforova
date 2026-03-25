#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
#include "perfect_hash.h"

using namespace itmo_algo;
using Dictionary = std::unordered_map<int64_t, int64_t>;

TEST(PerfectHashTest, BasicSearch)
{
    Dictionary data = {
        {1, 100}, {5, 500}, {42, 4200}, {100, 10000}, {-500, -5000}
    };
    PerfectHash ph(data);

    for (const auto& [key,val] : data)
    {
        auto result = ph.get(key);
        ASSERT_TRUE(result.has_value()) << "Should find key: " << key;
        EXPECT_EQ(*result, val) << "Value mismatch for key: " << key;
    }

    EXPECT_FALSE(ph.get(0).has_value());
    EXPECT_FALSE(ph.get(999).has_value());
}

TEST(PerfectHashTest, EmptyInput)
{
    Dictionary data;
    PerfectHash ph(data);

    EXPECT_FALSE(ph.get(0).has_value());
    EXPECT_FALSE(ph.get(1).has_value());
}

TEST(PerfectHashTest, SingleInput)
{
    Dictionary data = {{1, 777}};
    PerfectHash ph(data);

    auto result = ph.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 777);
}

TEST(PerfectHashTest, HandleDuplicates)
{
    Dictionary data = {
        {10, 1}, {20, 2}, {10, 3}, {30, 4}, {10, 5}
    };

    PerfectHash ph(data);

    EXPECT_TRUE(ph.get(10).has_value());
    EXPECT_TRUE(ph.get(20).has_value());
    EXPECT_TRUE(ph.get(30).has_value());

    EXPECT_FALSE(ph.get(15).has_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
