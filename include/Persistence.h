#pragma once
#include <string>
#include "OrderManager.h"

/**
 * Handles saving and restoring state to a JSON file so the program can resume after a crash or halt.
 */
namespace Persistence {
    /** Writes manager state to JSON file at path. */
    bool saveState(const OrderManager& manager, const std::string& path);
    /** Loads manager state from JSON file at path; resets manager first. */
    bool loadState(OrderManager& manager, const std::string& path);
}
