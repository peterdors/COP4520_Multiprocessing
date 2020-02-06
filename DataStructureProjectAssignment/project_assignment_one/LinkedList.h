#ifndef LINKEDLIST_H
#define LINKEDLIST_H

// Explicit definition, this actually allocates
// as well as describing.
// This makes use of an extern variable, though we could just put this value
// into our private portion of our class. 

template <typename T> 
struct Node
{
	T data; 
	struct Node<T> *next;
};

template <typename T> 
class LinkedList
{
private: 
	Node<T> *head;

	// The important functions that are relative to the assignment. 
	Node<T> *insert(Node<T> *head, T data);

	Node<T> *search(Node<T> *head, T data);

	Node<T> *remove(Node<T> *head, T data);

	// Just nice to have functions. 
	Node<T> *destroyList(Node<T> *head);

public:
	LinkedList();

	~LinkedList();

	// The important functions that are relative to the assignment. 
	void insert(T data);

	bool search(T data); 

	bool remove(T data);

	// Just nice functions to have.
	void destroyList();

	void print();

};


#endif
