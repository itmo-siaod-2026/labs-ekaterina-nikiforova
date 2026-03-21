#include "min_lsh_hash.h"

#include <algorithm>

MinHashLSH::MinHashLSH(int m_permutations, int b_bands, int n_gram_size)
    : _m_permutations(m_permutations), _bands_count(b_bands),
      _n_gram_size(n_gram_size), _next_id(0)
{
    _rows_per_band = _m_permutations / _bands_count;
    _lsh_ht_tables.resize(_bands_count);
    initPermutations();
}

void MinHashLSH::initPermutations()
{
    std::mt19937 gen(1337);
    std::uniform_int_distribution<uint64_t> dist(1, kPrime - 1);
    for (int i = 0; i < _m_permutations; ++i)
        _pi_functions.push_back({dist(gen), dist(gen)});
}

std::set<NgramHash> MinHashLSH::buildNgramSet(const std::string& text) const
{
    std::set<NgramHash> n_grams;
    if (text.length() < _n_gram_size)
        return n_grams;

    for (size_t i = 0; i <= text.length() - _n_gram_size; ++i)
    {
        std::string n_gram = text.substr(i, _n_gram_size);
        uint32_t h = static_cast<uint32_t>(std::hash<std::string>{}(n_gram));
        n_grams.insert(h);
    }
    return n_grams;
}

VSignature MinHashLSH::buildSignature(const std::set<NgramHash>& n_grams) const
{
    VSignature sig(_m_permutations, std::numeric_limits<uint32_t>::max());
    for (auto ngram_hash : n_grams)
    {
        for (int i = 0; i < _m_permutations; ++i)
        {
            uint32_t pi_val = static_cast<uint32_t>(
                (_pi_functions[i].a * ngram_hash + _pi_functions[i].b) % kPrime
            );
            if (pi_val < sig[i])
                sig[i] = pi_val;
        }
    }
    return sig;
}

size_t MinHashLSH::getBandBucketHash(const std::vector<uint32_t>& doc_signature, int band_idx) const
{
    int start_row = band_idx * _rows_per_band;
    if (start_row + _rows_per_band > doc_signature.size())
        return 0;

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
    // _docs_n_grams[id] = n_grams;

    VSignature doc_signature = buildSignature(n_grams);
    _docs_signatures[id] = doc_signature;

    for (int i = 0; i < _bands_count; ++i)
    {
        size_t h_bucket = getBandBucketHash(doc_signature, i);
        _lsh_ht_tables[i][h_bucket].push_back(id);
    }
    return id;
}

std::set<DocID> MinHashLSH::findCandidatesById(DocID local_id) const
{
    std::set<DocID> candidates;

    auto it_sig = _docs_signatures.find(local_id);
    if (it_sig == _docs_signatures.end())
        return candidates;

    const VSignature& sig = it_sig->second;

    for (int i = 0; i < _bands_count; ++i)
    {
        size_t h_bucket = getBandBucketHash(sig, i);

        auto it_table = _lsh_ht_tables[i].find(h_bucket);
        if (it_table != _lsh_ht_tables[i].end())
        {
            for (DocID cand_id : it_table->second)
            {
                if (cand_id != local_id)
                    candidates.insert(cand_id);
            }
        }
    }
    return candidates;
}

