#pragma once
#include <string>
#include <functional>

/**
 * Basic BST for menu items keyed by name.
 */
struct MenuItem {
    int itemId{0};
    std::string name;
    int defaultPrepMinutes{0};
};

struct MenuNode {
    MenuItem data;
    MenuNode* left{nullptr};
    MenuNode* right{nullptr};
};

class MenuBST {
public:
    MenuBST() = default;
    ~MenuBST();

    /** Inserts item if name is unique. */
    bool insert(const MenuItem& item);
    /** Finds an item by name; returns nullptr if missing. */
    MenuItem* find(const std::string& name);
    /** Removes item by name. */
    bool remove(const std::string& name);

    template <typename Func>
    void inOrder(Func fn) {
        inOrderInternal(root_, fn);
    }

private:
    MenuNode* root_{nullptr};

    void destroy(MenuNode* node);
    MenuNode* insertNode(MenuNode* node, const MenuItem& item, bool& inserted);
    MenuNode* findMin(MenuNode* node);
    MenuNode* removeNode(MenuNode* node, const std::string& name, bool& removed);

    template <typename Func>
    void inOrderInternal(MenuNode* node, Func fn) {
        if (!node) return;
        inOrderInternal(node->left, fn);
        fn(node->data);
        inOrderInternal(node->right, fn);
    }
};
