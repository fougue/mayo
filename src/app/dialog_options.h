/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_editor_factory.h"
#include "../base/settings.h"
#include <QtCore/QVariant>
#include <QtWidgets/QDialog>
#include <memory>
#include <unordered_map>

namespace Mayo {

// Provides a dialog to edit Settings(options) and exchange with INI file
class DialogOptions : public QDialog {
    Q_OBJECT
public:
    DialogOptions(Settings* settings, QWidget* parent = nullptr);
    ~DialogOptions();

    IPropertyEditorFactory* editorFactory() const { return m_editorFactory.get(); }
    void setPropertyEditorFactory(std::unique_ptr<IPropertyEditorFactory> editorFactory);

private:
    QWidget* createEditor(Property* property, QWidget* parentWidget) const;
    void syncEditor(QWidget* editor);

    void loadFromFile();
    void saveAs();
    void cancelChanges();

    void handleTreeViewButtonClick_restoreDefaults(const QModelIndex& index);

    class Ui_DialogOptions* m_ui = nullptr;
    std::unique_ptr<IPropertyEditorFactory> m_editorFactory;
    std::unordered_map<const Property*, QWidget*> m_mapSettingEditor;
    std::unordered_map<Property*, Settings::Variant> m_mapSettingInitialValue;
    Settings* m_settings = nullptr;
    SignalConnectionHandle m_connSettingsAboutToChange;
    SignalConnectionHandle m_connSettingsChanged;
    SignalConnectionHandle m_connSettingsEnabled;
};

} // namespace Mayo
