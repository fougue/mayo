/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/libtree.h"

namespace Mayo {

void TestBase::LibTree_test()
{
    const TreeNodeId nullptrId = 0;
    Tree<std::string> tree;
    std::vector<TreeNodeId> vecTreeNodeId;
    auto fnAppendChild = [&](TreeNodeId parentId, const char* str) {
        const TreeNodeId id = tree.appendChild(parentId, str);
        vecTreeNodeId.push_back(id);
        return id;
    };
    const TreeNodeId n0 = fnAppendChild(nullptrId, "0");
    const TreeNodeId n0_1 = fnAppendChild(n0, "0-1");
    const TreeNodeId n0_2 = fnAppendChild(n0, "0-2");
    const TreeNodeId n0_1_1 = fnAppendChild(n0_1, "0-1-1");
    const TreeNodeId n0_1_2 = fnAppendChild(n0_1, "0-1-2");
    std::sort(vecTreeNodeId.begin(), vecTreeNodeId.end());

    QCOMPARE(tree.nodeParent(n0_1), n0);
    QCOMPARE(tree.nodeParent(n0_2), n0);
    QCOMPARE(tree.nodeParent(n0_1_1), n0_1);
    QCOMPARE(tree.nodeParent(n0_1_2), n0_1);
    QCOMPARE(tree.nodeChildFirst(n0_1), n0_1_1);
    QCOMPARE(tree.nodeChildLast(n0_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingNext(n0_1_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingPrevious(n0_1_2), n0_1_1);
    QCOMPARE(tree.nodeSiblingNext(n0_1_2), nullptrId);

    {
        std::string strPreOrder;
        traverseTree_preOrder(tree, [&](TreeNodeId id) {
            strPreOrder += " " + tree.nodeData(id);
        });
        QCOMPARE(strPreOrder, " 0 0-1 0-1-1 0-1-2 0-2");
    }

    {
        std::string strPostOrder;
        traverseTree_postOrder(tree, [&](TreeNodeId id) {
            strPostOrder += " " + tree.nodeData(id);
        });
        QCOMPARE(strPostOrder, " 0-1-1 0-1-2 0-1 0-2 0");
    }

    {
        std::vector<TreeNodeId> vecTreeNodeIdVisited;
        traverseTree_unorder(tree, [&](TreeNodeId id) {
            vecTreeNodeIdVisited.push_back(id);
        });
        std::sort(vecTreeNodeIdVisited.begin(), vecTreeNodeIdVisited.end());
        QCOMPARE(vecTreeNodeIdVisited, vecTreeNodeId);
    }
}

void TestBase::LibTree_nodeRoot_test()
{
    Tree<int> tree;
    tree.appendChild(0, -1);
    tree.appendChild(0, -2);
    QCOMPARE(tree.nodeRoot(0), 0);
}

void TestBase::LibTree_removeRoot_test()
{
    Tree<std::string> tree;
    QCOMPARE(tree.nodeCount(), 0);

    const TreeNodeId root1 = tree.appendChild(0, "root1");
    const TreeNodeId root2 = tree.appendChild(0, "root2");
    QCOMPARE(tree.nodeCount(), 2);
    tree.appendChild(root1, "1-child1");
    tree.appendChild(root1, "1-child2");
    tree.appendChild(root2, "2-child1");
    tree.appendChild(root2, "2-child2");
    tree.appendChild(root2, "2-child3");
    QCOMPARE(tree.nodeCount(), 7);

    tree.removeRoot(root2);
    QVERIFY(tree.nodeCount() > 0);
    tree.removeRoot(root1);
    // All root nodes removed: the count of nodes must be 0
    QCOMPARE(tree.nodeCount(), 0);
}

} // namespace Mayo
