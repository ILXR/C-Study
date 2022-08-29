#include <iostream>
#include <string>
using namespace std;

// 动态开链线段树，以统计区间最大值为例
// 核心函数是以下几个，对不同的应用场景，需要修改具体实现
struct Node;
int query(Node *node, int l, int r);            // 查询一段区间内的值（最大/最小/区间和）
void addChild(Node *node);                      // 生成一个节点的孩子节点
void update(Node *node, int l, int r, int val); // 更新一段区间的值，可以加/减，单点操作只需 l==r
void pushDown(Node *node);                      // 将一个节点的 mask push 到子节点并清零

struct Node
{
    int left, right;
    int data, mask; // mask 只表示对孩子节点需要做的操作，不会对本节点造成影响
    Node *lchild, *rchild;
    Node(int l, int r) : left(l), right(r), data(0), mask(0), lchild(nullptr), rchild(nullptr) {}
    ~Node() { delete lchild, rchild; }
    void addMask(int val) { mask += val; }
    void clearMask() { mask = 0; }
    string toString() { return "node: " + to_string(left) + "-" + to_string(right); }
};

void addChild(Node *node)
{
    if (node->right > node->left && !node->lchild)
    {
        int mid = node->left + (node->right - node->left) / 2;
        node->lchild = new Node(node->left, mid);
        node->rchild = new Node(mid + 1, node->right);
    }
}

int query(Node *node, int l, int r)
{
    if (l > node->right || r < node->left)
        return -1;
    if (l <= node->left && r >= node->right)
        return node->data;
    addChild(node);
    pushDown(node); // 在查询的时候懒操作
    return max(query(node->lchild, l, r), query(node->rchild, l, r));
}

void update(Node *node, int l, int r, int val)
{
    if (l > node->right || r < node->left)
        return;
    if (l <= node->left && r >= node->right)
    {
        node->data += val;
        node->mask += val;
        return;
    }
    addChild(node);
    pushDown(node); // 在更新子节点时，需要先将当前节点的 mask 下放，否则最后对当前节点的值更新就是无效的
    update(node->lchild, l, r, val);
    update(node->rchild, l, r, val);
    node->data = max(node->lchild->data, node->rchild->data);
}

void pushDown(Node *node)
{
    if (node->left == node->right)
        return;
    node->lchild->addMask(node->mask);
    node->rchild->addMask(node->mask);
    node->lchild->data += node->mask;
    node->rchild->data += node->mask;
    node->clearMask();
}

int main(int argc, char const *argv[])
{
    Node *root = new Node(0, 1e9);
    return 0;
}