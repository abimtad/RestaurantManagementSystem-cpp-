#include "CliUtils.h"

#include "Sorts.h"

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {
long long placedSeconds(const Order& o) {
    return TimeUtils::toSeconds(o.placedAt);
}

long long servedSeconds(const Order& o) {
    return TimeUtils::toSeconds(o.servedAt);
}

long long totalOrEstimateSeconds(const Order& o) {
    long long placed = placedSeconds(o);
    long long served = servedSeconds(o);
    if (placed > 0 && served > placed) {
        return served - placed;
    }
    return static_cast<long long>(o.estimatedPrepMinutes) * 60;
}

std::string formatTimestamp(long long seconds) {
    if (seconds <= 0) return "-";
    std::time_t tt = static_cast<std::time_t>(seconds);
    std::tm* tm = std::localtime(&tt);
    if (!tm) return "-";
    std::ostringstream ss;
    ss << std::put_time(tm, "%d/%m/%Y %I:%M %p");
    std::string out = ss.str();
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

bool readPositiveInt(const std::string& prompt, int& out) {
    std::cout << prompt;
    std::string line = readLine();
    try {
        int v = std::stoi(line);
        if (v > 0) {
            out = v;
            return true;
        }
    } catch (...) {
    }
    std::cout << "Invalid number, must be > 0.\n";
    return false;
}

bool gatherOrderInput(OrderManager& manager, std::string& customer, bool& vip, int& estimate, std::vector<OrderItem>& items) {
    std::cout << "Customer name: ";
    customer = readLine();
    if (customer.empty()) {
        std::cout << "Customer name required.\n";
        return false;
    }
    std::cout << "VIP (y/n): ";
    std::string vipInput = readLine();
    vip = (!vipInput.empty() && (vipInput[0] == 'y' || vipInput[0] == 'Y'));

    std::cout << "Enter items (blank name to finish):\n";
    items.clear();
    int computedEstimate = 0;
    while (true) {
        std::cout << "  Item name: ";
        std::string name = readLine();
        if (name.empty()) break;
        int qty = 0;
        if (!readPositiveInt("  Quantity: ", qty)) {
            std::cout << "  Skipping this item due to invalid quantity.\n";
            continue;
        }
        OrderItem it;
        it.name = name;
        it.quantity = qty;
        MenuItem* menu = manager.findMenuItem(name);
        if (menu) {
            it.itemId = menu->itemId;
            computedEstimate += menu->defaultPrepMinutes * qty;
        }
        items.push_back(it);
    }

    if (computedEstimate > 0) {
        estimate = computedEstimate;
        std::cout << "Estimated prep minutes (from menu defaults): " << estimate << "\n";
    } else {
        if (!readPositiveInt("Estimated prep minutes: ", estimate)) {
            return false;
        }
    }
    return true;
}

bool parseId(const std::string& token, int& out) {
    try {
        out = std::stoi(token);
        return true;
    } catch (...) {
        return false;
    }
}

void printHelp() {
    std::cout << "Commands:\n"
              << "  new                 - create a new order\n"
              << "  edit <id>           - edit an order (name, VIP, items, estimate)\n"
              << "  next                - pull next order for kitchen (VIP first)\n"
              << "  start <id>          - mark order as PREPPING\n"
              << "  ready <id>          - mark order as READY\n"
              << "  serve <id>          - mark order as SERVED\n"
              << "  cancel <id>         - cancel an order\n"
              << "  show <id>           - show order details\n"
              << "  list [status]       - list orders (all or by status)\n"
              << "  report active       - list active orders sorted (placed time)\n"
              << "  report completed    - list completed orders sorted (served time)\n"
              << "  find <id>           - find order by id\n"
              << "  menu add/remove/find/list - manage menu (BST)\n"
              << "  save [path]         - save state to JSON (default data.json)\n"
              << "  load [path]         - load state from JSON (default data.json)\n"
              << "  clear               - clear the console\n"
              << "  help                - show this help\n"
              << "  exit                - quit\n";
}

void printOrder(const Order& o) {
    std::cout << "Order " << o.id << " | " << o.customerName
              << " | VIP: " << (o.isVip ? "yes" : "no")
              << " | Status: " << OrderStatusStrings::toString(o.status)
              << " | Est: " << o.estimatedPrepMinutes << " min\n";
    if (o.items.empty()) {
        std::cout << "  Items: none\n";
    } else {
        std::cout << "  Items:\n";
        for (const auto& it : o.items) {
            std::cout << "    - ";
            if (it.itemId > 0) {
                std::cout << "#" << it.itemId << " ";
            }
            std::cout << it.name << " x" << it.quantity << "\n";
        }
    }
}

void printOrdersTable(const std::vector<Order>& orders) {
    if (orders.empty()) {
        std::cout << "No orders.\n";
        return;
    }

    auto timeStr = [](long long s) {
        return formatTimestamp(s);
    };

    std::cout << std::left
              << std::setw(5) << "ID"
              << std::setw(14) << "Customer"
              << std::setw(5) << "VIP"
              << std::setw(10) << "Status"
              << std::setw(8) << "Est"
              << std::setw(20) << "Placed"
              << std::setw(20) << "Started"
              << std::setw(20) << "Ready"
              << std::setw(20) << "Served"
              << "\n";
    std::cout << std::string(122, '-') << "\n";

    for (const auto& o : orders) {
        std::cout << std::left
                  << std::setw(5) << o.id
                  << std::setw(14) << o.customerName.substr(0, 13)
                  << std::setw(5) << (o.isVip ? "yes" : "no")
                  << std::setw(10) << OrderStatusStrings::toString(o.status)
                  << std::setw(8) << o.estimatedPrepMinutes
                  << std::setw(20) << timeStr(placedSeconds(o))
                  << std::setw(20) << timeStr(TimeUtils::toSeconds(o.startedAt))
                  << std::setw(20) << timeStr(TimeUtils::toSeconds(o.readyAt))
                  << std::setw(20) << timeStr(TimeUtils::toSeconds(o.servedAt))
                  << "\n";
    }
}

void printMenuTable(const std::vector<MenuItem>& items) {
    if (items.empty()) {
        std::cout << "Menu empty.\n";
        return;
    }
    std::cout << std::left
              << std::setw(6) << "ID"
              << std::setw(18) << "Name"
              << std::setw(10) << "Prep(min)"
              << "\n";
    std::cout << std::string(40, '-') << "\n";
    for (const auto& m : items) {
        std::cout << std::left
                  << std::setw(6) << m.itemId
                  << std::setw(18) << m.name.substr(0, 17)
                  << std::setw(10) << m.defaultPrepMinutes
                  << "\n";
    }
}

void sortOrders(std::vector<Order>& orders, const std::string& metric) {
    auto cmp = [&](const Order& a, const Order& b) {
        if (metric == "prep") return a.estimatedPrepMinutes < b.estimatedPrepMinutes;
        if (metric == "total") return totalOrEstimateSeconds(a) < totalOrEstimateSeconds(b);
        if (metric == "served") return servedSeconds(a) < servedSeconds(b);
        return placedSeconds(a) < placedSeconds(b);
    };
    Sorts::mergeSort(orders, cmp);
}
