#ifndef RBTREE_H
#define RBTREE_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include <stack>
#include "Node.h"

enum RBColor { RED, BLACK };

struct RBTreeNode {
    Node data;
    RBColor color;
    RBTreeNode* parent;
    RBTreeNode* left;
    RBTreeNode* right;

    RBTreeNode(Node d = Node(), RBColor c = RED,
        RBTreeNode* p = nullptr,
        RBTreeNode* l = nullptr,
        RBTreeNode* r = nullptr)
        : data(d), color(c), parent(p), left(l), right(r) {
    }
};

class RBTree {
private:
    RBTreeNode* root;
    RBTreeNode* nil;
    RBTreeNode* minNode;

    // 高效内存池（带节点重用）
    struct MemoryPool {
        static constexpr size_t BLOCK_SIZE = 1 << 20; // 1MB块
        std::vector<std::unique_ptr<RBTreeNode[]>> blocks;
        std::vector<RBTreeNode*> free_list;  // 空闲节点列表
        size_t current_block_pos = 0;

        RBTreeNode* allocate(const Node& data) {
            // 优先从空闲列表获取节点
            if (!free_list.empty()) {
                RBTreeNode* node = free_list.back();
                free_list.pop_back();
                // 重用节点，就地构造
                *node = RBTreeNode(data, RED, nullptr, nullptr, nullptr);
                return node;
            }

            // 没有空闲节点时分配新块
            if (blocks.empty() || current_block_pos >= BLOCK_SIZE) {
                blocks.emplace_back(new RBTreeNode[BLOCK_SIZE]);
                current_block_pos = 0;
            }
            RBTreeNode* node = &blocks.back()[current_block_pos++];
            *node = RBTreeNode(data); // 就地构造
            return node;
        }

        void deallocate(RBTreeNode* node) {
            // 将节点加入空闲列表重用
            free_list.push_back(node);
        }

        void clear() {
            blocks.clear();
            free_list.clear();
            current_block_pos = 0;
        }
    } nodePool;

    RBTreeNode* allocateNode(const Node& data) {
        RBTreeNode* node = nodePool.allocate(data);
        node->color = RED;
        node->parent = nil;
        node->left = nil;
        node->right = nil;

        // 更新最小节点（插入时）
        if (minNode == nil || data.spill < minNode->data.spill)
            minNode = node;

        return node;
    }

    // 旋转函数
    void leftRotate(RBTreeNode* x) {
        RBTreeNode* y = x->right;
        x->right = y->left;
        if (y->left != nil) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        }
        else if (x == x->parent->left) {
            x->parent->left = y;
        }
        else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void rightRotate(RBTreeNode* y) {
        RBTreeNode* x = y->left;
        y->left = x->right;
        if (x->right != nil) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == nil) {
            root = x;
        }
        else if (y == y->parent->right) {
            y->parent->right = x;
        }
        else {
            y->parent->left = x;
        }
        x->right = y;
        y->parent = x;
    }

    // 插入修复
    void insertFixup(RBTreeNode* z) {
        while (z->parent != nil && z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                RBTreeNode* y = z->parent->parent->right;
                if (y != nil && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            }
            else {
                RBTreeNode* y = z->parent->parent->left;
                if (y != nil && y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    // 插入节点
    void insert(RBTreeNode* z) {
        RBTreeNode* y = nil;
        RBTreeNode* x = root;
        while (x != nil) {
            y = x;
            if (z->data < x->data) {
                x = x->left;
            }
            else {
                x = x->right;
            }
        }
        z->parent = y;
        if (y == nil) {
            root = z;
        }
        else if (z->data < y->data) {
            y->left = z;
        }
        else {
            y->right = z;
        }
        z->left = nil;
        z->right = nil;
        z->color = RED;
        insertFixup(z);
    }

    // 节点移植
    void transplant(RBTreeNode* u, RBTreeNode* v) {
        if (u->parent == nil) {
            root = v;
        }
        else if (u == u->parent->left) {
            u->parent->left = v;
        }
        else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    // 删除修复
    void deleteFixup(RBTreeNode* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                RBTreeNode* w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                }
                else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            }
            else {
                RBTreeNode* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                }
                else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    // 删除节点
    void deleteNode(RBTreeNode* z) {
        RBTreeNode* y = z;
        RBTreeNode* x;
        RBColor yOriginalColor = y->color;

        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        }
        else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        }
        else {
            y = z->right;
            while (y->left != nil) {
                y = y->left;
            }
            yOriginalColor = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            }
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        if (yOriginalColor == BLACK) {
            deleteFixup(x);
        }

        // 将删除的节点归还内存池以供重用
        nodePool.deallocate(z);
    }

public:
    RBTree() {
        nil = new RBTreeNode(Node(), BLACK);
        nil->left = nil;
        nil->right = nil;
        nil->parent = nil;
        root = nil;
        minNode = nil;
    }

    ~RBTree() {
        clear();
        delete nil;
    }

    // 迭代清除（避免递归栈溢出）
    void clear() {
        if (root == nil) return;

        std::stack<RBTreeNode*> st;
        st.push(root);

        while (!st.empty()) {
            RBTreeNode* node = st.top();
            st.pop();

            if (node->left != nil) st.push(node->left);
            if (node->right != nil) st.push(node->right);
        }

        // 重置内存池
        nodePool.clear();
        root = nil;
        minNode = nil;
    }

    void push(const Node& data) {
        RBTreeNode* z = allocateNode(data);
        insert(z);
    }

    void pop() {
        if (root == nil) return;

        RBTreeNode* oldMin = minNode;

        // 直接定位新minNode
        if (oldMin->right != nil) {
            // 后继节点在右子树中
            minNode = oldMin->right;
            while (minNode->left != nil) {
                minNode = minNode->left;
            }
        }
        else {
            // 后继节点在祖先路径中
            minNode = oldMin->parent;
            RBTreeNode* tmp = oldMin;
            while (minNode != nil && tmp == minNode->right) {
                tmp = minNode;
                minNode = minNode->parent;
            }
        }

        // 删除原最小节点
        deleteNode(oldMin);

        // 处理树空情况
        if (root == nil) minNode = nil;
    }

    Node top() const {
        if (root == nil) throw std::runtime_error("Tree is empty");
        return minNode->data;
    }

    bool empty() const {
        return root == nil;
    }
};

#endif // RBTREE_H