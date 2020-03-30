#include "LockFreeTransList.hpp"
#include <vector>
#include <array>
#include <thread>

#define NUM_THREADS 3

using namespace std;

thread_local HelpStack help_stack;

// Algorithm 3 Logical Status
bool IsKeyPresent(NodeDesc* nodeDesc, Desc* desc)
{
    if (!nodeDesc)
        return true;

    Operator op = nodeDesc->desc->ops[nodeDesc->opid];
    uint8_t status = nodeDesc->desc->status;

    if (status == ACTIVE)
    {
        if (desc && nodeDesc && nodeDesc->desc == desc)
        {
            return op.type == FIND || op.type == INSERT;
        }
        else
        {
            return op.type == FIND || op.type == DELETE;
        }
    }
    else if (status == COMMITTED)
    {
        return op.type == FIND || op.type == INSERT;
    }

    // The status has to be ABORTED.
    return op.type == FIND || op.type == DELETE;
}

bool IsNodePresent(Node* n, uint32_t key)
{
    return n->key == key;
}

ReturnCode UpdateDesc(Node* n, NodeDesc* nodeDesc, bool wantkey)
{
    NodeDesc* oldDesc = n->nodeDesc;

    if (IS_MARKED(oldDesc))
    {
        return FAIL;
    }

    if (oldDesc && (oldDesc->desc != nodeDesc->desc))
    {
        ExecuteOps(oldDesc->desc, oldDesc->opid);
    }
    else if (oldDesc && (oldDesc->opid >= nodeDesc->opid))
    {
        return SUCCESS;
    }

    bool haskey = IsKeyPresent(oldDesc, nodeDesc->desc);

    if ((!haskey && wantkey) || (haskey && !wantkey))
    {
        return FAIL;
    }

    if (nodeDesc->desc && (nodeDesc->desc->status != ACTIVE))
    {
        return FAIL;
    }

    if (__sync_bool_compare_and_swap(&(n->nodeDesc), oldDesc, nodeDesc))
    {
        return SUCCESS;
    }

    return RETRY;
}


// Algorithm 5 Transaction Execution
bool ExecuteTransaction(Desc* desc)
{
    help_stack.Init();

    ExecuteOps(desc, 0);

    return desc->status == COMMITTED;
}

void ExecuteOps(Desc* desc, uint8_t opid)
{
    bool ret = true;

    if (help_stack.Contain(desc))
    {
        __sync_bool_compare_and_swap(&(desc->status), ACTIVE, ABORTED);
        return;
    }

    help_stack.Push(desc);

    vector<Node*> delNodes;
    vector<Node*> delPredNodes;
    vector<Node*> insNodes;
    vector<Node*> insPredNodes;

    while (desc->status == ACTIVE && ret && opid < desc->size)
    {
        Operator op = (desc->ops[opid]);

        if (op.type == FIND)
        {
            ret = Find(op.key, desc, opid);
        }
        else if (op.type == INSERT)
        {
            Node* inserted;
            Node* pred;

            ret = Insert(op.key, desc, opid, ref(inserted), ref(pred));

            insNodes.push_back(inserted);
            insPredNodes.push_back(pred);
        }
        else if (op.type == DELETE)
        {
            Node* del;
            Node* pred;

            ret = Delete(op.key, desc, opid, ref(del), ref(pred));

            delNodes.push_back(del);
            delPredNodes.push_back(pred);
        }

        opid += 1;
    }

    help_stack.Pop();

    if (ret)
    {
        if (__sync_bool_compare_and_swap(&(desc->status), ACTIVE, COMMITTED))
        {
            MarkDelete(ref(delNodes), ref(delPredNodes), desc);
        }
    }
    else
    {
        __sync_bool_compare_and_swap(&(desc->status), ACTIVE, ABORTED);
    }
}

// Algorithm 6 Template for Transformed Insert Function
bool Insert(uint32_t key, Desc* desc, uint8_t opid, Node*& inserted, Node*& pred)
{
    NodeDesc* nodeDesc = new NodeDesc(desc, opid);
    ReturnCode ret = FAIL;
    Node* curr = head;
    pred = head;

    while (true)
    {
        DoLocatePred(ref(pred), ref(curr), key);

        if (IsNodePresent(curr, key))
        {
            ret = UpdateDesc(curr, nodeDesc, false);
        }
        else
        {
            if(desc->status != ACTIVE)
            {
                return FAIL;
            }

            Node* new_node = new Node(key, nullptr, nodeDesc);

            new_node->next = curr;

            Node* pred_next = __sync_val_compare_and_swap(&(pred->next), curr, new_node);

            if (pred_next == curr)
            {
                inserted = curr;
                return true;
            }

            // Restart
            curr = IS_MARKED(pred_next) ? head : pred;
        }

        if (ret == SUCCESS)
        {
            return true;
        }
        else if (ret == FAIL)
        {
            return false;
        }
    }
}

