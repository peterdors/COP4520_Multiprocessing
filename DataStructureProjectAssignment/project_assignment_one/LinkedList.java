// Sean Szumlanski
// COP 3503, Spring 2019

// LinkedList.java
// ===============
// This is a very powerful implementation of linked lists that lets you cram
// *any* type of data into your nodes. We achieve this using "generics" in Java.


import java.io.*;

// A node class whose 'data' field is designed to hold any type of data.
class Node<T>
{
	T data;
	Node<T> next;
	
	// Constructor method. Sets this object's 'data' field to 'data'.
	Node(T data)
	{
		this.data = data;
	}
}

// A linked list class designed to hold nodes with any type of data.
public class LinkedList<AnyType>
{	
	// Notice that when you create a LinkedList object (in main(), for example),
	// you tell it what kind of data it'll be holding. The LinkedList class
	// needs to pass that information on to the Node class, as well. That's
	// what's happening here.
	private Node<AnyType> head, tail;

	// Insert at the tail of the list.
	public void tailInsert(AnyType data)
	{
		// If the list is empty, set 'head' and 'tail' to the new node.
		if (head == null)
		{
			head = tail = new Node<>(data);
		}
		// Otherwise, append the new node to the end of the list and move the
		// tail reference forward.
		else
		{
			tail.next = new Node<>(data);
			tail = tail.next;
		}
	}

	// Insert at the head of the list.
	public void headInsert(AnyType data)
	{
		// First, create the node to be inserted.
		Node<AnyType> newNode = new Node<>(data);

		// Insert it at the beginning of the list.
		newNode.next = head;
		head = newNode;

		// If the list was empty before adding this node, 'head' AND 'tail'
		// need to reference this new node.
		if (tail == null)
			tail = newNode;
	}

	// Remove the head of the list (and return its 'data' value). We're using
	// Integer so that we can return a null reference if the list is empty.
	// Otherwise, the return value can be used in int contexts. This is a bit
	// nicer than returning -1 or Integer.MIN_VALUE, because we might actually
	// want to allow those values in our linked list nodes!
	public AnyType deleteHead()
	{
		// If the list is empty, signify that by returning null.
		if (head == null)
			return null;
		
		// Store the data from the head, then move the head reference forward.
		// Java will take care of the memory management when it realizes there
		// are no references to the old head anymore.
		AnyType temp = head.data;
		head = head.next;
		
		// If the list is now empty (i.e., if the node we just removed was the
		// only node in the list), update the tail reference, too!
		if (head == null)
			tail = null;
		
		// Return the value from the old head node.
		return temp;
	}

	// This one is left to you as an exercise. Don't forget to update the tail
	// pointer after removing the tail node!
	public AnyType deleteTail()
	{
		return null;  // not yet implemented
	}

	// Print the contents of the linked list.
	public void print()
	{
		for (Node<AnyType> temp = head; temp != null; temp = temp.next)
			System.out.print(temp.data + ((temp.next == null) ? "\n" : " "));
	}

	// Returns true if the list is empty, false otherwise.
	boolean isEmpty()
	{
		return (head == null);
	}

	public static void main(String [] args)
	{
		// Create a new linked list that holds integers.
		LinkedList<Integer> list = new LinkedList<>();

		list.headInsert(43);
		list.headInsert(58);
		list.headInsert(52);
		list.tailInsert(33);
		list.tailInsert(19);
		list.headInsert(12);

		// Print the list to verify everything got in there correctly.
		list.print();

		while (!list.isEmpty())
		{
			list.deleteHead();
			list.print();
		}

		// Create another linked list (this time, one that holds strings).
		LinkedList<String> stringList = new LinkedList<>();
		
		stringList.tailInsert("Sean");
		stringList.tailInsert("Szumlanski");
		stringList.tailInsert("wrote");
		stringList.tailInsert("this");
		stringList.tailInsert("fancy");
		stringList.tailInsert("beast!");

		// print the new list to verify everything got in there correctly
		while (!stringList.isEmpty())
			System.out.print(stringList.deleteHead() + " ");
		System.out.println();

		// Print the old list just to verify that there weren't any static
		// problems that messed things up when we created stringList.
		list.print();
	}
}
