#include "Queue.h"

IntQueue::IntQueue(size_t capacity) : data_(capacity + 1, 0) {}

bool IntQueue::enqueue(int value) {
    size_t nextTail = (tail_ + 1) % data_.size();
    if (nextTail == head_) {
        return false; // full
    }
    data_[tail_] = value;
    tail_ = nextTail;
    ++count_;
    return true;
}

bool IntQueue::dequeue(int& out) {
    if (isEmpty()) {
        return false;
    }
    out = data_[head_];
    head_ = (head_ + 1) % data_.size();
    --count_;
    return true;
}

std::vector<int> IntQueue::snapshot() const {
    std::vector<int> result;
    result.reserve(count_);
    size_t idx = head_;
    for (size_t i = 0; i < count_; ++i) {
        result.push_back(data_[idx]);
        idx = (idx + 1) % data_.size();
    }
    return result;
}
