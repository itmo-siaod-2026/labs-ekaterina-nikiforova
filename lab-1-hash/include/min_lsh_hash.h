#pragma once
#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using DocID = uint64_t;
using NgramHash = size_t;
using NgramsSet = std::unordered_set<NgramHash>;
using VSignature = std::vector<uint64_t>;

struct StoredDocument
{
    std::string text;
    NgramsSet n_grams;
    VSignature signature;
};

class MinHashLSH
{
public:
    struct Match
    {
        DocID id;
        double score;
    };

    explicit MinHashLSH(int64_t m_permutations = 64, int64_t b_bands = 8, int64_t shingle_size = 2);

    DocID addDocument(const std::string& text);

    std::set<DocID> findCandidates(const std::string& text) const;
    std::set<DocID> findCandidatesById(DocID local_id) const;

    std::vector<Match> findDuplicates(const std::string& text, double threshold) const;
    std::vector<Match> fullScanDuplicates(const std::string& text, double threshold) const;

    std::string getDocumentText(DocID id) const;

private:
    std::set<DocID> findCandidatesInternal(const VSignature& sig) const;

    NgramsSet buildNgramSet(const std::string& text) const;
    VSignature buildSignature(const NgramsSet& n_grams) const;
    size_t getBandBucketHash(const VSignature& sig, int64_t band_idx) const;
    static double calculateJaccard(const NgramsSet& s1, const NgramsSet& s2);

    int64_t _m_permutations;
    int64_t _bands_count;
    int64_t _rows_per_band;
    int64_t _shingle_size;
    DocID _next_id;

    struct PermutationFunc
    {
        uint64_t a;
        uint64_t b;
    };

    std::vector<PermutationFunc> _pi_functions;
    static constexpr uint64_t kPrime = (1ULL << 61) - 1;

    std::unordered_map<DocID, StoredDocument> _docs_storage;
    std::vector<std::unordered_map<size_t, std::vector<DocID>>> _lsh_ht_tables;

    void initPermutations();
};
