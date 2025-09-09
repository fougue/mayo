/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "cpp_utils.h"
#include "span.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace Mayo {

// Tree node identifier type
using TreeNodeId = uint32_t;

// Provides tree-like organization of data
//
// A tree node satisfies the following properties:
//     * accessible through an identifier which is unique in the tree it belongs to
//     * null identifier(0) refers to null(void) tree node
//     * single parent node. A tree node is a root in case its parent identifier is null
//     * has 0..N child tree nodes. A tree node is a leaf in case it has no child
//     * owns data of any type, though data type is the same for all nodes of the same tree
//
// Storage of nodes and associated data is memory efficient : all nodes are stored in a single array
// Use the traverseTree_() family of functions to visit nodes of a Tree object
//
// Data type 'T' must be default-constructible(see https://www.cplusplus.com/reference/type_traits/is_default_constructible/)
//
template<typename T> class Tree {
public:
    Tree();

    // Identifier of the sibling node previous to 'id'. Might returns 0
    TreeNodeId nodeSiblingPrevious(TreeNodeId id) const;

    // Identifier of the sibling node next to 'id'. Might returns 0
    TreeNodeId nodeSiblingNext(TreeNodeId id) const;

    // Identifier of the first child of node 'id'. Might returns 0
    TreeNodeId nodeChildFirst(TreeNodeId id) const;

    // Identifier of the last child of node 'id'. Might returns 0
    TreeNodeId nodeChildLast(TreeNodeId id) const;

    // Identifier of the parent of node 'id'. Might returns 0(in such case 'id' is a root)
    TreeNodeId nodeParent(TreeNodeId id) const;

    // Identifier of the node being root of node 'id'
    TreeNodeId nodeRoot(TreeNodeId id) const;

    // Data associated to node of identifier 'id' or default-constructed object of type 'T' in case of error
    const T& nodeData(TreeNodeId id) const;

    // Is node of identifier 'id' a root? Note: a root as parent nodes(ie nodeParent(id) == 0)
    bool nodeIsRoot(TreeNodeId id) const;

    // Is node of identifier 'id' a leaf? Note: a leaf as no child nodes
    bool nodeIsLeaf(TreeNodeId id) const;

    // Read-only array of all the roots
    Span<const TreeNodeId> roots() const;

    // Removes all nodes, tree will become empty
    void clear();

    // Appends child to node identified by 'parentId'. That new node will contain 'data'
    TreeNodeId appendChild(TreeNodeId parentId, const T& data);
    TreeNodeId appendChild(TreeNodeId parentId, T&& data);

    // Remove root node identified by 'id'
    void removeRoot(TreeNodeId id);

    // Total count of nodes in the tree, this includes also nodes marked as "deleted" but not yet
    // destroyed from the tree
    size_t nodeCount() const;

private:
    struct TreeNode {
        TreeNodeId siblingPrevious;
        TreeNodeId siblingNext;
        TreeNodeId childFirst;
        TreeNodeId childLast;
        TreeNodeId parent;
        T data;
        bool isDeleted;
    };

    template<typename U, typename FN>
    friend void traverseTree_unorder(const Tree<U>& tree, const FN& callback);

    template<typename U, typename FN>
    friend void traverseTree_preOrder(TreeNodeId node, const Tree<U>& tree, const FN& callback);

    template<typename U, typename FN>
    friend void traverseTree_postOrder(TreeNodeId node, const Tree<U>& tree, const FN& callback);

    template<typename U, typename FN>
    friend void visitDirectChildren(TreeNodeId id, const Tree<U>& tree, const FN& callback);

    TreeNodeId lastNodeId() const;
    TreeNode* ptrNode(TreeNodeId id);
    const TreeNode* ptrNode(TreeNodeId id) const;
    TreeNode* appendChild(TreeNodeId parentId);
    bool isNodeDeleted(TreeNodeId id) const;

    std::vector<TreeNode> m_vecNode;
    std::vector<TreeNodeId> m_vecRoot;
};

