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

#include <QtCore/QObject>
#include <AIS_InteractiveContext.hxx>
#include <Bnd_Box.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <vector>

namespace Mayo {

class Document;
class DocumentItem;
class GpxDocumentItem;

class GuiDocument : public QObject {
    Q_OBJECT

public:
    GuiDocument(Document* doc);

    Document* document() const;
    const Handle_V3d_View& v3dView() const;
    GpxDocumentItem* findItemGpx(const DocumentItem* item) const;

    const Bnd_Box& gpxBoundingBox() const;

signals:
    void gpxBoundingBoxChanged(const Bnd_Box& bndBox);

private:
    void onItemAdded(DocumentItem* item);
    void onItemErased(const DocumentItem* item);

    struct DocumentItem_Gpx {
        DocumentItem* item;
        GpxDocumentItem* gpx;
    };

    Document* m_document = nullptr;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_V3d_View m_v3dView;
    Handle_AIS_InteractiveContext m_aisContext;
    std::vector<DocumentItem_Gpx> m_vecDocItemGpx;
    Bnd_Box m_gpxBoundingBox;
};

} // namespace Mayo
