/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/message_collecter.h"
#include "../src/base/thread_messenger_channel.h"

#include <Message.hxx>
#include <Message_Messenger.hxx>

#include <thread>

namespace Mayo {

namespace {

// Executes `fn` in a distinct thread and waits for its completion
template<typename Fn>
void runOnThread(Fn&& fn)
{
    std::thread worker(std::forward<Fn>(fn));
    worker.join();
}

// True if `printer` is present in the sequence of printers currently registered on OpenCascade's
// default messenger
bool isRegisteredOnDefaultMessenger(const OccHandle<Message_Printer>& printer)
{
    for (const auto& registeredPrinter : ::Message::DefaultMessenger()->Printers()) {
        if (registeredPrinter == printer)
            return true;
    }
    return false;
}

} // namespace

void TestBase::ThreadMessengerChannel_emitMessageWithoutActiveScopeIsDiscardedSilently_test()
{
    ThreadMessengerChannel channel;
    // No active Scope on this thread at this point: must not throw, must not forward anywhere
    channel.emitMessage(MessageType::Error, "should be discarded");
    // No positive assertion possible for "nothing happened" other than the absence of a crash/UB
    // -- covered by simply reaching this point
    QVERIFY(true);
}

void TestBase::ThreadMessengerChannel_emitMessageWithActiveScopeIsForwarded_test()
{
    ThreadMessengerChannel channel;
    MessageCollecter recorder;

    {
        [[maybe_unused]] ThreadMessengerChannel::Scope scope(&recorder);
        channel.emitMessage(MessageType::Warning, "hello");
        channel.emitMessage(MessageType::Error, "world");
    }

    QCOMPARE(recorder.messages().size(), size_t(2));
    QCOMPARE(recorder.messages()[0].type, MessageType::Warning);
    QCOMPARE(recorder.messages()[0].text, "hello");
    QCOMPARE(recorder.messages()[1].type, MessageType::Error);
    QCOMPARE(recorder.messages()[1].text, "world");
}

void TestBase::ThreadMessengerChannelScope_restoresPreviousMessengerOnDestruction_test()
{
    ThreadMessengerChannel channel;
    MessageCollecter outer;

    ThreadMessengerChannel::Scope outerScope(&outer);
    channel.emitMessage(MessageType::Info, "before inner scope");

    {
        MessageCollecter inner;
        [[maybe_unused]] ThreadMessengerChannel::Scope innerScope(&inner);
        channel.emitMessage(MessageType::Info, "inside inner scope");

        QCOMPARE(outer.messages().size(), size_t(1)); // Not affected by innerScope yet
        QCOMPARE(inner.messages().size(), size_t(1));
    }

    // InnerScope destroyed -> should be back to 'outer'
    channel.emitMessage(MessageType::Info, "after inner scope");

    QCOMPARE(outer.messages().size(), size_t(2));
    QCOMPARE(outer.messages()[0].text, "before inner scope");
    QCOMPARE(outer.messages()[1].text, "after inner scope");
}

void TestBase::ThreadMessengerChannelScope_supportsNestedReentrance_test()
{
    // Simulates a Mayo Reader internally invoking another one (composite formats / nested imports)
    ThreadMessengerChannel channel;
    MessageCollecter level1;
    MessageCollecter level2;
    MessageCollecter level3;

    [[maybe_unused]] ThreadMessengerChannel::Scope scope1(&level1);
    channel.emitMessage(MessageType::Trace, "L1");
    {
        [[maybe_unused]] ThreadMessengerChannel::Scope scope2(&level2);
        channel.emitMessage(MessageType::Trace, "L2");
        {
            [[maybe_unused]] ThreadMessengerChannel::Scope scope3(&level3);
            channel.emitMessage(MessageType::Trace, "L3");
        }
        // expected to fall back to: level2
        channel.emitMessage(MessageType::Trace, "back to L2");
    }
    // expected to fall back to: level1
    channel.emitMessage(MessageType::Trace, "back to L1");

    QCOMPARE(level1.messages().size(), size_t(2));
    QCOMPARE(level2.messages().size(), size_t(2));
    QCOMPARE(level3.messages().size(), size_t(1));
}

void TestBase::ThreadMessengerChannelScope_boundToNullptrDiscardsMessages_test()
{
    ThreadMessengerChannel channel;
    MessageCollecter outer;

    [[maybe_unused]] ThreadMessengerChannel::Scope outerScope(&outer);
    {
        // A scope explicitly bound to nullptr must mute messages, not fall back to 'outer'
        [[maybe_unused]] ThreadMessengerChannel::Scope mutedScope(nullptr);
        channel.emitMessage(MessageType::Error, "should be discarded");
    }
    channel.emitMessage(MessageType::Error, "should reach outer");

    QCOMPARE(outer.messages().size(), size_t(1));
    QCOMPARE(outer.messages()[0].text, "should reach outer");
}

void TestBase::ThreadMessengerChannelScope_isIsolatedPerThread_test()
{
    ThreadMessengerChannel channel;
    MessageCollecter mainThreadMessenger;
    MessageCollecter workerThreadMessenger;

    [[maybe_unused]] ThreadMessengerChannel::Scope mainScope(&mainThreadMessenger);

    runOnThread([&]{
        // The worker thread must NOT see mainThreadMessenger: with no Scope bound on this thread
        // yet, messages are discarded first
        channel.emitMessage(MessageType::Warning, "before worker scope");

        [[maybe_unused]] ThreadMessengerChannel::Scope workerScope(&workerThreadMessenger);
        channel.emitMessage(MessageType::Warning, "inside worker scope");
    });

    channel.emitMessage(MessageType::Warning, "on main thread");

    QCOMPARE(workerThreadMessenger.messages().size(), size_t(1));
    QCOMPARE(workerThreadMessenger.messages()[0].text, "inside worker scope");

    // mainThreadMessenger must only have received the message emitted on the main thread, nothing
    // from what happened on the worker thread
    QCOMPARE(mainThreadMessenger.messages().size(), size_t(1));
    QCOMPARE(mainThreadMessenger.messages()[0].text, "on main thread");
}

void TestBase::ThreadMessengerChannel_addGlobalOccPrinterReturnsNonNullPrinter_test()
{
    auto printer = ThreadMessengerChannel::addGlobalOccPrinter();
    QVERIFY(!printer.IsNull());
}

void TestBase::ThreadMessengerChannel_addGlobalOccPrinterIsIdempotent_test()
{
    auto p1 = ThreadMessengerChannel::addGlobalOccPrinter();
    auto p2 = ThreadMessengerChannel::addGlobalOccPrinter();
    auto p3 = ThreadMessengerChannel::addGlobalOccPrinter();

    // Same underlying Message_Printer instance every time, not a new one per call
    QCOMPARE(p1.get(), p2.get());
    QCOMPARE(p1.get(), p3.get());
}

void TestBase::ThreadMessengerChannel_addGlobalOccPrinter_registersExactlyOnceWithDefaultMessenger_test()
{
    auto printer = ThreadMessengerChannel::addGlobalOccPrinter();
    QVERIFY(isRegisteredOnDefaultMessenger(printer));

    // Calling again must not add a second, duplicate entry
    int occurrenceCount = 0;
    for (const auto& registeredPrinter : ::Message::DefaultMessenger()->Printers()) {
        if (registeredPrinter == printer)
            ++occurrenceCount;
    }

    ThreadMessengerChannel::addGlobalOccPrinter();
    ThreadMessengerChannel::addGlobalOccPrinter();

    int occurrenceCountAfterMoreCalls = 0;
    for (const auto& registeredPrinter : ::Message::DefaultMessenger()->Printers()) {
        if (registeredPrinter == printer)
            ++occurrenceCountAfterMoreCalls;
    }

    QCOMPARE(occurrenceCountAfterMoreCalls, occurrenceCount);
    QCOMPARE(occurrenceCountAfterMoreCalls, 1);
}

void TestBase::ThreadMessengerChannel_addGlobalOccPrinter_concurrentFirstCallsInstallExactlyOnce_test()
{
    // Regardless of whether this test runs before or after the others in the suite, this verifies
    // the thread-safety property itself: many threads racing to call addGlobalOccPrinter() must all
    // observe the same instance, and the messenger must end up with exactly one registration
    constexpr int threadCount = 16;
    std::vector<OccHandle<Message_Printer>> resultsPerThread(threadCount, nullptr);
    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([i, &resultsPerThread]{
            resultsPerThread[i] = ThreadMessengerChannel::addGlobalOccPrinter();
        });
    }

