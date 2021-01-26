/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_gmio_amf_writer.h"

#include "../base/application_item.h"
#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/math_utils.h"
#include "../base/meta_enum.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/unit_system.h"
#include "../base/xcaf.h"

#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <gp_Quaternion.hxx>

#include <gmio_amf/amf_error.h>
#include <gmio_amf/amf_io.h>
#include <gmio_core/error.h>
#include <gmio_stl/stl_error.h>

#include <unordered_map>

namespace Mayo {
namespace IO {

namespace {

bool gmio_taskIsStopRequested(void* cookie)
{
    return TaskProgress::isAbortRequested(static_cast<const TaskProgress*>(cookie));
}

void gmio_handleProgress(void* cookie, intmax_t value, intmax_t maxValue)
{
    auto progress = static_cast<TaskProgress*>(cookie);
    if (progress && maxValue > 0) {
        const auto pctNorm = value / double(maxValue);
        const auto pct = qRound(pctNorm * 100);
        if (pct >= (progress->value() + 5))
            progress->setValue(pct);
    }
}

gmio_task_iface gmio_createTask(TaskProgress* progress)
{
    gmio_task_iface task = {};
    task.cookie = progress;
    task.func_is_stop_requested = gmio_taskIsStopRequested;
    task.func_handle_progress = gmio_handleProgress;
    return task;
}

//#ifdef HAVE_GMIO
#if 0
Format System::probeFormat(const QString& filepath) const
{
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
//#ifdef HAVE_GMIO
        gmio_stream qtstream = gmio_stream_qiodevice(&file);
       const gmio_stl_format stlFormat = gmio_stl_format_probe(&qtstream);
        if (stlFormat != GMIO_STL_FORMAT_UNKNOWN)
            return Format_STL;
//#endif
}

IO::Result IO::exportStl_gmio(ExportData data)
{
    QFile file(data.filepath);
    if (file.open(QIODevice::WriteOnly)) {
        gmio_stream stream = gmio_stream_qiodevice(&file);
        gmio_stl_write_options gmioOptions = {};
        gmioOptions.stla_float32_format = options.stlaFloat32Format;
        gmioOptions.stla_float32_prec = options.stlaFloat32Precision;
        gmioOptions.stla_solid_name = options.stlaSolidName.c_str();
        gmioOptions.task_iface = Internal::gmio_qttask_create_task_iface(progress);
        for (const DocumentItem* item : docItems) {
            if (progress) {
                progress->setStep(
                            tr("Writing item %1")
                            .arg(item->propertyLabel.value()));
            }

            int error = GMIO_ERROR_OK;
            if (sameType<XdeDocumentItem>(item)) {
                auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
                const TopoDS_Shape shape = Internal::xdeDocumentWholeShape(xdeDocItem);
                const gmio_stl_mesh_occshape gmioMesh(shape);
                error = gmio_stl_write(
                            options.stlFormat, &stream, &gmioMesh, &gmioOptions);
            }
            else if (sameType<MeshItem>(item)) {
                auto meshItem = static_cast<const MeshItem*>(item);
                const gmio_stl_mesh_occpolytri gmioMesh(meshItem->triangulation());
                error = gmio_stl_write(
                            options.stlFormat, &stream, &gmioMesh, &gmioOptions);
            }

            if (error != GMIO_ERROR_OK)
                return IoResult::error(Internal::gmioErrorToQString(error));
        }

        return IoResult::ok();
    }

    return Result::error(file.errorString());
}
#endif // HAVE_GMIO

} // namespace

class GmioAmfWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::GmioAmfWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->float64Format.mutableEnumeration().changeTrContext(this->textIdContext());
        this->float64Format.setDescription(
                    textIdTr("Format used when writting `double` values as strings"));
        this->float64Format.setDescriptions({
                    { FloatTextFormat::Decimal, textIdTr("Decimal floating point(ex: 392.65)") },
                    { FloatTextFormat::Scientific, textIdTr("Scientific notation(ex: 3.9265E+2)") },
                    { FloatTextFormat::Shortest, textIdTr("Use the shortest representation: decimal or scientific") }
        });

