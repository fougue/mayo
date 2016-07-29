#pragma once

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <QtWidgets/QWidget>

namespace Mayo {

class Document;
class Part;
class QtOccView;

class UiDocument : public QWidget
{
    Q_OBJECT

public:
    UiDocument(Document* doc, QWidget* parent = nullptr);

    const Document* document() const;
    Document* document();

private:
    void onPartImported(uint64_t partId, const Part& part);

    Document* m_document = nullptr;
    Handle_V3d_Viewer m_v3dViewer;
    Handle_AIS_InteractiveContext m_aisContext;
    QtOccView* m_view = nullptr;
};

} // namespace Mayo
