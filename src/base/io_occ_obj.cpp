/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_obj.h"
#include "scope_import.h"
#include "occ_progress_indicator.h"
#include "property_builtins.h"
#include "task_progress.h"
#include <fougtools/occtools/qt_utils.h>

namespace Mayo {
namespace IO {

namespace {

struct ObjReaderParameters : public PropertyGroup {
    ObjReaderParameters(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup),
          singlePrecisionVertexCoords(this, MAYO_TEXT_ID("Mayo::IO::ObjReader", "singlePrecisionVertexCoords")),
          rootPrefix(this, MAYO_TEXT_ID("Mayo::IO::ObjReader", "rootPrefix"))
    {
    }

    PropertyBool singlePrecisionVertexCoords;
    PropertyQString rootPrefix;
};

} // namespace

bool OccObjReader::readFile(const QString& filepath, TaskProgress* progress)
{
    m_filepath = filepath;
    progress->setValue(100);
    return true;
}

bool OccObjReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    m_reader.SetDocument(doc);
    Handle_Message_ProgressIndicator indicator = new OccProgressIndicator(progress);
    XCafScopeImport import(doc);
    const bool okPerform = m_reader.Perform(occ::QtUtils::toOccUtf8String(m_filepath), indicator);
    import.setConfirmation(okPerform && !TaskProgress::isAbortRequested(progress));
    return okPerform;
}

std::unique_ptr<PropertyGroup> OccObjReader::createParameters(PropertyGroup* parentGroup)
{
    return std::make_unique<ObjReaderParameters>(parentGroup);
}

void OccObjReader::applyParameters(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const ObjReaderParameters*>(params);
    if (ptr) {
        this->setSinglePrecisionVertexCoords(ptr->singlePrecisionVertexCoords.value());
        this->setRootPrefix(ptr->rootPrefix.value());
    }
}

QString OccObjReader::rootPrefix() const
{
    return occ::QtUtils::fromLatin1ToQString(m_reader.RootPrefix());
}

void OccObjReader::setRootPrefix(const QString& prefix)
{
    m_reader.SetRootPrefix(occ::QtUtils::toOccUtf8String(prefix));
}

} // namespace IO
} // namespace Mayo
