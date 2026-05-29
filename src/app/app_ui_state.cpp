/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "app_ui_state.h"

#include "../base/cpp_utils.h"
#include "../qtcommon/qtcore_utils.h"

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

#include <type_traits>

namespace Mayo {

namespace {

template<typename T> T read(QDataStream& stream)
{
    static_assert(!std::is_reference_v<T>);
    T value{};
    stream >> value;
    return value;
}

PropertyValueConversion::Variant readVariant(QDataStream& stream, uint8_t typeIndex)
{
    using Variant = PropertyValueConversion::Variant;
    static_assert(std::is_same_v<bool, std::variant_alternative_t<1, Variant>>);
    static_assert(std::is_same_v<int, std::variant_alternative_t<2, Variant>>);
    static_assert(std::is_same_v<double, std::variant_alternative_t<3, Variant>>);
    static_assert(std::is_same_v<std::string, std::variant_alternative_t<4, Variant>>);
    static_assert(std::is_same_v<std::vector<uint8_t>, std::variant_alternative_t<5, Variant>>);

    switch (typeIndex) {
    case 1: return Variant{read<bool>(stream)};
    case 2: return Variant{static_cast<int>(read<int32_t>(stream))};
    case 3: return Variant{read<double>(stream)};
    case 4: return Variant{read<QString>(stream).toStdString()};
    case 5: return Variant{QtCoreUtils::toStdByteArray(read<QByteArray>(stream))};
    default: return {};
    }

    return {};
}

} // namespace

template<> const char PropertyAppUiState::TypeName[] = "Mayo::PropertyAppUiState";

const AppUiState::Variant* AppUiState::find(std::string_view key) const
{
    auto it = m_map.find(key);
    return it != m_map.cend() ? &it->second : nullptr;
}

const AppUiState::Variant& AppUiState::get(std::string_view key) const
{
    const Variant* ptr = this->find(key);
    return ptr ? *ptr : Cpp::staticObject<Variant>();
}

void AppUiState::set(std::string_view key, Variant value)
{
    m_map.insert_or_assign(std::string{key}, std::move(value));
}

std::vector<uint8_t> AppUiState::toBlob(const AppUiState& state)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    stream << QByteArrayLiteral("Mayo::VariantMap");

    constexpr uint32_t version = 3;
    stream << version;

    stream << static_cast<uint32_t>(state.size());
    for (const auto& [key, value] : state) {
        stream << QtCoreUtils::QByteArray_fromRawData(key);
        stream << static_cast<uint8_t>(value.index());
        std::visit(Cpp::Overloaded{
            [](std::monostate) { /* No payload for empty variant value */ },
            [&](bool v) { stream << v; },
            [&](int v) { stream << static_cast<int32_t>(v); },
            [&](double v) { stream << v; },
            [&](const std::string& str) { stream << QtCoreUtils::QByteArray_fromRawData(str); },
            [&](const std::vector<uint8_t>& bytes) {
                stream << QtCoreUtils::QByteArray_fromRawData<uint8_t>(bytes);
            }},
            value
        );
    }

    return QtCoreUtils::toStdByteArray(blob);
}

AppUiState AppUiState::fromBlob(gsl::span<const uint8_t> blob, bool* ok)
{
    auto fnSetOk = [=](bool v) {
        if (ok)
            *ok = v;
    };

    fnSetOk(false);
    AppUiState state;

    QDataStream stream(QtCoreUtils::QByteArray_fromRawData(blob));
    auto identifier = read<QByteArray>(stream);
    if (identifier != "Mayo::VariantMap")
        return state;

    auto version = read<uint32_t>(stream);
    if (version < 3)
        return state;

    auto dataCount = read<uint32_t>(stream);
    for (uint32_t i = 0; i < dataCount; ++i) {
        auto key = read<QByteArray>(stream).toStdString();
        auto typeIndex = read<uint8_t>(stream);
        auto value = readVariant(stream, typeIndex);
        state.set(key, std::move(value));
        fnSetOk(true);
    }

    return state;
}

} // namespace Mayo