        this->float64Precision.setConstraintsEnabled(true);
        this->float64Precision.setRange(1, 16);
        this->float64Precision.setDescription(
                    textIdTr("Maximum number of significant digits when writting `double` values"));

        this->createZipArchive.setDescription(
                    textIdTr("Write AMF document in ZIP archive containing one file entry"));

        this->zipEntryFilename.setDescription(
                    textIdTr("Filename of the single AMF entry within the ZIP archive.\n"
                             "Only applicable if option `%1` is on").arg(this->createZipArchive.label()));

        this->useZip64.setDescription(
                    textIdTr("Use the ZIP64 format extensions.\n"
                             "Only applicable if option `%1` is on").arg(this->createZipArchive.label()));
    }

    void restoreDefaults() override {
        const GmioAmfWriter::Parameters params;
        this->float64Format.setValue(params.float64Format);
        this->float64Precision.setValue(params.float64Precision);
        this->createZipArchive.setValue(params.createZipArchive);
        this->zipEntryFilename.setValue(QString::fromStdString(params.zipEntryFilename));
        this->useZip64.setValue(params.useZip64);
    }

    PropertyEnum<GmioAmfWriter::FloatTextFormat> float64Format{ this, textId("float64Format") };
    PropertyInt float64Precision{ this, textId("float64Precision") };
    PropertyBool createZipArchive{ this, textId("createZipArchive") };
    PropertyQString zipEntryFilename{ this, textId("zipEntryFilename") };
    PropertyBool useZip64{ this, textId("useZip64") };
};

bool GmioAmfWriter::transfer(Span<const ApplicationItem> spanAppItem, TaskProgress* progress)
{
    m_vecMaterial.clear();
    m_vecMesh.clear();
    m_vecObject.clear();
    m_vecInstance.clear();

    Material defaultMaterial = {};
    defaultMaterial.id = 0;
    defaultMaterial.color.SetValues(Quantity_NOC_WHITE);
    defaultMaterial.isColor = true;
    m_vecMaterial.push_back(std::move(defaultMaterial));

    std::unordered_map<TDF_Label, int> mapLabelObjectId;
    auto fnFindObjectId = [&](const TDF_Label& label) {
        auto it = mapLabelObjectId.find(label);
        return it != mapLabelObjectId.cend() ? it->second : -1;
    };
    auto fnCreateObject = [&](const Tree<TDF_Label>& modelTree, TreeNodeId id) {
        const TDF_Label nodeLabel = modelTree.nodeData(id);
        if (modelTree.nodeIsLeaf(id)) {
            int objectId = fnFindObjectId(nodeLabel);
            if (objectId == -1) {
                objectId = this->createObject(nodeLabel);
                if (objectId == -1)
                    return;

                mapLabelObjectId.insert({ nodeLabel, objectId });
            }

            QStringList absoluteName;
            TreeNodeId itParent = id;
            do {
                itParent = modelTree.nodeParent(itParent);
                if (itParent != 0) {
                    const QString name = CafUtils::labelAttrStdName(modelTree.nodeData(itParent));
                    if (!name.trimmed().isEmpty())
                        absoluteName += name;
                    else
                        absoluteName += "anonymous";
                }
            } while (itParent != 0);

            if (!absoluteName.isEmpty()) {
                Instance instance;
                instance.objectId = objectId;
                instance.trsf = XCaf::shapeAbsoluteLocation(modelTree, id);
                instance.name = absoluteName.join('/').toStdString();
                m_vecInstance.push_back(std::move(instance));
            }
        }
    };

    for (const ApplicationItem& appItem : spanAppItem) {
        const int appItemIndex = &appItem - &spanAppItem.at(0);
        progress->setValue(MathUtils::mappedValue(appItemIndex, 0, spanAppItem.size() - 1, 0, 100));
        const Tree<TDF_Label>& modelTree = appItem.document()->modelTree();
        if (appItem.isDocument()) {
            traverseTree(modelTree, [&](TreeNodeId id) { fnCreateObject(modelTree, id); });
        }
        else if (appItem.isDocumentTreeNode()) {
            traverseTree(appItem.documentTreeNode().id(), modelTree, [&](TreeNodeId id) {
                fnCreateObject(modelTree, id);
            });
        }
    }

    return true;
}

