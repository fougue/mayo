/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application_ptr.h"
#include "../base/document.h"

#include <QtCore/QObject>
class QJSEngine;

#include <unordered_map>
#include <vector>

namespace Mayo {

class ScriptDocument;

class ScriptApplication : public QObject {
    Q_OBJECT
    Q_PROPERTY(int documentCount READ documentCount NOTIFY documentCountChanged)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
public:
    ScriptApplication(const ApplicationPtr& app, QJSEngine* jsEngine = nullptr);

    QString versionString() const;

    int documentCount() const;
    Q_INVOKABLE QObject* newDocument();
    Q_INVOKABLE QObject* documentAt(int docIndex) const;
    Q_INVOKABLE QObject* findDocumentByLocation(const QString& location) const;
    Q_INVOKABLE int findIndexOfDocument(QObject* doc) const;

    Q_INVOKABLE void closeDocument(QObject* doc);

    QJSEngine* jsEngine() const { return m_jsEngine; }

signals:
    void documentAdded(QObject* doc);
    void documentAboutToClose(QObject* doc);
    void documentCountChanged();

private:
    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);

    ApplicationPtr m_app;
    QJSEngine* m_jsEngine = nullptr;
    std::vector<ScriptDocument*> m_vecJsDoc;
    std::unordered_map<Document::Identifier, ScriptDocument*> m_mapIdToScriptDocument;
    ScopedSignalConnections<> m_sigConns;
};

} // namespace Mayo
