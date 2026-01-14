/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "dxf_format_common.h"
#include "dxf_format_blocks.h"
#include "dxf_format_entities.h"
#include "dxf_format_header.h"
#include "dxf_format_tables.h"

#include <functional>
#include <istream>
#include <iosfwd>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include <gsl/span>

#include "../base/string_cache.h"

class CDxfRead {
public:
    CDxfRead();
    virtual ~CDxfRead();

    bool failed() const { return m_fail; }

    double mm(double value) const;

    bool hasHeaderVariable(std::string_view name) const;
    Dxf_HeaderVariableValue headerVariableValue(std::string_view name) const;

    const Dxf_BLOCK* findBlock(DxfStringRef name) const;
    const Dxf_LAYER* findLayer(DxfStringRef name) const;
    const Dxf_STYLE* findStyle(DxfStringRef name) const;

    void read(std::istream& stream);

    gsl::span<const Dxf_EntityVariant> allEntities() const { return m_entities; }

protected:
    std::streamsize gcount() const;
    virtual void getLine();
    virtual void ReportError(const std::string& /*msg*/) {}

    virtual bool setSourceEncoding(const std::string& /*codepage*/) { return true; }
    virtual std::string toUtf8(const std::string& strSource) { return strSource; }

private:
    std::istream& inputStream();

    std::istream* m_inputStream = nullptr;

    bool m_fail = false;
    std::string m_str;
    std::string m_unusedLine;
    DxfUnit m_unit = DxfUnit::Millimeters;
    bool m_measurement_inch = false;

    std::streamsize m_gcount = 0;

    void resolveAcadVer(DxfStringRef strVersion);
    void resolveEncoding(DxfVersion version);

    bool ReadLayer();
    bool ReadStyle();
    bool ReadLine();
    bool ReadMText();
    bool ReadText();
    bool ReadArc();
    bool ReadCircle();
    bool ReadEllipse();
    bool ReadPoint();
    bool ReadSpline();
    bool ReadLwPolyLine();
    bool ReadPolyLine();
    bool ReadVertex(Dxf_POLYLINE::Vertex* vertex);
    bool Read3dFace();
    bool ReadSolid();
    bool ReadSection();
    bool ReadTable();
    bool ReadEndSec();

    bool ReadInsert();
    bool ReadDimension();
    bool ReadBlock();

    void readHeaderVariable();

    template<unsigned XCode = 10, unsigned YCode = 20, unsigned ZCode = 30>
    void handleCoordCode(int n, DxfCoords* coords);

    template<unsigned XCode, unsigned YCode, unsigned ZCode>
    void handleVectorCoordCode(int n, std::vector<DxfCoords>* ptrVecCoords);

    void handleCommonGroupCode(Dxf_BaseEntity* entity, int n);
    void handleCommonGroupCode(Dxf_BaseGeom2dEntity* entity, int n);

    void putLine(const std::string& value);

    void ReportError_readInteger(const char* context);

private:
    template<typename T, size_t PoolCapacity = 512>
    struct Storage {
        T& add(const T& object);
        T& add(T&& object);
        const T& back() const;
        void clear();

    private:
        template<typename U>
        T& impl_add(U&& object);

        using Pool = std::vector<T>;
        std::list<Pool> m_pools;
    };

    bool readEntity(
        const std::function<void()>& fnEntityHandler,
        const std::function<void(int)>& fnCodeHandler,
        std::string_view entityTypeName
    );

    template<typename EntityValue, typename Entity>
    void addEntity(Entity&& entity, Storage<EntityValue>& entityStore);

    // Version from $ACADVER variable in DXF
    DxfVersion m_version = DxfVersion::RUnknown;
    // Code Page name from $DWGCODEPAGE or null if none/not read yet
    std::string m_codePage;

    Mayo::StringCache m_strCache;

    Storage<Dxf_POINT> m_points;
    Storage<Dxf_ARC> m_arcs;
    Storage<Dxf_CIRCLE> m_circles;
    Storage<Dxf_ELLIPSE> m_ellipses;
    Storage<Dxf_TEXT> m_texts;
    Storage<Dxf_MTEXT> m_mtexts;
    Storage<Dxf_LINE> m_lines;
    Storage<Dxf_LWPOLYLINE> m_lwpolylines;
    Storage<Dxf_POLYLINE> m_polylines;
    Storage<Dxf_INSERT> m_inserts;
    Storage<Dxf_3DFACE> m_3dfaces;
    Storage<Dxf_SOLID> m_solids;
    Storage<Dxf_SPLINE> m_splines;
    std::vector<Dxf_EntityVariant> m_entities;

    std::unordered_map<std::string, std::function<bool()>> m_mapEntityHandler;

    Dxf_BLOCK* m_currentBlock = nullptr;
    Storage<Dxf_BLOCK> m_blocks;
    Storage<Dxf_STYLE> m_styles;
    Storage<Dxf_LAYER> m_layers;
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
void CDxfRead::handleCoordCode(int n, DxfCoords* coords)
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
void CDxfRead::handleVectorCoordCode(int n, std::vector<DxfCoords>* ptrVecCoords)
{
    if (n == XCode || ptrVecCoords->empty())
        ptrVecCoords->push_back({});

    this->handleCoordCode<XCode, YCode, ZCode>(n, &ptrVecCoords->back());
}

template<typename EntityValue, typename Entity>
void CDxfRead::addEntity(Entity&& entity, Storage<EntityValue>& entityStore)
{
    static_assert(std::is_constructible_v<EntityValue, Entity&&>);
    const auto& entityStored = entityStore.add(std::forward<Entity>(entity));
    if (m_currentBlock)
        m_currentBlock->entities.push_back(std::cref(entityStored));
    else
        m_entities.push_back(std::cref(entityStored));
}

template<typename T, size_t PoolCapacity>
T& CDxfRead::Storage<T, PoolCapacity>::add(const T& object)
{
    return impl_add(object);
}

template<typename T, size_t PoolCapacity>
T& CDxfRead::Storage<T, PoolCapacity>::add(T&& object)
{
    return impl_add(std::move(object));
}

template<typename T, size_t PoolCapacity>
const T& CDxfRead::Storage<T, PoolCapacity>::back() const
{
    const Pool& lastPool = m_pools.back();
    return lastPool.back();
}

template<typename T, size_t PoolCapacity>
void CDxfRead::Storage<T, PoolCapacity>::clear()
{
    return m_pools.clear();
}

template<typename T, size_t PoolCapacity>
template<typename U>
T& CDxfRead::Storage<T, PoolCapacity>::impl_add(U&& object)
{
    if (m_pools.empty() || m_pools.back().capacity() == m_pools.back().size()) {
        Pool pool;
        pool.reserve(PoolCapacity);
        m_pools.push_back(std::move(pool));
    }

    Pool& pool = m_pools.back();
    pool.emplace_back(std::forward<U>(object));
    return pool.back();
}