bool GmioAmfWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    gmio_amf_document amfDoc = {};
    amfDoc.cookie = this;
    amfDoc.unit = GMIO_AMF_UNIT_MILLIMETER;
    amfDoc.object_count = int(m_vecObject.size());
    amfDoc.material_count = int(m_vecMaterial.size());
    amfDoc.constellation_count = !m_vecInstance.empty() ? 1 : 0;
    amfDoc.func_get_document_element = &GmioAmfWriter::amf_getDocumentElement;
    amfDoc.func_get_document_element_metadata = &GmioAmfWriter::amf_getDocumentElementMetadata;
    amfDoc.func_get_object_mesh = &GmioAmfWriter::amf_getObjectMesh;
    amfDoc.func_get_object_mesh_element = &GmioAmfWriter::amf_getObjectMeshElement;
    amfDoc.func_get_object_mesh_volume_triangle = &GmioAmfWriter::amf_getObjectMeshVolumeTriangle;
    amfDoc.func_get_constellation_instance = &GmioAmfWriter::amf_getConstellationInstance;

    auto fnAmfFloat64Format = [](FloatTextFormat format) {
        switch (format) {
        case FloatTextFormat::Decimal: return GMIO_FLOAT_TEXT_FORMAT_DECIMAL_UPPERCASE;
        case FloatTextFormat::Scientific: return GMIO_FLOAT_TEXT_FORMAT_SCIENTIFIC_UPPERCASE;
        case FloatTextFormat::Shortest: return GMIO_FLOAT_TEXT_FORMAT_SHORTEST_UPPERCASE;
        }
    };

    gmio_amf_write_options amfOptions = {};
    amfOptions.task_iface.cookie = progress;
    amfOptions.task_iface.func_handle_progress = &gmio_handleProgress;
    amfOptions.task_iface.func_is_stop_requested = &gmio_taskIsStopRequested;
    amfOptions.float64_format = fnAmfFloat64Format(m_params.float64Format);
    amfOptions.float64_prec = m_params.float64Precision;
    amfOptions.create_zip_archive = m_params.createZipArchive;
    amfOptions.dont_use_zip64_extensions = !m_params.useZip64;
    amfOptions.zip_entry_filename = m_params.zipEntryFilename.c_str();
    amfOptions.zip_entry_filename_len = m_params.zipEntryFilename.size();
    // TODO Handle gmio_amf_write_options::z_compress_options
    const int error = gmio_amf_write_file(filepath.toUtf8().constData(), &amfDoc, &amfOptions);
    return gmio_no_error(error);
}

std::unique_ptr<PropertyGroup> GmioAmfWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void GmioAmfWriter::applyProperties(const PropertyGroup* group)
{
    auto ptr = dynamic_cast<const Properties*>(group);
    if (ptr) {
        m_params.float64Format = ptr->float64Format;
        m_params.float64Precision = ptr->float64Precision;
        m_params.createZipArchive = ptr->createZipArchive;
        m_params.zipEntryFilename = ptr->zipEntryFilename.value().toStdString();
        m_params.useZip64 = ptr->useZip64;
    }
}

