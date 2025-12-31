#include "LinkedList.h"

OrderList::OrderList() = default;

OrderList::~OrderList() {
    clear();
}

OrderNode* OrderList::pushBack(const Order& order) {
    OrderNode* node = new OrderNode();
    node->data = order;
    node->prev = tail_;
    if (tail_) {
        tail_->next = node;
    }
    tail_ = node;
    if (!head_) {
        head_ = node;
    }
    ++count_;
    return node;
}

OrderNode* OrderList::findById(int id) const {
    OrderNode* cur = head_;
    while (cur) {
        if (cur->data.id == id) {
            return cur;
        }
        cur = cur->next;
    }
    return nullptr;
}

bool OrderList::remove(OrderNode* node) {
    if (!node) {
        return false;
    }
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        head_ = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        tail_ = node->prev;
    }
    delete node;
    --count_;
    return true;
}

bool OrderList::removeById(int id) {
    OrderNode* node = findById(id);
    return remove(node);
}

void OrderList::clear() {
    OrderNode* cur = head_;
    while (cur) {
        OrderNode* next = cur->next;
        delete cur;
        cur = next;
    }
    head_ = nullptr;
    tail_ = nullptr;
    count_ = 0;
}

void OrderList::clearAll() {
    clear();
}
