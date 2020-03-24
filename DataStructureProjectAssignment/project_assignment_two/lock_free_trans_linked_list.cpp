// Peter Dorsaneo

// lock_free_trans_linked_list.cpp
// ===============================
#include "lock_free_trans_linked_list.hpp"

// Algorithm 2 Pointer Marking
#define SET_MARK(_p) (Node*)(((uintptr_t)(_p)) | 0x1)
#define SET_MARKD(_p) (NodeDesc*)(((uintptr_t)(_p)) | 0x1)
#define CLEAR_MARK(_p) (Node*)(((uintptr_t)(_p)) & ~0x1)
#define CLEAR_MARKD(_p) (NodeDesc*)(((uintptr_t)(_p)) & ~0x1)
#define IS_MARKED(_p) (((uintptr_t)(_p)) & 0x1)

thread_local HelpStack help_stack;

// Algorithm 3 Logical Status
bool IsNodePresent(Node* n, int key)
{
    return n != nullptr && n->key == key;
}

bool IsNodeActive(NodeDesc* nodeDesc)
{
    return nodeDesc->desc->tx_status == COMMITTED;
}

bool IsKeyPresent(NodeDesc* nodeDesc)
{
    bool isNodeActive = IsNodeActive(nodeDesc);
    uint8_t opType = nodeDesc->desc->ops[nodeDesc->op_id].type;

    return (opType == FIND) || (isNodeActive && opType == INSERT) || (!isNodeActive && opType == DELETE);
}

// Algorithm 5 Transaction Execution
bool ExecuteTransaction(Desc* desc)
{
    help_stack.Init();

    ExecuteOps(desc, 0);

    return desc->tx_status == COMMITTED;
}

void ExecuteOps(Desc* desc, int op_id)
{
    bool ret = true;

    if(desc->tx_status != ACTIVE)
    {
        return;
    }

    // Cyclic dependency check.
    if (help_stack.Contain(desc))
    {
        __sync_bool_compare_and_swap(&(desc->tx_status), ACTIVE, ABORTED);
        return;
    }

    vector<Node*> delNodes;
    vector<Node*> delPredNodes;
    vector<Node*> insNodes;
    vector<Node*> insPredNodes;

    help_stack.Push(desc);

    while ((desc->tx_status == ACTIVE) &&
            ret &&
            op_id < desc->size)
    {
        Operator* op = &(desc->ops[op_id]);

        if (op->type == FIND)
        {
            ret = Find(op->key, desc, op_id);
        }
        else if (op->type == INSERT)
        {
            Node* inserted;
            Node* pred;

            ret = Insert(op->key, desc, op_id, inserted, pred);

            insNodes.push_back(inserted);
            insPredNodes.push_back(pred);
        }
        else if (op->type == DELETE)
        {
            Node* deleted;
            Node* pred;

            // TODO: Write delete() method.
            ret = Delete(op->key, desc, op_id, deleted, pred);

            delNodes.push_back(deleted);
            delPredNodes.push_back(pred);
        }

        op_id++;
    }

    help_stack.Pop();

    if (ret)
    {
        if (__sync_bool_compare_and_swap(&desc->tx_status, ACTIVE, COMMITTED))
        {
            MarkForDelete(delNodes, delPredNodes, desc);
        }
    }
    else
    {
        __sync_bool_compare_and_swap(&desc->tx_status, ACTIVE, ABORTED);
        // MarkForDelete(delNodes, delPredNodes, desc);
    }
}

void MarkForDelete(const vector<Node*>& nodes, const vector<Node*>& preds, Desc* desc)
{
    for(int i = 0; i < nodes.size(); i++)
    {
        Node* n = nodes[i];

        if (n == nullptr)
        {
            continue;
        }

        NodeDesc* nodeDesc = n->nodeDesc;

        if(nodeDesc->desc == desc)
        {
            if(__sync_bool_compare_and_swap(&n->nodeDesc, nodeDesc, SET_MARKD(nodeDesc)))
            {
                Node* pred = preds[i];
                Node* succ = CLEAR_MARK(__sync_fetch_and_or(&n->next, ((Node*)0x1)));

                __sync_bool_compare_and_swap(&pred->next, n, succ);
            }
        }
    }
}

