#include "WorkflowGraph.h"

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
