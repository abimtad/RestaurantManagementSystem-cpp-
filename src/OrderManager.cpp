#include "OrderManager.h"
#include <chrono>

OrderManager::OrderManager() : normalQueue_(256) {}

OrderNode* OrderManager::createOrder(const std::string& customerName, bool isVip, const std::vector<OrderItem>& items, int estimatedPrepMinutes) {
    Order order;
    order.id = nextId_++;
    order.customerName = customerName;
    order.isVip = isVip;
    order.items = items;
    order.estimatedPrepMinutes = estimatedPrepMinutes;
    order.placedAt = std::chrono::system_clock::now();
    order.status = OrderStatus::Placed;

    OrderNode* node = active_.pushBack(order);
    // Move to queued state and enqueue
    transition(node->data, OrderStatus::Queued);
    if (isVip) {
        VipEntry e;
        e.orderId = node->data.id;
        e.placedAtSeconds = TimeUtils::toSeconds(node->data.placedAt);
        vipHeap_.push(e);
    } else {
        normalQueue_.enqueue(node->data.id);
    }
    return node;
}

bool OrderManager::editOrder(int id, const std::string& customerName, bool isVip, const std::vector<OrderItem>& items, int estimatedPrepMinutes) {
    OrderNode* node = active_.findById(id);
    if (!node) return false;
    Order& ord = node->data;
    if (ord.status == OrderStatus::Cancelled || ord.status == OrderStatus::Served) {
        return false;
    }
    ord.customerName = customerName;
    ord.items = items;
    ord.estimatedPrepMinutes = estimatedPrepMinutes;

    if (ord.isVip != isVip) {
        ord.isVip = isVip;
        if (isVip) {
            VipEntry e{ord.id, TimeUtils::toSeconds(ord.placedAt)};
            vipHeap_.push(e);
        } else {
            normalQueue_.enqueue(ord.id);
        }
    }
    return true;
}

bool OrderManager::cancelOrder(int id) {
    OrderNode* node = active_.findById(id);
    if (!node) return false;
    Order& ord = node->data;
    if (!transition(ord, OrderStatus::Cancelled)) {
        return false;
    }
    return true;
}

bool OrderManager::startOrder(int id) {
    OrderNode* node = active_.findById(id);
    if (!node) return false;
    Order& ord = node->data;
    if (!transition(ord, OrderStatus::Prepping)) return false;
    ord.startedAt = std::chrono::system_clock::now();
    return true;
}

bool OrderManager::readyOrder(int id) {
    OrderNode* node = active_.findById(id);
    if (!node) return false;
    Order& ord = node->data;
    if (!transition(ord, OrderStatus::Ready)) return false;
    ord.readyAt = std::chrono::system_clock::now();
    return true;
}

bool OrderManager::serveOrder(int id) {
    OrderNode* node = active_.findById(id);
    if (!node) return false;
    Order& ord = node->data;
    if (!transition(ord, OrderStatus::Served)) return false;
    ord.servedAt = std::chrono::system_clock::now();
    return true;
}

std::vector<OrderStatus> OrderManager::shortestPath(OrderStatus from, OrderStatus to) const {
    return workflow_.shortestPath(from, to);
}

bool OrderManager::nextForKitchen(int& orderId) {
    // Prefer VIP
    VipEntry top{};
    while (vipHeap_.pop(top)) {
        OrderNode* node = active_.findById(top.orderId);
        if (!node) continue;
        Order& ord = node->data;
        if (ord.status == OrderStatus::Queued || ord.status == OrderStatus::Placed) {
            orderId = ord.id;
            startOrder(orderId);
            return true;
        }
    }

    int fromQueue = 0;
    while (normalQueue_.dequeue(fromQueue)) {
        OrderNode* node = active_.findById(fromQueue);
        if (!node) continue;
        Order& ord = node->data;
        if (ord.status == OrderStatus::Queued || ord.status == OrderStatus::Placed) {
            orderId = ord.id;
            startOrder(orderId);
            return true;
        }
    }
    return false;
}

Order* OrderManager::getOrder(int id) {
    OrderNode* node = active_.findById(id);
    if (!node) return nullptr;
    return &node->data;
}

std::vector<Order> OrderManager::listByStatus(OrderStatus status) const {
    std::vector<Order> out;
    active_.forEach([&](OrderNode* node) {
        if (node->data.status == status) {
            out.push_back(node->data);
        }
    });
    return out;
}

std::vector<Order> OrderManager::snapshotAll() const {
    std::vector<Order> out;
    active_.forEach([&](OrderNode* node) {
        out.push_back(node->data);
    });
    return out;
}

bool OrderManager::transition(Order& order, OrderStatus to) {
    if (order.status == to) return true;
    if (!workflow_.canTransition(order.status, to)) {
        return false;
    }
    order.status = to;
    return true;
}

void OrderManager::reset() {
    active_.clearAll();
    normalQueue_ = IntQueue(256);
    vipHeap_ = VipHeap();
    menu_ = MenuBST();
    nextId_ = 1;
    nextMenuId_ = 1;
}

int OrderManager::addMenuItem(const std::string& name, int defaultPrepMinutes, int itemId) {
    if (name.empty() || defaultPrepMinutes <= 0) return -1;
    MenuItem item;
    if (itemId <= 0) {
        item.itemId = nextMenuId_++;
    } else {
        item.itemId = itemId;
        if (itemId >= nextMenuId_) {
            nextMenuId_ = itemId + 1;
        }
    }
    item.name = name;
    item.defaultPrepMinutes = defaultPrepMinutes;
    bool inserted = menu_.insert(item);
    if (!inserted) {
        return -1;
    }
    return item.itemId;
}

bool OrderManager::removeMenuItem(const std::string& name) {
    return menu_.remove(name);
}

MenuItem* OrderManager::findMenuItem(const std::string& name) {
    return menu_.find(name);
}

std::vector<MenuItem> OrderManager::listMenuItems() const {
    std::vector<MenuItem> items;
    menu_.inOrder([&](const MenuItem& m) {
        items.push_back(m);
    });
    return items;
}
