## Rank pairing heaps

### Introduction
This is a **header-only** implementation of rank-pairing heaps (**rp-heap**) written in **C++11**. The idea of rp-heap is based on lazy [binomial queue](https://en.wikipedia.org/wiki/Binomial_heap) with the rank restriction to ensure the balance of half trees. Heap has wide applications like heapsort, k-smallest elements, Prim's algorithm and notably well-known Dijkstra's shortest-path algorithm. We are implementing this data structure because:

* [std::priority_queue](http://www.cplusplus.com/reference/queue/priority_queue/) does not support decrease-key
* [Fibonacci heap](https://en.wikipedia.org/wiki/Fibonacci_heap) is theoretically fast but not good in practice
* we are looking for an efficient 'decrease key' operation in pathfinding algorithm which is better than practical [d-ary heap](https://en.wikipedia.org/wiki/D-ary_heap) and [pairing heap](https://en.wikipedia.org/wiki/Pairing_heap)

In game development, [A* algorithm](https://en.wikipedia.org/wiki/A*_search_algorithm) is a standard shortest path algorithm. You can download this repository to build the solution and run the demo of A* pathfinding using rp-heap in MSVC.

### Usage
The implementation mimics STL containers and provides **STL-like** member functions. 
To use it, simply include the header file
**rank_pairing_heaps/astarheap/rp_heap.h**

##### Basic member functions
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

// for type 1 rank reduction (default type 2)
#define TYPE1_RANK_REDUCTION
```

##### Test program

```C++
#include <iostream>
#include <algorithm> // std::random_shuffle
#include "rp_heap.h"
#include <vector>

int main()
{
	std::vector<int> v;
	int size = 1000;
	for (int i = 0; i < size; i++)
		v.push_back(i + 1); // v = {1, 2,... 1000}
	std::random_shuffle(v.begin(), v.end()); // shuffle v
	
	rp_heap<int> heap;
	std::vector<rp_heap<int>::const_iterator> its; // save the iterators returned from push
	for (int i = 0; i < size; i++)
		its.push_back(heap.push(v[i]));
		
	heap.decrease(its[0], 0); // a number decreases to 0
	heap.pop(); // pop that number
	
	for (int i = 1; i < size; i++)
		heap.decrease(its[i], *its[i] - 1); // each element in heap is decreasd by 1
	while (!heap.empty())
	{
		int x;
		heap.pop(x);
		std::cout << x << '\n'; // will print the number from {1, 2,...999} but missing the one in the first pop
	}
	return 0;
}
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
* For detailed analysis of rp-heap, see [1]


### A* algorithm using rp-heap
A sample map taking from MMORPG [Strugarden NEO](http://www.strugarden.info/)


<img src="https://raw.githubusercontent.com/shingyipcheung/rank_pairing_heaps/master/png/map1.png" width="258" height="360" />


Transformed 2D grid Map


<img src="https://raw.githubusercontent.com/shingyipcheung/rank_pairing_heaps/master/png/map2.png" width="258" height="360" />


Shortest path by BFS (unweighted, 4-direction movement)


<img src="https://raw.githubusercontent.com/shingyipcheung/rank_pairing_heaps/master/png/map3.png" width="258" height="360" />


Shortest path by A* (weighted, 8-direction movement)


<img src="https://raw.githubusercontent.com/shingyipcheung/rank_pairing_heaps/master/png/map4.png" width="258" height="360" />



### References
[1] B. Haeupler, S. Sen, and R. E. Tarjan. Rank-pairing heaps. SIAM J. Comput., 40:1463–1485, 2011.


[2] Cherkassky, Boris V.; Goldberg, Andrew V.; Radzik, Tomasz (1996), "Shortest paths algorithms: theory and experimental evaluation", Mathematical Programming, Series A 73 (2): 129–174, doi:10.1016/0025-5610(95)00021-6, MR 1392160


[3] [Introduction to A*](http://theory.stanford.edu/~amitp/GameProgramming/AStarComparison.html)
