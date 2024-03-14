/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/settings.h"

#include <QtCore/QSettings>

namespace Mayo {

// Provides QSettings backend for application settings storage
class QSettingsStorage : public Settings::Storage {
public:
    QSettingsStorage() = default;
    QSettingsStorage(const QString& fileName, QSettings::Format format);

    bool contains(std::string_view key) const override;
    Settings::Variant value(std::string_view key) const override;
    void setValue(std::string_view key, const Settings::Variant& value) override;
    void sync() override;

    const QSettings& get() const { return m_storage; }

private:
    QSettings m_storage;
};

} // namespace Mayo
