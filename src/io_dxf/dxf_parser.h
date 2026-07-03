/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "dxf_format_common.h"
#include "dxf_format_blocks.h"
#include "dxf_format_entities.h"
#include "dxf_format_header.h"
#include "dxf_format_tables.h"

#include <deque>
#include <functional>
#include <iosfwd>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include <gsl/span>

#include "../base/string_cache.h"

class DxfParser {
public:
    DxfParser();

    bool failed() const { return m_fail; }

    double mm(double value) const;

    bool hasHeaderVariable(std::string_view name) const;
    Dxf_HeaderVariableValue headerVariableValue(std::string_view name) const;

    std::string_view codePage() const { return m_codePage; }

    const Dxf_BLOCK* findBlock(DxfStringRef name) const;
    const Dxf_LAYER* findLayer(DxfStringRef name) const;
    const Dxf_STYLE* findStyle(DxfStringRef name) const;

    void parse(std::istream& stream);

    // NOTE std::getline() doesn't affect std::istream::gcount
    void setGetLinePostCallback(std::function<void(size_t)> fn);
    void setReportErrorCallback(std::function<void(std::string_view)> fn);

    gsl::span<const Dxf_EntityVariant> allEntities() const { return m_entities; }

private:
    std::istream& inputStream();
    void getLine();
    void putLine(std::string_view value);

    void reportError(std::string_view msg);
    void reportError_readInteger(std::string_view context);

    void resolveAcadVer(DxfStringRef strVersion);
    void resolveEncoding(DxfVersion version);

    bool parseLayer();
    bool parseStyle();
    bool parseLine();
    bool parseMText();
    bool parseText();
    bool parseAttrib();
    bool parseArc();
    bool parseCircle();
    bool parseEllipse();
    bool parsePoint();
    bool parseSpline();
    bool parseLwPolyLine();
    bool parsePolyLine();
    bool parseVertex(Dxf_POLYLINE::Vertex* vertex);
    bool parse3dFace();
    bool parseSolid();
    bool parseSection();
    bool parseTable();
    bool parseEndSec();

    bool parseInsert();
    bool parseDimension();
    bool parseBlock();

    void parseHeaderVariable();

    bool parseEntity(
        const std::function<void()>& fnEntityHandler,
        const std::function<void(int)>& fnCodeHandler,
        std::string_view entityTypeName
    );

    template<unsigned XCode = 10, unsigned YCode = 20, unsigned ZCode = 30>
    void handleCoordCode(int n, DxfCoords* coords) const;

    template<unsigned XCode, unsigned YCode, unsigned ZCode>
    void handleVectorCoordCode(int n, std::vector<DxfCoords>* ptrVecCoords) const;

    void handleCommonGroupCode(Dxf_BaseEntity* entity, int n);
    void handleCommonGroupCode(Dxf_BaseGeom2dEntity* entity, int n);

    void handleDxfTextCode(Dxf_TEXT& text, int n);

    void addLayer(Dxf_LAYER&& layer);

    template<typename EntityValue, typename Entity>
    void addEntity(Entity&& entity, std::deque<EntityValue>& entityStore);


    std::istream* m_inputStream = nullptr;

    bool m_fail = false;
    std::string m_str;
    std::string m_unusedLine;
    DxfUnit m_unit = DxfUnit::Millimeters;
    bool m_measurement_inch = false;

    // Version from $ACADVER variable in DXF
    DxfVersion m_version = DxfVersion::RUnknown;
    // Code Page name from $DWGCODEPAGE or null if none/not read yet
    std::string m_codePage;

    std::function<void(size_t)> m_getLinePostCallback;
    std::function<void(std::string_view)> m_reportErrorCallback;

    Mayo::StringCache m_strCache;

    std::deque<Dxf_POINT> m_points;
    std::deque<Dxf_ARC> m_arcs;
    std::deque<Dxf_CIRCLE> m_circles;
    std::deque<Dxf_ELLIPSE> m_ellipses;
    std::deque<Dxf_MTEXT> m_mtexts;
    std::deque<Dxf_TEXT> m_texts;
    std::deque<Dxf_ATTRIB> m_attribs;
    std::deque<Dxf_LINE> m_lines;
    std::deque<Dxf_LWPOLYLINE> m_lwpolylines;
    std::deque<Dxf_POLYLINE> m_polylines;
    std::deque<Dxf_INSERT> m_inserts;
    std::deque<Dxf_3DFACE> m_3dfaces;
    std::deque<Dxf_SOLID> m_solids;
    std::deque<Dxf_SPLINE> m_splines;
    std::vector<Dxf_EntityVariant> m_entities;

    std::unordered_map<std::string, std::function<bool()>> m_mapEntityHandler;

    Dxf_BLOCK* m_currentBlock = nullptr;
    std::deque<Dxf_BLOCK> m_blocks;
    std::deque<Dxf_STYLE> m_styles;
    std::deque<Dxf_LAYER> m_layers;
    std::unordered_map<DxfStringRef, const Dxf_BLOCK*> m_mapBlock;
    std::unordered_map<DxfStringRef, const Dxf_STYLE*> m_mapStyle;
    std::unordered_map<DxfStringRef, const Dxf_LAYER*> m_mapLayer;
    std::unordered_map<DxfStringRef, Dxf_HeaderVariableValue> m_mapHeaderVarValue;
};



//
// Implementation
//

namespace DxfPrivate {

enum class StringToErrorMode { Throw = 0x1, ReturnErrorValue = 0x2 };

double stringToDouble(
    const std::string& line, StringToErrorMode errorMode = StringToErrorMode::Throw
);

int stringToInt(
    const std::string& line, StringToErrorMode errorMode = StringToErrorMode::Throw
);

unsigned stringToUnsigned(
    const std::string& line, StringToErrorMode errorMode = StringToErrorMode::Throw
);

} // namespace DxfPrivate

template<unsigned XCode, unsigned YCode, unsigned ZCode>
void DxfParser::handleCoordCode(int n, DxfCoords* coords) const
{
    switch (n) {
    case XCode:
        coords->x = mm(DxfPrivate::stringToDouble(m_str));
        break;
    case YCode:
        coords->y = mm(DxfPrivate::stringToDouble(m_str));
        break;
    case ZCode:
        coords->z = mm(DxfPrivate::stringToDouble(m_str));
        break;
    }
}

template<unsigned XCode, unsigned YCode, unsigned ZCode>
void DxfParser::handleVectorCoordCode(int n, std::vector<DxfCoords>* ptrVecCoords) const
{
    if (n == XCode || ptrVecCoords->empty())
        ptrVecCoords->push_back({});

    this->handleCoordCode<XCode, YCode, ZCode>(n, &ptrVecCoords->back());
}

template<typename EntityValue, typename Entity>
void DxfParser::addEntity(Entity&& entity, std::deque<EntityValue>& entityStore)
{
    static_assert(std::is_constructible_v<EntityValue, Entity&&>);
    entityStore.push_back(std::forward<Entity>(entity));
    const auto& entityStored = entityStore.back();
    if (m_currentBlock)
        m_currentBlock->entities.push_back(std::cref(entityStored));
    else
        m_entities.push_back(std::cref(entityStored));
}
