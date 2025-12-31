#pragma once
#include <cstddef>
#include <vector>

/**
 * Simple circular queue for integer order IDs.
 */
class IntQueue {
public:
    explicit IntQueue(size_t capacity = 128);

    /** Inserts at tail; returns false if full. */
    bool enqueue(int value);
    /** Removes head into out; returns false if empty. */
    bool dequeue(int& out);
    bool isEmpty() const { return count_ == 0; }
    size_t size() const { return count_; }

    /** Returns a copy of current elements in FIFO order. */
    std::vector<int> snapshot() const;

private:
    std::vector<int> data_;
    size_t head_{0};
    size_t tail_{0};
    size_t count_{0};
};
