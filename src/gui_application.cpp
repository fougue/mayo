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

#include "gui_application.h"

#include "application.h"
#include "document.h"
#include "gui_document.h"

namespace Mayo {

GuiApplication::GuiApplication(QObject *parent)
    : QObject(parent)
{
    auto app = Application::instance();
    QObject::connect(
                app, &Application::documentAdded,
                this, &GuiApplication::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &GuiApplication::onDocumentErased);
}

GuiDocument *GuiApplication::findGuiDocument(const Document *doc) const
{
    auto itFound = std::find_if(
                m_vecDocGuiDoc.cbegin(),
                m_vecDocGuiDoc.cend(),
                [=](const Doc_GuiDoc& pair) { return pair.doc == doc; });
    return itFound != m_vecDocGuiDoc.cend() ? itFound->guiDoc : nullptr;
}

void GuiApplication::onDocumentAdded(Document *doc)
{
    const Doc_GuiDoc pair = { doc, new GuiDocument(doc) }; // TODO: set container widget
    m_vecDocGuiDoc.emplace_back(std::move(pair));
    emit guiDocumentAdded(pair.guiDoc);
}

void GuiApplication::onDocumentErased(const Document *doc)
{
    auto itFound = std::find_if(
                m_vecDocGuiDoc.begin(),
                m_vecDocGuiDoc.end(),
                [=](const Doc_GuiDoc& pair) { return pair.doc == doc; });
    if (itFound != m_vecDocGuiDoc.end()) {
        const GuiDocument* guiDoc = itFound->guiDoc;
        delete guiDoc;
        m_vecDocGuiDoc.erase(itFound);
        emit guiDocumentErased(guiDoc);
    }
}

} // namespace Mayo
