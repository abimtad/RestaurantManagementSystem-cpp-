#pragma once
#include <string>
#include <vector>
#include <chrono>

/**
 * Represents a single menu item inside an order.
 */
struct OrderItem {
    int itemId{0};
    std::string name;
    int quantity{0};
};

/**
 * Lifecycle states for an order.
 */
enum class OrderStatus {
    Placed,
    Queued,
    Prepping,
    Ready,
    Served,
    Cancelled
};

/**
 * Core order data structure.
 */
struct Order {
    int id{0};
    std::string customerName;
    bool isVip{false};
    std::vector<OrderItem> items;
    OrderStatus status{OrderStatus::Placed};
    int estimatedPrepMinutes{0};

    std::chrono::system_clock::time_point placedAt{};
    std::chrono::system_clock::time_point startedAt{};
    std::chrono::system_clock::time_point readyAt{};
    std::chrono::system_clock::time_point servedAt{};
};

/**
 * Helpers for converting order status to and from strings for persistence.
 */
namespace OrderStatusStrings {
    std::string toString(OrderStatus status);
    bool fromString(const std::string& text, OrderStatus& out);
}

/**
 * Utility conversions for timestamps.
 */
namespace TimeUtils {
    long long toSeconds(const std::chrono::system_clock::time_point& tp);
    std::chrono::system_clock::time_point fromSeconds(long long seconds);
}
