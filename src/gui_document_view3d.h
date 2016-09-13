#pragma once

#include <QtWidgets/QWidget>

namespace Mayo {

class GuiDocument;
class QtOccView;

class GuiDocumentView3d : public QWidget
{
    Q_OBJECT

public:
    GuiDocumentView3d(GuiDocument* guiDoc, QWidget* parent = nullptr);

    GuiDocument* guiDocument() const;
    QtOccView* qtOccView() const;

private:
    GuiDocument* m_guiDoc = nullptr;
    QtOccView* m_qtOccView = nullptr;
};

} // namespace Mayo
