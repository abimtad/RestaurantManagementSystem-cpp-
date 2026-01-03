#include <iostream>
#include <sstream>
#include <vector>

#include "OrderManager.h"
#include "Persistence.h"
#include "CliUtils.h"

namespace {
void printPathSuggestion(OrderManager& manager, OrderStatus from, OrderStatus to) {
    auto path = manager.shortestPath(from, to);
    if (path.empty()) {
        std::cout << "No allowed path from " << OrderStatusStrings::toString(from)
                  << " to " << OrderStatusStrings::toString(to) << ".\n";
        return;
    }
    std::cout << "Allowed path: ";
    for (size_t i = 0; i < path.size(); ++i) {
        std::cout << OrderStatusStrings::toString(path[i]);
        if (i + 1 < path.size()) std::cout << " -> ";
    }
    std::cout << "\n";
}
}

int main() {
    OrderManager manager;
    const std::string defaultPath = "db.json";

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
            Order* o = manager.getOrder(id);
            if (!o) {
                std::cout << "Not found.\n";
                continue;
            }
            if (cmd == "start") {
                if (manager.startOrder(id)) {
                    std::cout << "Order " << id << " PREPPING.\n";
                } else {
                    std::cout << "Unable to start order.\n";
                    printPathSuggestion(manager, o->status, OrderStatus::Prepping);
                }
            } else if (cmd == "ready") {
                if (manager.readyOrder(id)) {
                    std::cout << "Order " << id << " READY.\n";
                } else {
                    std::cout << "Unable to mark ready.\n";
                    printPathSuggestion(manager, o->status, OrderStatus::Ready);
                }
            } else if (cmd == "serve") {
                if (manager.serveOrder(id)) {
                    std::cout << "Order " << id << " SERVED.\n";
                } else {
                    std::cout << "Unable to serve.\n";
                    printPathSuggestion(manager, o->status, OrderStatus::Served);
                }
            } else if (cmd == "cancel") {
                if (manager.cancelOrder(id)) {
                    std::cout << "Order " << id << " cancelled.\n";
                } else {
                    std::cout << "Unable to cancel.\n";
                    printPathSuggestion(manager, o->status, OrderStatus::Cancelled);
                }
            } else if (cmd == "show") {
                printOrder(*o);
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
