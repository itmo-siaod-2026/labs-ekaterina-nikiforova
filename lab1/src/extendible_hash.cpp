#include "extendible_hash.h"
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace itmo_algo;

TableStatus ExtendibleHashing::createTable(const std::string &file_path, uint32_t bucket_capacity)
{
    if (bucket_capacity == 0)
        return TableStatus::InvalidCapacity;

    int fd = open(file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        return TableStatus::FileOpenError;

    uint64_t max_dir_entries = (1ULL << kMaxGlobalDepth);
    uint64_t max_dir_size_bytes = max_dir_entries * sizeof(uint64_t);

    _bucket_size = sizeof(BucketHeader) + (bucket_capacity * sizeof(Entry));

    uint64_t max_buckets_count = (1ULL << kInitialGlobalDepth);
    uint64_t file_size = sizeof(Header) + max_dir_size_bytes + (max_buckets_count * _bucket_size);

    if (ftruncate(fd, file_size) == -1)
    {
        close(fd);
        return TableStatus::IoError;
    }

    void *ptr = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        close(fd);
        return TableStatus::AllocationError;
    }

    std::memset(static_cast<uint8_t *>(ptr) + sizeof(Header), 0, max_dir_size_bytes);

    Header *h = static_cast<Header *>(ptr);
    h->global_depth = kInitialGlobalDepth;
    h->max_global_depth = kMaxGlobalDepth;
    h->bucket_capacity = bucket_capacity;
    h->bucket_count = max_buckets_count;
    h->directory_offset = sizeof(Header);
    h->first_bucket_offset = sizeof(Header) + max_dir_size_bytes;
    h->file_size = file_size;

    uint64_t *dir = reinterpret_cast<uint64_t *>(static_cast<uint8_t *>(ptr) + h->directory_offset);

    for (uint64_t i = 0; i < max_buckets_count; ++i)
    {
        uint64_t current_bucket_offset = h->first_bucket_offset + (i * _bucket_size);
        dir[i] = current_bucket_offset;

        uint8_t *b_ptr = static_cast<uint8_t *>(ptr) + current_bucket_offset;
        BucketHeader *bh = reinterpret_cast<BucketHeader *>(b_ptr);
        bh->local_depth = kInitialGlobalDepth;
        bh->count = 0;
    }

    msync(ptr, file_size, MS_SYNC);
    munmap(ptr, file_size);
    close(fd);
    return TableStatus::Ok;
}

