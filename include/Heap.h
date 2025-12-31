#pragma once
#include <vector>
#include <cstddef>

/**
 * Entry for VIP heap. Lower placedAtSeconds means higher priority (earlier arrival wins among VIPs).
 */
struct VipEntry {
    int orderId{0};
    long long placedAtSeconds{0};
};

/**
 * Min-heap implemented manually to avoid std::priority_queue.
 */
class VipHeap {
public:
    VipHeap() = default;

    /** Adds a VIP entry into the heap. */
    void push(const VipEntry& entry);
    /** Pops the highest priority entry; returns false if empty. */
    bool pop(VipEntry& out);
    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }
    /** Snapshot of order ids in the heap (heap order not guaranteed). */
    std::vector<int> snapshotIds() const;

private:
    std::vector<VipEntry> data_;

    void heapifyUp(size_t idx);
    void heapifyDown(size_t idx);
    static bool higherPriority(const VipEntry& a, const VipEntry& b);
};
