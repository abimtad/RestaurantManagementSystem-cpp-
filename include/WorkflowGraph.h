#pragma once
#include "Order.h"
#include <array>
#include <vector>

/**
 * Simple directed graph encoded as adjacency matrix for allowed status transitions.
 */
class WorkflowGraph {
public:
    WorkflowGraph();
    /** Returns true if moving from -> to is allowed. */
    bool canTransition(OrderStatus from, OrderStatus to) const;
    /** Returns the shortest legal status path from -> to (inclusive); empty if unreachable. */
    std::vector<OrderStatus> shortestPath(OrderStatus from, OrderStatus to) const;

private:
    std::array<std::array<bool, 6>, 6> allowed_{};
};
