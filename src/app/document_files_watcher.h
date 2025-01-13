/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application_ptr.h"
#include "../base/document_ptr.h"
#include "../base/filepath.h"
#include "../base/quantity.h"
#include "../base/signal.h"

#include <QtCore/QObject>

#include <unordered_set>
#include <vector>

class QFileSystemWatcher;

namespace Mayo {

// Monitors the file system for changes to Document files owned by Application object
// When a Document is opened then DocumentFilesWatcher automatically monitors the corresponding file
// for changes. When an Document is closed then monitoring for that file is terminated
// File changes need to be acknowledged with DocumentFilesWatcher:acknowledgeDocumentFileChange()
// otherwise there won't be further signal notification for the changed file
class DocumentFilesWatcher : public QObject {
public:
    DocumentFilesWatcher(const ApplicationPtr& app, QObject* parent = nullptr);

    Signal<DocumentPtr> signalDocumentFileChanged;

    void enable(bool on);
    bool isEnabled() const;

    void acknowledgeDocumentFileChange(const DocumentPtr& doc);

    QuantityTime signalSendDelay() const;
    void setSignalSendDelay(QuantityTime delay);

private:
    QFileSystemWatcher* fileSystemWatcher();
    void destroyFileSystemWatcher();
    void onFileChanged(const QString& strFilePath);

    bool isDocumentChangeAcknowledged(const DocumentPtr& doc) const;

    void onDocumentFilePathChanged(const DocumentPtr& doc, const FilePath& fp);
    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);

    ApplicationPtr m_app;
    QFileSystemWatcher* m_fileSystemWatcher = nullptr;
    bool m_isEnabled = false;
    std::unordered_set<DocumentPtr> m_setDocBeingChanged;
    std::vector<DocumentPtr> m_vecNonAckDocumentChanged;
    QuantityTime m_signalSendDelay = 1 * Quantity_Second;
};

} // namespace Mayo
