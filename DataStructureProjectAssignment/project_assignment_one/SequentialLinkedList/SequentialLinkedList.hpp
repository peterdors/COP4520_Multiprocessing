#include <iostream>
#include <vector>

using namespace std;

#ifndef SEQUENTIALLINKEDLIST_HPP
#define SEQUENTIALLINKEDLIST_HPP


struct Node
{
    uint32_t val;
    Node* next;

    Node(const uint32_t& val) :
        val(val), next(nullptr)
        {}
};

class SequentialLinkedList
{
private:
    const static int kNodeBankSize = 1e6;
    const static int kInitListSize = 1e2;

    Node* head;
    vector<Node*> node_bank;


    bool Validate(Node* pred, Node* curr);
    void PreBuildList(void);
    void BuildNodeBank(void);

public:
    SequentialLinkedList();

    bool Insert(uint32_t value);
    bool Find(uint32_t value);
    bool Delete(uint32_t value);

    void Print(void);
};
#endif
