# M2 Technical & Design Understanding

## 1. Polymorphism and dynamic dispatch

`ProcessingCore` uses strategy base classes so it can use different implementations at runtime.

For example, `ProcessingCore::search()` calls:

`impl_->retrieval->search(...)`

`retrieval` is a `std::unique_ptr<RetrievalStrategy>`. Since `search()` is virtual, C++ calls the correct derived version at runtime.

With the default constructor, the object is `RetrievalEngine`. In my student test, it can instead be `TestRetrieval`.

If `search()` was not virtual, the selected derived strategy would not be used correctly.

---

## 2. Ownership and lifetime

`ProcessingCore` owns the three strategy objects using `std::unique_ptr`.

The strategies are passed into the constructor and moved into `ProcessingCore::Impl` using `std::move`.

This means `ProcessingCore` becomes the owner of the strategies and they are automatically destroyed when `ProcessingCore` is destroyed.

`ProcessingCore` is move-only because `std::unique_ptr` cannot be copied. Moving transfers ownership safely.

The strategy base classes also need virtual destructors so derived objects can be destroyed correctly through base-class pointers.

---

## 3. Architecture, extensibility, and M1 compatibility

M2 changes `ProcessingCore` so it uses strategy interfaces instead of fixed processing objects.

The default constructor still creates:

- `Chunker`
- `RetrievalEngine`
- `ContextBuilder`

This keeps the original M1 behavior.

The second constructor allows different strategies to be passed in.

`rebuild()` uses the chunking strategy, `search()` uses the retrieval strategy, and `build_context()` uses the context strategy.

This makes the program easier to extend because new strategies can be added without changing the main `ProcessingCore` interface.

---

## 4. Testing and defect reasoning

My `test_custom_strategies()` test checks that custom strategies are actually used.

It gives `ProcessingCore`:

- `TestChunking`
- `TestRetrieval`
- `TestContext`

The test checks for custom behavior such as:

- chunk IDs ending in `#test`
- retrieval score being `100.0`
- context text being `"custom context"`

If `ProcessingCore` was still using the default M1 classes directly, these results would not appear.

I also test null strategies, moving a `ProcessingCore`, failed rebuilds, and default M1 behavior.