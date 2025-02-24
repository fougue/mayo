/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_off_reader.h"

#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/triangulation_annex_data.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/math_utils.h"
#include "../base/mesh_utils.h"
#include "../base/messenger.h"
#include "../base/property_builtins.h"
#include "../base/span.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <Quantity_Color.hxx>
#include <Poly_Triangulation.hxx>
#include <TDataStd_Name.hxx>

#if __cpp_lib_to_chars
#  include <charconv>
#else
#  include <cstdlib>
#endif
#include <array>
#include <fstream>
#include <locale>
#include <string>
#include <type_traits>

namespace Mayo::IO {

struct OffReaderI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OffReaderI18N) };

namespace {

const char* strEnd(std::string_view str)
{
    return str.data() + str.size();
}

std::string_view getWord(const std::string& strLine, std::string::size_type pos = 0)
{
    using IntType = std::string::size_type;
    auto fnIsSpace = [](char ch) {
        return std::isspace(ch, std::locale::classic());
    };

    IntType wordStart = pos;
    for (; wordStart < strLine.size() && fnIsSpace(strLine.at(wordStart)); ++wordStart);

    IntType wordEnd = wordStart;
    for (; wordEnd < strLine.size() && !fnIsSpace(strLine.at(wordEnd)) && strLine.at(wordEnd) != '#'; ++wordEnd);

    return std::string_view{ strLine.c_str() + wordStart, wordEnd - wordStart };
}

template<unsigned N>
std::array<std::string_view, N> getWords(const std::string& strLine, std::string::size_type pos = 0)
{
    std::array<std::string_view, N> arrayWord;
    for (unsigned i = 0; i < N; ++i) {
        const std::string::size_type offset = i > 0 ? strEnd(arrayWord[i - 1]) - strLine.data() : pos;
        std::string_view word = getWord(strLine, offset);
        if (!word.empty())
            arrayWord[i] = word;
        else
            return arrayWord;
    }

    return arrayWord;
}

void getWords(const std::string& strLine, std::vector<std::string_view>& vecOutWord, std::string::size_type pos = 0)
{
    vecOutWord.clear();
    while (true) {
        const std::string::size_type offset = !vecOutWord.empty() ? strEnd(vecOutWord.back()) - strLine.data() : pos;
        std::string_view word = getWord(strLine, offset);
        if (!word.empty())
            vecOutWord.push_back(word);
        else
            return;
    }
}

bool hasEmptyString(Span<const std::string_view> spanStr)
{
    for (std::string_view str : spanStr) {
        if (str.empty())
            return true;
    }

    return false;
}

bool isAnyOf(std::string_view str, std::initializer_list<std::string_view> listCandidates)
{
    for (std::string_view candidate : listCandidates) {
        if (str == candidate)
            return true;
    }

    return false;
}

template<typename T>
T strToNum(std::string_view str)
{
    static_assert(std::is_arithmetic_v<std::remove_cv_t<T>>, "Input template type must be arithmetic type");
    T num = {};
    if (str.empty())
        return num;

#if __cpp_lib_to_chars
    auto result = std::from_chars(str.data(), str.data() + str.size(), num);
    if (result.ec != std::errc()) {
        // TODO Handle error code
        // throw std::runtime_error(std::make_error_code(err).message());
    }
#else
    errno = 0;
    num = std::strtod(str.data(), nullptr);
    if (errno != 0) {
        // TODO Handle error code
        // throw std::runtime_error(std::strerror(errno));
    }
#endif
    return num;
}

void getNonCommentLine(std::istream& istr, std::string& strLine)
{
    std::getline(istr >> std::ws, strLine);
    while (!istr.eof() && !strLine.empty() && strLine.front() == '#')
        std::getline(istr >> std::ws, strLine);
}

std::uint32_t strToColorComponent(std::string_view str)
{
    const double v = strToNum<double>(str);
    return unsigned(v > 1. ? v : v * 255);
}

std::uint32_t toRgbaColor(Span<const std::string_view> spanWord)
{
    const unsigned r = spanWord.size() > 0 ? strToColorComponent(spanWord[0]) : 0;
    const unsigned g = spanWord.size() > 1 ? strToColorComponent(spanWord[1]) : 0;
    const unsigned b = spanWord.size() > 2 ? strToColorComponent(spanWord[2]) : 0;
    const unsigned a = spanWord.size() > 3 ? strToColorComponent(spanWord[3]) : 0;
    const std::uint32_t color =
            ((r << 24)   & 0xFF000000)
            | ((g << 16) & 0x00FF0000)
            | ((b << 8)  & 0x0000FF00)
            | (a         & 0x000000FF);
    return color;
}

} // namespace

