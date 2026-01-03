#include "WorkflowGraph.h"

#include <algorithm>
#include <queue>

WorkflowGraph::WorkflowGraph() {
    // Initialize all to false
    for (auto& row : allowed_) {
        row.fill(false);
    }
    // Define valid transitions
    allowed_[static_cast<int>(OrderStatus::Placed)][static_cast<int>(OrderStatus::Queued)] = true;
    allowed_[static_cast<int>(OrderStatus::Queued)][static_cast<int>(OrderStatus::Prepping)] = true;
    allowed_[static_cast<int>(OrderStatus::Prepping)][static_cast<int>(OrderStatus::Ready)] = true;
    allowed_[static_cast<int>(OrderStatus::Ready)][static_cast<int>(OrderStatus::Served)] = true;

    // Cancellation is allowed from any active state
    allowed_[static_cast<int>(OrderStatus::Placed)][static_cast<int>(OrderStatus::Cancelled)] = true;
    allowed_[static_cast<int>(OrderStatus::Queued)][static_cast<int>(OrderStatus::Cancelled)] = true;
    allowed_[static_cast<int>(OrderStatus::Prepping)][static_cast<int>(OrderStatus::Cancelled)] = true;
    allowed_[static_cast<int>(OrderStatus::Ready)][static_cast<int>(OrderStatus::Cancelled)] = true;
}

bool WorkflowGraph::canTransition(OrderStatus from, OrderStatus to) const {
    int f = static_cast<int>(from);
    int t = static_cast<int>(to);
    if (f < 0 || f >= 6 || t < 0 || t >= 6) {
        return false;
    }
    return allowed_[f][t];
}

std::vector<OrderStatus> WorkflowGraph::shortestPath(OrderStatus from, OrderStatus to) const {
    int start = static_cast<int>(from);
    int goal = static_cast<int>(to);
    if (start == goal) {
        return {from};
    }

    const int n = static_cast<int>(allowed_.size());
    std::vector<int> prev(n, -1);
    std::vector<bool> visited(n, false);
    std::queue<int> q;

    q.push(start);
    visited[start] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v = 0; v < n; ++v) {
            if (!allowed_[u][v] || visited[v]) {
                continue;
            }
            visited[v] = true;
            prev[v] = u;
            if (v == goal) {
                break;
            }
            q.push(v);
        }
    }

    if (!visited[goal]) {
        return {};
    }

    std::vector<OrderStatus> path;
    for (int at = goal; at != -1; at = prev[at]) {
        path.push_back(static_cast<OrderStatus>(at));
    }
    std::reverse(path.begin(), path.end());
    return path;
}
