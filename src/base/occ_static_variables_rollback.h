/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <variant>
#include <vector>

namespace Mayo {
namespace IO {

class OccStaticVariablesRollback {
public:
    ~OccStaticVariablesRollback();

    void change(const char* strKey, int newValue);

private:
    struct Private;

    struct StaticVariableRecord {
        using Value = std::variant<int, double, const char*>;

        const char* strKey = nullptr;
        Value value;

        bool isValid() const;
    };

    std::vector<StaticVariableRecord> m_vecRecord;
};

} // namespace IO
} // namespace Mayo
