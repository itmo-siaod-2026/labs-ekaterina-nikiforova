#pragma once

#include <memory>
#include <string>

namespace itmo_algo
{
    enum class TableStatus
    {
        Ok = 0,
        NotFound,
        FileOpenError,
        InvalidCapacity,
        InvalidFile,
        IoError,
        AllocationError,
        TableFull,
        DuplicateKey,
    };

    struct Entry
    {
        int64_t key;
        int64_t value;
    };

    struct Header
    {
        uint32_t global_depth;
        uint32_t max_global_depth;
        uint64_t bucket_capacity;
        uint64_t bucket_count;
        uint64_t directory_offset;
        uint64_t first_bucket_offset;
        uint64_t file_size;
    };

    struct BucketHeader
    {
        uint32_t local_depth;
        uint32_t count;
    };

    class ExtendibleHashing
    {
    public:
        ExtendibleHashing() : _fd(-1), _mmap_ptr(nullptr), _header(nullptr), _bucket_size(0)
        {
        }

        ~ExtendibleHashing() { closeTable(); }

        TableStatus openTable(const std::string &file_path, uint32_t bucket_capacity = 0);

        TableStatus insertRecord(int64_t key, int64_t value);

        TableStatus removeRecord(int64_t key);

        TableStatus updateRecord(int64_t key, int64_t value);

        TableStatus getRecord(int64_t key, int64_t &value);

    private:
        int _fd;
        void *_mmap_ptr;
        Header *_header;
        uint64_t _bucket_size;

        TableStatus splitBucket(uint32_t dir_idx);

        TableStatus createNewBucket(uint32_t depth, uint64_t &out_offset);

        TableStatus duplicateDirectory();

        void closeTable();

        TableStatus remapFile(uint64_t new_size);

        uint64_t getDirectoryIndex(uint64_t key);

        uint64_t *getDirectoryStart() const;

        BucketHeader *getBucketHeader(uint64_t bucket_offset) const;

        Entry *getBucketEntries(uint64_t bucket_offset) const;

        uint64_t hash(int64_t key);

        TableStatus createTable(const std::string &file_path, uint32_t bucket_capacity);

        static constexpr uint32_t kInitialGlobalDepth = 1;
        static constexpr uint32_t kMaxGlobalDepth = 20;
    };
}
