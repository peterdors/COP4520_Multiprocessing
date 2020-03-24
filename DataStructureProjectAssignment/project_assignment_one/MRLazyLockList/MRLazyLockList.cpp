#include "MRLazyLockList.hpp"
#include <random>
#include <thread>

using namespace std;

/*
@def: Constructor for this class.
@params: none
@return: An instance of this class
*/
MRLazyLockList::MRLazyLockList()
{
    init(ref(l), kNumThreads);

    PreBuildList();

    BuildNodeBank();
}

/*
@def: Verifies if the pred->next still points to the curr node.
@params: (pred)ecessor node, and the (curr)ent node
@return: True if pred->next is still pointing to curr, false otherwise.
*/
bool MRLazyLockList::Validate(Node* pred, Node* curr)
{
    if (!curr)
    {
        return !pred->marked && pred->next == curr;
    }

    return !pred->marked && !curr->marked && pred->next == curr;
}

/*
@def: In class method for preallocating and inserting nodes into the list.
        This just inserts even values from [0, kInitListSize) but
        can easily be adapted to insert random values as necessary.
@params: none
@return: none
*/
void MRLazyLockList::PreBuildList(void)
{
    uint32_t size = kInitListSize;

    for (uint32_t value = 0; value < size; value += 2)
    {
        uint32_t key = value;

        if (!head)
        {
            head = new Node(value);
            continue;
        }

        if (!head->next && head->key != key)
        {
            head->next = new Node(value);
            continue;
        }

        Node* n = new Node(value);
        Node* pred = head;
        Node* curr = head->next;

        // Find where we are to insert the node.
        while (curr && curr->key < key)
        {
            pred = curr;
            curr = curr->next;
        }

        // Conditional prevents duplicates from being inserted.
        if (n->key != pred->key)
        {
            n->next = curr;
            pred->next = n;
        }
    }
}

/*
@def: In class method for preallocating a node bank for a thread use case.
@params: none
@return: none
*/
void MRLazyLockList::BuildNodeBank(void)
{
    // Preallocate and fill the node bank up for our use.
    for (int i = 0; i < kNumThreads; i++)
    {
        for (int k = 0; k < kNodeBankSize; k++)
        {
            // The value in the node will get replaced when Insert(...) is
            // called.
            node_bank[i].push_back(new Node(0));
        }
    }
}

/*
@def: Inserts a node with the specified value into the list.
@params: the value to be inserted, and the thread_id to signify which node_bank
            the thread is assigned to pull from.
@return: true if the insert was successful, false otherwise.
*/
bool MRLazyLockList::Insert(uint32_t value, uint8_t thread_id)
{
    uint32_t key = value;

    while (true)
    {
        Node* pred = head;
        Node* curr = head->next;

        while (curr && curr->key < key)
        {
            pred = curr;
            curr = curr->next;
        }

        bits[thread_id].set(pred->key);

        if (curr)
        {
            bits[thread_id].set(curr->key);
        }

        uint32_t handle = lock(ref(l), bits[thread_id]);



        if (Validate(pred, curr))
        {
            // Handling when pred is the last element of the list and curr is
            // nullptr.
            if (!curr)
            {
                pred->next = node_bank[thread_id].back();
                node_bank[thread_id].pop_back();

                pred->next->val = value;

                bits[thread_id].set(pred->key, 0);
                unlock(ref(l), handle);
                return true;
            }
            else if (curr->key == key)
            {
                bits[thread_id].set(pred->key, 0);
                bits[thread_id].set(curr->key, 0);
                unlock(ref(l), handle);
                return false;
            }
            else
            {
                Node* n = node_bank[thread_id].back();

                n->val = value;
                n->key = value;
                n->marked = false;

                if (n->key == pred->key || n->key == curr->key)
                {
                    bits[thread_id].set(pred->key, 0);
                    bits[thread_id].set(curr->key, 0);
                    unlock(ref(l), handle);
                    return false;
                }

                n->next = curr;
                pred->next = n;

                // Successful insertion means we can remove the node from the
                // node_bank.
                node_bank[thread_id].pop_back();
                bits[thread_id].set(pred->key, 0);
                bits[thread_id].set(curr->key, 0);
                unlock(ref(l), handle);

                return true;
            }
        }

        bits[thread_id].set(pred->key, 0);
        bits[thread_id].set(curr->key, 0);
        unlock(ref(l), handle);
    }
}

