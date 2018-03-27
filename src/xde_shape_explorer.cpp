/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
