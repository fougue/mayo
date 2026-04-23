/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/unit_system.h"

#include <QtGui/QPixmap>
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
    QWidget* createEditor(Property* property, QWidget* parentWidget) const override;
    void syncEditorWithProperty(QWidget* editor) const override;
};

} // namespace Mayo
