#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include <array>
#include <vector>

using namespace std;
// Explicit definition, this actually allocates
// as well as describing.
// This makes use of an extern variable, though we could just put this value
// into our private portion of our class.

template <typename T>
struct Node
{
	T data;
	struct Node<T> *next;

	Node():
		data(0), next(NULL) {}
    Node(int _data, Node* _next) :
		data(_data), next(_next) {}
};

template <typename T>
class LinkedList
{
private:
	int node_bank_index_;

	// Assignment says only to spawn 4 threads. So we make this
	// only 4 threads for simplicity of dealing with a node bank.
	const static int kNumThreads = 4;

	const static int kMaxNodeBankSize = (int) 1e4;

	// Have to allocate a bunch of nodes in the constructor.
	array<vector<Node<T>*>, kNumThreads> node_bank_;
	Node<T>* head;

	// The important functions that are relative to the assignment.
	Node<T>* Search(T data);
	Node<T>* Remove(T data);

	// Just nice to have functions.
	Node<T>* DestroyList(Node<T> *head);

public:

	// Have a construct for allocating nodebank for number of kNumThreads
	LinkedList();

	~LinkedList();

	// The important functions that are relative to the assignment.
	void Insert(T data, int thread_id);

	bool Find(T data);

	bool Delete(T data, int thread_id);

	// Just nice functions to have.
	void DestroyList();

	void Print();

};

#endif
