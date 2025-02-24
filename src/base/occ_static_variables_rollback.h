/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Mayo::IO {

// Resets an OpenCascade static variable(see Interface_Static) to its previous value on destruction
// It can be used to revert state when an exception is thrown without needing to write
// try-catch blocks.
// It can also be used to manage variables that are temporarily set, such as reentrancy guards.
// By using this class, the variable will be reset whether the function is exited normally, exited
// early by a return statement, or exited by an exception
//
// Typical usage:
//     {
//         OccStaticVariablesRollback varsRollback;
//         varsRollback.change("write.step.schema", "AP203");
//         varsRollback.change("write.surfacecurve.mode", 0);
//         // Write STEP file(s) ...
//     }
//     // Changed OpenCascade variables automatically rolled back to their previous values ...
class OccStaticVariablesRollback {
public:
    ~OccStaticVariablesRollback();

    void change(const char* strKey, int newValue);
    void change(const char* strKey, double newValue);
    void change(const char* strKey, std::string_view newValue);

private:
    struct Private;

    struct StaticVariableRecord {
        using Value = std::variant<int, double, std::string>;

        std::string strKey;
        Value value;

        bool isValid() const;
    };

    std::vector<StaticVariableRecord> m_vecRecord;
};

} // namespace Mayo::IO
