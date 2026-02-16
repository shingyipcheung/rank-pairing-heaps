#include <algorithm>
#include <queue>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>
#include "rp_heap.h"
#include "pool_allocator.h"

static std::vector<int> make_random_ints(int n) {
    std::mt19937 rng(42);
    std::vector<int> v(n);
    for (int i = 0; i < n; i++)
        v[i] = rng();
    return v;
}

// ---------- rp_heap benchmarks ----------

static void BM_Push(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    for (auto _ : state) {
        rp_heap<int> heap;
        for (int i = 0; i < n; i++)
            heap.push(data[i]);
        benchmark::DoNotOptimize(heap.top());
    }
}
BENCHMARK(BM_Push)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_PopAll(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    for (auto _ : state) {
        rp_heap<int> heap;
        for (int i = 0; i < n; i++)
            heap.push(data[i]);
        while (!heap.empty())
            heap.pop();
    }
}
BENCHMARK(BM_PopAll)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_PushPop(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n * 2);
    for (auto _ : state) {
        rp_heap<int> heap;
        for (int i = 0; i < n; i++)
            heap.push(data[i]);
        for (int i = n; i < n * 2; i++) {
            heap.push(data[i]);
            heap.pop();
        }
        benchmark::DoNotOptimize(heap.size());
    }
}
BENCHMARK(BM_PushPop)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_DecreaseKey(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    std::mt19937 rng(123);
    std::vector<int> decrements(n);
    for (int i = 0; i < n; i++)
        decrements[i] = rng() % 1000 + 1;

    for (auto _ : state) {
        rp_heap<int> heap;
        std::vector<rp_heap<int>::const_iterator> its;
        its.reserve(n);
        for (int i = 0; i < n; i++)
            its.push_back(heap.push(data[i]));
        for (int i = 0; i < n; i++)
            heap.decrease(its[i], *its[i] - decrements[i]);
        benchmark::DoNotOptimize(heap.top());
    }
}
BENCHMARK(BM_DecreaseKey)->RangeMultiplier(10)->Range(1000, 1000000);

// ---------- rp_heap + pool_allocator benchmarks ----------

using PoolHeap = rp_heap<int, std::less<int>, pool_allocator<int>>;

static void BM_Pool_Push(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    for (auto _ : state) {
        PoolHeap heap;
        for (int i = 0; i < n; i++)
            heap.push(data[i]);
        benchmark::DoNotOptimize(heap.top());
    }
}
BENCHMARK(BM_Pool_Push)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_Pool_PopAll(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    for (auto _ : state) {
        PoolHeap heap;
        for (int i = 0; i < n; i++)
            heap.push(data[i]);
        while (!heap.empty())
            heap.pop();
    }
}
BENCHMARK(BM_Pool_PopAll)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_Pool_PushPop(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n * 2);
    for (auto _ : state) {
        PoolHeap heap;
        for (int i = 0; i < n; i++)
            heap.push(data[i]);
        for (int i = n; i < n * 2; i++) {
            heap.push(data[i]);
            heap.pop();
        }
        benchmark::DoNotOptimize(heap.size());
    }
}
BENCHMARK(BM_Pool_PushPop)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_Pool_DecreaseKey(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    std::mt19937 rng(123);
    std::vector<int> decrements(n);
    for (int i = 0; i < n; i++)
        decrements[i] = rng() % 1000 + 1;

    for (auto _ : state) {
        PoolHeap heap;
        std::vector<PoolHeap::const_iterator> its;
        its.reserve(n);
        for (int i = 0; i < n; i++)
            its.push_back(heap.push(data[i]));
        for (int i = 0; i < n; i++)
            heap.decrease(its[i], *its[i] - decrements[i]);
        benchmark::DoNotOptimize(heap.top());
    }
}
BENCHMARK(BM_Pool_DecreaseKey)->RangeMultiplier(10)->Range(1000, 1000000);

// ---------- std::priority_queue baselines ----------

static void BM_StdPQ_Push(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    for (auto _ : state) {
        std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
        for (int i = 0; i < n; i++)
            pq.push(data[i]);
        benchmark::DoNotOptimize(pq.top());
    }
}
BENCHMARK(BM_StdPQ_Push)->RangeMultiplier(10)->Range(1000, 1000000);

static void BM_StdPQ_PopAll(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    auto data = make_random_ints(n);
    for (auto _ : state) {
        std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
        for (int i = 0; i < n; i++)
            pq.push(data[i]);
        while (!pq.empty())
            pq.pop();
    }
}
BENCHMARK(BM_StdPQ_PopAll)->RangeMultiplier(10)->Range(1000, 1000000);
