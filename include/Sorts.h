#pragma once
#include <vector>
#include <functional>
#include "Order.h"

/**
 * Collection of manual sorting implementations for educational use.
 */
namespace Sorts {
    void selectionSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp);
    void insertionSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp);
    void bubbleSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp);
    void mergeSort(std::vector<Order>& items, const std::function<bool(const Order&, const Order&)>& cmp);
}
