#include "SequentialLinkedList.hpp"

SequentialLinkedList::SequentialLinkedList(void)
{
    PreBuildList();

    BuildNodeBank();
}

/*
@def: Verifies if the pred->next still points to the curr node.
@params: (pred)ecessor node, and the (curr)ent node
@return: True if pred->next is still pointing to curr, false otherwise.
*/
bool SequentialLinkedList::Validate(Node* pred, Node* curr)
{
    return pred->next == curr;
}

/*
@def: Inner class method for preallocating and inserting nodes into the list.
        This just inserts even values from [0, kInitListSize) but
        can easily be adapted to insert random values as necessary.
@params: none
@return: none
*/
void SequentialLinkedList::PreBuildList(void)
{
    uint32_t size = kInitListSize;

    for (uint32_t value = 0; value < size; value++)
    {
        if (value % 2 != 0)
        {
            continue;
        }

        uint32_t key = value;

        if (!head)
        {
            head = new Node(value);
            continue;
        }

        if (!head->next && head->val != key)
        {
            head->next = new Node(value);
            continue;
        }

        Node* n = new Node(value);
        Node* pred = head;
        Node* curr = head->next;

        // Find where we are to insert the node.
        while (curr && curr->val < value)
        {
            pred = curr;
            curr = curr->next;
        }

        // Conditional prevents duplicates from being inserted.
        if (n->val != pred->val)
        {
            n->next = curr;
            pred->next = n;
        }
    }
}

/*
@def: In class method for preallocating a node bank to reuse nodes from.
@params: none
@return: none
*/
void SequentialLinkedList::BuildNodeBank(void)
{
    // Preallocate and fill the node bank up for our use.
    for (int k = 0; k < kNodeBankSize; k++)
    {
        // The value in the node will get replaced when Insert(...) is
        // called.
        node_bank.push_back(new Node(0));
    }
}

/*
@def: Inserts a node with the specified value into the list.
@params: The value to be inserted.
@return: True if the insert was successful, false otherwise.
*/
bool SequentialLinkedList::Insert(uint32_t value)
{
    while (true)
    {
        Node* pred = head;
        Node* curr = head->next;

        while (curr && curr->val < value)
        {
            pred = curr;
            curr = curr->next;
        }

        if (!curr)
        {
            pred->next = node_bank.back();
            node_bank.pop_back();

            pred->next->val = value;

            return true;
        }

        if (Validate(pred, curr))
        {
            if (curr->val == value)
            {
                return false;
            }
            else
            {
                Node* n = node_bank.back();
                node_bank.pop_back();

                n->val = value;

                n->next = curr;
                pred->next = n;

                return true;
            }
        }
    }
}

/*
@def: Traverses the list to search for a node with the specified value.
@params: The value to search for.
@return: True if the find was successful, false otherwise.
*/
bool SequentialLinkedList::Find(uint32_t value)
{
    Node* curr = head;

    while (curr && curr->val < value)
    {
        curr = curr->next;
    }

    return curr && curr->val == value;
}

/*
@def: Searches for and deletes the node with the specified value in the list if
        found.
@params: The value to be deleted
@return: True if the delete was successful, false otherwise.
*/
bool SequentialLinkedList::Delete(uint32_t value)
{
    Node* pred, *curr;

    while (true)
    {
        pred = head;
        curr = head->next;

        while (curr && curr->val < value)
        {
            pred = curr;
            curr = curr->next;
        }

        // Handling for when the pred has reached the last node of the list.
        if (!curr)
        {
            return false;
        }

        if (Validate(pred, curr))
        {
            if (curr->val != value)
            {
                return false;
            }
            else
            {
                pred->next = curr->next;

                node_bank.push_back(curr);
                return true;
            }
        }
    }
}

void SequentialLinkedList::Print(void)
{
    Node* curr = head;

    while (curr)
    {
        cout << curr->val << " ";
        curr = curr->next;
    }

    cout << endl;
}

int main(void)
{
    SequentialLinkedList list;
    const uint32_t numOps = 10000;

    cout << "[INFO] List before running insertions: ";
    list.Print();
    cout << endl;

    cout << "[INFO] Running insertions: " << endl;
    for (uint32_t value = 1; value < numOps; value += 2)
    {
        list.Insert(value);
    }
    cout << endl;

    cout << "[INFO] List after running insertions: ";
    list.Print();
    cout << endl;

    cout << "[INFO] Running find: " << endl;
    // Search for every third element.
    for (uint32_t value = 0; value < numOps; value += 3)
    {
         cout << list.Find(value) << " ";
    }
    cout << endl;

    cout << "[INFO] List after running find: ";
    list.Print();

    cout << "[INFO] Running deletions: " << endl;
    for (uint32_t value = 0; value < numOps; value += 1)
    {
        cout << list.Delete(value) << " ";
    }
    cout << endl;

    cout << "[INFO] List after running deletions: ";
    list.Print();
    cout << endl;

    return 0;
}
