#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace aiws {

double RetrievalEngine::canonical_score(double value) {
    // TODO: return the score in the required canonical form.

    return std::round(value * 1.0e12) / 1.0e12;
}


std::vector<SearchResult> RetrievalEngine::search(
    // TODO: return the ranked search results for the requested query.
    
    const std::string& query,
    int k,
    const std::vector<Chunk>& chunks,
    const CorpusIndex& index) const {

    if (k < 0) {
        throw std::invalid_argument("k must be nonnegative");
    }

    if (k == 0) {
        return {};
    }

    std::vector<std::string> all_terms =
        TextProcessor::terms(query);

    std::vector<std::string> query_terms;

    // Remove duplicate query terms
    for (const auto& term : all_terms) {

        bool already_added = false;

        for (const auto& saved_term : query_terms) {
            if (term == saved_term) {
                already_added = true;
            }
        }

        if (!already_added) {
            query_terms.push_back(term);
        }
    }

    if (query_terms.empty() || chunks.empty()) {
        return {};
    }


    // Keep track of scores for each chunk
    std::vector<double> scores(chunks.size(), 0.0);
    std::vector<std::size_t> matched(chunks.size(), 0);


    // Calculate score for each query term
    for (const auto& term : query_terms) {

        const auto* postings = index.postings(term);

        if (postings != nullptr) {

            double n = static_cast<double>(chunks.size());

            double df =
                static_cast<double>(postings->size());

            double idf =
                std::log((n + 1.0) / (df + 1.0)) + 1.0;


            for (const auto& posting : *postings) {

                std::size_t chunk_index =
                    posting.chunk_index;

                double frequency =
                    static_cast<double>(posting.frequency);

                double tf =
                    1.0 + std::log(frequency);

                scores[chunk_index] += tf * idf;
                matched[chunk_index]++;
            }
        }
    }


    std::vector<SearchResult> results;


    // Create search results
    for (std::size_t i = 0; i < chunks.size(); i++) {

        if (matched[i] > 0) {

            double coverage =
                1.0 +
                0.10 *
                static_cast<double>(matched[i]) /
                static_cast<double>(query_terms.size());

            double final_score =
                scores[i] * coverage;

            final_score =
                canonical_score(final_score);

            results.push_back(
                SearchResult{
                    chunks[i].id,
                    chunks[i].document_id,
                    chunks[i].sequence,
                    chunks[i].text,
                    final_score,
                    matched[i]
                }
            );
        }
    }


    // Sort highest score first
    std::sort(
        results.begin(),
        results.end(),
        [&](const SearchResult& a, const SearchResult& b) {

            if (a.score != b.score) {
                return a.score > b.score;
            }

            std::size_t a_index =
                index.chunk_index(a.chunk_id);

            std::size_t b_index =
                index.chunk_index(b.chunk_id);

            if (chunks[a_index].document_order !=
                chunks[b_index].document_order) {

                return chunks[a_index].document_order <
                       chunks[b_index].document_order;
            }

            return a.chunk_sequence <
                   b.chunk_sequence;
        }
    );


    if (results.size() > static_cast<std::size_t>(k)) {
        results.resize(static_cast<std::size_t>(k));
    }

    return results;
}

} // namespace aiws