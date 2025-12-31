#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cctype>
#include "OrderManager.h"
#include "Persistence.h"
#include "Sorts.h"

/**
 * Clears the console to keep the CLI tidy.
 */
void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

/**
 * Reads a single line from stdin.
 */
std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

/**
 * Helper to convert duration to seconds if timestamps exist; returns 0 when missing.
 */
long long secondsBetween(const std::chrono::system_clock::time_point& a, const std::chrono::system_clock::time_point& b) {
    if (a.time_since_epoch().count() == 0 || b.time_since_epoch().count() == 0) return 0;
    return std::chrono::duration_cast<std::chrono::seconds>(b - a).count();
}

/**
 * Prints a brief description of commands.
 */
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

/**
 * Prints a human readable view of an order.
 */
void printOrder(const Order& o) {
    std::cout << "Order " << o.id << " | " << o.customerName
              << " | VIP: " << (o.isVip ? "yes" : "no")
              << " | Status: " << OrderStatusStrings::toString(o.status)
              << " | Est: " << o.estimatedPrepMinutes << " min\n";
}

bool readPositiveInt(const std::string& prompt, int& out) {
    std::cout << prompt;
    std::string line = readLine();
    try {
        int v = std::stoi(line);
        if (v > 0) { out = v; return true; }
    } catch (...) {}
    std::cout << "Invalid number, must be > 0.\n";
    return false;
}

/**
 * Gathers order info and items from user; uses menu defaults when available.
 * Returns false if critical input is invalid.
 */
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

/**
 * Parses an integer id argument.
 */
bool parseId(const std::string& token, int& out) {
    try {
        out = std::stoi(token);
        return true;
    } catch (...) {
        return false;
    }
}

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

/**
 * Formats epoch seconds into dd/mm/yyyy hh:mm am/pm, or "-" if unset.
 */
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

/**
 * Pretty table for order lists.
 */
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

/**
 * Table for menu items.
 */
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
        // default: placed time
        return placedSeconds(a) < placedSeconds(b);
    };
    Sorts::mergeSort(orders, cmp);
}

