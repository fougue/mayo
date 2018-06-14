/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "xde_shape_explorer.h"

namespace Mayo {

XdeShapeExplorer::XdeShapeExplorer(const Handle_XCAFDoc_ShapeTool &shapeTool)
    : m_shapeTool(shapeTool)
{
}

XdeShapeExplorer::XdeShapeExplorer(
        const Handle_XCAFDoc_ShapeTool &shapeTool, const TDF_Label &startLabel)
    : m_shapeTool(shapeTool)
{
    this->begin(startLabel);
}

void XdeShapeExplorer::begin(const TDF_Label &label)
{
    while (!m_queueMaster.empty())
        m_queueMaster.pop();
    m_currentIterationId = 0;
    ChildrenQueue firstQueue;
    firstQueue.push(label);
    m_queueMaster.push(firstQueue);
    this->goNext();
}

void XdeShapeExplorer::goNext()
{
    ChildrenQueue* frontQueue = nullptr;
    while (frontQueue == nullptr && !m_queueMaster.empty()) {
        frontQueue = &m_queueMaster.front();
        if (frontQueue->isEmpty()) {
            m_queueMaster.pop();
            frontQueue = nullptr;
        }
    }
    if (!m_queueMaster.empty()) {
        m_atEnd = false;
        m_current = frontQueue->front();
        frontQueue->pop();
        ++m_currentIterationId;
        this->enqueueChildren(m_current, m_currentIterationId);
    }
    else {
        m_atEnd = true;
    }
}

bool XdeShapeExplorer::atEnd() const
{
    return m_atEnd;
}

const TDF_Label &XdeShapeExplorer::current() const
{
    return m_current;
}

unsigned XdeShapeExplorer::currentIterationId() const
{
    return m_currentIterationId;
}

const TDF_Label &XdeShapeExplorer::currentParent() const
{
    static const TDF_Label nullLabel;
    return !m_queueMaster.empty() ?
                m_queueMaster.front().parentLabel :
                nullLabel;
}

unsigned XdeShapeExplorer::currentParentIterationId() const
{
    return !m_queueMaster.empty() ?
                m_queueMaster.front().parentIterationId :
                0;
}

void XdeShapeExplorer::enqueueChildren(
        const TDF_Label &label, unsigned iterationId)
{
    ChildrenQueue queueChild;
    queueChild.parentLabel = label;
    queueChild.parentIterationId = iterationId;
    if (m_shapeTool->IsAssembly(label)) {
        TDF_LabelSequence seqComponentLabel;
        m_shapeTool->GetComponents(label, seqComponentLabel);
        for (const TDF_Label& compLabel : seqComponentLabel)
            queueChild.push(compLabel);
    }
    else if (m_shapeTool->IsReference(label)) {
        TDF_Label labelRef;
        m_shapeTool->GetReferredShape(label, labelRef);
        queueChild.push(labelRef);
    }
    else if (m_shapeTool->IsSimpleShape(label)) {
        TDF_LabelSequence seqSubShapeLabel;
        if (m_shapeTool->GetSubShapes(label, seqSubShapeLabel)) {
            for (const TDF_Label& subShapeLabel : seqSubShapeLabel)
                queueChild.push(subShapeLabel);
        }
    }
    if (!queueChild.isEmpty())
        m_queueMaster.push(queueChild);
}

void XdeShapeExplorer::ChildrenQueue::push(const TDF_Label &lbl)
{
    this->queueChildLabel.push(lbl);
}

void XdeShapeExplorer::ChildrenQueue::pop()
{
    this->queueChildLabel.pop();
}

const TDF_Label &XdeShapeExplorer::ChildrenQueue::front() const
{
    return this->queueChildLabel.front();
}

bool XdeShapeExplorer::ChildrenQueue::isEmpty() const
{
    return this->queueChildLabel.empty();
}

} // namespace Mayo
