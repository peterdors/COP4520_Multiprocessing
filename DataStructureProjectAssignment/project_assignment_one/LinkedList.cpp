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

// Removes and returns the element at head of the list. Returns NULL if the list
// is empty. 
template <typename T>
T LinkedList<T>::removeHead()
{
	if (head == NULL) 
	{
		return (T) 0;
	}

	return removeHead(&head); 
}

template <typename T>
T LinkedList<T>::removeHead(Node<T> **head)
{
	Node<T> *old_head = *head; 

	T data = old_head->data; 

	*head = (*head)->next; 

	free(old_head); 

	return data; 
}

// Removes and returns the nodes data stored at the end of the list. 
template <typename T> 
T LinkedList<T>::removeTail()
{
	// Case for an empty list. 
	if (head == NULL) 
	{
		return (T) 0;
	}
	
	// Case for when there is only one element in the list. 
	if (head->next == NULL)
	{
		T elem = head->data; 

		delete head; 

		head = NULL;

		return elem;
	}

	return removeTail(&head); 
}

// Recursion because why not. ;)
template <typename T> 
T LinkedList<T>::removeTail(Node<T> **head)
{

	if ((*head)->next->next == NULL)
	{
		T elem = (*head)->next->data; 

		Node<T> *old_tail = (*head)->next; 
		(*head)->next = NULL;
		free(old_tail);

		return elem;
	}

	return removeTail(&((*head)->next));
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
	LinkedList<int> *ll = new LinkedList<int>();
	ll->insert(1); 
	ll->insert(2);
	ll->insert(3);
	ll->insert(4);
	ll->print();
	ll->removeHead();
	ll->print();
	ll->removeTail();
	ll->print();
	ll->destroyList();
	ll->print();

	cout << endl;

	LinkedList<string> *lst = new LinkedList<string>();
	lst->insert("Peter");
	lst->insert("is");
	lst->insert("123");
	lst->insert("Dookie");
	lst->insert("Street.");

	lst->print();
	lst->removeHead(); 
	lst->print();
	lst->removeHead(); 
	lst->print();
	lst->destroyList();
	lst->print();


	return 0; 
}
