## Rank pairing heaps

### Introduction
This is a **header-only** implementation of rank-pairing heaps (**rp-heap**) in **C++11**. The idea of rp-heap is based on lazy [binomial queue](https://en.wikipedia.org/wiki/Binomial_heap) with rank restriction to ensure the balance of half trees. Heap has wide applications like heapsort, k-smallest elements, Prim's algorithm and well-known Dijkstra's shortest-path algorithm. We are implementing this data structure because:

* [std::priority_queue](http://www.cplusplus.com/reference/queue/priority_queue/) does not support the decrease-key operation.
* [Fibonacci heap](https://en.wikipedia.org/wiki/Fibonacci_heap) is theoretical fast only
* The-state-of-art heap
* Looking for a efficient decrease_key operation in pathfinding which is better than practical [d-ary heap](https://en.wikipedia.org/wiki/D-ary_heap) and [pairing heap](https://en.wikipedia.org/wiki/Pairing_heap)

In game development, [A* algorithm](https://en.wikipedia.org/wiki/A*_search_algorithm) is a standard shortest path algorithm. We will have a demo using rp-heap with A* in the comming part.

### Usage
The implementation mimics STL containers and provides **STL-like** member functions. 
You can simply download this header file:
**rank_pairing_heaps/astarheap/rp_heap.h**
```C++
// make heap
rp_heap(const _Pr& _Pred = _Pr());
// capacity
bool empty() const;
size_type size() const;
// find-min
const_reference top() const;
// insert
const_iterator push(const value_type& _Val);
const_iterator push(value_type&& x);
// delete-min
void pop();
void pop(value_type& _Val);
// delete-all
void clear();
// decrease-key
void decrease(const_iterator _It, const value_type& _Val);
```

### Performance
| Operations    | Amortized time|
| ------------- |:-------------:|
|find-min|*O*(1)|
|delete-min|*O*(*log* n)|
|insert|*O*(1)|
|decrease-key|*O*(1)|
|size|*O*(1)|
|delete-all|*O*(n)|
* Detailed Analysis of rp-heap please refer to [1]

### Simple demo

```C++
#include <iostream>
#include <algorithm> // std::random_shuffle
#include "rp_heap.h"
#include <vector>

int main()
{
	std::vector<int> v;
	std::vector<rp_heap<int>::const_iterator> iter_v;
	rp_heap<int> heap;
	int size = 1000;
	for (int i = 0; i < size; i++)
		v.push_back(i + 1);
	std::random_shuffle(v.begin(), v.end());
	for (int i = 0; i < size; i++)
		iter_v.push_back(heap.push(v[i]));
	heap.decrease(iter_v[0], 0);
	heap.pop();
	for (int i = 1; i < size; i++)
		heap.decrease(iter_v[i], *iter_v[i] - 1);
	while (!heap.empty())
	{
		int x;
		heap.pop(x);
		std::cout << x << '\n';
	}
	return 0;
}
```

## Use rp-heap in A* algorithm

## References
[1] B. Haeupler, S. Sen, and R. E. Tarjan. Rank-pairing heaps. SIAM J. Comput., 40:1463–1485, 2011.

## Later updates
I tested the A* algorithm with several data structures including binary heap, d-ary heap, pairing heap, and rp-heap. It is found that rp-heap is the fastest and outperforms pairing heap. In later updates, I will complete the functions including iterator, meld, delete and finally give the benchmark of comparision of different heap data structures.
