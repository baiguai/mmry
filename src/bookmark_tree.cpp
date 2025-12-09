#include "../include/bookmark_tree.h"
#include <algorithm> // For std::remove, std::find
#include <stdexcept> // For std::runtime_error
#include <functional> // For std::function

// Helper function for recursive flattening
void BookmarkTree::flattenNode(std::shared_ptr<BookmarkGroupNode> node, std::vector<std::shared_ptr<BookmarkGroupNode>>& list) {
    if (!node) return;

    list.push_back(node);
    node->absoluteIndex = list.size() - 1; // Assign absolute index

    if (node->expanded) {
        for (const auto& child : node->children) {
            flattenNode(child, list);
        }
    }
}

// Builds the flattenedTree for UI display
void BookmarkTree::updateFlattenedTree() {
    // Save current selected node's fullPath
    std::string selectedNodeFullPath;
    if (selectedNode) {
        selectedNodeFullPath = selectedNode->fullPath;
        selectedNode->isSelected = false; // Deselect old node
    }

    flattenedTree.clear();
    for (const auto& root : rootNodes) {
        flattenNode(root, flattenedTree);
    }

    // Re-select node if it still exists in the flattened tree
    selectedNode = nullptr;
    selectedAbsoluteIndex = 0;
    if (!selectedNodeFullPath.empty()) {
        for (const auto& node : flattenedTree) {
            if (node->fullPath == selectedNodeFullPath) {
                selectedNode = node;
                selectedNode->isSelected = true;
                selectedAbsoluteIndex = node->absoluteIndex;
                break;
            }
        }
    }

    // If no node was re-selected, or if flattened tree is now empty, select the first if available
    if (!selectedNode && !flattenedTree.empty()) {
        selectedNode = flattenedTree[0];
        selectedNode->isSelected = true;
        selectedAbsoluteIndex = 0;
    } else if (flattenedTree.empty()) {
        selectedNode = nullptr;
        selectedAbsoluteIndex = 0;
    }
}

// Helper to find a node by its fullPath (recursive)
std::shared_ptr<BookmarkGroupNode> BookmarkTree::findNodeByFullPath(const std::string& fullPath) {
    // Traverse root nodes
    std::function<std::shared_ptr<BookmarkGroupNode>(const std::string&, std::shared_ptr<BookmarkGroupNode>)>
    findInSubtree = [&](const std::string& path, std::shared_ptr<BookmarkGroupNode> current) -> std::shared_ptr<BookmarkGroupNode> {
        if (!current) return nullptr;
        if (current->fullPath == path) return current;
        for (const auto& child : current->children) {
            if (auto found = findInSubtree(path, child)) {
                return found;
            }
        }
        return nullptr;
    };

    for (const auto& root : rootNodes) {
        if (auto found = findInSubtree(fullPath, root)) {
            return found;
        }
    }
    return nullptr;
}


void BookmarkTree::addNode(const std::string& nodeName, std::shared_ptr<BookmarkGroupNode> parentNode) {
    // Check for duplicate name at the same level
    auto& targetChildren = (parentNode ? parentNode->children : rootNodes);
    for (const auto& existingNode : targetChildren) {
        if (existingNode->name == nodeName) {
            // Or throw an exception
            return;
        }
    }

    auto newNode = std::make_shared<BookmarkGroupNode>(nodeName, parentNode.get());
    targetChildren.push_back(newNode);
    std::sort(targetChildren.begin(), targetChildren.end(), [](const auto& a, const auto& b) {
        return a->name < b->name;
    });
    updateFlattenedTree();
}

void BookmarkTree::removeNode(std::shared_ptr<BookmarkGroupNode> nodeToRemove) {
    if (!nodeToRemove) return;

    // First, remove from parent's children or rootNodes
    if (nodeToRemove->parent) {
        auto& siblings = nodeToRemove->parent->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), nodeToRemove), siblings.end());
    } else {
        rootNodes.erase(std::remove(rootNodes.begin(), rootNodes.end(), nodeToRemove), rootNodes.end());
    }

    // Unselect if the removed node was selected
    if (selectedNode == nodeToRemove) {
        selectedNode = nullptr;
        selectedAbsoluteIndex = 0;
        if (!flattenedTree.empty()) {
            selectedNode = flattenedTree[0];
            selectedNode->isSelected = true;
        }
    }
    updateFlattenedTree();
}

