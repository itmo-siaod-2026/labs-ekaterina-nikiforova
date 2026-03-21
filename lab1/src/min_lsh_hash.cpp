#include "min_lsh_hash.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <string_view>

MinHashLSH::MinHashLSH(int m_permutations, int b_bands, int shingle_size)
    : _m_permutations(m_permutations), _bands_count(b_bands),
      _shingle_size(shingle_size), _next_id(0)
{
    _rows_per_band = _m_permutations / _bands_count;
    _lsh_ht_tables.resize(_bands_count);
    initPermutations();
}

void MinHashLSH::initPermutations()
{
    std::mt19937 gen(1337);
    std::uniform_int_distribution<uint64_t> dist_a(1, kPrime - 1);
    std::uniform_int_distribution<uint64_t> dist_b(0, kPrime - 1);

    for (int i = 0; i < _m_permutations; ++i)
        _pi_functions.push_back({dist_a(gen), dist_b(gen)});
}

std::set<NgramHash> MinHashLSH::buildNgramSet(const std::string& text) const
{
    std::set<NgramHash> shingles;
    std::string normalized_text;
    for (char c : text)
    {
        if (std::isalpha(c) || std::isspace(c))
            normalized_text += std::tolower(c);
    }

    std::stringstream ss(normalized_text);
    std::vector<std::string> words;
    std::string word;
    while (ss >> word)
        words.push_back(word);

    if (words.size() < _shingle_size)
    {
        if (_shingle_size == 1 && !words.empty())
            shingles.insert(std::hash<std::string>{}(words[0]));
        return shingles;
    }

    for (size_t i = 0; i <= words.size() - _shingle_size; ++i)
    {
        std::string shingle_text;
        for (int j = 0; j < _shingle_size; ++j)
            shingle_text += words[i + j] + " ";
        shingles.insert(std::hash<std::string>{}(shingle_text));
    }
    return shingles;
}

VSignature MinHashLSH::buildSignature(const std::set<NgramHash>& n_grams) const
{
    VSignature sig(_m_permutations, std::numeric_limits<uint32_t>::max());
    for (auto ngram_hash : n_grams)
    {
        for (int i = 0; i < _m_permutations; ++i)
        {
            uint32_t pi_val = static_cast<uint32_t>(
                (static_cast<unsigned __int128>(_pi_functions[i].a) * ngram_hash + _pi_functions[i].b) % kPrime
            );
            if (pi_val < sig[i])
                sig[i] = pi_val;
        }
    }
    return sig;
}

size_t MinHashLSH::getBandBucketHash(const VSignature& doc_signature, int band_idx) const
{
    int start_row = band_idx * _rows_per_band;

    std::string_view band_view(
        reinterpret_cast<const char*>(&doc_signature[start_row]),
        _rows_per_band * sizeof(uint32_t)
    );

    return std::hash<std::string_view>{}(band_view);
}

DocID MinHashLSH::addDocument(const std::string& text)
{
    DocID id = _next_id++;
    std::set<NgramHash> n_grams = buildNgramSet(text);
    VSignature doc_signature = buildSignature(n_grams);

    for (int i = 0; i < _bands_count; ++i)
    {
        size_t h_bucket = getBandBucketHash(doc_signature, i);
        _lsh_ht_tables[i][h_bucket].push_back(id);
    }

    _docs_n_grams[id] = std::move(n_grams);
    _docs_signatures[id] = std::move(doc_signature);

    return id;
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

std::set<DocID> MinHashLSH::findCandidates(const std::string& text) const
{
    auto n_grams = buildNgramSet(text);
    auto sig = buildSignature(n_grams);
    return findCandidatesInternal(sig);
}

std::set<DocID> MinHashLSH::findCandidatesById(DocID local_id) const
{
    auto it = _docs_signatures.find(local_id);
    if (it == _docs_signatures.end())
        return {};

    auto candidates = findCandidatesInternal(it->second);
    candidates.erase(local_id);
    return candidates;
}

std::vector<DocID> MinHashLSH::findDuplicatesFullScan(const std::string& text, double threshold) const
{
    std::vector<DocID> duplicates;
    auto query_ngrams = buildNgramSet(text);

    if (query_ngrams.empty())
        return duplicates;

    for (const auto& [id, stored_ngrams] : _docs_n_grams)
    {
        if (stored_ngrams.empty()) continue;

        std::vector<NgramHash> intersect;
        std::ranges::set_intersection(query_ngrams, stored_ngrams,
                                      std::back_inserter(intersect));

        double u_size = query_ngrams.size() + stored_ngrams.size() - intersect.size();
        double jaccard = (u_size == 0) ? 1.0 : (double)intersect.size() / u_size;

        if (jaccard >= threshold)
            duplicates.push_back(id);
    }
    return duplicates;
}
