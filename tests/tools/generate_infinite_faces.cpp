/****************************************************************************
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

// Public, synthetic reproduction for boundaryless unbounded STEP faces.
// Build with Mayo_BuildTests=ON, then run:
//   mayo-repro-infinite-faces <output-directory>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Precision.hxx>
#include <STEPConstruct.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_Version.hxx>
#include <StepBasic_Product.hxx>
#include <StepData_StepModel.hxx>
#include <StepShape_AdvancedFace.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_WorkSession.hxx>

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

TopoDS_Compound compoundOf(const TopoDS_Shape& box, const TopoDS_Face& face)
{
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    builder.Add(compound, box);
    builder.Add(compound, face);
    return compound;
}

void generate(const std::filesystem::path& directory, const std::string& kind,
              const Handle(Geom_Surface)& surface)
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(80., 60., 40.);

    // Construct the actual topology directly: there must be no generated
    // seam, outer wire, or natural boundary edge on the additional face.
    TopoDS_Face unbounded;
    BRep_Builder().MakeFace(unbounded, surface, Precision::Confusion());
    const auto directShape = compoundOf(box, unbounded);
    const auto stem = directory / ("box-with-unbounded-" + kind);
    const std::string brepPath = stem.string() + ".brep";
    if (!BRepTools::Write(directShape, brepPath.c_str()))
        throw std::runtime_error("Failed to write " + brepPath);

    // OCCT's regular STEP writer intentionally skips faces without an outer
    // wire (TopoDSToStep_MakeStepFace). First transfer a bounded placeholder,
    // then empty only its STEP Face.bounds field. This keeps the product,
    // units, placement and shell representation written by OCCT, and emits
    // the exact ADVANCED_FACE(...,(),surface,...) condition under test.
    const TopoDS_Face bounded = kind == "plane"
            ? BRepBuilderAPI_MakeFace(surface, -20., 20., -15., 15., Precision::Confusion()).Face()
            : BRepBuilderAPI_MakeFace(surface, 0., 6.283185307179586, -5., 5., Precision::Confusion()).Face();
    STEPControl_Writer writer;
    if (writer.Transfer(compoundOf(box, bounded), STEPControl_AsIs) != IFSelect_RetDone)
        throw std::runtime_error("STEP transfer failed for " + kind);

    int productIndex = 0;
    const auto model = writer.Model();
    for (int i = 1; i <= model->NbEntities(); ++i) {
        const auto product = Handle(StepBasic_Product)::DownCast(model->Value(i));
        if (product.IsNull())
            continue;
        const std::string name = productIndex == 0 ? "Synthetic box + " + kind
                : productIndex == 1 ? "Finite box 80 x 60 x 40" : kind + " test face";
        product->SetName(new TCollection_HAsciiString(name.c_str()));
        ++productIndex;
    }

    const std::string controlPath = (directory / ("box-with-bounded-" + kind + ".step")).string();
    if (writer.Write(controlPath.c_str()) != IFSelect_RetDone)
        throw std::runtime_error("Failed to write control " + kind);

    const auto finder = writer.WS()->TransferWriter()->FinderProcess();
    const auto stepFace = Handle(StepShape_AdvancedFace)::DownCast(STEPConstruct::FindEntity(finder, bounded));
    if (stepFace.IsNull())
        throw std::runtime_error("STEP face mapping not found for " + kind);
    stepFace->SetBounds(Handle(StepShape_HArray1OfFaceBound)());
    stepFace->SetName(new TCollection_HAsciiString(("Unbounded " + kind + " test face").c_str()));

    const std::string stepPath = stem.string() + ".step";
    if (writer.Write(stepPath.c_str()) != IFSelect_RetDone)
        throw std::runtime_error("Failed to write " + stepPath);

    // Verify the file that users will actually open, not only the in-memory
    // generator shape. Fail if STEP import silently drops or bounds the face.
    STEPControl_Reader reader;
    if (reader.ReadFile(stepPath.c_str()) != IFSelect_RetDone || reader.TransferRoots() == 0)
        throw std::runtime_error("STEP re-import failed for " + kind);

    int faceCount = 0;
    int unboundedFaceCount = 0;
    for (TopExp_Explorer it(reader.OneShape(), TopAbs_FACE); it.More(); it.Next()) {
        ++faceCount;
        const TopoDS_Face face = TopoDS::Face(it.Current());
        if (TopExp_Explorer(face, TopAbs_WIRE).More())
            continue;
        double u1, u2, v1, v2;
        BRepTools::UVBounds(face, u1, u2, v1, v2);
        if (Precision::IsInfinite(u1) || Precision::IsInfinite(u2)
                || Precision::IsInfinite(v1) || Precision::IsInfinite(v2))
        {
            ++unboundedFaceCount;
            std::cout << "UV " << kind << ": " << u1 << ' ' << u2 << ' ' << v1 << ' ' << v2 << '\n';
        }
    }
    if (faceCount != 7 || unboundedFaceCount != 1)
        throw std::runtime_error("Expected 7 faces and 1 boundaryless unbounded face after STEP import");

    std::cout << "VERIFIED " << kind << ": faces=" << faceCount
              << " unbounded=" << unboundedFaceCount
              << " step_bytes=" << std::filesystem::file_size(stepPath) << '\n';
}

} // namespace

int main(int argc, char** argv)
{
    try {
        const std::filesystem::path directory = argc > 1 ? argv[1] : "infinite-face-repro";
        std::filesystem::create_directories(directory);
        std::cout << "OCCT " << OCC_VERSION_COMPLETE << '\n';
        const gp_Ax3 axis(gp_Pnt(40., 30., 20.), gp::DZ());
        generate(directory, "cylinder", new Geom_CylindricalSurface(axis, 15.));
        generate(directory, "plane", new Geom_Plane(axis));
        generate(directory, "cone", new Geom_ConicalSurface(axis, 0.3, 15.));
        return 0;
    }
    catch (const Standard_Failure& error) {
        std::cerr << error.GetMessageString() << '\n';
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
