/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <vector>

namespace Mayo {

using TreeNodeId = uint32_t;

template<typename T> class Tree {
public:
    Tree();

    TreeNodeId nodeSiblingPrevious(TreeNodeId id) const;
    TreeNodeId nodeSiblingNext(TreeNodeId id) const;
    TreeNodeId nodeChildFirst(TreeNodeId id) const;
    TreeNodeId nodeChildLast(TreeNodeId id) const;
    TreeNodeId nodeParent(TreeNodeId id) const;
    const T& nodeData(TreeNodeId id) const;
    bool nodeIsRoot(TreeNodeId id) const;
    Span<const TreeNodeId> roots() const;

    void clear();
    TreeNodeId appendChild(TreeNodeId parentId, const T& data);
    TreeNodeId appendChild(TreeNodeId parentId, T&& data);
    void removeRoot(TreeNodeId id);

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

    template<typename T, typename FN>
    friend void deepForeachTreeNode(TreeNodeId node, const Tree<T>& tree, const FN& callback);

    TreeNodeId lastNodeId() const;
    TreeNode* ptrNode(TreeNodeId id);
    const TreeNode* ptrNode(TreeNodeId id) const;
    TreeNode* appendChild(TreeNodeId parentId);

    std::vector<TreeNode> m_vecNode;
    std::vector<TreeNodeId> m_vecRoot;
};

template<typename T, typename FN>
void deepForeachTreeNode(const Tree<T>& tree, const FN& callback);

template<typename T, typename FN>
void deepForeachTreeNode(TreeNodeId node, const Tree<T>& tree, const FN& callback);



// --
// -- Implementation
// --

template<typename T> Tree<T>::Tree() {}

template<typename T> TreeNodeId Tree<T>::nodeSiblingPrevious(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->siblingPrevious : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeSiblingNext(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->siblingNext : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeChildFirst(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->childFirst : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeChildLast(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->childLast : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeParent(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->parent : 0;
}

template<typename T> const T& Tree<T>::nodeData(TreeNodeId id) const {
    static const T nullObject = {};
    const TreeNode* node = this->ptrNode(id);
    return node ? node->data : nullObject;
}

template<typename T> bool Tree<T>::nodeIsRoot(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->parent == 0 : false;
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

template<typename T> void Tree<T>::removeRoot(TreeNodeId id)
{
    Expects(this->nodeIsRoot(id));

    auto it = std::find(m_vecRoot.begin(), m_vecRoot.end(), id);
    if (it != m_vecRoot.end()) {
        TreeNode* node = this->ptrNode(id);
        Expects(node != nullptr);
        node->isDeleted = true;
        m_vecRoot.erase(it);
    }
}

template<typename T>
Span<const TreeNodeId> Tree<T>::roots() const {
    return m_vecRoot;
}

template<typename T>
TreeNodeId Tree<T>::lastNodeId() const {
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
void deepForeachTreeNode(TreeNodeId node, const Tree<T>& tree, const FN& callback)
{
    const Tree<T>::TreeNode* ptrNode = tree.ptrNode(node);
    if (ptrNode && !ptrNode->isDeleted)
        callback(node);

    const TreeNodeId childFirst = tree.nodeChildFirst(node);
    for (auto it = childFirst; it != 0; it = tree.nodeSiblingNext(it))
        deepForeachTreeNode(it, tree, callback);
}

template<typename T, typename FN>
void deepForeachTreeNode(const Tree<T>& tree, const FN& callback)
{
    for (TreeNodeId node : tree.roots())
        deepForeachTreeNode(node, tree, callback);
}

} // namespace Mayo