// Algorithm 7 Template for Transformed Find Function
bool Find(uint32_t key, Desc* desc, int opid)
{
    NodeDesc* nodeDesc = new NodeDesc(desc, opid);
    ReturnCode ret = FAIL;
    Node* curr = head;
    Node* pred = head;

    while (true)
    {
        DoLocatePred(ref(pred), ref(curr), key);

        if (IsNodePresent(curr, key))
        {
            ret = UpdateDesc(curr, nodeDesc, true);
        }
        else
        {
            ret = FAIL;
        }

        if (ret == SUCCESS)
        {
            return true;
        }
        else if (ret == FAIL)
        {
            return false;
        }
    }
}

// Algorithm 8 Template for Transformed Delete Function
bool Delete(uint32_t key, Desc* desc, uint8_t opid, Node*& del, Node*& pred)
{
    NodeDesc* nodeDesc = new NodeDesc(desc, opid);
    ReturnCode ret = FAIL;
    Node* curr = head;

    while (true)
    {
        DoLocatePred(ref(pred), ref(curr), key);

        if (IsNodePresent(curr, key))
        {
            ret = UpdateDesc(curr, nodeDesc, true);
        }
        else
        {
            ret = FAIL;
        }

        if (ret == SUCCESS)
        {
            del = curr;
            return true;
        }
        else if (ret == FAIL)
        {
            del = nullptr;
            return false;
        }
    }
}

void MarkDelete(vector<Node*>& delNodes, vector<Node*>& delPredNodes, Desc* desc)
{
    ReturnCode ret = FAIL;
    Node* pred = nullptr;
    Node* succ = nullptr;
    Node* n = nullptr;
    NodeDesc* nodeDesc = nullptr;

    for (int i = 0; i < delNodes.size(); i++)
    {
        n = delNodes[i];

        if (!n)
        {
            continue;
        }

        nodeDesc = n->nodeDesc;

        if (nodeDesc && nodeDesc->desc != desc)
        {
            continue;
        }

        if (__sync_bool_compare_and_swap(&(n->nodeDesc), nodeDesc, SET_MARKD(nodeDesc)))
        {
            if (pred && pred->next == n)
            {
                __sync_bool_compare_and_swap(&(pred->next), n, CLEAR_MARK(n->next));
            }
            else
            {
                pred = delPredNodes[i];

                // Node* succ = CLEAR_MARK(__sync_fetch_and_or(&(n->next), ((Node*)0x1)));
                succ = CLEAR_MARK(n->next);

                __sync_bool_compare_and_swap(&(pred->next), n, succ);
            }
        }
    }
}

void DoLocatePred(Node*& pred, Node*& curr, uint32_t key)
{
    Node* pred_next;

    while (curr->key < key)
    {
        pred = curr;

        // nullptr checking.
        if (!pred->next)
            return;

        pred_next = CLEAR_MARK(pred->next);

        curr = pred_next;

        while (IS_MARKED(curr->next))
        {
            curr = CLEAR_MARK(curr->next);
        }

        if (curr != pred_next)
        {
            // Failed to remove deleted nodes, start over from pred.
            if (!__sync_bool_compare_and_swap(&pred->next, pred_next, curr))
            {
                curr = head;
            }
        }
    }
}

// Below are initializer methods for descriptors.
void setOpsArray(Desc* desc, Operator** ops, OpType op_type)
{
    Operator *op = *ops;

    for (uint8_t i = 0; i < desc->size; i++)
    {
        if (op_type == DELETE)
        {
            op[i].type = op_type;
            op[i].key = (2 * (i + 6)) + 1; // odd numbers starting from 13
            cout << op[i].key << endl;
        }
        else
        {
            op[i].type = op_type;
            op[i].key = i + 2;
        }
    }
}

void buildDescriptors(vector<Desc*>& descs, int size)
{
    OpType op_type = FIND;
    uint8_t cnt = 0;

    for (uint8_t i = 0; i < NUM_THREADS; i++)
    {
        descs.push_back(new Desc(size));

        if (i == 1)
        {
            op_type = INSERT;
        }
        else if (i == 2)
        {
            op_type = DELETE;
        }

        setOpsArray(descs.back(), &((descs.back())->ops), op_type);
    }
}

void preBuildList(void)
{
    for (uint32_t i = 12; i < 30; i++)
    {
        tail->next = new Node(i, nullptr, nullptr);
        tail = tail->next;
    }
}

int main(void)
{
    const int size = 5;
    tail = new Node(1, nullptr, nullptr);
    head = new Node(0, tail, nullptr);

    vector<Desc*> descs;

    buildDescriptors(ref(descs), size);
    preBuildList();

    // for (int i = 0; i < descs.size(); i++)
    // {
    //     for (int j = 0; j < size; j++)
    //     {
    //         std::cout << descs[i]->ops[j].type << std::endl;
    //         std::cout << descs[i]->ops[j].key << std::endl;
    //         std::cout << std::endl;
    //     }
    // }

    vector<thread *> threads;

    // Find
    threads.push_back(new thread(ExecuteTransaction, descs[0]));

    // Insert
    threads.push_back(new thread(ExecuteTransaction, descs[1]));

    // Deletion
    threads.push_back(new thread(ExecuteTransaction, descs[2]));

    for (auto& t : threads)
    {
        t->join();
    }

    Node* curr = head;

    while (curr)
    {
        printf("%u ", curr->key);
        fflush(stdout);

        curr = curr->next;
    }

    cout << endl;

    return 0;
}
