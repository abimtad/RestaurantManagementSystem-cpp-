#include "Heap.h"

void VipHeap::push(const VipEntry& entry) {
    data_.push_back(entry);
    heapifyUp(data_.size() - 1);
}

bool VipHeap::pop(VipEntry& out) {
    if (data_.empty()) {
        return false;
    }
    out = data_.front();
    data_[0] = data_.back();
    data_.pop_back();
    if (!data_.empty()) {
        heapifyDown(0);
    }
    return true;
}

std::vector<int> VipHeap::snapshotIds() const {
    std::vector<int> ids;
    ids.reserve(data_.size());
    for (const auto& e : data_) {
        ids.push_back(e.orderId);
    }
    return ids;
}

bool VipHeap::higherPriority(const VipEntry& a, const VipEntry& b) {
    return a.placedAtSeconds < b.placedAtSeconds;
}

void VipHeap::heapifyUp(size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (higherPriority(data_[idx], data_[parent])) {
            std::swap(data_[idx], data_[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void VipHeap::heapifyDown(size_t idx) {
    size_t n = data_.size();
    while (true) {
        size_t left = idx * 2 + 1;
        size_t right = idx * 2 + 2;
        size_t smallest = idx;
        if (left < n && higherPriority(data_[left], data_[smallest])) {
            smallest = left;
        }
        if (right < n && higherPriority(data_[right], data_[smallest])) {
            smallest = right;
        }
        if (smallest != idx) {
            std::swap(data_[idx], data_[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}
