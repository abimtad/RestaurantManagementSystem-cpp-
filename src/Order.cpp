#include "Order.h"
#include <sstream>

std::string OrderStatusStrings::toString(OrderStatus status) {
    switch (status) {
        case OrderStatus::Placed: return "PLACED";
        case OrderStatus::Queued: return "QUEUED";
        case OrderStatus::Prepping: return "PREPPING";
        case OrderStatus::Ready: return "READY";
        case OrderStatus::Served: return "SERVED";
        case OrderStatus::Cancelled: return "CANCELLED";
    }
    return "UNKNOWN";
}

bool OrderStatusStrings::fromString(const std::string& text, OrderStatus& out) {
    if (text == "PLACED") { out = OrderStatus::Placed; return true; }
    if (text == "QUEUED") { out = OrderStatus::Queued; return true; }
    if (text == "PREPPING") { out = OrderStatus::Prepping; return true; }
    if (text == "READY") { out = OrderStatus::Ready; return true; }
    if (text == "SERVED") { out = OrderStatus::Served; return true; }
    if (text == "CANCELLED") { out = OrderStatus::Cancelled; return true; }
    return false;
}

long long TimeUtils::toSeconds(const std::chrono::system_clock::time_point& tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

std::chrono::system_clock::time_point TimeUtils::fromSeconds(long long seconds) {
    return std::chrono::system_clock::time_point(std::chrono::seconds(seconds));
}
