/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/unit_system.h"

#include <QtGui/QPixmap>

#include <functional>
#include <unordered_map>

class QWidget;

namespace Mayo {

class BasePropertyQuantity;
class Property;

// Provides widgets for editing Property objects within views and delegates
class IPropertyEditorFactory {
public:
    virtual ~IPropertyEditorFactory() = default;

    virtual QWidget* createEditor(Property* property, QWidget* parentWidget) const = 0;
    virtual void syncEditorWithProperty(QWidget* editor) const = 0;

    // Helpers
    static UnitSystem::TranslateResult unitTranslate(const BasePropertyQuantity* property);
    static QPixmap colorSquarePixmap(const QColor& c, int sideLen = 16);
};

class DefaultPropertyEditorFactory : public IPropertyEditorFactory {
public:
    DefaultPropertyEditorFactory();

    QWidget* createEditor(Property* property, QWidget* parentWidget) const override;
    void syncEditorWithProperty(QWidget* editor) const override;

private:
    template<typename PropType, typename EditorType>
    void registerPropertyEditor();

    using Creator = std::function<QWidget*(Property*, QWidget*)>;
    std::unordered_map<const char*, Creator> m_mapCreator;
};

// --
// -- Implementation
// --

template<typename PropType, typename EditorType>
void DefaultPropertyEditorFactory::registerPropertyEditor()
{
    Creator fnCreateEditor = [](Property* prop, QWidget* parent) -> QWidget* {
        return new EditorType(static_cast<PropType*>(prop), parent);
    };
    m_mapCreator[PropType::TypeName] = std::move(fnCreateEditor);
}

} // namespace Mayo
