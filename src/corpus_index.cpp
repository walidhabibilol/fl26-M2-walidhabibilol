#include "aiws/corpus_index.hpp"
#include "aiws/text_processor.hpp"

#include <stdexcept>
#include <unordered_map>

namespace aiws {

CorpusIndex::CorpusIndex(const std::vector<Chunk>& chunks) {
    build(chunks);
}

void CorpusIndex::build(const std::vector<Chunk>& chunks) {
    // TODO: build the searchable index from the supplied chunks.

    postings_.clear();
    chunk_by_id_.clear();
    for (std::size_t i = 0; i < chunks.size(); ++i) {
        if (!chunk_by_id_.emplace(chunks[i].id, i).second)
            throw std::invalid_argument("duplicate chunk id");
        std::unordered_map<std::string, std::size_t> counts;
        for (const auto& term : TextProcessor::terms(chunks[i].text)) {
            ++counts[term];
        }
        for (const auto& kv : counts) {
            postings_[kv.first].push_back(Posting{i, kv.second});
        }
    }
}

std::size_t CorpusIndex::document_frequency(
    const std::string& term) const noexcept {
    // TODO: return how many chunks contain the requested term.

    auto it = postings_.find(term);
    return it == postings_.end() ? 0 : it->second.size();
}

std::size_t CorpusIndex::term_frequency(
    const std::string& term, 
    const std::string& chunk_id) const noexcept {
    // TODO: return the requested term's frequency in the specified chunk.
    
    auto ci = chunk_by_id_.find(chunk_id);
    if (ci == chunk_by_id_.end()) return 0;
    auto pi = postings_.find(term);
    if (pi == postings_.end()) return 0;
    for (const auto& p : pi->second){
        if (p.chunk_index == ci->second){

            return p.frequency;
        }
    }
    return 0;
}

const std::vector<CorpusIndex::Posting>* CorpusIndex::postings(
    const std::string& term) const noexcept {
    // TODO: return the postings associated with the requested term.
        auto it = postings_.find(term);
        return it == postings_.end() ? nullptr : &it->second;
}

const Chunk* CorpusIndex::find_chunk(
    const std::vector<Chunk>& chunks, 
    const std::string& chunk_id) const noexcept {
    // TODO: find the chunk identified by the requested chunk ID.
    auto it = chunk_by_id_.find(chunk_id);
    if (it == chunk_by_id_.end() || it->second >= chunks.size()) return nullptr;
    return &chunks[it->second];
}

std::size_t CorpusIndex::chunk_index(const std::string& chunk_id) const {
    // TODO: return the stored index of the requested chunk ID.
    auto it = chunk_by_id_.find(chunk_id);
    if (it == chunk_by_id_.end()) throw std::out_of_range("unknown chunk id");
    return it->second;
}

}  // namespace aiws
