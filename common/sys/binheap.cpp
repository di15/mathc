#include "binheap.h"
#include "../path/pathnode.h"
#include "../utils.h"

BinHeap::BinHeap(bool (*comparef)(void* a, void* b))
{
#if 0
	heap = NULL;
	allocsz = 0;
	nelements = 0;
#endif
	comparefunc = comparef;
}

BinHeap::~BinHeap()
{
	freemem();
}

void BinHeap::alloc(int ncells)
{
#if 0
	freemem();

	heap = new PathNode* [ ncells ];
	allocsz = ncells;
	nelements = 0;
#endif
}

void BinHeap::freemem()
{
#if 0
	if(heap)
	{
		delete [] heap;
		heap = NULL;
	}

	allocsz = 0;
	nelements = 0;
#else
	heap.clear();
#endif
}

void BinHeap::resetelems()
{
#if 0
	nelements = 0;
#else
	heap.clear();
#endif
}

bool BinHeap::insert(void* element)
{
#if 0

	if(nelements >= allocsz)
		return false;

	heap[nelements] = element;
	nelements++;
	heapifyup(nelements - 1);
#else
	heap.push_back(element);
	heapifyup(heap.size() - 1);
#endif

	return true;
}

bool BinHeap::hasmore()
{
#if 0
	return nelements > 0;
#else
	return heap.size() > 0;
#endif
}

void* BinHeap::deletemin()
{
#if 0
	PathNode* pmin = heap[0];
	heap[0] = heap[nelements - 1];
	nelements--;
	heapifydown(0);
	return pmin;
#else
	void* pmin = heap.front();
	heap[0] = heap.at(heap.size() - 1);
	heap.pop_back();
	heapifydown(0);
	return pmin;
#endif
}

void BinHeap::heapify(void* element)
{
#if 1
	if(heap.size() <= 0)
		return;

	if(element)
	{
		bool found = false;
		int i = 0;

		for(auto iter = heap.begin(); iter != heap.end(); iter++, i++)
			if(*iter == element)
			{
				found = true;
				break;
			}

		if(found)
		{
			heapifydown(i);
			heapifydown(i);
		}
		return;
	}
	for(int i = (int)floor(heap.size()/2); i; i--)
	{
		heapifydown(i);
	}
	return;
#endif
}

#if 0
void BinHeap::print()
{
	std::vector<int>::iterator pos = heap.begin();
	cout << "BinHeap = ";
	while ( pos != heap.end() )
	{
		cout << *pos << " ";
		++pos;
	}
	cout << std::endl;
}
#endif

void BinHeap::heapifyup(int index)
{
	//cout << "index=" << index << std::endl;
	//cout << "parent(index)=" << parent(index) << std::endl;
	//cout << "heap[parent(index)]=" << heap[parent(index)] << std::endl;
	//cout << "heap[index]=" << heap[index] << std::endl;
	while ( ( index > 0 ) && ( parent(index) >= 0 ) && comparefunc(heap[parent(index)], heap[index])
	                /* ( heap[parent(index)]->score > heap[index]->score ) */ )
	{
		void* tmp = heap[parent(index)];
		heap[parent(index)] = heap[index];
		heap[index] = tmp;
		index = parent(index);
	}
}

void BinHeap::heapifydown(int index)
{
	//cout << "index=" << index << std::endl;
	//cout << "left(index)=" << left(index) << std::endl;
	//cout << "right(index)=" << right(index) << std::endl;
	int child = left(index);
	if ( ( child > 0 ) && ( right(index) > 0 ) && comparefunc(heap[child], heap[right(index)])
	              /*  ( heap[child]->score > heap[right(index)]->score ) */ )
	{
		child = right(index);
	}
	if ( child > 0 )
	{
		void* tmp = heap[index];
		heap[index] = heap[child];
		heap[child] = tmp;
		heapifydown(child);
	}
}

int BinHeap::left(int parent)
{
	int i = ( parent << 1 ) + 1; // 2 * parent + 1
#if 0
	return ( i < nelements ) ? i : -1;
#else
	return ( i < heap.size() ) ? i : -1;
#endif
}

int BinHeap::right(int parent)
{
	int i = ( parent << 1 ) + 2; // 2 * parent + 2
#if 0
	return ( i < nelements ) ? i : -1;
#else
	return ( i < heap.size() ) ? i : -1;
#endif
}

int BinHeap::parent(int child)
{
	if (child != 0)
	{
		int i = (child - 1) >> 1;
		return i;
	}
	return -1;
}

#if 0
int main()
{
	// Create the heap
	BinHeap* myheap = new BinHeap();
	myheap->insert(700);
	myheap->print();
	myheap->insert(500);
	myheap->print();
	myheap->insert(100);
	myheap->print();
	myheap->insert(800);
	myheap->print();
	myheap->insert(200);
	myheap->print();
	myheap->insert(400);
	myheap->print();
	myheap->insert(900);
	myheap->print();
	myheap->insert(1000);
	myheap->print();
	myheap->insert(300);
	myheap->print();
	myheap->insert(600);
	myheap->print();

	// Get priority element from the heap
	int heapSize = myheap->size();
	for ( int i = 0; i < heapSize; i++ )
		cout << "Get std::min element = " << myheap->deletemin() << std::endl;

	// Cleanup
	delete myheap;
}
#endif
