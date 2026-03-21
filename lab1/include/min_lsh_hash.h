#pragma once

#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <cstdint>
#include <random>

using DocID = size_t;
using NgramHash = uint64_t;
using VSignature = std::vector<uint32_t>;

class MinHashLSH
{
public:
    static constexpr uint64_t kPrime = 4294967291u;

    MinHashLSH(int m_permutations = 128, int b_bands = 16, int shingle_size = 2);

    DocID addDocument(const std::string& local_text);

    std::set<DocID> findCandidates(const std::string& local_text) const;

    std::set<DocID> findCandidatesById(DocID local_id) const;

    std::vector<DocID> findDuplicatesFullScan(const std::string& local_text, double local_threshold) const;

private:
    int _m_permutations;
    int _bands_count;
    int _rows_per_band;
    int _shingle_size;
    DocID _next_id;

    struct PermutationFunc
    {
        uint64_t a;
        uint64_t b;
    };

    std::vector<PermutationFunc> _pi_functions;

    std::unordered_map<DocID, std::set<NgramHash>> _docs_n_grams;
    std::unordered_map<DocID, VSignature> _docs_signatures;

    std::vector<std::unordered_map<size_t, std::vector<DocID>>> _lsh_ht_tables;

    void initPermutations();
    std::set<NgramHash> buildNgramSet(const std::string& local_text) const;
    VSignature buildSignature(const std::set<NgramHash>& local_n_grams) const;
    size_t getBandBucketHash(const VSignature& doc_signature, int local_band_idx) const;

    std::set<DocID> findCandidatesInternal(const VSignature& sig) const;
};
