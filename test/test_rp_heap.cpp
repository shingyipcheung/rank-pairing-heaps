#include <gtest/gtest.h>
#include "rp_heap.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <random>
#include <vector>

// ---------- counting allocator for leak detection ----------

static std::atomic<int> g_alloc_count{0};
static std::atomic<int> g_dealloc_count{0};

template <class T>
struct CountingAllocator {
    using value_type = T;

    CountingAllocator() noexcept = default;
    template <class U>
    CountingAllocator(const CountingAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        g_alloc_count.fetch_add(static_cast<int>(n));
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        g_dealloc_count.fetch_add(static_cast<int>(n));
        ::operator delete(p);
    }

    template <class U>
    bool operator==(const CountingAllocator<U>&) const noexcept { return true; }
    template <class U>
    bool operator!=(const CountingAllocator<U>&) const noexcept { return false; }
};

static void reset_counters() {
    g_alloc_count.store(0);
    g_dealloc_count.store(0);
}

// ---------- basic operations ----------

TEST(RpHeap, PushTopSizeEmpty) {
    rp_heap<int> h;
    EXPECT_TRUE(h.empty());
    EXPECT_EQ(h.size(), 0u);

    h.push(42);
    EXPECT_FALSE(h.empty());
    EXPECT_EQ(h.size(), 1u);
    EXPECT_EQ(h.top(), 42);

    h.push(10);
    EXPECT_EQ(h.size(), 2u);
    EXPECT_EQ(h.top(), 10);

    h.push(99);
    EXPECT_EQ(h.size(), 3u);
    EXPECT_EQ(h.top(), 10);
}

TEST(RpHeap, PopExtractMinOrdering) {
    rp_heap<int> h;
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(0, 100000);

    const int N = 200;
    std::vector<int> vals;
    vals.reserve(N);
    for (int i = 0; i < N; ++i) {
        int v = dist(rng);
        vals.push_back(v);
        h.push(v);
    }

    std::sort(vals.begin(), vals.end());

    for (int i = 0; i < N; ++i) {
        ASSERT_FALSE(h.empty());
        EXPECT_EQ(h.top(), vals[i]);
        h.pop();
    }
    EXPECT_TRUE(h.empty());
}

TEST(RpHeap, PopWithOutputParameter) {
    rp_heap<int> h;
    h.push(5);
    h.push(3);
    h.push(7);

    int val = -1;
    h.pop(val);
    EXPECT_EQ(val, 3);

    h.pop(val);
    EXPECT_EQ(val, 5);

    h.pop(val);
    EXPECT_EQ(val, 7);

    EXPECT_TRUE(h.empty());
}

TEST(RpHeap, PopOnEmptyThrows) {
    rp_heap<int> h;
    EXPECT_THROW(h.pop(), std::runtime_error);

    int val;
    EXPECT_THROW(h.pop(val), std::runtime_error);
}

TEST(RpHeap, Clear) {
    rp_heap<int> h;
    for (int i = 0; i < 50; ++i)
        h.push(i);

    EXPECT_EQ(h.size(), 50u);
    h.clear();
    EXPECT_TRUE(h.empty());
    EXPECT_EQ(h.size(), 0u);
}

// ---------- decrease key ----------

TEST(RpHeap, DecreaseKeyBasic) {
    rp_heap<int> h;
    h.push(10);
    auto it = h.push(20);
    h.push(30);

    EXPECT_EQ(h.top(), 10);

    h.decrease(it, 5);
    EXPECT_EQ(h.top(), 5);
}

TEST(RpHeap, DecreaseKeyOnRoot) {
    rp_heap<int> h;
    auto it = h.push(10);
    h.push(20);
    h.push(30);

    EXPECT_EQ(h.top(), 10);

    h.decrease(it, 1);
    EXPECT_EQ(h.top(), 1);
}

TEST(RpHeap, DecreaseKeyNonRootBecomesMin) {
    rp_heap<int> h;
    h.push(5);
    h.push(10);
    auto it = h.push(15);
    h.push(20);

    // Force some internal structure by popping and re-inserting
    EXPECT_EQ(h.top(), 5);

    h.decrease(it, 1);
    EXPECT_EQ(h.top(), 1);

    // Verify full ordering still works after decrease
    std::vector<int> result;
    while (!h.empty()) {
        result.push_back(h.top());
        h.pop();
    }
    EXPECT_TRUE(std::is_sorted(result.begin(), result.end()));
}

