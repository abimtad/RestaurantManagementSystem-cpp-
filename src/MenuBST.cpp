#include "MenuBST.h"

MenuBST::~MenuBST() {
    destroy(root_);
}

void MenuBST::destroy(MenuNode* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

bool MenuBST::insert(const MenuItem& item) {
    bool inserted = false;
    root_ = insertNode(root_, item, inserted);
    return inserted;
}

MenuNode* MenuBST::insertNode(MenuNode* node, const MenuItem& item, bool& inserted) {
    if (!node) {
        inserted = true;
        MenuNode* created = new MenuNode();
        created->data = item;
        return created;
    }
    if (item.name < node->data.name) {
        node->left = insertNode(node->left, item, inserted);
    } else if (item.name > node->data.name) {
        node->right = insertNode(node->right, item, inserted);
    } else {
        inserted = false; // duplicate
    }
    return node;
}

MenuItem* MenuBST::find(const std::string& name) {
    MenuNode* cur = root_;
    while (cur) {
        if (name < cur->data.name) {
            cur = cur->left;
        } else if (name > cur->data.name) {
            cur = cur->right;
        } else {
            return &cur->data;
        }
    }
    return nullptr;
}

MenuNode* MenuBST::findMin(MenuNode* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

MenuNode* MenuBST::removeNode(MenuNode* node, const std::string& name, bool& removed) {
    if (!node) return nullptr;
    if (name < node->data.name) {
        node->left = removeNode(node->left, name, removed);
    } else if (name > node->data.name) {
        node->right = removeNode(node->right, name, removed);
    } else {
        removed = true;
        if (!node->left) {
            MenuNode* right = node->right;
            delete node;
            return right;
        }
        if (!node->right) {
            MenuNode* left = node->left;
            delete node;
            return left;
        }
        MenuNode* successor = findMin(node->right);
        node->data = successor->data;
        node->right = removeNode(node->right, successor->data.name, removed);
    }
    return node;
}

bool MenuBST::remove(const std::string& name) {
    bool removed = false;
    root_ = removeNode(root_, name, removed);
    return removed;
}
