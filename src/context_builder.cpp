#include "aiws/context_builder.hpp"
#include "aiws/text_processor.hpp"

#include <unordered_set>

namespace aiws {

std::vector<ContextItem> ContextBuilder::build(const std::vector<SearchResult>& ranked, std::size_t token_budget) const {
    // TODO: assemble ranked results into context items within the supplied token budget.

    std::vector<ContextItem> out;
    if (token_budget == 0) {
        return out;
    }
    std::size_t remaining = token_budget;
    std::unordered_set<std::string> used;
    for (const auto& r : ranked) {
        if (!used.insert(r.chunk_id).second) {
            continue;
        }
        const auto terms = TextProcessor::terms(r.text);
        if (terms.empty()) {
            continue;
        }
        const std::size_t take = terms.size() < remaining ? terms.size() : remaining;
        const bool truncated = take < terms.size();
        out.push_back(ContextItem{r.chunk_id, r.document_id, r.chunk_sequence,
                                  TextProcessor::join(terms, 0, take), take, r.score, truncated});
        remaining -= take;
        if (truncated || remaining == 0) {
            break;
        }
    }
    return out;
}

}  // namespace aiws
