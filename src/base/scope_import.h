/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include <TDF_LabelSequence.hxx>

namespace Mayo {

class BaseScopeImport {
public:
    BaseScopeImport(const DocumentPtr& doc);
    BaseScopeImport(BaseScopeImport&& other);

    bool isConfirmed() const { return m_invoke; }
    void setConfirmation(bool on);
    void dismiss();

protected:
    Document* document() { return m_doc; }

private:
    Document* m_doc = nullptr;
    bool m_invoke = true;
};

class XCafScopeImport : public BaseScopeImport {
public:
    XCafScopeImport(const DocumentPtr& doc);
    XCafScopeImport(XCafScopeImport&& other);
    ~XCafScopeImport();

    XCafScopeImport(const XCafScopeImport&) = delete;
    XCafScopeImport& operator=(const XCafScopeImport&) = delete;

private:
    const TDF_LabelSequence m_seqLabelEntityOnEntry;
};

class SingleScopeImport : public BaseScopeImport {
public:
    SingleScopeImport(const DocumentPtr& doc);
    SingleScopeImport(XCafScopeImport&& other);
    ~SingleScopeImport();

    const TDF_Label& entityLabel() const { return m_newEntityLabel; }

    SingleScopeImport(const SingleScopeImport&) = delete;
    SingleScopeImport& operator=(const XCafScopeImport&) = delete;

private:
    TDF_Label m_newEntityLabel;
};

} // namespace Mayo
