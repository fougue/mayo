#pragma once

#include "property_builtins.h"
#include <cstring>
#include <vector>

namespace Mayo {

class Document;

class DocumentItem : public PropertyOwner
{
public:
    DocumentItem();

    Document* document();
    const Document* document() const;
    void setDocument(Document* doc);

    PropertyQString propertyLabel;

    const std::vector<DocumentItem*>& outItems() const; // For future use

    virtual const char* dynType() const = 0;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    Document* m_document = nullptr;
    std::vector<DocumentItem*> m_outItems;
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
