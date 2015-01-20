#ifndef BINHEAP_H
#define BINHEAP_H

#include "../platform.h"

class PathNode;

// http://www.sourcetricks.com/2011/06/c-heaps.html

class BinHeap
{
public:
	BinHeap(bool (*comparef)(void* a, void* b));
	~BinHeap();
	bool insert(void* element);
	void* deletemin();
	bool hasmore();
	void alloc(int ncells);
	void freemem();
	void resetelems();
	void heapify(void* element);
#if 0
	void print();
#endif
private:
	int left(int parent);
	int right(int parent);
	int parent(int child);
	void heapifyup(int index);
	void heapifydown(int index);
private:
#if 0
	PathNode** heap;
	int nelements;
	int allocsz;
#else
	std::vector<void*> heap;
	bool (*comparefunc)(void* a, void* b);
#endif
};

#endif
