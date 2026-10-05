#include "aiws/chunker.hpp"
#include "aiws/text_processor.hpp"

#include <stdexcept>

namespace aiws {

Chunker::Chunker(ChunkingPolicy policy) : policy_(policy) {
    if (policy_.max_tokens == 0 || policy_.overlap >= policy_.max_tokens ||
        policy_.paragraph_window > policy_.max_tokens) {
        throw std::invalid_argument("invalid chunking policy");
    }
}

std::vector<Chunk> Chunker::chunk(const Document& document, std::size_t document_order) const {
    // TODO: produce deterministic, source-attributed chunks for the supplied document.


    const auto tokens = TextProcessor::tokenize(document.text());
    std::vector<Chunk> chunks;
    if (tokens.empty()) return chunks;

    std::size_t start = 0;
    std::size_t sequence = 0;
    while (start < tokens.size()) {
        std::size_t end = tokens.size();

        if (tokens.size() - start > policy_.max_tokens) {
            end = start + policy_.max_tokens;

            const std::size_t earliest = start + (policy_.max_tokens - policy_.paragraph_window);

            for (std::size_t p = end; p >= earliest; --p) {
                if (p < tokens.size() &&
                    tokens[p].paragraph != tokens[p - 1].paragraph) {
                    end = p;
                    break;
                }

                if (p == earliest) {
                    break;
                }
            }
        }

        Chunk c;
        c.id = document.id() + "#" + std::to_string(sequence);
        c.document_id = document.id();
        c.document_order = document_order;
        c.sequence = sequence;
        c.text = TextProcessor::join(tokens, start, end);
        c.token_count = end - start;
        c.source_begin = tokens[start].begin;
        c.source_end = tokens[end - 1].end;
        chunks.push_back(std::move(c));

        if (end == tokens.size()) {
            break;
        }
        start = end - policy_.overlap;
        ++sequence;
    }
    return chunks;
}

}  // namespace aiws
