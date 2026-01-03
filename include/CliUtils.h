#pragma once

#include <string>
#include <vector>

#include "OrderManager.h"

void clearScreen();
std::string readLine();
bool readPositiveInt(const std::string& prompt, int& out);
bool gatherOrderInput(OrderManager& manager, std::string& customer, bool& vip, int& estimate, std::vector<OrderItem>& items);
bool parseId(const std::string& token, int& out);
void printHelp();
void printOrder(const Order& o);
void printOrdersTable(const std::vector<Order>& orders);
void printMenuTable(const std::vector<MenuItem>& items);
void sortOrders(std::vector<Order>& orders, const std::string& metric);
