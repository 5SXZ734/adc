// test.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <vector>

#include "../MyMemoryPool.h"

using namespace std;

void print(TMemoryPool<int>::Helper &H)
{
	for (TMemoryPool<int>::EltIterator i(H); i; i++)
		cout << *i << "|" << H.toSqueezedIndex(i.data()) << " ";
	cout << endl;
}

int main(int argc, char* argv[])
{
	TMemoryPool<int> M(4);

	vector<int *> v(15);
	for (size_t i(0); i < v.size(); i++)
	{
		v[i] = M.allocate();
		*(v[i]) = i;
	}

	M.deallocate(v[0]);
	//M.deallocate(v[2]);
	M.deallocate(v[5]);
	M.deallocate(v[6]);
	M.deallocate(v[11]);
	//M.deallocate(v[0]);
	M.deallocate(v[1]);
	//M.deallocate(v[3]);
	M.deallocate(v[4]);
	M.deallocate(v[14]);
//	v[2] = M.allocate();
//	*v[2] = 5;

	M.print_stats(cout);

	TMemoryPool<int>::Helper H(M);

	print(H);

	std::getchar();
	return 0;
}