enum class TreeTraversal {
    Unorder, PreOrder, PostOrder
};

// Fastest tree traversal, but nodes are visited unordered
template<typename T, typename FN>
void traverseTree_unorder(const Tree<T>& tree, const FN& callback);

template<typename T, typename FN>
void traverseTree_preOrder(const Tree<T>& tree, const FN& callback);

template<typename T, typename FN>
void traverseTree_preOrder(TreeNodeId id, const Tree<T>& tree, const FN& callback);

template<typename T, typename FN>
void traverseTree_postOrder(const Tree<T>& tree, const FN& callback);

template<typename T, typename FN>
void traverseTree_postOrder(TreeNodeId id, const Tree<T>& tree, const FN& callback);

template<typename T, typename FN>
void traverseTree(const Tree<T>& tree, const FN& callback, TreeTraversal mode = TreeTraversal::PreOrder);

template<typename T, typename FN>
void traverseTree(TreeNodeId id, const Tree<T>& tree, const FN& callback, TreeTraversal mode = TreeTraversal::PreOrder);

template<typename U, typename FN>
void visitDirectChildren(TreeNodeId id, const Tree<U>& tree, const FN& callback);

// --
// -- Implementation
// --

template<typename T> Tree<T>::Tree() {}

template<typename T> TreeNodeId Tree<T>::nodeSiblingPrevious(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->siblingPrevious : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeSiblingNext(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->siblingNext : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeChildFirst(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->childFirst : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeChildLast(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->childLast : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeParent(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->parent : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeRoot(TreeNodeId id) const
{
    if (id == 0)
        return 0;

    while (!this->nodeIsRoot(id))
        id = this->nodeParent(id);

    return id;
}

template<typename T> const T& Tree<T>::nodeData(TreeNodeId id) const
{
    static const T nullObject = {};
    const TreeNode* node = this->ptrNode(id);
    return node ? node->data : nullObject;
}

template<typename T> bool Tree<T>::nodeIsRoot(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->parent == 0 : false;
}

template<typename T> bool Tree<T>::nodeIsLeaf(TreeNodeId id) const
{
    return this->nodeChildFirst(id) == 0;
}

template<typename T> void Tree<T>::clear()
{
    m_vecNode.clear();
    m_vecRoot.clear();
}

template<typename T>
TreeNodeId Tree<T>::appendChild(TreeNodeId parentId, const T& data)
{
    TreeNode* node = this->appendChild(parentId);
    node->data = data;
    return this->lastNodeId();
}

template<typename T>
TreeNodeId Tree<T>::appendChild(TreeNodeId parentId, T&& data)
{
    TreeNode* node = this->appendChild(parentId);
    node->data = std::forward<T>(data);
    return this->lastNodeId();
}

template<typename T>
typename Tree<T>::TreeNode* Tree<T>::appendChild(TreeNodeId parentId)
{
    m_vecNode.push_back({});
    const TreeNodeId nodeId = this->lastNodeId();
    TreeNode* node = &m_vecNode.back();
    node->parent = parentId;
    node->siblingPrevious = this->nodeChildLast(parentId);
    if (parentId != 0) {
        TreeNode* parentNode = this->ptrNode(parentId);
        if (parentNode->childFirst == 0)
            parentNode->childFirst = nodeId;

        if (parentNode->childLast != 0)
            this->ptrNode(parentNode->childLast)->siblingNext = nodeId;

        parentNode->childLast = nodeId;
    }
    else {
        m_vecRoot.push_back(nodeId);
    }

    return node;
}

template<typename T> bool Tree<T>::isNodeDeleted(TreeNodeId id) const
{
    const auto node = this->ptrNode(id);
    return !node || node->isDeleted;
}

template<typename T> void Tree<T>::removeRoot(TreeNodeId id)
{
    Expects(this->nodeIsRoot(id));

    traverseTree_postOrder(id, *this, [=](TreeNodeId visitId) {
        TreeNode* visitNode = this->ptrNode(visitId);
        if (visitNode)
            visitNode->isDeleted = true;
    });
    Expects(this->isNodeDeleted(id));

    auto it = std::find(m_vecRoot.begin(), m_vecRoot.end(), id);
    if (it != m_vecRoot.end())
        m_vecRoot.erase(it);

    if (m_vecRoot.empty())
        this->clear();
}

template<typename T> size_t Tree<T>::nodeCount() const
{
    return m_vecNode.size();
}

template<typename T> Span<const TreeNodeId> Tree<T>::roots() const
{
    return m_vecRoot;
}

template<typename T> TreeNodeId Tree<T>::lastNodeId() const
{
    return static_cast<TreeNodeId>(m_vecNode.size());
}

template<typename T>
typename Tree<T>::TreeNode* Tree<T>::ptrNode(TreeNodeId id)
{
    return id != 0 && id <= m_vecNode.size() ? &m_vecNode.at(id - 1) : nullptr;
}

template<typename T>
const typename Tree<T>::TreeNode* Tree<T>::ptrNode(TreeNodeId id) const
{
    return id != 0 && id <= m_vecNode.size() ? &m_vecNode.at(id - 1) : nullptr;
}

template<typename T, typename FN>
void traverseTree(const Tree<T>& tree, const FN& callback, TreeTraversal mode)
{
    switch (mode) {
    case TreeTraversal::Unorder:
        return traverseTree_unorder(tree, callback);
    case TreeTraversal::PreOrder:
        return traverseTree_preOrder(tree, callback);
    case TreeTraversal::PostOrder:
        return traverseTree_postOrder(tree, callback);
    }
}

template<typename T, typename FN>
void traverseTree(TreeNodeId id, const Tree<T>& tree, const FN& callback, TreeTraversal mode)
{
    switch (mode) {
    case TreeTraversal::Unorder:
    case TreeTraversal::PreOrder:
        return traverseTree_preOrder(id, tree, callback);
    case TreeTraversal::PostOrder:
        return traverseTree_postOrder(id, tree, callback);
    }
}

template<typename T, typename FN>
void traverseTree_unorder(const Tree<T>& tree, const FN& callback)
{
    for (const typename Tree<T>::TreeNode& node : tree.m_vecNode) {
        const auto id = CppUtils::safeStaticCast<TreeNodeId>((&node - &tree.m_vecNode.front()) + 1);
        if (!tree.isNodeDeleted(id))
            callback(id);
    }
}

template<typename T, typename FN>
void traverseTree_preOrder(const Tree<T>& tree, const FN& callback)
{
    for (TreeNodeId id : tree.roots())
        traverseTree_preOrder(id, tree, callback);
}

template<typename T, typename FN>
void traverseTree_preOrder(TreeNodeId id, const Tree<T>& tree, const FN& callback)
{
    if (!tree.isNodeDeleted(id)) {
        callback(id);
        for (auto it = tree.nodeChildFirst(id); it != 0; it = tree.nodeSiblingNext(it))
            traverseTree_preOrder(it, tree, callback);
    }
}

template<typename T, typename FN>
void traverseTree_postOrder(const Tree<T>& tree, const FN& callback)
{
    for (TreeNodeId id : tree.roots())
        traverseTree_postOrder(id, tree, callback);
}

template<typename T, typename FN>
void traverseTree_postOrder(TreeNodeId id, const Tree<T>& tree, const FN& callback)
{
    if (!tree.isNodeDeleted(id)) {
        for (auto it = tree.nodeChildFirst(id); it != 0; it = tree.nodeSiblingNext(it))
            traverseTree_postOrder(it, tree, callback);

        callback(id);
    }
}

template<typename U, typename FN>
void visitDirectChildren(TreeNodeId id, const Tree<U>& tree, const FN& callback)
{
    if (tree.isNodeDeleted(id))
        return;

    for (auto idChild = tree.nodeChildFirst(id); idChild != 0; idChild = tree.nodeSiblingNext(idChild)) {
        if (!tree.isNodeDeleted(idChild))
            callback(idChild);
    }
}

} // namespace Mayo
