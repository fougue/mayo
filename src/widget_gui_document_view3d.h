#pragma once

#include <QtWidgets/QWidget>

namespace Mayo {

class GuiDocument;
class WidgetOccView;

class WidgetGuiDocumentView3d : public QWidget
{
    Q_OBJECT

public:
    WidgetGuiDocumentView3d(GuiDocument* guiDoc, QWidget* parent = nullptr);

    GuiDocument* guiDocument() const;
    WidgetOccView* widgetOccView() const;

private:
    GuiDocument* m_guiDoc = nullptr;
    WidgetOccView* m_qtOccView = nullptr;
};

} // namespace Mayo
