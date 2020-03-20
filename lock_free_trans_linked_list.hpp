#ifndef LOCK_FREE_TRANS_LINKED_LIST_H
#define LOCK_FREE_TRANS_LINKED_LIST_H

#include <iostream>
#include <atomic>
#include <thread>
#include <unordered_set>
#include <stack>
#include <vector>

using namespace std;

#define NUM_OPS ((int)1e9)

// Algorithm 1 Type Definitions
enum TxStatus {ACTIVE = 0, COMMITTED, ABORTED};

enum OpType {FIND = 0, INSERT, DELETE};

enum ReturnCode {OK = 0, SKIP, FAIL};

struct Operator
{
    OpType type;
    uint32_t key;
};

struct Desc
{
    int size;
    volatile TxStatus tx_status;
    Operator ops[NUM_OPS];

    static size_t SizeOf(uint8_t size)
    {
        return sizeof(int) + sizeof(int) + sizeof(Operator) * size;
    }
};

struct NodeDesc
{
    Desc* desc;
    int op_id;

    NodeDesc(Desc* _desc, uint8_t _opid)
            : desc(_desc), op_id(_opid){}
};

struct Node
{
    uint32_t key;
    Node* next;
    NodeDesc* nodeDesc;

    Node(): key(0), next(NULL), nodeDesc(NULL) {}
    Node(int _key, Node* _next, NodeDesc* _nodeDesc)
        : key(_key), next(_next), nodeDesc(_nodeDesc) {}
};

struct HelpStack
{
    Desc* helps[256];
    uint8_t index;

    void Init()
    {
        index = 0;
    }

    void Push(Desc* desc)
    {
        if (index > 255)
        {
            cout << "Index out of range." << endl;
            return;
        }

        helps[index++] = desc;
    }

    void Pop()
    {
        if (index < 0)
        {
            cout << "Nothing to pop." << endl;
            return;
        }

        index--;
    }

    bool Contain(Desc* desc)
    {
        for(uint8_t i = 0; i < index; i++)
        {
            if(helps[i] == desc)
            {
                return true;
            }
        }

        return false;
    }
};

Node* head;
Node* tail;

void FinishPendingTxn(NodeDesc* nodeDesc, Desc* desc);
void DoLocatePred(Node*& pred, Node*& curr, int key);
bool IsSameOperation(NodeDesc* nd1, NodeDesc* nd2);
ReturnCode Find(int key, Desc* desc, int op_id);
ReturnCode Delete(int key, Desc* desc, int op_id, Node*& deleted, Node*& pred);
ReturnCode Insert(int key, Desc* desc, int op_id, Node*& inserted, Node*& pred);
void MarkForDelete(const vector<Node*>& nodes, const vector<Node*>& pred, Desc* desc);
void ExecuteOps(Desc* desc, int op_id);
bool ExecuteTransaction(Desc* desc);
bool IsKeyPresent(NodeDesc* nodeDesc);
bool IsNodePresent(Node* n, int key);
bool isNodeActive(NodeDesc* nodeDesc);

#endif
