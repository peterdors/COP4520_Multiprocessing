// Peter Dorsaneo

// LinkedList.cpp
// ===============
// A nice way of practicing how to use generics on data structures in C++.

#include <iostream>
#include "LinkedList.hpp"

using namespace std;

template <typename T>
LinkedList<T>::LinkedList()
{
	head = nullptr;

	// Preallocate and fill the node bank up for our use.
	for (int i = 0; i < kNumThreads; i++)
	{
		for (int k = 0; k < kMaxNodeBankSize; k++)
		{
			node_bank_[i].push_back(new Node<T>());
		}
	}
}

template <typename T>
LinkedList<T>::~LinkedList()
{
	// Not to worry about that...
}

// Insertion at the head of the list.
template <typename T>
void LinkedList<T>::Insert(T data, int thread_id)
{
	// head = insert(head, data);

	if (head == nullptr)
	{
		head = node_bank_[thread_id].back();
		node_bank_[thread_id].pop_back();

		head->data = data;
		return;
	}

	// Insert at the head of the list.
	Node<T> *newHead = node_bank_[thread_id].back();
	node_bank_[thread_id].pop_back();

	newHead->data = data;
	newHead->next = head;

	head = newHead;
}

template <typename T>
Node<T>* LinkedList<T>::Search(T data)
{
	if (head == nullptr)
		return nullptr;

	Node<T> *sought = head;

	while (sought != nullptr && sought->data != data)
	{
		sought = sought->next;
	}

	return sought;
}


template <typename T>
bool LinkedList<T>::Find(T data)
{
	return (Search(data) == nullptr) ? false : true;
}

template <typename T>
bool LinkedList<T>::Delete(T data, int thread_id)
{
	T retval;

	if (head == nullptr)
	{
		return false;
	}

	if (head->data == data)
	{
		Node<T> *curr = head;

		retval = curr->data;

		head = head->next;

		node_bank_[thread_id].push_back(curr);

		return true;
	}

	Node<T> *prev = Remove(data);

	if (prev->next == nullptr && prev->data != data)
	{
		return false;
	}
	else if (prev->next == nullptr && prev->data == data)
	{
		node_bank_[thread_id].push_back(prev);

		prev = nullptr;

		return true;
	}

	Node<T> *toDel = prev->next;

	prev->next = prev->next->next;

	node_bank_[thread_id].push_back(toDel);

	return true;
}

template <typename T>
Node<T> *LinkedList<T>::Remove(T data)
{
	if (head == NULL)
	{
		return NULL;
	}

	Node<T> *prev = head;
	Node<T> *sought = head;

	while (sought != NULL && sought->data != data)
	{
		prev = sought;
		sought = sought->next;
	}

	return prev;
}

template <typename T>
void LinkedList<T>::DestroyList()
{
	head = DestroyList(head);
}

template <typename T>
Node<T> *LinkedList<T>::DestroyList(Node<T> *head)
{
	if (head == NULL)
		return NULL;

	Node<T> *temp = head;
	while (head != NULL)
	{
		temp = head;
		head = head->next;
		delete temp;
	}

	return NULL;
}

template <typename T>
void LinkedList<T>::Print()
{
	if (head == NULL)
	{
		cout << "empty list." << endl;
		return;
	}

	Node<T> *temp = head;
	while (temp != NULL)
	{
		cout << temp->data << " ";
		temp = temp->next;
	}

	cout << endl;
}

int main(void)
{
	LinkedList<int> ll;

	ll.Print();

	for (int i = 0; i < 10; i++)
	{
		ll.Insert((i * i) + 0, 0);
		ll.Insert((i * i) + 1, 1);
		ll.Insert((i * i) + 2, 2);
		ll.Insert((i * i) + 3, 3);
	}

	ll.Print();

	cout << endl;
	// Should all be true or 1.
	cout << ll.Find(19) << endl;
	cout << ll.Find(12) << endl;
	cout << ll.Find(3) << endl;
	cout << ll.Find(0) << endl;
	cout << ll.Find(84) << endl;
	cout << ll.Find(66) << endl;

	cout << endl;
	// Should all be true or 1.
	cout << ll.Delete(19, 0) << endl;
	cout << ll.Delete(12, 1) << endl;
	cout << ll.Delete(3, 2) << endl;
	cout << ll.Delete(0, 3) << endl;
	cout << ll.Delete(84, 0) << endl;
	cout << ll.Delete(66, 1) << endl;

	cout << endl;
	// Should all be false or 0, except for 3 which is in the list twice.
	cout << ll.Find(19) << endl;
	cout << ll.Find(12) << endl;
	cout << ll.Find(3) << endl;
	cout << ll.Find(0) << endl;
	cout << ll.Find(84) << endl;
	cout << ll.Find(66) << endl;

	ll.Print();

	return 0;
}
