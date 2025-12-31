#include "Sorts.h"

namespace {
    void mergeRange(std::vector<Order>& items, std::vector<Order>& buffer, int left, int mid, int right, const std::function<bool(const Order&, const Order&)>& cmp) {
        int i = left;
        int j = mid + 1;
        int k = left;
        while (i <= mid && j <= right) {
            if (cmp(items[i], items[j])) {
                buffer[k++] = items[i++];
            } else {
                buffer[k++] = items[j++];
            }
        }
        while (i <= mid) buffer[k++] = items[i++];
        while (j <= right) buffer[k++] = items[j++];
        for (int idx = left; idx <= right; ++idx) {
            items[idx] = buffer[idx];
        }
    }

    void mergeSortInternal(std::vector<Order>& items, std::vector<Order>& buffer, int left, int right, const std::function<bool(const Order&, const Order&)>& cmp) {
        if (left >= right) return;
        int mid = left + (right - left) / 2;
        mergeSortInternal(items, buffer, left, mid, cmp);
        mergeSortInternal(items, buffer, mid + 1, right, cmp);
        mergeRange(items, buffer, left, mid, right, cmp);
    }
}

void Sorts::selectionSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp) {
    int n = static_cast<int>(items.size());
    for (int i = 0; i < n - 1; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < n; ++j) {
            if (cmp(items[j], items[minIndex])) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            std::swap(items[i], items[minIndex]);
        }
    }
}

void Sorts::insertionSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp) {
    int n = static_cast<int>(items.size());
    for (int i = 1; i < n; ++i) {
        Order key = items[i];
        int j = i - 1;
        while (j >= 0 && cmp(key, items[j])) {
            items[j + 1] = items[j];
            --j;
        }
        items[j + 1] = key;
    }
}

void Sorts::bubbleSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp) {
    int n = static_cast<int>(items.size());
    bool swapped = true;
    while (swapped) {
        swapped = false;
        for (int i = 0; i < n - 1; ++i) {
            if (cmp(items[i + 1], items[i])) {
                std::swap(items[i], items[i + 1]);
                swapped = true;
            }
        }
        --n;
    }
}

void Sorts::mergeSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp) {
    if (items.empty()) return;
    std::vector<Order> buffer(items.size());
    mergeSortInternal(items, buffer, 0, static_cast<int>(items.size()) - 1, cmp);
}
