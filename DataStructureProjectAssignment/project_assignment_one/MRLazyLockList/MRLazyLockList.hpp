#include <mutex>
#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <map>
#include "mrlock.h"

#ifndef MRLAZYLOCKLIST_HPP
#define MRLAZYLOCKLIST_HPP


struct Node
{
    uint32_t val;
    uint32_t key;
    bool marked;
    Node* next;

    // Our node's key is going to be the value since we won't be inserting the
    // same value twice in our tests. Could use a better method such as hashcode
    // to improve distinguishing the key.
    Node(const uint32_t& val) :
        val(val), key(val), marked(false), next(nullptr)
        {}
};

class MRLazyLockList
{
private:
    const static int kNodeBankSize = 1e7;
    const static int kNumThreads = 4;
    const static int kInitListSize = 1e4;

    mrlock l;

    Node* head;

    vector<Node*> node_bank[kNumThreads];
    array<bitset<NUM_BITS>, kNumThreads> bits;

    bool Validate(Node* pred, Node* curr);
    void PreBuildList(void);
    void BuildNodeBank(void);

public:
    MRLazyLockList(void);
    ~MRLazyLockList(void);

    bool Insert(uint32_t value, uint8_t thread_id);
    bool Find(uint32_t value);
    bool Delete(uint32_t value, uint8_t thread_id);

    void Print(void);
};

#endif