int GmioAmfWriter::createObject(const TDF_Label& labelShape)
{
    // Object meshes
    const int meshCount = int(m_vecMesh.size());

    auto fnAddMesh = [&](const Handle_Poly_Triangulation& polyTri, const TopLoc_Location& loc) {
        if (!polyTri.IsNull()) {
            Mesh mesh;
            mesh.id = int(m_vecMesh.size());
            mesh.triangulation = polyTri;
            mesh.location = loc;
            // TODO mesh.materialId = ?
            m_vecMesh.push_back(std::move(mesh));
        }
    };

    // -- Shape ?
    const TopoDS_Shape shape = XCaf::shape(labelShape);
    if (!shape.IsNull()) {
        BRepUtils::forEachSubFace(shape, [=](const TopoDS_Face& face){
            TopLoc_Location loc;
            const Handle_Poly_Triangulation& polyTri = BRep_Tool::Triangulation(face, loc);
            fnAddMesh(polyTri, loc);
        });
    }

    // -- Triangulation ?
    auto attrPolyTri = CafUtils::findAttribute<TDataXtd_Triangulation>(labelShape);
    if (!attrPolyTri.IsNull()) {
        fnAddMesh(attrPolyTri->Get(), TopLoc_Location());
    }

    if (m_vecMesh.size() == meshCount)
        return -1;

    // Object material
    int materialId = -1;
    DocumentPtr doc = Document::findFrom(labelShape);
    if (doc && doc->xcaf().hasShapeColor(labelShape)) {
        const Quantity_Color color = doc->xcaf().shapeColor(labelShape);
        auto itColor = std::find_if(
                    m_vecMaterial.cbegin(), m_vecMaterial.cend(), [=](const Material& mat) {
            return mat.color == color;
        });
        if (itColor != m_vecMaterial.cend()) {
            materialId = itColor - m_vecMaterial.cbegin();
        }
        else {
            materialId = m_vecMaterial.size();
            Material material;
            material.id = materialId;
            material.color = color;
            material.isColor = true;
            m_vecMaterial.push_back(std::move(material));
        }
    }

    // Add object
    Object object;
    object.id = m_vecObject.size();
    object.firstMeshId = meshCount;
    object.lastMeshId = m_vecMesh.size() - 1;
    object.name = CafUtils::labelAttrStdName(labelShape).toStdString();
    object.materialId = materialId;
    m_vecObject.push_back(std::move(object));
    return m_vecObject.back().id;
}

const GmioAmfWriter* GmioAmfWriter::from(const void* cookie) {
    return static_cast<const GmioAmfWriter*>(cookie);
}

void GmioAmfWriter::amf_getDocumentElement(
        const void* cookie, gmio_amf_document_element element, uint32_t elementIndex, void* ptrElement)
{
    auto writer = GmioAmfWriter::from(cookie);
    if (element == GMIO_AMF_DOCUMENT_ELEMENT_OBJECT) {
        const Object& object = writer->m_vecObject.at(elementIndex);
        auto amfObject = static_cast<gmio_amf_object*>(ptrElement);
        *amfObject = {};
        amfObject->id = object.id;
        amfObject->mesh_count = object.lastMeshId - object.firstMeshId + 1;
        amfObject->metadata_count = 1;
    }
    else if (element == GMIO_AMF_DOCUMENT_ELEMENT_MATERIAL) {
        const Material& material = writer->m_vecMaterial.at(elementIndex);
        auto amfMaterial = static_cast<gmio_amf_material*>(ptrElement);
        *amfMaterial = {};
        if (material.isColor) {
            amfMaterial->id = material.id;
            amfMaterial->color.r = material.color.Red();
            amfMaterial->color.g = material.color.Green();
            amfMaterial->color.b = material.color.Blue();
        }
    }
    else if (element == GMIO_AMF_DOCUMENT_ELEMENT_CONSTELLATION) {
        auto amfConstellation = static_cast<gmio_amf_constellation*>(ptrElement);
        *amfConstellation = {};
        // At most one constellation
        amfConstellation->id = 0;
        amfConstellation->instance_count = int(writer->m_vecInstance.size());
    }
}

void GmioAmfWriter::amf_getDocumentElementMetadata(
        const void* cookie,
        gmio_amf_document_element element,
        uint32_t elementIndex,
        uint32_t metadataIndex,
        gmio_amf_metadata* ptrMetadata)
{
    auto writer = GmioAmfWriter::from(cookie);
    if (element == GMIO_AMF_DOCUMENT_ELEMENT_OBJECT) {
        const Object& object = writer->m_vecObject.at(elementIndex);
        if (metadataIndex == 0) {
            ptrMetadata->type = "name";
            ptrMetadata->data = object.name.c_str();
        }
    }
}

