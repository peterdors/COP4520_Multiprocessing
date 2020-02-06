// Peter Dorsaneo

// LinkedList.cpp
// ===============
// A nice way of practicing how to use generics on data structures in C++. 

#include <iostream> 
#include "LinkedList.h"

using namespace std; 

template <typename T> 
LinkedList<T>::LinkedList()
{
	head = NULL;
}

template <typename T> 
LinkedList<T>::~LinkedList()
{
	// Works?
	delete head;
}

// Insertion at the head of the list. 
template <typename T> 
void LinkedList<T>::insert(T data)
{
	head = insert(head, data);
}

template <typename T> 
Node<T> *LinkedList<T>::insert(Node<T> *head, T data)
{
	if (head == NULL) 
	{
		head = new Node<T>();
		head->data = data; 
		head->next = NULL;
	}
	else
	{
		Node<T> *newHead = new Node<T>(); 
		newHead->data = data; 
		newHead->next = head; 
		head = newHead; 
	}

	return head;
}

template <typename T> 
Node<T> *LinkedList<T>::search(Node<T> *head, T data)
{
	if (head == NULL)
		return NULL; 

	Node<T> *sought = head; 

	while (sought != NULL && sought->data != data)
	{
		sought = sought->next;
	}

	return sought; 
}

template <typename T> 
bool LinkedList<T>::search(T data)
{
	Node<T> *sought = search(head, data);

	return (sought == NULL) ? false : true;
}

template <typename T> 
Node<T> *LinkedList<T>::remove(Node<T> *head, T data)
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
bool LinkedList<T>::remove(T data)
{
	T retval; 

	if (head == NULL)
	{ 
		return false;
	}

	if (head->data == data)
	{
		Node<T> *curr = head; 

		retval = curr->data;

		head = head->next; 

		delete curr; 

		return true;
	}

	Node<T> *prev = remove(head, data);

	if (prev->next == NULL && prev->data != data)
	{
		return false; 
	}
	else if (prev->next == NULL && prev->data == data)
	{
		delete prev; 

		prev = NULL; 

		return true;
	}

	Node<T> *toDel = prev->next; 

	prev->next = prev->next->next; 

	delete toDel; 

	return true; 
}

template <typename T>
void LinkedList<T>::destroyList()
{
	head = destroyList(head);
}

template <typename T> 
Node<T> *LinkedList<T>::destroyList(Node<T> *head)
{
	if (head == NULL)
		return NULL;

	Node<T> *temp = head; 
	while (head != NULL)
	{
		temp = head; 
		head = head->next; 
		free(temp);
	}

	return NULL;
}

template <typename T>
void LinkedList<T>::print()
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

	for (int i = 0; i < 10; i++)
		ll.insert(i);

	ll.print();

	if (ll.search(-1))
	{
		cout << "found " << -1 << endl;
	}

	if (ll.search(0))
	{
		cout << "found " << 0 << endl;
	}

	if (ll.remove(0))
	{
		cout << "removed " << 0 << endl;
	}

	ll.print();

	if (ll.search(5))
	{
		cout << "found " << 5 << endl;
	}

	if (ll.remove(5))
	{
		cout << "removed " << 5 << endl;
	}

	ll.print();

	if (ll.search(9))
	{
		cout << "found " << 9 << endl;
	}

	if (ll.remove(9))
	{
		cout << "removed " << 9 << endl;
	}

	if (ll.search(9))
	{
		cout << "found " << 9 << endl;
	}
	
	ll.print();

	return 0; 
}
