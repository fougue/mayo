#pragma once

#include <QtCore/QString>
#include <cstring>
#include <vector>

namespace Mayo {

class Document;

class DocumentItem
{
public:
    Document* document();
    const Document* document() const;
    void setDocument(Document* doc);

    const QString& label() const;
    void setLabel(const QString& v);

    const std::vector<DocumentItem*>& outItems() const; // For future use

    virtual const char* dynType() const = 0;

private:
    Document* m_document = nullptr;
    std::vector<DocumentItem*> m_outItems;
    QString m_label;
};

class PartItem : public DocumentItem
{
public:
    const QString& filePath() const;
    void setFilePath(const QString& v);

    virtual bool isNull() const;

    static const char* type;
    const char* dynType() const override;

private:
    QString m_filePath;
};

bool sameType(const DocumentItem* lhs, const DocumentItem* rhs);

template<typename T>
bool sameType(const DocumentItem* item)
{
    return item != nullptr ?
                std::strcmp(item->dynType(), T::type) == 0 :
                false;
}

} // namespace Mayo
