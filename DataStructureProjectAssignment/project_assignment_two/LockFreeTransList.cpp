#include "LockFreeTransList.hpp"
#include <vector>
#include <array>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>

#define NUM_TRANSACTIONS 250

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

ReturnCode UpdateDesc(Node* n, NodeDesc* nodeDesc, bool wantkey, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank)
{
    NodeDesc* oldDesc = n->nodeDesc;

    if (IS_MARKED(oldDesc))
    {
        return FAIL;
    }

    if (oldDesc && (oldDesc->desc != nodeDesc->desc))
    {
        ExecuteOps(oldDesc->desc, oldDesc->opid, nodeDescBank, nodeBank);
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
void ExecuteMultiTransaction(vector<Desc*> desc, vector<NodeDesc*>& nodeDescBank, vector<Node*>& nodeBank)
{
	help_stack.Init();

    for (uint8_t i=0; i<NUM_TRANSACTIONS; i++)
    	ExecuteTransaction(desc[i], &nodeDescBank, &nodeBank);

    return;
}

// Algorithm 5 Transaction Execution
bool ExecuteTransaction(Desc* desc, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank)
{
    ExecuteOps(desc, 0, nodeDescBank, nodeBank);

    return desc->status == COMMITTED;
}

void ExecuteOps(Desc* desc, uint8_t opid, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank)
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
            ret = Find(op.key, desc, opid, nodeDescBank, nodeBank);
        }
        else if (op.type == INSERT)
        {
            Node* inserted;
            Node* pred;

            ret = Insert(op.key, desc, opid, ref(inserted), ref(pred), nodeDescBank, nodeBank);

            insNodes.push_back(inserted);
            insPredNodes.push_back(pred);
        }
        else if (op.type == DELETE)
        {
            Node* del;
            Node* pred;

            ret = Delete(op.key, desc, opid, ref(del), ref(pred), nodeDescBank, nodeBank);

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
bool Insert(uint32_t key, Desc* desc, uint8_t opid,
            Node*& inserted, Node*& pred,
            vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank)
{
    // NodeDesc* nodeDesc = new NodeDesc(desc, opid);
    NodeDesc* nodeDesc = nodeDescBank->back();
    nodeDesc->desc = desc;
    nodeDesc->opid = opid;
    nodeDescBank->pop_back();


    ReturnCode ret = FAIL;
    Node* curr = head;
    pred = head;

    while (true)
    {
        DoLocatePred(ref(pred), ref(curr), key);

        if (IsNodePresent(curr, key))
        {
            ret = UpdateDesc(curr, nodeDesc, false, nodeDescBank, nodeBank);
        }
        else
        {
            if(desc->status != ACTIVE)
            {
                return FAIL;
            }

            // Node* new_node = new Node(key, nullptr, nodeDesc);
            Node* new_node = nodeBank->back();
            new_node->key = key;
            new_node->next = curr;
            new_node->nodeDesc = nodeDesc;
            nodeBank->pop_back();

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
bool Find(uint32_t key, Desc* desc, int opid, vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank)
{
    // NodeDesc* nodeDesc = new NodeDesc(desc, opid);
    NodeDesc* nodeDesc = nodeDescBank->back();
    nodeDesc->desc = desc;
    nodeDesc->opid = opid;
    nodeDescBank->pop_back();

    ReturnCode ret = FAIL;
    Node* curr = head;
    Node* pred = head;

    while (true)
    {
        DoLocatePred(ref(pred), ref(curr), key);

        if (IsNodePresent(curr, key))
        {
            ret = UpdateDesc(curr, nodeDesc, true, nodeDescBank, nodeBank);
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
bool Delete(uint32_t key, Desc* desc, uint8_t opid,
            Node*& del, Node*& pred,
            vector<NodeDesc*>* nodeDescBank, vector<Node*>* nodeBank)
{
    // NodeDesc* nodeDesc = new NodeDesc(desc, opid);
    NodeDesc* nodeDesc = nodeDescBank->back();
    nodeDesc->desc = desc;
    nodeDesc->opid = opid;
    nodeDescBank->pop_back();

    ReturnCode ret = FAIL;
    Node* curr = head;

    while (true)
    {
        DoLocatePred(ref(pred), ref(curr), key);

        if (IsNodePresent(curr, key))
        {
            ret = UpdateDesc(curr, nodeDesc, true, nodeDescBank, nodeBank);
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
        op[i].type = op_type;
        op[i].key = (rand()%1000)+1;
        //cout << op[i].key << endl;
    }
}

void buildDescriptors(vector<Desc*>& descs, int size)
{
	OpType op_type;
	for (uint8_t i = 0; i < NUM_TRANSACTIONS; i++)
    {
    	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    	if (r <= 0.15)
    		op_type = INSERT;
    	else if (r <= 0.2)
    		op_type = DELETE;
    	else
    		op_type = FIND;

        descs[i]=new Desc(size);
        setOpsArray(descs[i], &((descs[i])->ops), op_type);
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

int main(int argc, char *argv[])
{
    const int size = 1;
    if (argc < 2)
    {
        cout << "Usage: ./a.out <n>" << endl;
        return -1;
    }

    const int num_threads = atoi(argv[1]);
    tail = new Node(1001, nullptr, nullptr);
    head = new Node(0, tail, nullptr);
    vector<NodeDesc*> nodeDescBank[num_threads];
    vector<Node*> nodeBank[num_threads];

    // pre fill the nodeDescBank and nodeBank.
    for (int thrd = 0; thrd < num_threads; thrd++)
    {
        for (int j = 0; j < 1e5; j++)
        {
            nodeDescBank[thrd].push_back(new NodeDesc());
            nodeBank[thrd].push_back(new Node());
        }
    }

    // list of lists of descriptors
    // each thread has its own lists.
    vector<vector<Desc*>> descs(num_threads, vector<Desc*>(NUM_TRANSACTIONS, nullptr));

    for (vector<Desc*> list : descs)
    {
        for (auto& d : list)
        {
            delete d;
        }

        list.clear();
    }

    for (uint8_t d = 0; d < num_threads; d++)
    {
    	buildDescriptors(ref(descs[d]), size);
    }

    // preBuildList();

    vector<thread *> threads;

    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; i++)
    {
    	threads.push_back(new thread(ExecuteMultiTransaction, descs[i],
                        ref(nodeDescBank[i]), ref(nodeBank[i])));
    }

    for (auto& t : threads)
    {
        t->join();
    }

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop-start).count();
    ofstream outfile;

   	outfile.open("runtimes.txt", fstream::app);
    outfile << num_threads << " duration: " << duration << endl;
    outfile.close();

    return 0;
}