    for (auto& t : threads)
        t.join();

    for (int i = 1; i < threadCount; ++i)
        QCOMPARE(resultsPerThread[i].get(), resultsPerThread[0].get());

    int occurrenceCount = 0;
    for (const auto& registeredPrinter : ::Message::DefaultMessenger()->Printers()) {
        if (registeredPrinter == resultsPerThread[0])
            ++occurrenceCount;
    }

    QCOMPARE(occurrenceCount, 1);
}

void TestBase::ThreadMessengerChannel_addGlobalOccPrinter_integrationForwardsOcctMessagesToScopedMessenger_test()
{
    ThreadMessengerChannel::addGlobalOccPrinter(); // ensure installed, idempotent

    MessageCollecter recorder;
    {
        [[maybe_unused]] ThreadMessengerChannel::Scope scope(&recorder);
        ::Message::DefaultMessenger()->Send("integration warning", Message_Warning);
        ::Message::DefaultMessenger()->Send("integration failure", Message_Fail);
    }
    // Messages emitted after the scope ends must not reach 'recorder'
    ::Message::DefaultMessenger()->Send("should not be recorded", Message_Warning);

    QCOMPARE(recorder.messages().size(), size_t(2));
    QCOMPARE(recorder.messages()[0].type, MessageType::Warning);
    QCOMPARE(recorder.messages()[1].type, MessageType::Error); // Message_Fail -> Error
}

} // namespace Mayo
