## Implementation of rank-pairing heaps in C++

### Introduction

### Usage

### Performance

### Application
```C++

#include <iostream>
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
		cout << x << endl;
	}
	return 0;
}

```

## References

