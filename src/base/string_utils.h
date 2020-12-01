/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "unit_system.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <IFSelect_ReturnStatus.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <utility>
class Quantity_Color;
class gp_Pnt;
class gp_Dir;
class gp_Trsf;

namespace Mayo {

class StringUtils {
    Q_DECLARE_TR_FUNCTIONS(Mayo::StringUtils)
public:
    struct TextOptions {
        QLocale locale;
        UnitSystem::Schema unitSchema;
        int unitDecimals;
    };

    static QString text(double value, const TextOptions& opt);
    static QString text(const gp_Pnt& pos, const TextOptions& opt);
    static QString text(const gp_Dir& pos, const TextOptions& opt);
    static QString text(const gp_Trsf& trsf, const TextOptions& opt);
    static QString text(const Quantity_Color& color, const QString& format = "R:%1 G:%2 B:%3");

    static QString bytesText(uint64_t sizeBytes, const QLocale& locale = QLocale());

    static void append(QString* dst, const QString& str, const QLocale& locale = QLocale());
};

} // namespace Mayo
