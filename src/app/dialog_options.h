/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_editor_factory.h"
#include <QtWidgets/QDialog>
#include <memory>

namespace Mayo {

class Settings;

class DialogOptions : public QDialog {
    Q_OBJECT
public:
    DialogOptions(Settings* settings, QWidget* parent = nullptr);
    ~DialogOptions();

    PropertyEditorFactory* editorFactory() const { return m_editorFactory.get(); }
    void setPropertyEditorFactory(std::unique_ptr<PropertyEditorFactory> editorFactory);

private:
    QWidget* createEditor(Property* property, QWidget* parentWidget) const;
    void restoreDefaults();

    class Ui_DialogOptions* m_ui = nullptr;
    std::unique_ptr<PropertyEditorFactory> m_editorFactory;
    Settings* m_settings = nullptr;
};

} // namespace Mayo
