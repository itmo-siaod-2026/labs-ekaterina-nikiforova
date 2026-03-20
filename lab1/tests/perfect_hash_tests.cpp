#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <random>
#include "perfect_hash.h"

using namespace itmo_algo;

TEST(PerfectHashTest, BasicSearch)
{
    std::vector<int> keys = {1, 5, 42, 100, -500};
    PerfectHash ph(keys);

    for (int k : keys)
        EXPECT_TRUE(ph.find(k)) << "Should find key: " << k;

    EXPECT_FALSE(ph.find(0));
    EXPECT_FALSE(ph.find(999));
}

TEST(PerfectHashTest, EmptyInput)
{
    std::vector<int> keys = {};
    PerfectHash ph(keys);

    EXPECT_FALSE(ph.find(0));
    EXPECT_FALSE(ph.find(1));
}

TEST(PerfectHashTest, SingleInput)
{
    std::vector<int> keys = {1};
    PerfectHash ph(keys);

    EXPECT_TRUE(ph.find(1));
}

TEST(PerfectHashTest, HandleDuplicates)
{
    std::vector<int> keys = {10, 10, 20, 20, 10, 30, 30, 30};

    PerfectHash ph(keys);

    EXPECT_TRUE(ph.find(10));
    EXPECT_TRUE(ph.find(20));
    EXPECT_TRUE(ph.find(30));
    EXPECT_FALSE(ph.find(15));
}

TEST(PerfectHashTest, NegativeSearchStress)
{
    std::vector<int> keys = {10, 20, 30, 40, 50};
    PerfectHash ph(keys);

    std::set<int> s(keys.begin(), keys.end());
    for (int i = 0; i < 1000; ++i)
    {
        if (s.contains(i))
            EXPECT_TRUE(ph.find(i));
        else
            EXPECT_FALSE(ph.find(i)) << "Found key " << i << " which shouldn't be there!";
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
