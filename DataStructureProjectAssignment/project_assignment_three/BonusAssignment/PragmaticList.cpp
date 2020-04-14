// TODO:

// 1. Cleanup random number generator so not to generate a random number
// when the threads are running.

// 2. Run tests on a ratio of Insert, Delete, and Find calls.

// 3. Plot tests as throughput vs threads graph to include in the final
// document.

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <random>
#include <stdio.h>

using namespace std;

#define SET_MARK(_p) (Node*)(((uintptr_t)(_p)) | 0x1)
#define CLEAR_MARK(_p) (Node*) (((uintptr_t)(_p)) & ~0x1)
#define IS_MARKED(_p) (((uintptr_t) (_p)) & 0x1)

#define NUM_THREADS 3
#define NODE_BANK_SIZE 1e6

struct Node
{
    int key;
    atomic<Node*> next;
    // volatile Node* next;

    Node() : key(INT_MIN) {this->next = nullptr;}
    Node(int _key) : key(_key) {this->next = nullptr;}
    Node(int _key, Node* _next) : key(_key) {atomic_init(&this->next, _next);}
};

class List
{
public:
    Node* head;
    Node* tail;

    vector<Node*> node_bank[NUM_THREADS];

    List()
    {
        tail = new Node(INT_MAX);
        head = new Node(0, tail);

        for (int i = 0; i < NUM_THREADS; i++)
        {
            for (int j = 0; j < NODE_BANK_SIZE; j++)
            {
                node_bank[i].push_back(new Node(INT_MIN));
            }
        }
    }

    Node* Search(int key, Node** left_node)
    {
        Node* left_node_next, *right_node;

        search_again:
        do
        {
            Node* t = head, *t_next = head->next;

            /* 1: Find left_node and right_node */
            do
            {
                if (!IS_MARKED(t_next))
                {
                    *left_node = t;
                    left_node_next = t_next;
                }

                t = CLEAR_MARK(t_next);

                if (t == tail)
                {
                    break;
                }

                t_next = t->next;

            } while (IS_MARKED(t_next) || t->key < key);

            right_node = t;

            /* 2: Check nodes are adjacent */
            if (left_node_next == right_node)
            {
                if (right_node != tail && IS_MARKED(reinterpret_cast<uintptr_t>((right_node->next).load())))
                {
                    goto search_again;
                }
                else
                {
                    return right_node;
                }
            }

            /* 3: Remove one or more marked nodes */
            if ((*left_node)->next.compare_exchange_strong(left_node_next, right_node))
            // if (__sync_bool_compare_and_swap(&(), left_node_next, right_node))
            {
                if (right_node != tail && IS_MARKED((right_node->next).load()))
                {
                    goto search_again;
                }
                else
                {
                    return right_node;
                }
            }

        } while(true);
    }

    bool Insert(int key, int thread_id)
    {
        Node* new_node = node_bank[thread_id].back();
        new_node->key = key;
        node_bank[thread_id].pop_back();

        Node* right_node, *left_node;

        do
        {
            right_node = Search(key, &left_node);

            if (right_node != tail && right_node->key == key)
            {
                return false;
            }

            new_node->next = right_node;

            if (left_node->next.compare_exchange_strong(right_node, new_node))
            // if (__sync_bool_compare_and_swap(&(left_node->next), right_node, new_node))
                return true;

        } while (true);
    }

    bool Delete(int key)
    {
        Node* right_node, *right_node_next, *left_node;

        do
        {
            right_node = Search(key, &left_node);

            if (right_node == tail || right_node->key != key)
            {
                return false;
            }

            right_node_next = right_node->next;

            if (!IS_MARKED(right_node_next))
            {
                if (right_node->next.compare_exchange_strong(right_node_next, SET_MARK(right_node_next)))
                // if (__sync_bool_compare_and_swap(&(right_node->next), right_node_next, SET_MARK(right_node_next)))
                {
                    break;
                }
            }
        } while(true);

        if (!left_node->next.compare_exchange_strong(right_node, right_node_next))
        // if (!__sync_bool_compare_and_swap(&(left_node->next), right_node, right_node_next))
        {
            right_node = Search(right_node->key, &(left_node));
        }

        return true;
    }

    bool Find(int key)
    {
        Node* right_node, *left_node;

        right_node = Search(key, &left_node);

        if (right_node == tail || right_node->key != key)
        {
            return false;
        }

        return true;
    }

    void Print()
    {
        for (Node* curr = head; curr != nullptr; curr = curr->next)
            cout << curr->key << " ";
        cout << endl;
    }
};


void doInsert(List& l)
{
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<std::mt19937::result_type> dist(0, 100);

    for (int i = 1; i <= 100; i++)
    {
        l.Insert(dist(rng), 0);
    }
}

void doDelete(List& l)
{
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<std::mt19937::result_type> dist(0, 100);

    for (int i = 1; i <= 100; i++)
    {
        l.Delete(dist(rng));
    }
}

void doFind(List& l)
{
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<std::mt19937::result_type> dist(0, 100);

    for (int i = 1; i <= 100; i++)
    {
        l.Find(dist(rng));
    }
}

int main(void)
{
    List l;

    vector<thread*> threads;

    threads.push_back(new thread(doInsert, ref(l)));
    threads.push_back(new thread(doDelete, ref(l)));
    threads.push_back(new thread(doFind, ref(l)));

    for (auto& t : threads)
    {
        t->join();
    }

    l.Print();

    return 0;
}
