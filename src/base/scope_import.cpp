/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "scope_import.h"

#include "document.h"
#include "xcaf.h"

namespace Mayo {

BaseScopeImport::BaseScopeImport(const DocumentPtr &doc)
    : m_doc(doc.get())
{}

BaseScopeImport::BaseScopeImport(BaseScopeImport&& other)
    : m_doc(other.m_doc),
      m_invoke(other.m_invoke)
{
    other.dismiss();
}

void BaseScopeImport::setConfirmation(bool on) {
    m_invoke = on;
}

void BaseScopeImport::dismiss() {
    m_invoke = false;
}

XCafScopeImport::XCafScopeImport(const DocumentPtr& doc)
    : BaseScopeImport(doc),
      m_seqLabelEntityOnEntry(doc->xcaf().topLevelFreeShapes())
{}

XCafScopeImport::XCafScopeImport(XCafScopeImport&& other)
    : BaseScopeImport(std::move(other)),
      m_seqLabelEntityOnEntry(std::move(other.m_seqLabelEntityOnEntry))
{ }

XCafScopeImport::~XCafScopeImport()
{
    if (this->isConfirmed()) {
        this->document()->notifyNewXCafEntities(m_seqLabelEntityOnEntry);
    }
    else {
        // TODO Remove new entities
    }
}

SingleScopeImport::SingleScopeImport(const DocumentPtr& doc)
    : BaseScopeImport(doc),
      m_newEntityLabel(doc->newEntityLabel())
{}

SingleScopeImport::SingleScopeImport(XCafScopeImport&& other)
    : BaseScopeImport(std::move(other)),
      m_newEntityLabel(std::move(m_newEntityLabel))
{}

SingleScopeImport::~SingleScopeImport()
{
    if (this->isConfirmed()) {
        this->document()->notifyNewEntity(m_newEntityLabel);
    }
    else {
        // TODO Remove m_newEntityLabel
    }
}

} // namespace Mayo
