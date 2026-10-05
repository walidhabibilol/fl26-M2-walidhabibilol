#include "aiws/chunking_strategy.hpp"
#include "aiws/context_strategy.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/document.hpp"
#include "aiws/processing_core.hpp"
#include "aiws/retrieval_strategy.hpp"
#include "aiws/workspace.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace aiws;


// test Chunking strategy
class TestChunking : public ChunkingStrategy {
public:
    std::vector<Chunk> chunk(const Document& document,
                             std::size_t order) const override {
        return {{
            document.id() + "#test",
            document.id(),
            order,
            0,
            "test " + document.id(),
            2,
            0,
            document.text().size()
        }};
    }
};


// test retrieval strategy
class TestRetrieval : public RetrievalStrategy {
public:
    std::vector<SearchResult> search(
        const std::string&,
        int k,
        const std::vector<Chunk>& chunks,
        const CorpusIndex&) const override {

        if (k <= 0 || chunks.empty()) {
            return {};
        }

        const auto& chunk = chunks.back();

        return {{
            chunk.id,
            chunk.document_id,
            chunk.sequence,
            chunk.text,
            100.0,
            1
        }};
    }
};


// test context strategy
class TestContext : public ContextStrategy {
public:
    std::vector<ContextItem> build(
        const std::vector<SearchResult>& results,
        std::size_t token_budget) const override {

        if (results.empty() || token_budget == 0) {
            return {};
        }

        const auto& result = results.front();

        return {{
            result.chunk_id,
            result.document_id,
            result.chunk_sequence,
            "custom context",
            2,
            result.score,
            false
        }};
    }
};


void test_custom_strategies() {
    ProcessingCore core(
        std::make_unique<TestChunking>(),
        std::make_unique<TestRetrieval>(),
        std::make_unique<TestContext>());

    Workspace workspace;
    workspace.add_document(Document{"one", "", "hello world"});
    workspace.add_document(Document{"Two", "", "good morning"});

    core.rebuild(workspace);

    // Shows chunking implementation
    assert(core.chunks().size() == 2);
    assert(core.chunks()[0].id == "one#test");


    // shows retreival implementation
    auto results = core.search("hello", 1);

    assert(results.size() == 1);
    assert(results[0].document_id == "Two");
    assert(results[0].score == 100.0);

    // Shows context sraetgy implementation
    auto context = core.build_context("hello", 1, 5);

    assert(context.size() == 1);
    assert(context[0].text == "custom context");
}


void test_null_strategy() {
    bool threw = false;

    try {
        ProcessingCore core(
            nullptr,
            std::make_unique<TestRetrieval>(),
            std::make_unique<TestContext>());
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}


void test_move() {
    ProcessingCore core1(
        std::make_unique<TestChunking>(),
        std::make_unique<TestRetrieval>(),
        std::make_unique<TestContext>());

    ProcessingCore core2(std::move(core1));

    Workspace workspace;
    workspace.add_document(Document{"move", "", "hello world"});

    core2.rebuild(workspace);
    assert(core2.chunk_count() == 1);
    assert(core2.chunks()[0].id == "move#test");
}


void test_failed_rebuild() {
    ProcessingCore core;

    Workspace good;
    good.add_document(Document{"good", "", "hello world"});
    core.rebuild(good);

    assert(core.chunk_count() == 1);

    Workspace bad;
    bad.add_document(Document{"same", "", "one"});
    bad.add_document(Document{"same", "", "two"});

    bool threw = false;

    try {
        core.rebuild(bad);
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }

    // Invalid rebuild should throw
    assert(threw);

    // Old valid corpus should still exist
    assert(core.chunk_count() == 1);
    assert(core.chunks()[0].document_id == "good");
}


void test_default_behavior() {
    ProcessingCore core;

    Workspace workspace;
    workspace.add_document(
        Document{"doc", "", "Hello, WORLD! Test"});

    core.rebuild(workspace);

    // M1 normalization
    assert(
        ProcessingCore::normalize(
            "Hello, WORLD! Test") == "hello world test");

    // M1 budget behavior
    auto context =
        core.build_context("hello", 3, 3);

    assert(context.size() == 1);
    assert(context[0].text == "hello world test");
    assert(context[0].token_count == 3);
    assert(!context[0].truncated);
}


int main() {
    test_custom_strategies();
    test_null_strategy();
    test_move();
    test_failed_rebuild();
    test_default_behavior();

    return 0;
}