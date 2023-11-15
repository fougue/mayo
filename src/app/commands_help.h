/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "commands_api.h"
#include "../base/span.h"

namespace Mayo {

class CommandReportBug : public Command {
public:
    CommandReportBug(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "report-bug";
};

class CommandSystemInformation : public Command {
public:
    CommandSystemInformation(IAppContext* context);
    void execute() override;

    static QString data();

    struct LibraryInfo {
        std::string name;
        std::string version;
        std::string versionDetails;
    };
    static void addLibraryInfo(
        std::string_view libName,
        std::string_view version,
        std::string_view versionDetails = ""
    );
    static Span<const LibraryInfo> libraryInfos();

    static constexpr std::string_view Name = "system-info";
};

class CommandAbout : public Command {
public:
    CommandAbout(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "about";
};

} // namespace Mayo
