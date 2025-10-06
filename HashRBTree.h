#ifndef HASH_RBTREE_H
#define HASH_RBTREE_H

#include "Node.h"
#include "RBTree.h"
#include <vector>
#include <unordered_map>
#include <stdexcept>

class HashRBTree {
private:
    // 优化：直接存储Node对象，避免指针开销
    std::unordered_map<float, std::vector<Node>> spillBuckets;
    RBTree spillTree;

    // 新增：维护实际元素数量
    size_t elementCount = 0;

public:
    void push(const Node& node) {
        const float spill = node.spill;
        auto& bucket = spillBuckets[spill];

        if (bucket.empty()) {
            // 使用spill值创建最小节点
            spillTree.push(Node(spill, 0, 0));
        }

        // 直接存储Node对象，避免内存池开销
        bucket.push_back(node);
        elementCount++;
    }

    Node pop() {
        if (spillTree.empty()) {
            throw std::runtime_error("Priority queue is empty");
        }

        const Node minSpillNode = spillTree.top();
        const float minSpill = minSpillNode.spill;
        auto& bucket = spillBuckets[minSpill];

        // 移动语义避免拷贝
        Node minNode = std::move(bucket.back());
        bucket.pop_back();
        elementCount--;

        if (bucket.empty()) {
            spillBuckets.erase(minSpill);
            spillTree.pop();
        }

        return minNode;
    }

    Node top() {
        if (spillTree.empty()) {
            throw std::runtime_error("Priority queue is empty");
        }
        return spillBuckets[spillTree.top().spill].back();
    }

    bool empty() {
        return spillTree.empty();
    }

    size_t size() const {
        return elementCount;
    }

    // 新增：清空函数，释放内存
    void clear() {
        spillBuckets.clear();
        spillTree.clear();
        elementCount = 0;
    }
};

#endif // HASH_RBTREE_H