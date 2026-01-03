#pragma once
#include <vector>
#include <string>
#include "Order.h"
#include "LinkedList.h"
#include "Queue.h"
#include "Heap.h"
#include "MenuBST.h"
#include "WorkflowGraph.h"

/**
 * Coordinates all order operations and scheduling structures.
 */
class OrderManager {
public:
    OrderManager();

    /** Creates a new order and enqueues it. */
    OrderNode* createOrder(const std::string& customerName, bool isVip, const std::vector<OrderItem>& items, int estimatedPrepMinutes);
    /** Edits an order if it is not served/cancelled. */
    bool editOrder(int id, const std::string& customerName, bool isVip, const std::vector<OrderItem>& items, int estimatedPrepMinutes);
    /** Cancels an order if still active. */
    bool cancelOrder(int id);

    /** Marks an order as PREPPING. */
    bool startOrder(int id);
    /** Marks an order as READY. */
    bool readyOrder(int id);
    /** Marks an order as SERVED. */
    bool serveOrder(int id);

    /** Returns the next order id for the kitchen; applies VIP priority. */
    bool nextForKitchen(int& orderId);

    /** Finds an order by id. */
    Order* getOrder(int id);
    /** Lists orders filtered by status. */
    std::vector<Order> listByStatus(OrderStatus status) const;

    size_t activeCount() const { return active_.size(); }
    int nextIdValue() const { return nextId_; }
    void setNextId(int value) { nextId_ = value; }
    int nextMenuIdValue() const { return nextMenuId_; }
    void setNextMenuId(int value) { nextMenuId_ = value; }
    /** Clears all internal structures to allow a fresh load from disk. */
    void reset();

    /** Shortest valid status path using workflow graph; empty if unreachable. */
    std::vector<OrderStatus> shortestPath(OrderStatus from, OrderStatus to) const;

    /** Menu operations using BST. */
    int addMenuItem(const std::string& name, int defaultPrepMinutes, int itemId = 0);
    bool removeMenuItem(const std::string& name);
    MenuItem* findMenuItem(const std::string& name);
    std::vector<MenuItem> listMenuItems() const;

    // Expose internal snapshots for persistence
    std::vector<Order> snapshotAll() const;
    IntQueue& normalQueue() { return normalQueue_; }
    const IntQueue& normalQueue() const { return normalQueue_; }
    VipHeap& vipHeap() { return vipHeap_; }
    const VipHeap& vipHeap() const { return vipHeap_; }
    OrderList& registry() { return active_; }
    const OrderList& registry() const { return active_; }
    const MenuBST& menu() const { return menu_; }
    MenuBST& menu() { return menu_; }

private:
    OrderList active_;
    IntQueue normalQueue_;
    VipHeap vipHeap_;
    WorkflowGraph workflow_;
    MenuBST menu_;
    int nextId_{1};
    int nextMenuId_{1};

    bool transition(Order& order, OrderStatus to);
};
