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

#pragma once

#include <XCAFDoc_ShapeTool.hxx>
#include <queue>

namespace Mayo {

class XdeShapeExplorer
{
public:
    XdeShapeExplorer(const Handle_XCAFDoc_ShapeTool& shapeTool);
    XdeShapeExplorer(const Handle_XCAFDoc_ShapeTool& shapeTool,
                     const TDF_Label& startLabel);

    void begin(const TDF_Label& label);
    void goNext();
    bool atEnd() const;

    const TDF_Label& current() const;
    unsigned currentIterationId() const;
    const TDF_Label& currentParent() const;
    unsigned currentParentIterationId() const;

private:
    void enqueueChildren(const TDF_Label& label, unsigned iterationId);

    struct ChildrenQueue {
        TDF_Label parentLabel;
        unsigned parentIterationId = 0;
        std::queue<TDF_Label> queueChildLabel;
        void push(const TDF_Label& lbl);
        void pop();
        const TDF_Label& front() const;
        bool isEmpty() const;
    };

    std::queue<ChildrenQueue> m_queueMaster;
    Handle_XCAFDoc_ShapeTool m_shapeTool;
    TDF_Label m_current;
    unsigned m_currentIterationId = 0;
    bool m_atEnd = true;
};

} // namespace Mayo
