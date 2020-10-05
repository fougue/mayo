/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/unit.h"
#include <QtWidgets/QStyledItemDelegate>
#include <unordered_map>

namespace Mayo {

class BasePropertyQuantity;

class PropertyItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    PropertyItemDelegate(QObject* parent = nullptr);

    double rowHeightFactor() const { return m_rowHeightFactor; }
    void setRowHeightFactor(double v) { m_rowHeightFactor = v; }

    struct UnitTranslation {
      Unit unit;
      const char* strUnit; // UTF8
      double factor;
    };
    bool overridePropertyUnitTranslation(
            const BasePropertyQuantity* prop, UnitTranslation unitTr);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QString displayText(const QVariant& value, const QLocale&) const override;

    QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem&,
            const QModelIndex& index) const override;

    void setModelData(
            QWidget*, QAbstractItemModel*, const QModelIndex&) const override;

    QSize sizeHint(
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

private:
    double m_rowHeightFactor = 1.;
    std::unordered_map<const BasePropertyQuantity*, UnitTranslation> m_mapPropUnitTr;
};

} // namespace Mayo
