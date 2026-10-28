/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../src/base/application.h"
#include "../src/base/occ_handle.h"
#include "../src/base/task_progress.h"
#include "../src/scripting/quickjs_mayo_api.h"
#include "../src/scripting/quickjs_script_engine.h"
#include "../src/scripting/quickjs_script_evaluation.h"
#include "../src/scripting/script_host_environment.h"

#include <any>
#include <string>

Q_DECLARE_METATYPE(std::string)

namespace Mayo {

namespace {

void installMayoApi(QuickJsScriptEngine& engine, const ScriptHostEnvironment& hostEnvironment)
{
    engine.addPreEvaluateCallback(
        [=](QuickJsScriptEvaluation& evaluation) {
            return quickjs_installMayoApi(evaluation, hostEnvironment);
        }
    );
}

} // namespace

void TestScripting::QuickJsMayoApi_hostApplication_test()
{
    ScriptHostEnvironment hostEnvironment;
    hostEnvironment.application = makeOccHandle<Application>();
    hostEnvironment.application->newDocument();
    hostEnvironment.application->newDocument();
    hostEnvironment.ioEntityImportPostProcess = [](TDF_Label, TaskProgress*) {};

    QuickJsScriptEngine engine;
    installMayoApi(engine, hostEnvironment);

    const auto eval = this->evaluateScript(engine, R"(
        export default mayo.app instanceof mayo.Application
            && mayo.app.documentCount === 2
            && mayo.app.graphics === true;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<bool>(eval.result.value), true);
}

void TestScripting::QuickJsMayoApi_applicationGraphics_test_data()
{
    QTest::addColumn<std::string>("construction");
    QTest::addColumn<bool>("expectedGraphics");

    QTest::newRow("default options")
        << std::string("new mayo.Application()") << false;
    QTest::newRow("graphics enabled")
        << std::string("new mayo.Application({ graphics: true })") << true;
    QTest::newRow("graphics disabled")
        << std::string("new mayo.Application({ graphics: false })") << false;
}

void TestScripting::QuickJsMayoApi_applicationGraphics_test()
{
    QFETCH(std::string, construction);
    QFETCH(bool, expectedGraphics);

    ScriptHostEnvironment hostEnvironment;
    hostEnvironment.application = makeOccHandle<Application>();

    QuickJsScriptEngine engine;
    installMayoApi(engine, hostEnvironment);

    const std::string script =
        "const app = " + construction + "; export default "
        "app instanceof mayo.Application && app.graphics === "
        + (expectedGraphics ? "true" : "false")
        + " && app.documentCount === 0;";

    const auto eval = this->evaluateScript(engine, script);

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<bool>(eval.result.value), true);
}

void TestScripting::QuickJsMayoApi_applicationRejectsInvalidGraphics_test()
{
    ScriptHostEnvironment hostEnvironment;
    hostEnvironment.application = makeOccHandle<Application>();

    QuickJsScriptEngine engine;
    installMayoApi(engine, hostEnvironment);

    const auto eval = this->evaluateScript(
        engine, "const app = new mayo.Application({ graphics: 1 }); export default app;"
    );

    QVERIFY(!eval.result.success);

    const auto errorMessage = eval.lastMessage(MessageType::Error);
    QVERIFY(errorMessage);
    QVERIFY(errorMessage->text.find("graphics") != std::string::npos);
}

} // namespace Mayo