void GmioAmfWriter::amf_getObjectMesh(
        const void* cookie, uint32_t objectIndex, uint32_t meshIndex, gmio_amf_mesh* ptrMesh)
{
    auto writer = GmioAmfWriter::from(cookie);
    const Object& object = writer->m_vecObject.at(objectIndex);
    const Mesh& mesh = writer->m_vecMesh.at(object.firstMeshId + meshIndex);
    *ptrMesh = {};
    ptrMesh->vertex_count = mesh.triangulation->NbNodes();
    ptrMesh->volume_count = 1;
}

void GmioAmfWriter::amf_getObjectMeshElement(
        const void* cookie, const gmio_amf_object_mesh_element_index* elementIndex, void* ptrElement)
{
    auto writer = GmioAmfWriter::from(cookie);
    const Object& object = writer->m_vecObject.at(elementIndex->object_index);
    const Mesh& mesh = writer->m_vecMesh.at(object.firstMeshId + elementIndex->mesh_index);
    if (elementIndex->element_type == GMIO_AMF_MESH_ELEMENT_VERTEX) {
        const gp_Pnt& pnt = mesh.triangulation->Node(elementIndex->value + 1);
        auto amfVertex = static_cast<gmio_amf_vertex*>(ptrElement);
        *amfVertex = {};
        amfVertex->coords.x = pnt.X();
        amfVertex->coords.y = pnt.Y();
        amfVertex->coords.z = pnt.Z();
    }
    else if (elementIndex->element_type == GMIO_AMF_MESH_ELEMENT_VOLUME) {
        auto amfVolume = static_cast<gmio_amf_volume*>(ptrElement);
        *amfVolume = {};
        amfVolume->type = GMIO_AMF_VOLUME_TYPE_OBJECT;
        amfVolume->triangle_count = mesh.triangulation->NbTriangles();
        amfVolume->materialid = object.materialId;
    }
}

void GmioAmfWriter::amf_getObjectMeshVolumeTriangle(
        const void* cookie,
        const gmio_amf_object_mesh_element_index* volumeIndex,
        uint32_t triangleIndex,
        gmio_amf_triangle* ptrTriangle)
{
    auto writer = GmioAmfWriter::from(cookie);
    const Object& object = writer->m_vecObject.at(volumeIndex->object_index);
    const Mesh& mesh = writer->m_vecMesh.at(object.firstMeshId + volumeIndex->mesh_index);
    if (volumeIndex->value == 0) {
        const Poly_Triangle& triangle = mesh.triangulation->Triangle(triangleIndex + 1);
        *ptrTriangle = {};
        ptrTriangle->v1 = triangle.Value(1) - 1;
        ptrTriangle->v2 = triangle.Value(2) - 1;
        ptrTriangle->v3 = triangle.Value(3) - 1;
    }
}

void GmioAmfWriter::amf_getConstellationInstance(
        const void* cookie,
        uint32_t constellationIndex,
        uint32_t instanceIndex,
        gmio_amf_instance* ptrInstance)
{
    auto writer = GmioAmfWriter::from(cookie);
    if (constellationIndex == 0) {
        const Instance& instance = writer->m_vecInstance.at(instanceIndex);
        *ptrInstance = {};
        ptrInstance->objectid = writer->m_vecInstance.at(instanceIndex).objectId;
        ptrInstance->delta.x = instance.trsf.TranslationPart().X();
        ptrInstance->delta.y = instance.trsf.TranslationPart().Y();
        ptrInstance->delta.z = instance.trsf.TranslationPart().Z();

        double xRotVal, yRotVal, zRotVal;
        instance.trsf.GetRotation().GetEulerAngles(gp_Intrinsic_XYZ, xRotVal, yRotVal, zRotVal);
        ptrInstance->rot.x = UnitSystem::degrees(xRotVal * Mayo::Quantity_Radian);
        ptrInstance->rot.y = UnitSystem::degrees(yRotVal * Mayo::Quantity_Radian);
        ptrInstance->rot.z = UnitSystem::degrees(zRotVal * Mayo::Quantity_Radian);
    }
}

} // namespace IO
} // namespace Mayo
