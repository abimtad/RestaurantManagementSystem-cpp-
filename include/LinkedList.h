#pragma once
#include "Order.h"
#include <functional>

/**
 * Doubly linked list to store active orders. Provides O(1) removal by node pointer.
 */
struct OrderNode {
    Order data;
    OrderNode* prev{nullptr};
    OrderNode* next{nullptr};
};

class OrderList {
public:
    OrderList();
    ~OrderList();

    /** Appends an order and returns the created node. */
    OrderNode* pushBack(const Order& order);
    /** Finds a node by id using linear search. */
    OrderNode* findById(int id) const;
    /** Removes the given node pointer. */
    bool remove(OrderNode* node);
    /** Finds and removes by id. */
    bool removeById(int id);

    template <typename Func>
    void forEach(Func fn) const {
        OrderNode* cur = head_;
        while (cur) {
            fn(cur);
            cur = cur->next;
        }
    }

    size_t size() const { return count_; }
    void clearAll();

private:
    OrderNode* head_{nullptr};
    OrderNode* tail_{nullptr};
    size_t count_{0};

    void clear();
};
