#include <gtest/gtest.h>
#include "min_lsh_hash.h"

class MinHashBoundaryTest : public ::testing::Test
{
protected:
    MinHashLSH lsh{64, 8, 2};
};

TEST_F(MinHashBoundaryTest, AddingIdenticalTexts)
{
    std::string text = "unique document content";
    DocID id1 = lsh.addDocument(text);
    DocID id2 = lsh.addDocument(text);

    EXPECT_NE(id1, id2);

    auto candidates = lsh.findCandidatesById(id1);
    EXPECT_TRUE(candidates.contains(id2));
}

TEST_F(MinHashBoundaryTest, FindByNonExistentId)
{
    auto candidates = lsh.findCandidatesById(999999);
    EXPECT_TRUE(candidates.empty());
}

TEST_F(MinHashBoundaryTest, TextShorterThanShingle)
{
    DocID id = lsh.addDocument("Hello");

    auto results = lsh.findDuplicates("hello", 0.9);
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results[0].id, id);
}

TEST_F(MinHashBoundaryTest, TotallyDifferentTexts)
{
    lsh.addDocument("apple banana orange");
    lsh.addDocument("computer processor memory");

    auto matches = lsh.findDuplicates("weather rain clouds", 0.1);
    EXPECT_TRUE(matches.empty());
}

TEST_F(MinHashBoundaryTest, EmptyStringHandling)
{
    DocID id = lsh.addDocument("");

    EXPECT_NO_THROW({
        auto results = lsh.findDuplicates("", 0.1);
        });
}

TEST_F(MinHashBoundaryTest, NormalizationTest)
{
    DocID id1 = lsh.addDocument("Data   Science");

    auto results = lsh.findDuplicates("data science", 0.9);

    ASSERT_FALSE(results.empty());
    EXPECT_EQ(results[0].id, id1);
}
