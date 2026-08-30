/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/message_collecter.h"
#include "../src/base/meta_enum.h"

Q_DECLARE_METATYPE(Mayo::MessageType)

namespace Mayo {

void TestBase::MessageCollecter_ignoreSingleMessageType_test()
{
    QFETCH(MessageType, msgTypeToIgnore);

    MessageCollecter msgCollect;
    msgCollect.ignore(msgTypeToIgnore);
    QVERIFY(msgCollect.isIgnored(msgTypeToIgnore));
    for (auto msgType : MetaEnum::values<MessageType>()) {
        if (msgType != msgTypeToIgnore)
            QVERIFY(!msgCollect.isIgnored(msgType));
    }

    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        msgCollect.emitMessage(value, std::string{name} + " message");

    QCOMPARE(msgCollect.messages().size(), MetaEnum::count<MessageType>() - 1);
    for (const auto& msg : msgCollect.messages())
        QVERIFY(msg.type != msgTypeToIgnore);
}

void TestBase::MessageCollecter_ignoreSingleMessageType_test_data()
{
    QTest::addColumn<MessageType>("msgTypeToIgnore");
    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        QTest::newRow(std::string{name}.c_str()) << value;
}

void TestBase::MessageCollecter_only_test()
{
    QFETCH(MessageType, msgTypeSingle);

    MessageCollecter msgCollect;
    msgCollect.only(msgTypeSingle);
    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        msgCollect.emitMessage(value, std::string{name} + " message");

    QCOMPARE(msgCollect.messages().size(), 1);
    QCOMPARE(msgCollect.messages().front().type, msgTypeSingle);
    QCOMPARE(msgCollect.asString(" "), msgCollect.messages().front().text);
    QCOMPARE(msgCollect.asString(" ", msgTypeSingle), msgCollect.messages().front().text);
    QVERIFY(msgCollect.asString(" ").back() != ' ');
}

void TestBase::MessageCollecter_only_test_data()
{
    QTest::addColumn<MessageType>("msgTypeSingle");
    for (const auto& [value, name] : MetaEnum::entries<MessageType>())
        QTest::newRow(std::string{name}.c_str()) << value;
}

} // namespace Mayo