// ---------- large random test ----------

TEST(RpHeap, LargeRandomSortedOrder) {
    rp_heap<int> h;
    std::mt19937 rng(98765);
    std::uniform_int_distribution<int> dist(-1000000, 1000000);

    const int N = 10000;
    for (int i = 0; i < N; ++i)
        h.push(dist(rng));

    EXPECT_EQ(h.size(), static_cast<size_t>(N));

    std::vector<int> result;
    result.reserve(N);
    while (!h.empty()) {
        result.push_back(h.top());
        h.pop();
    }
    EXPECT_EQ(static_cast<int>(result.size()), N);
    EXPECT_TRUE(std::is_sorted(result.begin(), result.end()));
}

// ---------- custom comparator (max-heap) ----------

TEST(RpHeap, CustomComparatorMaxHeap) {
    rp_heap<int, std::greater<int>> h;
    h.push(10);
    h.push(30);
    h.push(20);

    EXPECT_EQ(h.top(), 30);
    h.pop();
    EXPECT_EQ(h.top(), 20);
    h.pop();
    EXPECT_EQ(h.top(), 10);
    h.pop();
    EXPECT_TRUE(h.empty());
}

// ---------- move semantics ----------

TEST(RpHeap, MoveSemantics) {
    rp_heap<std::string> h;
    std::string s = "hello";
    h.push(std::move(s));
    EXPECT_EQ(h.top(), "hello");
    EXPECT_EQ(h.size(), 1u);
}

// ---------- memory leak tests ----------

TEST(RpHeapMemory, DestructorFreesAll) {
    reset_counters();
    {
        rp_heap<int, std::less<int>, CountingAllocator<int>> h;
        for (int i = 0; i < 100; ++i)
            h.push(i);
    } // destructor calls clear()
    EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
    EXPECT_GT(g_alloc_count.load(), 0);
}

TEST(RpHeapMemory, PopAllFreesAll) {
    reset_counters();
    {
        rp_heap<int, std::less<int>, CountingAllocator<int>> h;
        for (int i = 0; i < 100; ++i)
            h.push(i);
        while (!h.empty())
            h.pop();
    }
    EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
}

TEST(RpHeapMemory, ClearFreesAll) {
    reset_counters();
    {
        rp_heap<int, std::less<int>, CountingAllocator<int>> h;
        for (int i = 0; i < 100; ++i)
            h.push(i);
        h.clear();
        EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
        // reuse after clear
        for (int i = 0; i < 50; ++i)
            h.push(i);
    }
    EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
}

TEST(RpHeapMemory, PartialPopThenDestruct) {
    reset_counters();
    {
        rp_heap<int, std::less<int>, CountingAllocator<int>> h;
        for (int i = 0; i < 200; ++i)
            h.push(i);
        // pop only half
        for (int i = 0; i < 100; ++i)
            h.pop();
        // destructor frees the rest
    }
    EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
}

TEST(RpHeapMemory, DecreaseKeyNoLeak) {
    reset_counters();
    {
        rp_heap<int, std::less<int>, CountingAllocator<int>> h;
        std::vector<decltype(h.push(0))> iters;
        for (int i = 0; i < 100; ++i)
            iters.push_back(h.push(i * 10));

        // pop a few to build internal tree structure
        for (int i = 0; i < 10; ++i)
            h.pop();

        // decrease several keys
        for (int i = 50; i < 60; ++i)
            h.decrease(iters[i], -i);

        // drain the rest
        while (!h.empty())
            h.pop();
    }
    EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
}

TEST(RpHeapMemory, LargeRandomNoLeak) {
    reset_counters();
    {
        rp_heap<int, std::less<int>, CountingAllocator<int>> h;
        std::mt19937 rng(55555);
        std::uniform_int_distribution<int> dist(0, 1000000);
        for (int i = 0; i < 10000; ++i)
            h.push(dist(rng));
        while (!h.empty())
            h.pop();
    }
    EXPECT_EQ(g_alloc_count.load(), g_dealloc_count.load());
}