void BookmarkTree::moveNode(std::shared_ptr<BookmarkGroupNode> nodeToMove, std::shared_ptr<BookmarkGroupNode> newParent, size_t position) {
    if (!nodeToMove) return;

    // Check if newParent is a descendant of nodeToMove (to prevent moving a node into its own subtree)
    std::function<bool(std::shared_ptr<BookmarkGroupNode>, std::shared_ptr<BookmarkGroupNode>)> isDescendant =
        [&](std::shared_ptr<BookmarkGroupNode> possibleParent, std::shared_ptr<BookmarkGroupNode> child) {
        if (!possibleParent || !child) return false;
        if (possibleParent == child) return true; // A node is its own descendant in a broad sense for this check
        for (const auto& c : possibleParent->children) {
            if (isDescendant(c, child)) return true;
        }
        return false;
    };

    if (newParent && isDescendant(nodeToMove, newParent)) {
        // Log error or throw exception
        throw std::runtime_error("Cannot move a node into its own descendant.");
    }

    // Remove from current parent/rootNodes
    if (nodeToMove->parent) {
        auto& currentSiblings = nodeToMove->parent->children;
        currentSiblings.erase(std::remove(currentSiblings.begin(), currentSiblings.end(), nodeToMove), currentSiblings.end());
    } else {
        rootNodes.erase(std::remove(rootNodes.begin(), rootNodes.end(), nodeToMove), rootNodes.end());
    }

    // Add to new parent's children or rootNodes
    if (newParent) {
        nodeToMove->parent = newParent.get();
        nodeToMove->level = newParent->level + 1;
        nodeToMove->fullPath = newParent->fullPath + "/" + nodeToMove->name;
        
        // Adjust position if necessary
        if (position > newParent->children.size()) position = newParent->children.size();
        newParent->children.insert(newParent->children.begin() + position, nodeToMove);
    } else {
        nodeToMove->parent = nullptr;
        nodeToMove->level = 0;
        nodeToMove->fullPath = nodeToMove->name;
        
        // Adjust position if necessary
        if (position > rootNodes.size()) position = rootNodes.size();
        rootNodes.insert(rootNodes.begin() + position, nodeToMove);
    }
    
    // Recursively update fullPath and level for all descendants
    std::function<void(std::shared_ptr<BookmarkGroupNode>)> updateDescendantPaths =
        [&](std::shared_ptr<BookmarkGroupNode> current) {
        if (current->parent) {
            current->level = current->parent->level + 1;
            current->fullPath = current->parent->fullPath + "/" + current->name;
        } else {
            current->level = 0;
            current->fullPath = current->name;
        }
        for (const auto& child : current->children) {
            child->parent = current.get();
            updateDescendantPaths(child);
        }
    };
    updateDescendantPaths(nodeToMove);

    updateFlattenedTree();
}

void BookmarkTree::moveNodeSibling(std::shared_ptr<BookmarkGroupNode> nodeToMove, int direction) {
    if (!nodeToMove) return;

    std::vector<std::shared_ptr<BookmarkGroupNode>>* siblings = nullptr;
    if (nodeToMove->parent) {
        siblings = &nodeToMove->parent->children;
    } else {
        siblings = &rootNodes;
    }

    auto it = std::find(siblings->begin(), siblings->end(), nodeToMove);
    if (it == siblings->end()) return; // Should not happen

    size_t currentIndex = std::distance(siblings->begin(), it);
    size_t newIndex = currentIndex + direction;

    if (newIndex < siblings->size() && newIndex >= 0) { // Check bounds
        std::iter_swap(it, siblings->begin() + newIndex);
        updateFlattenedTree();
    }
}