ReturnCode Insert(int key, Desc* desc, int op_id, Node*& inserted, Node*& pred)
{
    int ret;
    inserted = nullptr;
    NodeDesc* nodeDesc = new NodeDesc(desc, op_id);
    Node* new_node = nullptr;
    Node* curr = head;

    while (true)
    {
        DoLocatePred(pred, curr, key);

        if (!IsNodePresent(curr, key))
        {
            if(desc->tx_status != ACTIVE)
            {
                return FAIL;
            }
            if (new_node == nullptr)
            {
                new_node = new Node(key, nullptr, nodeDesc);
            }

            new_node->next = curr;

            Node* pred_next = __sync_val_compare_and_swap(&pred->next, curr, new_node);

            if (pred_next == curr)
            {
                inserted = new_node;
                return OK;
            }

            // Restart
            curr = IS_MARKED(pred_next) ? head : pred;
        }

        else
        {
            NodeDesc *oldCurrDesc = curr->nodeDesc;

            if(IS_MARKED(oldCurrDesc))
            {
                if(!IS_MARKED(curr->next))
                {
                    (__sync_fetch_and_or(&curr->next, ((Node*)0x1)));
                }

                curr = head;
                continue;
            }

            FinishPendingTxn(oldCurrDesc, desc);

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                // If it's the same operation, some other thread is already doing it, so we're not going to?
                return SKIP;
            }

            if(!IsKeyPresent(oldCurrDesc))
            {
                NodeDesc* currDesc = curr->nodeDesc;

                if (desc->tx_status != ACTIVE)
                {
                    return FAIL;
                }

                currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);

                if(currDesc == oldCurrDesc)
                {
                    inserted = curr;
                    return OK;
                }
            }
            else
            {
                return FAIL;
            }
        }
    }

    return FAIL;
}

ReturnCode Delete(int key, Desc* desc, int op_id, Node*& deleted, Node*& pred)
{
    deleted = nullptr;
    NodeDesc* nodeDesc = new NodeDesc(desc, op_id);
    Node* curr = head;

    while (true)
    {
        DoLocatePred(pred, curr, key);

        if (IsNodePresent(curr, key))
        {
            NodeDesc* oldCurrDesc = curr->nodeDesc;

            if(IS_MARKED(oldCurrDesc))
            {
                return FAIL;
            }

            FinishPendingTxn(oldCurrDesc, desc);

            if(IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return SKIP;
            }

            if (IsKeyPresent(oldCurrDesc))
            {
                NodeDesc* currDesc = curr->nodeDesc;

                if(desc->tx_status != ACTIVE)
                {
                    return FAIL;
                }

                currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);

                if(currDesc == oldCurrDesc)
                {
                    deleted = curr;
                    return OK;
                }
            }
            else
            {
                return FAIL;
            }
        }
        else
        {
            return FAIL;
        }
    }

    return FAIL;
}

bool IsSameOperation(NodeDesc* nd1, NodeDesc* nd2)
{
    return nd1->desc == nd2->desc && nd1->op_id == nd2->op_id;
}

ReturnCode Find(int key, Desc* desc, int op_id)
{
    NodeDesc* nodeDesc = NULL;
    Node* pred;
    Node* curr = head;

    while (true)
    {
        DoLocatePred(pred, curr, key);

        if (IsNodePresent(curr, key))
        {
            NodeDesc* oldCurrDesc = curr->nodeDesc;

            if (IS_MARKED(oldCurrDesc))
            {
                if (!IS_MARKED(curr->next))
                {
                    (__sync_fetch_and_or(&curr->next, ((Node*) 0x1)));
                }

                curr = head;
                continue;
            }

            FinishPendingTxn(oldCurrDesc, desc);

            if (nodeDesc == NULL)
            {
                nodeDesc = new NodeDesc(desc, op_id);
            }

            if (IsSameOperation(oldCurrDesc, nodeDesc))
            {
                return SKIP;
            }

            if (IsKeyPresent(oldCurrDesc))
            {
                NodeDesc* currDesc = curr->nodeDesc;

                if(desc->tx_status != ACTIVE)
                {
                    return FAIL;
                }

                currDesc = __sync_val_compare_and_swap(&curr->nodeDesc, oldCurrDesc, nodeDesc);

                if (currDesc == oldCurrDesc)
                {
                    return OK;
                }
            }
            else
            {
                return FAIL;
            }
        }
        else
        {
            return FAIL;
        }
    }

    return FAIL;
}

void FinishPendingTxn(NodeDesc* nodeDesc, Desc* desc)
{
    if (nodeDesc->desc == desc)
    {
        return;
    }

    ExecuteOps(nodeDesc->desc, nodeDesc->op_id + 1);
}

void DoLocatePred(Node*& pred, Node*& curr, int key)
{
    Node* pred_next;

    while (curr->key < key)
    {
        pred = curr;
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

int main(void)
{


    return 0;
}