/*
@def: Traverses the list to search for a node with the specified value.
@params: The value to search for.
@return: True if the find was successful, false otherwise.
*/
bool MRLazyLockList::Find(uint32_t value)
{
    uint32_t key = value;
    Node* curr = head;

    while (curr && curr->key < key)
    {
        curr = curr->next;
    }

    return curr && curr->key == key && !curr->marked;
}

/*
@def: Searches for and deletes the node with the specified value in the list if
        found.
@params: The value to be deleted, and the thread_id to signify which node_bank
            the thread is assigned to add that node back to.
@return: True if the delete was successful, false otherwise.
*/
bool MRLazyLockList::Delete(uint32_t value, uint8_t thread_id)
{
    uint32_t key = value;
    Node* pred, *curr;

    while (true)
    {
        pred = head;
        curr = head->next;

        while (curr && curr->key < key)
        {
            pred = curr;
            curr = curr->next;
        }

        bits[thread_id].set(pred->key);

        if (curr)
        {
            bits[thread_id].set(curr->key);
        }

        uint32_t handle = lock(ref(l), bits[thread_id]);

        if (Validate(pred, curr))
        {
            // Handling for when we are at the tail of the list.
            if (!curr)
            {
                bits[thread_id].set(pred->key, 0);
                unlock(ref(l), handle);
                return false;
            }
            else if (curr->key != key)
            {
                bits[thread_id].set(pred->key, 0);
                bits[thread_id].set(curr->key, 0);

                unlock(ref(l), handle);
                return false;
            }
            else
            {
                curr->marked = true;
                pred->next = curr->next;

                node_bank[thread_id].push_back(curr);

                bits[thread_id].set(pred->key, 0);
                bits[thread_id].set(curr->key, 0);

                unlock(ref(l), handle);
                return true;
            }
        }

        bits[thread_id].set(pred->key, 0);
        bits[thread_id].set(curr->key, 0);
        unlock(ref(l), handle);
    }
}


void MRLazyLockList::Print(void)
{
    Node* curr = head;

    while (curr)
    {
        cout << curr->val << " ";
        curr = curr->next;
    }

    cout << endl;
}

void do_Insert(MRLazyLockList& list, uint8_t thread_id, uint32_t numOps)
{
    for (uint32_t i = 0; i < numOps; i++)
    {
        // Inserting only odd numbers into a list prepopulated with evens.
        list.Insert((2 * i) + 1, thread_id);

        // Could also comment out the line above and uncomment this line below to
        // test for trying to insert duplicate values.
        // list.Insert(i, thread_id);
    }
}

void do_Delete(MRLazyLockList& list, uint8_t thread_id, uint32_t numOps)
{
    for (uint32_t i = 0; i < numOps; i++)
    {
        // Deleting only even numbers from a list prepopulated with evens.
        list.Delete((2 * i), thread_id);

        // Could also comment out the line above and uncomment this line below to
        // test for trying to delete values from [0, numOps).
        // list.Delete(i, thread_id);
    }
}

void do_Find(MRLazyLockList& list, uint32_t numOps)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    // Distribution with set range.
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,(int)1e4);
    uint32_t val;

    for (uint32_t i = 0; i < numOps; i++)
    {
        // Searching for a random number.
        val = dist(rng);
        list.Find(val);
    }
}

int main(void)
{
    MRLazyLockList list;
    vector<thread *> threads;
    const uint32_t numOps = 1000;

    threads.push_back(new thread(do_Insert, ref(list), 0, numOps));
    threads.push_back(new thread(do_Delete, ref(list), 1, numOps));
    threads.push_back(new thread(do_Find, ref(list), numOps));
    threads.push_back(new thread(do_Find, ref(list), numOps));

    for (auto& t : threads)
    {
        t->join();
    }

    // Uncomment this line to view the list after the threads have completed
    // running:
    // list.Print();

    return 0;
}
