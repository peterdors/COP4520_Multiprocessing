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

	Node<T> *insert(Node<T> *head, T data);
	
	Node<T> *destroyList(Node<T> *head);

	T removeHead(Node<T> **head);

	T removeTail(Node<T> **head);

public:
	LinkedList();

	void insert(T data);

	void destroyList();

	void print();

	T removeHead(); 

	T removeTail();

	~LinkedList();
};


#endif
