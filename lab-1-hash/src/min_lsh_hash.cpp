#include "min_lsh_hash.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <string_view>

MinHashLSH::MinHashLSH(const int64_t m_permutations, const int64_t b_bands, const int64_t shingle_size)
    : _m_permutations(m_permutations), _bands_count(b_bands),
      _rows_per_band(m_permutations / b_bands), _shingle_size(shingle_size), _next_id(0)
{
    _lsh_ht_tables.resize(_bands_count);
    initPermutations();
}

DocID MinHashLSH::addDocument(const std::string& text)
{
    const DocID id = _next_id++;
    NgramsSet n_grams = buildNgramSet(text);
    const VSignature sig = buildSignature(n_grams);

    _docs_storage[id] = {text, std::move(n_grams), sig};

    for (int64_t i = 0; i < _bands_count; ++i)
    {
        size_t h_bucket = getBandBucketHash(sig, i);
        _lsh_ht_tables[i][h_bucket].push_back(id);
    }
    return id;
}

std::set<DocID> MinHashLSH::findCandidates(const std::string& text) const
{
    NgramsSet n_grams = buildNgramSet(text);
    VSignature sig = buildSignature(n_grams);
    return findCandidatesInternal(sig);
}

std::set<DocID> MinHashLSH::findCandidatesById(DocID local_id) const
{
    auto it = _docs_storage.find(local_id);
    if (it == _docs_storage.end())
        return {};

    std::set<DocID> candidates = findCandidatesInternal(it->second.signature);
    candidates.erase(local_id);
    return candidates;
}

std::vector<MinHashLSH::Match> MinHashLSH::findDuplicates(const std::string& text, double threshold) const
{
    NgramsSet query_ngrams = buildNgramSet(text);
    VSignature sig = buildSignature(query_ngrams);
    std::set<DocID> candidates = findCandidatesInternal(sig);

    std::vector<Match> matches;
    for (DocID id : candidates)
    {
        double score = calculateJaccard(query_ngrams, _docs_storage.at(id).n_grams);
        if (score >= threshold)
            matches.push_back({id, score});
    }
    return matches;
}

std::vector<MinHashLSH::Match> MinHashLSH::fullScanDuplicates(const std::string& text, double threshold) const
{
    NgramsSet query_ngrams = buildNgramSet(text);
    std::vector<Match> matches;
    for (const auto& [id, doc] : _docs_storage)
    {
        double score = calculateJaccard(query_ngrams, doc.n_grams);
        if (score >= threshold)
            matches.push_back({id, score});
    }
    return matches;
}

std::string MinHashLSH::getDocumentText(DocID id) const
{
    auto it = _docs_storage.find(id);
    return (it != _docs_storage.end()) ? it->second.text : "";
}

std::set<DocID> MinHashLSH::findCandidatesInternal(const VSignature& sig) const
{
    std::set<DocID> candidates;

    for (int i = 0; i < _bands_count; ++i)
    {
        size_t h_bucket = getBandBucketHash(sig, i);

        auto it_table = _lsh_ht_tables[i].find(h_bucket);
        if (it_table != _lsh_ht_tables[i].end())
            candidates.insert(it_table->second.begin(), it_table->second.end());
    }
    return candidates;
}

NgramsSet MinHashLSH::buildNgramSet(const std::string& text) const
{
    std::vector<std::string> words;
    std::string word;
    std::stringstream ss;
    for (unsigned char c : text)
        if (std::isalnum(c) || std::isspace(c))
            ss << static_cast<char>(std::tolower(c));
    while (ss >> word)
        words.push_back(word);

    NgramsSet shingles;
    if (words.size() < static_cast<size_t>(_shingle_size))
    {
        if (!words.empty())
        {
            std::string s;
            for (auto& w : words)
                s += w;
            shingles.insert(std::hash<std::string>{}(s));
        }
        return shingles;
    }
    for (size_t i = 0; i <= words.size() - _shingle_size; ++i)
    {
        std::string g;
        for (int64_t j = 0; j < _shingle_size; ++j)
            g += words[i + j] + (j == _shingle_size - 1 ? "" : " ");
        shingles.insert(std::hash<std::string>{}(g));
    }
    return shingles;
}

VSignature MinHashLSH::buildSignature(const NgramsSet& n_grams) const
{
    VSignature sig(_m_permutations, std::numeric_limits<uint64_t>::max());
    for (auto nh : n_grams)
    {
        for (int64_t i = 0; i < _m_permutations; ++i)
        {
            const uint64_t val = static_cast<uint64_t>((static_cast<unsigned __int128>(_pi_functions[i].a) * nh +
                    _pi_functions[i].b) %
                kPrime);
            if (val < sig[i])
                sig[i] = val;
        }
    }
    return sig;
}

size_t MinHashLSH::getBandBucketHash(const VSignature& sig, int64_t band_idx) const
{
    const std::string_view bv(reinterpret_cast<const char*>(&sig[band_idx * _rows_per_band]),
                              _rows_per_band * sizeof(uint64_t));
    return std::hash<std::string_view>{}(bv);
}

double MinHashLSH::calculateJaccard(const NgramsSet& s1, const NgramsSet& s2)
{
    if (s1.empty() || s2.empty())
        return 0.0;
    size_t inter = 0;
    for (auto& h : s1)
        if (s2.contains(h))
            inter++;
    return static_cast<double>(inter) / (s1.size() + s2.size() - inter);
}

void MinHashLSH::initPermutations()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist_a(1, kPrime - 1), dist_b(0, kPrime - 1);
    for (int64_t i = 0; i < _m_permutations; ++i)
        _pi_functions.push_back({dist_a(gen), dist_b(gen)});
}
