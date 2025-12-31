#include "Persistence.h"
#include <fstream>
#include <sstream>
#include <cctype>

namespace {
    std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == '"' || c == '\\') {
                out.push_back('\\');
            }
            out.push_back(c);
        }
        return out;
    }

    bool readFile(const std::string& path, std::string& out) {
        std::ifstream in(path);
        if (!in.is_open()) return false;
        std::stringstream buffer;
        buffer << in.rdbuf();
        out = buffer.str();
        return true;
    }

    bool writeFile(const std::string& path, const std::string& data) {
        std::ofstream out(path);
        if (!out.is_open()) return false;
        out << data;
        return true;
    }

    std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    bool extractInt(const std::string& src, const std::string& key, int& out) {
        auto pos = src.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = src.find(":", pos);
        if (pos == std::string::npos) return false;
        size_t end = src.find_first_of(",}\n", pos + 1);
        std::string val = trim(src.substr(pos + 1, end - pos - 1));
        try { out = std::stoi(val); } catch (...) { return false; }
        return true;
    }

    bool extractLong(const std::string& src, const std::string& key, long long& out) {
        auto pos = src.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = src.find(":", pos);
        if (pos == std::string::npos) return false;
        size_t end = src.find_first_of(",}\n", pos + 1);
        std::string val = trim(src.substr(pos + 1, end - pos - 1));
        try { out = std::stoll(val); } catch (...) { return false; }
        return true;
    }

    bool extractBool(const std::string& src, const std::string& key, bool& out) {
        auto pos = src.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = src.find(":", pos);
        if (pos == std::string::npos) return false;
        size_t end = src.find_first_of(",}\n", pos + 1);
        std::string val = trim(src.substr(pos + 1, end - pos - 1));
        if (val == "true") { out = true; return true; }
        if (val == "false") { out = false; return true; }
        return false;
    }

    bool extractString(const std::string& src, const std::string& key, std::string& out) {
        auto pos = src.find("\"" + key + "\"");
        if (pos == std::string::npos) return false;
        pos = src.find('"', pos + key.size() + 2);
        if (pos == std::string::npos) return false;
        size_t end = src.find('"', pos + 1);
        if (end == std::string::npos) return false;
        out = src.substr(pos + 1, end - pos - 1);
        return true;
    }

    std::vector<int> parseIntArray(const std::string& src, const std::string& key) {
        std::vector<int> values;
        auto pos = src.find("\"" + key + "\"");
        if (pos == std::string::npos) return values;
        pos = src.find('[', pos);
        if (pos == std::string::npos) return values;
        auto end = src.find(']', pos);
        if (end == std::string::npos) return values;
        std::string content = src.substr(pos + 1, end - pos - 1);
        std::stringstream ss(content);
        std::string token;
        while (std::getline(ss, token, ',')) {
            token = trim(token);
            if (token.empty()) continue;
            try {
                int v = std::stoi(token);
                values.push_back(v);
            } catch (...) {
            }
        }
        return values;
    }
}

bool Persistence::saveState(const OrderManager& manager, const std::string& path) {
    std::ostringstream out;
    auto orders = manager.snapshotAll();
    auto normalIds = manager.normalQueue().snapshot();

    out << "{\n";
    out << "  \"nextId\": " << manager.nextIdValue() << ",\n";
    out << "  \"orders\": [\n";
    for (size_t i = 0; i < orders.size(); ++i) {
        const Order& o = orders[i];
        out << "    {";
        out << "\"id\": " << o.id << ",";
        out << " \"customer\": \"" << escape(o.customerName) << "\",";
        out << " \"vip\": " << (o.isVip ? "true" : "false") << ",";
        out << " \"estimated\": " << o.estimatedPrepMinutes << ",";
        out << " \"status\": \"" << OrderStatusStrings::toString(o.status) << "\",";
        out << " \"placed\": " << TimeUtils::toSeconds(o.placedAt) << ",";
        out << " \"started\": " << TimeUtils::toSeconds(o.startedAt) << ",";
        out << " \"ready\": " << TimeUtils::toSeconds(o.readyAt) << ",";
        out << " \"served\": " << TimeUtils::toSeconds(o.servedAt) << "";
        out << " }";
        if (i + 1 < orders.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n";
    out << "  \"queue\": [";
    for (size_t i = 0; i < normalIds.size(); ++i) {
        out << normalIds[i];
        if (i + 1 < normalIds.size()) out << ",";
    }
    out << "],\n";
    out << "  \"version\": 1\n";
    out << "}\n";

    return writeFile(path, out.str());
}

bool Persistence::loadState(OrderManager& manager, const std::string& path) {
    std::string content;
    if (!readFile(path, content)) {
        return false;
    }

    manager.reset();

    int nextId = 1;
    extractInt(content, "nextId", nextId);
    manager.setNextId(nextId);

    // Parse orders array
    auto ordersPos = content.find("\"orders\"");
    if (ordersPos != std::string::npos) {
        auto arrayStart = content.find('[', ordersPos);
        auto arrayEnd = content.find(']', arrayStart);
        size_t cursor = arrayStart;
        while (true) {
            auto objStart = content.find('{', cursor);
            if (objStart == std::string::npos || objStart > arrayEnd) break;
            auto objEnd = content.find('}', objStart);
            if (objEnd == std::string::npos) break;
            std::string obj = content.substr(objStart, objEnd - objStart + 1);

            Order order;
            extractInt(obj, "id", order.id);
            extractString(obj, "customer", order.customerName);
            extractBool(obj, "vip", order.isVip);
            extractInt(obj, "estimated", order.estimatedPrepMinutes);
            std::string statusText;
            if (extractString(obj, "status", statusText)) {
                OrderStatus s;
                if (OrderStatusStrings::fromString(statusText, s)) {
                    order.status = s;
                }
            }
            long long placed = 0, started = 0, ready = 0, served = 0;
            extractLong(obj, "placed", placed);
            extractLong(obj, "started", started);
            extractLong(obj, "ready", ready);
            extractLong(obj, "served", served);
            order.placedAt = TimeUtils::fromSeconds(placed);
            order.startedAt = TimeUtils::fromSeconds(started);
            order.readyAt = TimeUtils::fromSeconds(ready);
            order.servedAt = TimeUtils::fromSeconds(served);

            manager.registry().pushBack(order);
            cursor = objEnd + 1;
        }
    }

    // Rebuild queues
    auto queueIds = parseIntArray(content, "queue");
    for (int id : queueIds) {
        Order* order = manager.getOrder(id);
        if (order && !order->isVip && (order->status == OrderStatus::Queued || order->status == OrderStatus::Placed)) {
            manager.normalQueue().enqueue(order->id);
        }
    }

    // Rebuild VIP heap from orders flagged as VIP and queued/placed
    manager.registry().forEach([&](OrderNode* node) {
        Order& o = node->data;
        if (o.isVip && (o.status == OrderStatus::Queued || o.status == OrderStatus::Placed)) {
            VipEntry e{o.id, TimeUtils::toSeconds(o.placedAt)};
            manager.vipHeap().push(e);
        }
    });

    return true;
}