int main() {
    OrderManager manager;
    const std::string defaultPath = "data.json";

    // Auto-load from db.json on first run if present
    Persistence::loadState(manager, "db.json");

    clearScreen();
    std::cout << "Restaurant Management CLI (DSA edition)\n";
    printHelp();

    while (true) {
        std::cout << "\n> ";
        std::string line = readLine();
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "help") {
            printHelp();
        } else if (cmd == "clear") {
            clearScreen();
        } else if (cmd == "new") {
            std::string customer;
            bool vip = false;
            int estimate = 0;
            std::vector<OrderItem> items;
            if (!gatherOrderInput(manager, customer, vip, estimate, items)) {
                std::cout << "Order creation aborted due to invalid input.\n";
                continue;
            }
            OrderNode* node = manager.createOrder(customer, vip, items, estimate);
            printOrder(node->data);
        } else if (cmd == "edit") {
            std::string idToken;
            ss >> idToken;
            int id = 0;
            if (!parseId(idToken, id)) {
                std::cout << "Invalid id.\n";
                continue;
            }
            std::string customer;
            bool vip = false;
            int estimate = 0;
            std::vector<OrderItem> items;
            if (!gatherOrderInput(manager, customer, vip, estimate, items)) {
                std::cout << "Edit aborted due to invalid input.\n";
                continue;
            }
            if (manager.editOrder(id, customer, vip, items, estimate)) {
                std::cout << "Order " << id << " updated.\n";
            } else {
                std::cout << "Unable to edit order.\n";
            }
        } else if (cmd == "next") {
            int nextId = 0;
            if (manager.nextForKitchen(nextId)) {
                std::cout << "Next order: " << nextId << " now PREPPING\n";
            } else {
                std::cout << "No orders pending.\n";
            }
        } else if (cmd == "start" || cmd == "ready" || cmd == "serve" || cmd == "cancel" || cmd == "show") {
            std::string idToken;
            ss >> idToken;
            int id = 0;
            if (!parseId(idToken, id)) {
                std::cout << "Invalid id.\n";
                continue;
            }
            if (cmd == "start") {
                if (manager.startOrder(id)) std::cout << "Order " << id << " PREPPING.\n";
                else std::cout << "Unable to start order.\n";
            } else if (cmd == "ready") {
                if (manager.readyOrder(id)) std::cout << "Order " << id << " READY.\n";
                else std::cout << "Unable to mark ready.\n";
            } else if (cmd == "serve") {
                if (manager.serveOrder(id)) std::cout << "Order " << id << " SERVED.\n";
                else std::cout << "Unable to serve.\n";
            } else if (cmd == "cancel") {
                if (manager.cancelOrder(id)) std::cout << "Order " << id << " cancelled.\n";
                else std::cout << "Unable to cancel.\n";
            } else if (cmd == "show") {
                Order* o = manager.getOrder(id);
                if (o) printOrder(*o); else std::cout << "Not found.\n";
            }
        } else if (cmd == "list") {
            std::string statusToken;
            ss >> statusToken;
            if (statusToken.empty()) {
                auto all = manager.snapshotAll();
                sortOrders(all, "placed");
                printOrdersTable(all);
            } else {
                OrderStatus status;
                if (!OrderStatusStrings::fromString(statusToken, status)) {
                    std::cout << "Unknown status token.\n";
                    continue;
                }
                auto orders = manager.listByStatus(status);
                sortOrders(orders, "placed");
                printOrdersTable(orders);
            }
        } else if (cmd == "report") {
            std::string which;
            ss >> which;
            if (which == "active") {
                auto active = manager.listByStatus(OrderStatus::Placed);
                auto queued = manager.listByStatus(OrderStatus::Queued);
                auto prepping = manager.listByStatus(OrderStatus::Prepping);
                auto ready = manager.listByStatus(OrderStatus::Ready);
                active.insert(active.end(), queued.begin(), queued.end());
                active.insert(active.end(), prepping.begin(), prepping.end());
                active.insert(active.end(), ready.begin(), ready.end());
                sortOrders(active, "placed");
                printOrdersTable(active);
            } else if (which == "completed") {
                auto done = manager.listByStatus(OrderStatus::Served);
                sortOrders(done, "served");
                printOrdersTable(done);
            } else {
                std::cout << "Usage: report active|completed\n";
            }
        } else if (cmd == "find") {
            std::string idToken;
            ss >> idToken;
            int id = 0;
            if (!parseId(idToken, id)) {
                std::cout << "Invalid id.\n";
                continue;
            }
            Order* o = manager.getOrder(id);
            if (o) printOrder(*o); else std::cout << "Not found.\n";
        } else if (cmd == "menu") {
            std::string sub;
            ss >> sub;
            if (sub == "add") {
                std::string name;
                std::cout << "Menu name: ";
                name = readLine();
                if (name.empty()) {
                    std::cout << "Name required.\n";
                    continue;
                }
                int prep = 0;
                if (!readPositiveInt("Default prep minutes: ", prep)) {
                    std::cout << "Menu add aborted.\n";
                    continue;
                }
                int assigned = manager.addMenuItem(name, prep);
                if (assigned > 0) std::cout << "Menu item added with id " << assigned << ".\n"; else std::cout << "Duplicate item name or invalid data.\n";
            } else if (sub == "remove") {
                std::string name;
                std::cout << "Menu name: ";
                name = readLine();
                if (manager.removeMenuItem(name)) std::cout << "Removed.\n"; else std::cout << "Not found.\n";
            } else if (sub == "find") {
                std::string name;
                std::cout << "Menu name: ";
                name = readLine();
                MenuItem* item = manager.findMenuItem(name);
                if (item) {
                    std::cout << "Item " << item->itemId << " | " << item->name << " | prep: " << item->defaultPrepMinutes << " min\n";
                } else {
                    std::cout << "Not found.\n";
                }
            } else if (sub == "list") {
                auto items = manager.listMenuItems();
                printMenuTable(items);
            } else {
                std::cout << "Usage: menu add|remove|find|list\n";
            }
        } else if (cmd == "save" || cmd == "load") {
            std::string path;
            ss >> path;
            if (path.empty()) path = defaultPath;
            if (cmd == "save") {
                if (Persistence::saveState(manager, path)) {
                    std::cout << "Saved to " << path << "\n";
                } else {
                    std::cout << "Save failed.\n";
                }
            } else {
                if (Persistence::loadState(manager, path)) {
                    std::cout << "Loaded from " << path << "\n";
                } else {
                    std::cout << "Load failed.\n";
                }
            }
        } else if (cmd == "exit" || cmd == "quit") {
            std::cout << "Goodbye.\n";
            break;
        } else {
            std::cout << "Unknown command. Type 'help'.\n";
        }
    }
    return 0;
}
