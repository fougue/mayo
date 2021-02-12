/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>

namespace Mayo {

class Messenger : public QObject {
    Q_OBJECT
public:
    Messenger(QObject* parent = nullptr);
    static Messenger* defaultInstance();

    enum class MessageType {
        Trace,
        Info,
        Warning,
        Error
    };

    void emitTrace(const QString& text);
    void emitInfo(const QString& text);
    void emitWarning(const QString& text);
    void emitError(const QString& text);
    virtual void emitMessage(MessageType msgType, const QString& text);

signals:
    void message(Messenger::MessageType msgType, const QString& text);
};

class NullMessenger : public Messenger {
public:
    static NullMessenger* instance();
    void emitMessage(MessageType msgType, const QString& text) override;

private:
    NullMessenger() = default;
};

} // namespace Mayo

#include <QtCore/QMetaType>
Q_DECLARE_METATYPE(Mayo::Messenger::MessageType)
