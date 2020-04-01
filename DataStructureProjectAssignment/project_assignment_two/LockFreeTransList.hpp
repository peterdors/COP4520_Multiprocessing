#ifndef LOCKFREETRANSLIST_HPP
#define LOCKFREETRANSLIST_HPP

#include <iostream>
#include <vector>
using namespace std;

// Algorithm 2 Pointer Marking
#define SET_MARK(_p) (Node*)(((uintptr_t)(_p)) | 0x1)
#define SET_MARKD(_p) (NodeDesc*)(((uintptr_t)(_p)) | 0x1)
#define CLEAR_MARK(_p) (Node*)(((uintptr_t)(_p)) & ~0x1)
#define CLEAR_MARKD(_p) (NodeDesc*)(((uintptr_t)(_p)) & ~0x1)
#define IS_MARKED(_p) (((uintptr_t)(_p)) & 0x1)

// Algorithm 1 Type Definitions
enum TxStatus {ACTIVE = 0, COMMITTED, ABORTED};

enum OpType {FIND = 0, INSERT, DELETE};

enum ReturnCode {SUCCESS = 0, RETRY, FAIL};

struct Operator
{
    OpType type;
    uint32_t key;
};

struct Desc
{
    uint8_t size;
    volatile uint8_t status;
    Operator* ops;

    Desc(const uint8_t& _size)
        : size(_size), status(ACTIVE)
    {
        ops = new Operator[_size];
    }
};

struct NodeDesc
{
    Desc* desc;
    uint8_t opid;

    NodeDesc()
            : desc(nullptr), opid(0) {}
    NodeDesc(Desc* _desc, const uint8_t& _opid)
            : desc(_desc), opid(_opid) {}
};

struct Node
{
    uint32_t key;
    Node* next;
    NodeDesc* nodeDesc;

    Node()
        : key(0), next(nullptr), nodeDesc(nullptr) {}

    Node(const uint32_t& _key, Node* _next, NodeDesc* _nodeDesc)
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
            // Index out of range.
            return;
        }

        helps[index++] = desc;
    }

    void Pop()
    {
        if (index < 0)
        {
            // Nothing to pop.
            return;
        }

        index--;
    }

    bool Contain(Desc* desc)
    {
        for (uint8_t i = 0; i < index; i++)
        {
            if (helps[i] == desc)
            {
                return true;
            }
        }

        return false;
    }
};

bool IsNodeActive(NodeDesc* nodeDesc);
bool IsKeyPresent(NodeDesc* nodeDesc);
bool IsNodePresent(Node* n, uint32_t key);
ReturnCode UpdateDesc(Node* n, NodeDesc* nodeDesc, bool wantkey, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank);

ReturnCode DoDelete(Node* n); // to be implemented
ReturnCode DoInsert(Node* n); // to be implemented
ReturnCode DoFind(Node* n); // to be implemented

void ExecuteOps(Desc* desc, uint8_t opid, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank);
bool ExecuteTransaction(Desc* desc, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank);
void ExecuteMultiTransaction(vector<Desc*> desc, vector<NodeDesc*>& nodeDescBank, vector<Node*>& nodeBank);

bool Insert(uint32_t key, Desc* desc, uint8_t opid,
            Node*& inserted, Node*& pred,
            vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank);

bool Delete(uint32_t key, Desc* desc, uint8_t opid, Node*& del, Node*& pred, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank);
void MarkDelete(vector<Node*>& delNodes, vector<Node*>& delPredNodes, Desc* desc);

bool Find(uint32_t key, Desc* desc, int opid, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank);
void DoLocatePred(Node*& pred, Node*& curr, uint32_t key);

Node* head = nullptr;
Node* tail = nullptr;

#endif
