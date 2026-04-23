/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/global.h"
#include "../base/unit_system.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QString>
class Quantity_Color;
class gp_Pnt;
class gp_Dir;
class gp_Trsf;

namespace Mayo {

class QStringUtils {
    Q_DECLARE_TR_FUNCTIONS(Mayo::QStringUtils)
public:
    struct TextOptions {
        QLocale locale;
        UnitSystem::Schema unitSchema;
        int unitDecimals;
    };

    // TODO add overload for 'int' type
    static QString text(double value, const TextOptions& opt);
    static QString text(const gp_Pnt& pos, const TextOptions& opt);
    static QString text(const gp_Dir& pos, const TextOptions& opt);
    static QString text(const gp_Trsf& trsf, const TextOptions& opt);
    static QString text(const Quantity_Color& color, const QString& format = "RGB(%1, %2, %3)");

    static QString bytesText(uint64_t sizeBytes, const QLocale& locale = {});

    static QString yesNoText(bool on);
    static QString yesNoText(CheckState state);

    static void append(QString* dst, const QString& str, const QLocale& locale = {});
};

} // namespace Mayo