TableStatus ExtendibleHashing::openTable(const std::string &file_path, uint32_t bucket_capacity)
{
    _fd = open(file_path.c_str(), O_RDWR);

    if (_fd == -1 && errno == ENOENT)
    {
        TableStatus s = createTable(file_path, bucket_capacity);
        if (s != TableStatus::Ok)
            return s;
        _fd = open(file_path.c_str(), O_RDWR);
    }

    if (_fd == -1)
        return TableStatus::FileOpenError;

    struct stat st;
    if (fstat(_fd, &st) == -1)
        return TableStatus::IoError;
    if (st.st_size < sizeof(Header))
        return TableStatus::InvalidFile;

    _mmap_ptr = mmap(nullptr, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    if (_mmap_ptr == MAP_FAILED)
        return TableStatus::AllocationError;

    _header = static_cast<Header *>(_mmap_ptr);
    if (_header->bucket_capacity == 0)
        return TableStatus::InvalidFile;

    _bucket_size = sizeof(BucketHeader) + (_header->bucket_capacity * sizeof(Entry));
    return TableStatus::Ok;
}

TableStatus ExtendibleHashing::remapFile(uint64_t new_size)
{
    if (_mmap_ptr != nullptr && _mmap_ptr != MAP_FAILED)
    {
        uint64_t old_size = _header->file_size;
        msync(_mmap_ptr, old_size, MS_SYNC);
        munmap(_mmap_ptr, old_size);
    }

    if (ftruncate(_fd, new_size) == -1)
        return TableStatus::AllocationError;

    _mmap_ptr = mmap(nullptr, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    if (_mmap_ptr == MAP_FAILED)
        return TableStatus::AllocationError;

    _header = static_cast<Header *>(_mmap_ptr);
    _header->file_size = new_size;
    return TableStatus::Ok;
}

TableStatus ExtendibleHashing::duplicateDirectory()
{
    if (_header->global_depth >= _header->max_global_depth)
        return TableStatus::TableFull;

    uint64_t old_entries_count = (1ULL << _header->global_depth);
    uint64_t *dir = getDirectoryStart();
    if (!dir)
        return TableStatus::AllocationError;

    for (uint64_t i = 0; i < old_entries_count; ++i)
        dir[i + old_entries_count] = dir[i];

    _header->global_depth++;
    return TableStatus::Ok;
}

TableStatus ExtendibleHashing::createNewBucket(uint32_t depth, uint64_t &out_offset)
{
    out_offset = _header->file_size;
    uint64_t new_file_size = _header->file_size + _bucket_size;

    TableStatus s = remapFile(new_file_size);
    if (s != TableStatus::Ok)
        return s;

    _header->bucket_count++;
    BucketHeader *bh = getBucketHeader(out_offset);
    bh->local_depth = depth;
    bh->count = 0;
    return TableStatus::Ok;
}

TableStatus ExtendibleHashing::splitBucket(uint32_t dir_idx)
{
    auto dir_start = getDirectoryStart();
    if (!dir_start)
        return TableStatus::NotFound;
    uint64_t bucket_offset = getDirectoryStart()[dir_idx];
    uint32_t current_local_depth = getBucketHeader(bucket_offset)->local_depth;

    if (current_local_depth == _header->global_depth)
    {
        TableStatus s = duplicateDirectory();
        if (s != TableStatus::Ok)
            return s;
    }

    uint64_t new_bucket_offset;
    TableStatus s = createNewBucket(current_local_depth + 1, new_bucket_offset);
    if (s != TableStatus::Ok)
        return s;

    BucketHeader *old_bh = getBucketHeader(bucket_offset);
    BucketHeader *new_bh = getBucketHeader(new_bucket_offset);

    old_bh->local_depth++;

    Entry *old_entries = getBucketEntries(bucket_offset);
    Entry *new_entries = getBucketEntries(new_bucket_offset);

    uint32_t old_total = old_bh->count;
    old_bh->count = 0;
    new_bh->count = 0;

    for (uint32_t i = 0; i < old_total; i++)
    {
        Entry current = old_entries[i];
        if (hash(current.key) & (1ULL << (old_bh->local_depth - 1)))
            new_entries[new_bh->count++] = current;
        else
            old_entries[old_bh->count++] = current;
    }

    uint64_t *dir = getDirectoryStart();
    uint64_t dir_size = (1ULL << _header->global_depth);
    uint64_t bit_mask = (1ULL << (old_bh->local_depth - 1));

    for (uint64_t i = 0; i < dir_size; i++)
    {
        if (dir[i] == bucket_offset && (i & bit_mask))
            dir[i] = new_bucket_offset;
    }

    return TableStatus::Ok;
}

TableStatus ExtendibleHashing::insertRecord(int64_t key, int64_t value)
{
    int64_t t_val;
    if (getRecord(key, t_val) == TableStatus::Ok)
        return TableStatus::DuplicateKey;

    uint64_t dir_idx = getDirectoryIndex(key);
    uint64_t *dir = getDirectoryStart();
    if (!dir)
        return TableStatus::AllocationError;

    uint64_t bucket_offset = dir[dir_idx];

    BucketHeader *bh = getBucketHeader(bucket_offset);
    if (!bh)
        return TableStatus::AllocationError;

    Entry *entries = getBucketEntries(bucket_offset);

    for (uint32_t i = 0; i < bh->count; i++)
    {
        if (entries[i].key == key)
        {
            entries[i].value = value;
            return TableStatus::Ok;
        }
    }

    if (bh->count < _header->bucket_capacity)
    {
        entries[bh->count] = {key, value};
        bh->count++;
        return TableStatus::Ok;
    }

    TableStatus s = splitBucket(dir_idx);
    if (s != TableStatus::Ok)
        return s;

    return insertRecord(key, value);
}

TableStatus ExtendibleHashing::removeRecord(int64_t key)
{
    uint64_t dir_idx = getDirectoryIndex(key);
    uint64_t *dir = getDirectoryStart();
    if (!dir)
        return TableStatus::AllocationError;
    uint64_t bucket_offset = dir[dir_idx];

    BucketHeader *bh = getBucketHeader(bucket_offset);
    Entry *entries = getBucketEntries(bucket_offset);

    for (uint32_t i = 0; i < bh->count; i++)
    {
        if (entries[i].key == key)
        {
            if (i < bh->count - 1)
                entries[i] = entries[bh->count - 1];
            bh->count--;
            return TableStatus::Ok;
        }
    }
    return TableStatus::NotFound;
}

TableStatus ExtendibleHashing::updateRecord(int64_t key, int64_t value)
{
    uint64_t dir_idx = getDirectoryIndex(key);
    uint64_t *dir = getDirectoryStart();
    if (!dir)
        return TableStatus::NotFound;
    uint64_t bucket_offset = dir[dir_idx];

    BucketHeader *bh = getBucketHeader(bucket_offset);
    Entry *entries = getBucketEntries(bucket_offset);

    if (!bh || !entries)
        return TableStatus::NotFound;

    for (uint32_t i = 0; i < bh->count; i++)
    {
        if (entries[i].key == key)
        {
            entries[i].value = value;
            return TableStatus::Ok;
        }
    }
    return TableStatus::NotFound;
}

TableStatus ExtendibleHashing::getRecord(int64_t key, int64_t &value)
{
    uint64_t dir_idx = getDirectoryIndex(key);
    uint64_t *dir = getDirectoryStart();
    if (!dir)
        return TableStatus::NotFound;

    uint64_t bucket_offset = dir[dir_idx];

    BucketHeader *bh = getBucketHeader(bucket_offset);
    Entry *entries = getBucketEntries(bucket_offset);

    for (uint32_t i = 0; i < bh->count; ++i)
    {
        if (entries[i].key == key)
        {
            value = entries[i].value;
            return TableStatus::Ok;
        }
    }
    return TableStatus::NotFound;
}

void ExtendibleHashing::closeTable()
{
    if (_mmap_ptr != nullptr && _mmap_ptr != MAP_FAILED)
    {
        msync(_mmap_ptr, _header->file_size, MS_SYNC);
        munmap(_mmap_ptr, _header->file_size);
        _mmap_ptr = nullptr;
        _header = nullptr;
    }
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

uint64_t ExtendibleHashing::getDirectoryIndex(uint64_t key)
{
    return hash(key) & ((1ULL << _header->global_depth) - 1);
}

uint64_t *ExtendibleHashing::getDirectoryStart() const
{
    if (_mmap_ptr == nullptr || _mmap_ptr == MAP_FAILED)
        return nullptr;
    return reinterpret_cast<uint64_t *>(static_cast<uint8_t *>(_mmap_ptr) + _header->directory_offset);
}

BucketHeader *ExtendibleHashing::getBucketHeader(uint64_t bucket_offset) const
{
    if (_mmap_ptr == nullptr || _mmap_ptr == MAP_FAILED)
        return nullptr;
    if (bucket_offset == 0 || bucket_offset + sizeof(BucketHeader) > _header->file_size)
        return nullptr;

    uint8_t *base = static_cast<uint8_t *>(_mmap_ptr);
    return reinterpret_cast<BucketHeader *>(base + bucket_offset);
}

Entry *ExtendibleHashing::getBucketEntries(uint64_t bucket_offset) const
{
    BucketHeader *bh = getBucketHeader(bucket_offset);
    if (!bh)
        return nullptr;

    uint8_t *bucket_start = reinterpret_cast<uint8_t *>(bh);
    return reinterpret_cast<Entry *>(bucket_start + sizeof(BucketHeader));
}

uint64_t ExtendibleHashing::hash(int64_t key)
{
    return static_cast<uint64_t>(key);
}
