/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <XCAFDoc_ShapeTool.hxx>
#include <queue>

namespace Mayo {

class XdeShapeExplorer {
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