bool OffReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    auto fnError = [=](std::string_view strMessage) {
        this->messenger()->emitError(strMessage);
        return false;
    };

    // Reset internal data
    m_baseFilename = filepath.stem();
    m_vecVertex.clear();
    m_vecAllFacetIndex.clear();
    m_vecFacet.clear();

    std::ifstream ifs(filepath);
    if (!ifs.is_open())
        return fnError(OffReaderI18N::textIdTr("Can't open input file"));

    std::string strLine;

    // Consume header keyword
    {
        getNonCommentLine(ifs, strLine);
        if (!ifs.good())
            return fnError(OffReaderI18N::textIdTr("Unexpected end of file"));

        std::string_view headerKeyword = getWord(strLine);
        if (!isAnyOf(headerKeyword, { "OFF", "COFF", "NOFF", "4OFF" }))
            return fnError(OffReaderI18N::textIdTr("Wrong header keyword(should be [C][N][4]OFF"));
    }

    // Consume count of vertices/faces/edges
    int vertexCount = 0;
    int facetCount = 0;
    {
        // Normally vertex/face/edge counts are specified on a dedicated line coming after OFF
        // But for some files they are wrongly specified on the line containing OFF token, eg "OFF 24 12 0"
        const auto arrayStrFirstLine = getWords<3>(strLine);
        if (!arrayStrFirstLine[1].empty() && !arrayStrFirstLine[2].empty()) {
            vertexCount = strToNum<int>(arrayStrFirstLine[1]);
            facetCount = strToNum<int>(arrayStrFirstLine[2]);
        }
        else {
            if (!ifs.good())
                return fnError(OffReaderI18N::textIdTr("Unexpected end of file"));

            getNonCommentLine(ifs, strLine);
            const auto arrayStrCount = getWords<2>(strLine);
            if (hasEmptyString(arrayStrCount))
                return fnError(OffReaderI18N::textIdTr("No vertex or face count"));

            vertexCount = strToNum<int>(arrayStrCount[0]);
            facetCount = strToNum<int>(arrayStrCount[1]);
        }
    }

    // Helper function for progress report
    auto fnUpdateProgress = [=]{
        const auto total = vertexCount + facetCount;
        const auto current = m_vecVertex.size() + m_vecFacet.size();
        if (current % 100 || CppUtils::cmpGreaterEqual(current, total))
            progress->setValue(MathUtils::toPercent(current, 0, total));
    };

    // Consume vertices
    m_vecVertex.reserve(vertexCount);
    while (!ifs.eof() && CppUtils::cmpLess(m_vecVertex.size(), vertexCount)) {
        getNonCommentLine(ifs, strLine);
        const auto arrayStrCoord = getWords<3>(strLine);
        if (hasEmptyString(arrayStrCoord))
            return fnError(OffReaderI18N::textIdTr("No vertex coordinates at current line"));

        Vertex vertex = {};
        vertex.coords.SetX(strToNum<double>(arrayStrCoord[0]));
        vertex.coords.SetY(strToNum<double>(arrayStrCoord[1]));
        vertex.coords.SetZ(strToNum<double>(arrayStrCoord[2]));

        const auto arrayStrColor = getWords<4>(strLine, strEnd(arrayStrCoord.back()) - strLine.data());
        vertex.hasColor = !arrayStrColor.front().empty();
        if (vertex.hasColor)
            vertex.color = toRgbaColor(arrayStrColor);

        m_vecVertex.push_back(std::move(vertex));
        fnUpdateProgress();
    }

    // Consume faces
    m_vecAllFacetIndex.reserve(facetCount * 3);
    m_vecFacet.reserve(facetCount);
    std::vector<std::string_view> vecWord;
    while (!ifs.eof() && CppUtils::cmpLess(m_vecFacet.size(), facetCount)) {
        getNonCommentLine(ifs, strLine);
        getWords(strLine, vecWord);
        const Facet facet = { int(m_vecAllFacetIndex.size()), strToNum<int>(vecWord.front()) };
        if (CppUtils::cmpLess((vecWord.size() + 1), facet.vertexCount))
            return fnError(OffReaderI18N::textIdTr("Inconsistent vertex count of face"));

        for (int i = 0; i < facet.vertexCount; ++i) {
            const int facetVertexId = strToNum<int>(vecWord.at(i + 1));
            m_vecAllFacetIndex.push_back(facetVertexId);
        }

#if 0
        const bool facetHasColor = (vecWord.size() - 1 - facet.vertexCount) >= 3;
        std::uint32_t facetColor = 0;
        if (facetHasColor) {
            const std::size_t colorComponentCount = vecWord.size() - (facet.vertexCount + 1);
            facetColor = toRgbaColor({ &vecWord.at(facet.vertexCount + 1), colorComponentCount });
            for (int i = 0; i < facet.vertexCount; ++i) {
                const int ivertex = m_vecAllFacetIndex.at(facet.startIndexInArray + i);
                Vertex& vertex = m_vecVertex.at(ivertex);
            }
        }
#endif

        m_vecFacet.push_back(std::move(facet));
        fnUpdateProgress();
    }

    return true;
}

