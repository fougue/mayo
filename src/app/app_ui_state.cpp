#include "app_ui_state.h"

#include "../qtcommon/qtcore_utils.h"
#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

namespace Mayo {

template<> const char PropertyAppUiState::TypeName[] = "Mayo::PropertyAppUiState";

std::vector<uint8_t> AppUiState::toBlob(const AppUiState& state)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    stream << QtCoreUtils::QByteArray_fromRawData(std::string_view{PropertyAppUiState::TypeName});
    constexpr uint32_t version = 2;
    stream << version;
    stream << QtCoreUtils::QByteArray_fromRawData<uint8_t>(state.mainWindowGeometry);
    stream << state.pageDocuments_isLeftSideBarVisible;
    stream << state.pageDocuments_widgetLeftSideBarWidthFactor;
    return QtCoreUtils::toStdByteArray(blob);
}

AppUiState AppUiState::fromBlob(Span<const uint8_t> blob, bool* ok)
{
    auto fnSetOk = [=](bool v) {
        if (ok)
            *ok = v;
    };

    fnSetOk(false);
    AppUiState state;

    QDataStream stream(QtCoreUtils::QByteArray_fromRawData(blob));
    QByteArray identifier;
    stream >> identifier;
    if (identifier == PropertyAppUiState::TypeName) {
        uint32_t version = 0;
        stream >> version;
        if (version >= 1) {
            QByteArray blobMainWindowGeom;
            stream >> blobMainWindowGeom;
            state.mainWindowGeometry = QtCoreUtils::toStdByteArray(blobMainWindowGeom);
            stream >> state.pageDocuments_isLeftSideBarVisible;
            fnSetOk(true);
        }

        if (version >= 2) {
            stream >> state.pageDocuments_widgetLeftSideBarWidthFactor;
            fnSetOk(true);
        }
    }

    return state;
}

} // namespace Mayo
