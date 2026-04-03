#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "extendible_hash.h"

namespace fs = std::filesystem;
using namespace itmo_algo;

static constexpr uint32_t kMaxGlobalDepth = 20;

class ExtendibleHashingTest : public ::testing::Test
{
protected:
    std::string current_test_db;

    void SetUp() override
    {
        const ::testing::TestInfo *const test_info = ::testing::UnitTest::GetInstance()->current_test_info();

        current_test_db = std::string(test_info->test_suite_name()) + "_" + std::string(test_info->name()) + ".bin";

        if (fs::exists(current_test_db))
        {
            fs::remove(current_test_db);
        }
    }

    void TearDown() override
    {
        if (fs::exists(current_test_db))
            fs::remove(current_test_db);
    }
};

TEST_F(ExtendibleHashingTest, CreateWithInvalidCapacity)
{
    ExtendibleHashing db;
    TableStatus status = db.openTable(current_test_db, 0);

    ASSERT_EQ(status, TableStatus::InvalidCapacity);
    EXPECT_FALSE(fs::exists(current_test_db));
}

TEST_F(ExtendibleHashingTest, CreateValidTable)
{
    ExtendibleHashing db;
    uint32_t capacity = 10;

    TableStatus status = db.openTable(current_test_db, capacity);

    ASSERT_EQ(status, TableStatus::Ok);
    EXPECT_TRUE(fs::exists(current_test_db));

    uint64_t max_dir_entries = (1ULL << kMaxGlobalDepth);
    uint64_t dir_size_bytes = max_dir_entries * sizeof(uint64_t);

    uint64_t bucket_size = sizeof(BucketHeader) + (capacity * sizeof(Entry));
    uint64_t expected_size = sizeof(Header) + dir_size_bytes + (2 * bucket_size);

    EXPECT_EQ(fs::file_size(current_test_db), expected_size);
}


TEST_F(ExtendibleHashingTest, OpenExistingTable)
{
    {
        ExtendibleHashing db_init;
        db_init.openTable(current_test_db, 5);
        db_init.insertRecord(42, 420);
    }

    ExtendibleHashing db_open;
    TableStatus status = db_open.openTable(current_test_db);

    ASSERT_EQ(status, TableStatus::Ok);
    int64_t val;
    EXPECT_EQ(db_open.getRecord(42, val), TableStatus::Ok);
    EXPECT_EQ(val, 420);
}

TEST_F(ExtendibleHashingTest, OpenNonExistent)
{
    ExtendibleHashing db;
    TableStatus status = db.openTable("non_existent.bin");
    EXPECT_EQ(status, TableStatus::InvalidCapacity);
}

TEST_F(ExtendibleHashingTest, OpenTooSmallFile)
{
    std::ofstream ofs(current_test_db);
    ofs << "too small";
    ofs.close();

    ExtendibleHashing db;
    TableStatus status = db.openTable(current_test_db);

    EXPECT_EQ(status, TableStatus::InvalidFile);
}

TEST_F(ExtendibleHashingTest, StressSplitsAndGrowth)
{
    ExtendibleHashing db;
    ASSERT_EQ(db.openTable(current_test_db, 2), TableStatus::Ok);

    const int count = 1000;
    for (int i = 0; i < count; ++i)
    {
        TableStatus status = db.insertRecord(i, i * 10);
        ASSERT_EQ(status, TableStatus::Ok) << "Failed at i=" << i;
    }

    for (int i = 0; i < count; ++i)
    {
        int64_t val;
        ASSERT_EQ(db.getRecord(i, val), TableStatus::Ok) << "Failed at i=" << i;
        EXPECT_EQ(val, i * 10);
    }
}

TEST_F(ExtendibleHashingTest, UpdateAndRemove)
{
    ExtendibleHashing db;
    db.openTable(current_test_db, 10);

    db.insertRecord(1, 10);

    EXPECT_EQ(db.updateRecord(1, 100), TableStatus::Ok);
    int64_t val;
    db.getRecord(1, val);
    EXPECT_EQ(val, 100);

    EXPECT_EQ(db.removeRecord(1), TableStatus::Ok);
    EXPECT_EQ(db.getRecord(1, val), TableStatus::NotFound);

    EXPECT_EQ(db.updateRecord(999, 0), TableStatus::NotFound);
}

TEST_F(ExtendibleHashingTest, DuplicateKeyInsert)
{
    ExtendibleHashing db;
    ASSERT_EQ(db.openTable(current_test_db, 10), TableStatus::Ok);

    ASSERT_EQ(db.insertRecord(42, 100), TableStatus::Ok);
    ASSERT_EQ(db.insertRecord(42, 200), TableStatus::DuplicateKey);

    int64_t val;
    ASSERT_EQ(db.getRecord(42, val), TableStatus::Ok);
    EXPECT_EQ(val, 100);
}

TEST_F(ExtendibleHashingTest, OperationsOnEmptyTable)
{
    ExtendibleHashing db;
    ASSERT_EQ(db.openTable(current_test_db, 10), TableStatus::Ok);

    int64_t val;
    EXPECT_EQ(db.getRecord(999, val), TableStatus::NotFound);
    EXPECT_EQ(db.updateRecord(999, 100), TableStatus::NotFound);
    EXPECT_EQ(db.removeRecord(999), TableStatus::NotFound);
}

TEST_F(ExtendibleHashingTest, UpdateAfterDelete)
{
    ExtendibleHashing db;
    ASSERT_EQ(db.openTable(current_test_db, 10), TableStatus::Ok);

    ASSERT_EQ(db.insertRecord(1, 10), TableStatus::Ok);
    ASSERT_EQ(db.removeRecord(1), TableStatus::Ok);

    EXPECT_EQ(db.updateRecord(1, 20), TableStatus::NotFound);

    ASSERT_EQ(db.insertRecord(1, 30), TableStatus::Ok);

    int64_t val;
    ASSERT_EQ(db.getRecord(1, val), TableStatus::Ok);
    EXPECT_EQ(val, 30);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