TDF_LabelSequence OffReader::transfer(DocumentPtr doc, TaskProgress* progress)
{
    if (m_vecVertex.empty())
        return {};

    TDF_Label entityLabel;
    if (!m_vecAllFacetIndex.empty())
        entityLabel = this->transferMesh(doc, progress);
    else
        entityLabel = this->transferPointCloud(doc, progress);

    if (!entityLabel.IsNull()) {
        TDataStd_Name::Set(entityLabel, filepathTo<TCollection_ExtendedString>(m_baseFilename));
        return CafUtils::makeLabelSequence({ entityLabel });
    }

    return {};
}

TDF_Label OffReader::transferMesh(DocumentPtr doc, TaskProgress* progress)
{
    // Vertex and triangle count
    const int vertexCount = CppUtils::safeStaticCast<int>(m_vecVertex.size());
    int triangleCount = 0;
    for (const Facet& facet : m_vecFacet)
        triangleCount += facet.vertexCount - 2;

    // Create mesh object
    auto mesh = makeOccHandle<Poly_Triangulation>(vertexCount, triangleCount, false/*!hasUvNodes*/);

    // Helper function for progress report
    auto fnUpdateProgress = [=](int current) {
        const auto total = vertexCount + triangleCount;
        if (current % 100 || current >= total)
            progress->setValue(MathUtils::toPercent(current, 0, total));
    };

    // Transfer vertices and prepare vertex colors
    std::vector<Quantity_Color> vecVertexColor;
    vecVertexColor.reserve(m_vecVertex.size());
    for (const Vertex& vertex : m_vecVertex) {
        const auto ivertex = Span_itemIndex(m_vecVertex, vertex);
        MeshUtils::setNode(mesh, ivertex + 1, vertex.coords);
        const std::uint32_t c = vertex.color;
        if (vertex.hasColor) {
            vecVertexColor.push_back(
                        Quantity_Color{
                            ((c & 0xFF000000) >> 24) / 255.f,
                            ((c & 0x00FF0000) >> 16) / 255.f,
                            ((c & 0x0000FF00) >> 8)  / 255.f,
                            TKernelUtils::preferredRgbColorType()
                        });
        }
        else {
            vecVertexColor.push_back(Quantity_NOC_BEIGE);
        }

        fnUpdateProgress(ivertex);
    }

    // Transfer faces
    int iTriangle = 0;
    for (const Facet& facet: m_vecFacet) {
        const int facet0 = m_vecAllFacetIndex.at(facet.startIndexInArray);
        if (facet.vertexCount == 3) {
            const int facet1 = m_vecAllFacetIndex.at(facet.startIndexInArray + 1);
            const int facet2 = m_vecAllFacetIndex.at(facet.startIndexInArray + 2);
            MeshUtils::setTriangle(mesh, iTriangle + 1, { facet0 + 1, facet1 + 1, facet2 + 1 });
            ++iTriangle;
        }
        else if (facet.vertexCount > 3) {
            for (int i = 1; i <= facet.vertexCount - 2; ++i) {
                const int facetN = m_vecAllFacetIndex.at(facet.startIndexInArray + i);
                const int facetM = m_vecAllFacetIndex.at(facet.startIndexInArray + i + 1);
                MeshUtils::setTriangle(mesh, iTriangle + 1, { facet0 + 1, facetN + 1, facetM + 1 });
                ++iTriangle;
            }
        }

        fnUpdateProgress(vertexCount + iTriangle);
    }

    // Insert mesh as a document entity
    const TDF_Label entityLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(entityLabel, BRepUtils::makeFace(mesh)); // IMPORTANT: pure mesh part marker!
    TriangulationAnnexData::Set(entityLabel, std::move(vecVertexColor));
    return entityLabel;
}

TDF_Label OffReader::transferPointCloud(DocumentPtr /*doc*/, TaskProgress* /*progress*/)
{
    return {};
}

} // namespace Mayo::IO
