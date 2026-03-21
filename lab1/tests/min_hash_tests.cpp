#include <gtest/gtest.h>
#include "min_lsh_hash.h"

class AnimalLSHTest : public ::testing::Test
{
protected:
    MinHashLSH index{128, 32, 1};
};

TEST_F(AnimalLSHTest, LionExactMatch)
{
    std::string text = "The lion is a species in the family Felidae; it is a muscular, deep-chested cat.";
    DocID id = index.addDocument(text);
    auto candidates = index.findCandidates(text);
    EXPECT_TRUE(candidates.count(id));
}

TEST_F(AnimalLSHTest, PenguinFamilySearch)
{
    DocID parent_id = index.addDocument("Emperor penguins are the tallest and heaviest of all living penguin species.");
    DocID chick_id = index.addDocument(
        "Emperor penguins are the tallest and heaviest penguin species, even the chicks.");
    auto candidates = index.findCandidatesById(parent_id);
    EXPECT_TRUE(candidates.count(chick_id));
}

TEST_F(AnimalLSHTest, WhaleNearDuplicate)
{
    index.addDocument("Blue whales travel thousands of miles every year to feed in cold polar waters.");
    std::string query = "Blue whales travel thousands of miles every year to breed in warm tropical waters.";
    auto candidates = index.findCandidates(query);
    EXPECT_FALSE(candidates.empty());
}

TEST_F(AnimalLSHTest, WolfIsNotShark)
{
    index.addDocument("Gray wolves are highly adaptable to various environments, from tundras to forests.");
    std::string another_animal =
        "The great white shark is a species of large lamniform shark found in coastal surface waters.";
    auto candidates = index.findCandidates(another_animal);
    EXPECT_TRUE(candidates.empty());
}

TEST_F(AnimalLSHTest, GiraffePartialOverlap)
{
    index.addDocument("Giraffes are the tallest living terrestrial animals and the largest ruminants.");
    index.addDocument("Giraffes use their extremely long necks to reach leaves and buds high in trees.");

    auto fs_results = index.findDuplicatesFullScan("Giraffes are large animals with very long necks.", 0.15);

    EXPECT_GE(fs_results.size(), 1);
}
