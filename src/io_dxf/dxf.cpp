// dxf.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
// modified 2018 wandererfan

// MAYO: file taken from FreeCad/src/Mod/Import/App/dxf.cpp -- commit #55292e9

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
//required by windows for M_PI definition
#  define _USE_MATH_DEFINES
#endif
#include <cassert>
#include <clocale>
#include <cmath>

#include <iomanip>
#include <stdexcept>

#include "../base/filepath.h"
#include "dxf.h"

#include <iostream>

namespace {

template<size_t N1, size_t N2>
void safe_strcpy(char (&dst)[N1], const char (&src)[N2])
{
    strncpy(dst, src, std::min(N1, N2));
}

class ScopedCLocale {
public:
    ScopedCLocale(int category)
        : m_category(category),
          m_savedLocale(std::setlocale(category, nullptr))
    {
        std::setlocale(category, "C");
    }

    ~ScopedCLocale()
    {
        std::setlocale(m_category, m_savedLocale);
    }

private:
    int m_category = 0;
    const char* m_savedLocale = nullptr;
};

double stringToDouble(const std::string& line)
{
    try {
        return std::stod(line);
    } catch (...) {
        throw std::runtime_error("Failed to fetch double value from line:\n" + line);
    }

    return 0.;
}

int stringToInt(const std::string& line)
{
    try {
        return std::stoi(line);
    } catch (...) {
        throw std::runtime_error("Failed to fetch int value from line:\n" + line);
    }

    return 0;
}

unsigned stringToUnsigned(const std::string& line)
{
    try {
        return std::stoul(line);
    } catch (...) {
        throw std::runtime_error("Failed to fetch unsigned int value from line:\n" + line);
    }

    return 0;
}

} // namespace

Base::Vector3d toVector3d(const double* a)
{
    Base::Vector3d result;
    result.x = a[0];
    result.y = a[1];
    result.z = a[2];
    return result;
}

CDxfWrite::CDxfWrite(const char* filepath)
    : m_ofs(filepath, std::ios::out),
    //TODO: these should probably be parameters in config file
    //handles:
    //boilerplate 0 - A00
    //used by dxf.cpp A01 - FFFE
    //ACAD HANDSEED  FFFF

    m_handle(0xA00),                       //room for 2560 handles in boilerplate files
    //m_entityHandle(0x300),               //don't need special ranges for handles
    //m_layerHandle(0x30),
    //m_blockHandle(0x210),
    //m_blkRecordHandle(0x110),
    m_polyOverride(false),
    m_layerName("none")
{
    // start the file
    m_fail = false;
    m_version = 12;

    if (!m_ofs)
        m_fail = true;
    else
        m_ofs.imbue(std::locale::classic());
}

CDxfWrite::~CDxfWrite()
{
}

void CDxfWrite::init()
{
    writeHeaderSection();
    makeBlockRecordTableHead();
    makeBlockSectionHead();
}

//! assemble pieces into output file
void CDxfWrite::endRun()
{
    makeLayerTable();
    makeBlockRecordTableBody();
    
    writeClassesSection();
    writeTablesSection();
    writeBlocksSection();
    writeEntitiesSection();
    writeObjectsSection();

    m_ofs << "  0"         << std::endl;
    m_ofs << "EOF";
}

//***************************
//writeHeaderSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeHeaderSection()
{
    std::stringstream ss;
    //header & version
    m_ofs << "999"      << std::endl;
    m_ofs << ss.str()   << std::endl;

    //static header content
    ss.str("");
    ss.clear();
    ss << "header" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    m_ofs << getPlateFile(fileSpec);
}

//***************************
//writeClassesSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeClassesSection()
{
    if (m_version < 14) {
        return;
    }
    
    //static classes section content
    std::stringstream ss;
    ss << "classes" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    m_ofs << getPlateFile(fileSpec);
}

//***************************
//writeTablesSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeTablesSection()
{
    //static tables section head end content
    std::stringstream ss;
    ss << "tables1" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    m_ofs << getPlateFile(fileSpec);

    m_ofs << m_ssLayer.str();

    //static tables section tail end content
    ss.str("");
    ss.clear();
    ss << "tables2" << m_version << ".rub";
    fileSpec = m_dataDir + ss.str();
    m_ofs << getPlateFile(fileSpec);

    if (m_version > 12) {
        m_ofs << m_ssBlkRecord.str();
        m_ofs << "  0"      << std::endl;
        m_ofs << "ENDTAB"   << std::endl;
    }
    m_ofs << "  0"      << std::endl;
    m_ofs << "ENDSEC"   << std::endl;
}

//***************************
//makeLayerTable
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeLayerTable()
{
    std::string tablehash = getLayerHandle();
    m_ssLayer << "  0"      << std::endl;
    m_ssLayer << "TABLE"    << std::endl;
    m_ssLayer << "  2"      << std::endl;
    m_ssLayer << "LAYER"    << std::endl;
    m_ssLayer << "  5"      << std::endl;
    m_ssLayer << tablehash  << std::endl;
    if (m_version > 12) {
        m_ssLayer << "330"      << std::endl;
        m_ssLayer << 0          << std::endl;
        m_ssLayer << "100"      << std::endl;
        m_ssLayer << "AcDbSymbolTable"   << std::endl;
    }
    m_ssLayer << " 70"      << std::endl;
    m_ssLayer << m_layerList.size() + 1 << std::endl;

    m_ssLayer << "  0"      << std::endl;
    m_ssLayer << "LAYER"    << std::endl;
    m_ssLayer << "  5"      << std::endl;
    m_ssLayer << getLayerHandle()  << std::endl;
    if (m_version > 12) {
        m_ssLayer << "330"      << std::endl;
        m_ssLayer << tablehash  << std::endl;
        m_ssLayer << "100"      << std::endl;
        m_ssLayer << "AcDbSymbolTableRecord"      << std::endl;
        m_ssLayer << "100"      << std::endl;
        m_ssLayer << "AcDbLayerTableRecord"      << std::endl;
    }
    m_ssLayer << "  2"      << std::endl;
    m_ssLayer << "0"        << std::endl;
    m_ssLayer << " 70"      << std::endl;
    m_ssLayer << "   0"     << std::endl;
    m_ssLayer << " 62"      << std::endl;
    m_ssLayer << "   7"     << std::endl;
    m_ssLayer << "  6"      << std::endl;
    m_ssLayer << "CONTINUOUS" << std::endl;

    for (auto& l: m_layerList) {
        m_ssLayer << "  0"      << std::endl;
        m_ssLayer << "LAYER"      << std::endl;
        m_ssLayer << "  5"      << std::endl;
        m_ssLayer << getLayerHandle() << std::endl;
        if (m_version > 12) {
            m_ssLayer << "330"      << std::endl;
            m_ssLayer << tablehash  << std::endl;
            m_ssLayer << "100"      << std::endl;
            m_ssLayer << "AcDbSymbolTableRecord"      << std::endl;
            m_ssLayer << "100"      << std::endl;
            m_ssLayer << "AcDbLayerTableRecord"      << std::endl;
        }
        m_ssLayer << "  2"      << std::endl;
        m_ssLayer << l << std::endl;
        m_ssLayer << " 70"      << std::endl;
        m_ssLayer << "    0"      << std::endl;
        m_ssLayer << " 62"      << std::endl;
        m_ssLayer << "    7"      << std::endl;
        m_ssLayer << "  6"      << std::endl;
        m_ssLayer << "CONTINUOUS"      << std::endl;
    }
    m_ssLayer << "  0"      << std::endl;
    m_ssLayer << "ENDTAB"   << std::endl;
}

//***************************
//makeBlockRecordTableHead
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeBlockRecordTableHead()
{
    if (m_version < 14) {
        return;
    }
    std::string tablehash = getBlkRecordHandle();
    m_saveBlockRecordTableHandle = tablehash;
    m_ssBlkRecord << "  0"      << std::endl;
    m_ssBlkRecord << "TABLE"      << std::endl;
    m_ssBlkRecord << "  2"      << std::endl;
    m_ssBlkRecord << "BLOCK_RECORD"      << std::endl;
    m_ssBlkRecord << "  5"      << std::endl;
    m_ssBlkRecord << tablehash  << std::endl;
    m_ssBlkRecord << "330"      << std::endl;
    m_ssBlkRecord << "0"        << std::endl;
    m_ssBlkRecord << "100"      << std::endl;
    m_ssBlkRecord << "AcDbSymbolTable"      << std::endl;
    m_ssBlkRecord << "  70"      << std::endl;
    m_ssBlkRecord << (m_blockList.size() + 5)   << std::endl;

    m_saveModelSpaceHandle = getBlkRecordHandle();
    m_ssBlkRecord << "  0"      << std::endl;
    m_ssBlkRecord << "BLOCK_RECORD"      << std::endl;
    m_ssBlkRecord << "  5"      << std::endl;
    m_ssBlkRecord << m_saveModelSpaceHandle  << std::endl;
    m_ssBlkRecord << "330"      << std::endl;
    m_ssBlkRecord << tablehash  << std::endl;
    m_ssBlkRecord << "100"      << std::endl;
    m_ssBlkRecord << "AcDbSymbolTableRecord"      << std::endl;
    m_ssBlkRecord << "100"      << std::endl;
    m_ssBlkRecord << "AcDbBlockTableRecord"      << std::endl;
    m_ssBlkRecord << "  2"      << std::endl;
    m_ssBlkRecord << "*MODEL_SPACE"   << std::endl;
    //        m_ssBlkRecord << "  1"      << std::endl;
    //        m_ssBlkRecord << " "        << std::endl;

    m_savePaperSpaceHandle = getBlkRecordHandle();
    m_ssBlkRecord << "  0"      << std::endl;
    m_ssBlkRecord << "BLOCK_RECORD"  << std::endl;
    m_ssBlkRecord << "  5"      << std::endl;
    m_ssBlkRecord << m_savePaperSpaceHandle  << std::endl;
    m_ssBlkRecord << "330"      << std::endl;
    m_ssBlkRecord << tablehash  << std::endl;
    m_ssBlkRecord << "100"      << std::endl;
    m_ssBlkRecord << "AcDbSymbolTableRecord"      << std::endl;
    m_ssBlkRecord << "100"      << std::endl;
    m_ssBlkRecord << "AcDbBlockTableRecord"      << std::endl;
    m_ssBlkRecord << "  2"      << std::endl;
    m_ssBlkRecord << "*PAPER_SPACE"   << std::endl;
    //        m_ssBlkRecord << "  1"      << std::endl;
    //        m_ssBlkRecord << " "        << std::endl;
}

//***************************
//makeBlockRecordTableBody
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeBlockRecordTableBody()
{
    if (m_version < 14) {
        return;
    }
    
    int iBlkRecord = 0;
    for (auto& b: m_blockList) {
        m_ssBlkRecord << "  0"      << std::endl;
        m_ssBlkRecord << "BLOCK_RECORD"      << std::endl;
        m_ssBlkRecord << "  5"      << std::endl;
        m_ssBlkRecord << m_blkRecordList.at(iBlkRecord)      << std::endl;
        m_ssBlkRecord << "330"      << std::endl;
        m_ssBlkRecord << m_saveBlockRecordTableHandle  << std::endl;
        m_ssBlkRecord << "100"      << std::endl;
        m_ssBlkRecord << "AcDbSymbolTableRecord"      << std::endl;
        m_ssBlkRecord << "100"      << std::endl;
        m_ssBlkRecord << "AcDbBlockTableRecord"      << std::endl;
        m_ssBlkRecord << "  2"      << std::endl;
        m_ssBlkRecord << b          << std::endl;
        //        m_ssBlkRecord << " 70"      << std::endl;
        //        m_ssBlkRecord << "    0"      << std::endl;
        iBlkRecord++;
    }
}

//***************************
//makeBlockSectionHead
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::makeBlockSectionHead()
{
    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "SECTION"      << std::endl;
    m_ssBlock << "  2"          << std::endl;
    m_ssBlock << "BLOCKS"       << std::endl;
    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "BLOCK"        << std::endl;
    m_ssBlock << "  5"          << std::endl;
    m_currentBlock = getBlockHandle();
    m_ssBlock << m_currentBlock << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"      << std::endl;
        m_ssBlock << m_saveModelSpaceHandle << std::endl;
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbEntity"      << std::endl;
    }
    m_ssBlock << "  8"          << std::endl;
    m_ssBlock << "0"            << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbBlockBegin"  << std::endl;
    }
    m_ssBlock << "  2"          << std::endl;
    m_ssBlock << "*MODEL_SPACE" << std::endl;
    m_ssBlock << " 70"          << std::endl;
    m_ssBlock << "   0"         << std::endl;
    m_ssBlock << " 10"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << " 20"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << " 30"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << "  3"          << std::endl;
    m_ssBlock << "*MODEL_SPACE" << std::endl;
    m_ssBlock << "  1"          << std::endl;
    m_ssBlock << " "            << std::endl;
    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "ENDBLK"       << std::endl;
    m_ssBlock << "  5"          << std::endl;
    m_ssBlock << getBlockHandle()   << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"      << std::endl;
        m_ssBlock << m_saveModelSpaceHandle << std::endl;
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbEntity"  << std::endl;
    }
    m_ssBlock << "  8"          << std::endl;
    m_ssBlock << "0"            << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbBlockEnd"      << std::endl;
    }

    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "BLOCK"        << std::endl;
    m_ssBlock << "  5"          << std::endl;
    m_currentBlock = getBlockHandle();
    m_ssBlock << m_currentBlock << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"      << std::endl;
        m_ssBlock << m_savePaperSpaceHandle << std::endl;
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbEntity"      << std::endl;
        m_ssBlock << " 67"          << std::endl;
        m_ssBlock << "1"            << std::endl;
    }
    m_ssBlock << "  8"          << std::endl;
    m_ssBlock << "0"            << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbBlockBegin"  << std::endl;
    }
    m_ssBlock << "  2"          << std::endl;
    m_ssBlock << "*PAPER_SPACE" << std::endl;
    m_ssBlock << " 70"          << std::endl;
    m_ssBlock << "   0"         << std::endl;
    m_ssBlock << " 10"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << " 20"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << " 30"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << "  3"          << std::endl;
    m_ssBlock << "*PAPER_SPACE" << std::endl;
    m_ssBlock << "  1"          << std::endl;
    m_ssBlock << " "            << std::endl;
    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "ENDBLK"       << std::endl;
    m_ssBlock << "  5"          << std::endl;
    m_ssBlock << getBlockHandle()   << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"      << std::endl;
        m_ssBlock << m_savePaperSpaceHandle << std::endl;
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbEntity"      << std::endl;
        m_ssBlock << " 67"      << std::endl;      //paper_space flag
        m_ssBlock << "    1"    << std::endl;
    }
    m_ssBlock << "  8"          << std::endl;
    m_ssBlock << "0"            << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbBlockEnd" << std::endl;
    }
}

std::string CDxfWrite::getPlateFile(const std::string& fileSpec)
{
    std::stringstream outString;
    const Mayo::FilePath fpath(fileSpec);
    if (!Mayo::filepathExists(fpath)) { // TODO Check read permissions
        //Base::Console().Message("dxf unable to open %s!\n",fileSpec.c_str());
    } else {
        std::string line;
        std::ifstream inFile (fpath);

        while (!inFile.eof()) {
            std::getline(inFile,line);
            if (!inFile.eof()) {
                outString << line << '\n';
            }
        }
    }
    return outString.str();
}

std::string CDxfWrite::getHandle()
{
    m_handle++;
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    ss << m_handle;
    return ss.str();
}

std::string CDxfWrite::getEntityHandle()
{
    return getHandle();
    //    m_entityHandle++;
    //    std::stringstream ss;
    //    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    //    ss << m_entityHandle;
    //    return ss.str();
}

std::string CDxfWrite::getLayerHandle()
{
    return getHandle();
    //    m_layerHandle++;
    //    std::stringstream ss;
    //    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    //    ss << m_layerHandle;
    //    return ss.str();
}

std::string CDxfWrite::getBlockHandle()
{
    return getHandle();
    //    m_blockHandle++;
    //    std::stringstream ss;
    //    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    //    ss << m_blockHandle;
    //    return ss.str();
}

std::string CDxfWrite::getBlkRecordHandle()
{
    return getHandle();
    //    m_blkRecordHandle++;
    //    std::stringstream ss;
    //    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    //    ss << m_blkRecordHandle;
    //    return ss.str();
}

void CDxfWrite::addBlockName(std::string b, std::string h) 
{
    m_blockList.push_back(b);
    m_blkRecordList.push_back(h);
}

void CDxfWrite::setLayerName(std::string s)
{
    m_layerName = s;
    m_layerList.push_back(s);
}

void CDxfWrite::writeLine(const double* s, const double* e)
{
    putLine(toVector3d(s),toVector3d(e),m_ssEntity, getEntityHandle(), m_saveModelSpaceHandle);
}

void CDxfWrite::putLine(const Base::Vector3d& s,
                        const Base::Vector3d& e,
                        std::ostringstream& outStream,
                        const std::string& handle,
                        const std::string& ownerHandle)
{
    outStream << "  0"       << std::endl;
    outStream << "LINE"      << std::endl;
    outStream << "  5"       << std::endl;
    outStream << handle      << std::endl;
    if (m_version > 12) {
        outStream << "330"      << std::endl;
        outStream << ownerHandle  << std::endl;
        outStream << "100"      << std::endl;
        outStream << "AcDbEntity"      << std::endl;
    }
    outStream << "  8"       << std::endl;    // Group code for layer name
    outStream << getLayerName()  << std::endl;    // Layer number
    if (m_version > 12) {
        outStream << "100"      << std::endl;
        outStream << "AcDbLine" << std::endl;
    }
    outStream << " 10"       << std::endl;    // Start point of line
    outStream << s.x         << std::endl;    // X in WCS coordinates
    outStream << " 20"       << std::endl;
    outStream << s.y         << std::endl;    // Y in WCS coordinates
    outStream << " 30"       << std::endl;
    outStream << s.z         << std::endl;    // Z in WCS coordinates
    outStream << " 11"       << std::endl;    // End point of line
    outStream << e.x         << std::endl;    // X in WCS coordinates
    outStream << " 21"       << std::endl;
    outStream << e.y         << std::endl;    // Y in WCS coordinates
    outStream << " 31"       << std::endl;
    outStream << e.z         << std::endl;    // Z in WCS coordinates
}


//***************************
//writeLWPolyLine  (Note: LWPolyline might not be supported in R12
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeLWPolyLine(const LWPolyDataOut &pd)
{
    m_ssEntity << "  0"               << std::endl;
    m_ssEntity << "LWPOLYLINE"     << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    if (m_version > 12) {
        m_ssEntity << "100"            << std::endl;    //100 groups are not part of R12
        m_ssEntity << "AcDbPolyline"   << std::endl;
    }
    m_ssEntity << "  8"            << std::endl;    // Group code for layer name
    m_ssEntity << getLayerName()   << std::endl;    // Layer name
    m_ssEntity << " 90"            << std::endl;
    m_ssEntity << pd.nVert         << std::endl;    // number of vertices
    m_ssEntity << " 70"            << std::endl;
    m_ssEntity << pd.Flag          << std::endl;
    m_ssEntity << " 43"            << std::endl;
    m_ssEntity << "0"              << std::endl;    //Constant width opt
    //    m_ssEntity << pd.Width         << std::endl;    //Constant width opt
    //    m_ssEntity << " 38"            << std::endl;
    //    m_ssEntity << pd.Elev          << std::endl;    // Elevation
    //    m_ssEntity << " 39"            << std::endl;
    //    m_ssEntity << pd.Thick         << std::endl;    // Thickness
    for (auto& p: pd.Verts) {
        m_ssEntity << " 10"        << std::endl;    // Vertices
        m_ssEntity << p.x          << std::endl;
        m_ssEntity << " 20"        << std::endl;
        m_ssEntity << p.y          << std::endl;
    } 
    for (auto& s: pd.StartWidth) {
        m_ssEntity << " 40"        << std::endl;
        m_ssEntity << s            << std::endl;    // Start Width
    }
    for (auto& e: pd.EndWidth) {
        m_ssEntity << " 41"        << std::endl;
        m_ssEntity << e            << std::endl;    // End Width
    }
    for (auto& b: pd.Bulge) {                // Bulge
        m_ssEntity << " 42"        << std::endl;
        m_ssEntity << b            << std::endl;
    }
    //    m_ssEntity << "210"            << std::endl;    //Extrusion dir
    //    m_ssEntity << pd.Extr.x        << std::endl;
    //    m_ssEntity << "220"            << std::endl;
    //    m_ssEntity << pd.Extr.y        << std::endl;
    //    m_ssEntity << "230"            << std::endl;
    //    m_ssEntity << pd.Extr.z        << std::endl;
}

//***************************
//writePolyline
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writePolyline(const LWPolyDataOut &pd)
{
    m_ssEntity << "  0"            << std::endl;
    m_ssEntity << "POLYLINE"       << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"            << std::endl;
    m_ssEntity << getLayerName()       << std::endl;    // Layer name
    if (m_version > 12) {
        m_ssEntity << "100"            << std::endl;    //100 groups are not part of R12
        m_ssEntity << "AcDbPolyline"   << std::endl;
    }
    m_ssEntity << " 66"            << std::endl;
    m_ssEntity << "     1"         << std::endl;    // vertices follow
    m_ssEntity << " 10"            << std::endl;
    m_ssEntity << "0.0"            << std::endl;
    m_ssEntity << " 20"            << std::endl;
    m_ssEntity << "0.0"            << std::endl;
    m_ssEntity << " 30"            << std::endl;
    m_ssEntity << "0.0"            << std::endl;
    m_ssEntity << " 70"            << std::endl;
    m_ssEntity << "0"              << std::endl;
    for (auto& p: pd.Verts) {
        m_ssEntity << "  0"        << std::endl;
        m_ssEntity << "VERTEX"     << std::endl;
        m_ssEntity << "  5"      << std::endl;
        m_ssEntity << getEntityHandle() << std::endl;
        m_ssEntity << "  8"        << std::endl;
        m_ssEntity << getLayerName()   << std::endl;
        m_ssEntity << " 10"        << std::endl;
        m_ssEntity << p.x          << std::endl;
        m_ssEntity << " 20"        << std::endl;
        m_ssEntity << p.y          << std::endl;
        m_ssEntity << " 30"        << std::endl;
        m_ssEntity << "0.0"        << std::endl;
    } 
    m_ssEntity << "  0"            << std::endl;
    m_ssEntity << "SEQEND"         << std::endl;
    m_ssEntity << "  5"            << std::endl;
    m_ssEntity << getEntityHandle()      << std::endl;
    m_ssEntity << "  8"            << std::endl;
    m_ssEntity << getLayerName()       << std::endl;
}

void CDxfWrite::writePoint(const double* s)
{
    m_ssEntity << "  0"            << std::endl;
    m_ssEntity << "POINT"          << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"            << std::endl;    // Group code for layer name
    m_ssEntity << getLayerName()       << std::endl;    // Layer name
    if (m_version > 12) {
        m_ssEntity << "100"       << std::endl;
        m_ssEntity << "AcDbPoint" << std::endl;
    }
    m_ssEntity << " 10"            << std::endl;
    m_ssEntity << s[0]             << std::endl;    // X in WCS coordinates
    m_ssEntity << " 20"            << std::endl;
    m_ssEntity << s[1]             << std::endl;    // Y in WCS coordinates
    m_ssEntity << " 30"            << std::endl;
    m_ssEntity << s[2]             << std::endl;    // Z in WCS coordinates
}

void CDxfWrite::writeArc(const double* s, const double* e, const double* c, bool dir)
{
    double ax = s[0] - c[0];
    double ay = s[1] - c[1];
    double bx = e[0] - c[0];
    double by = e[1] - c[1];

    double start_angle = atan2(ay, ax) * 180/M_PI;
    double end_angle = atan2(by, bx) * 180/M_PI;
    double radius = sqrt(ax*ax + ay*ay);
    if (!dir){
        double temp = start_angle;
        start_angle = end_angle;
        end_angle = temp;
    }
    m_ssEntity << "  0"       << std::endl;
    m_ssEntity << "ARC"       << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"       << std::endl;    // Group code for layer name
    m_ssEntity << getLayerName()  << std::endl;    // Layer number
    //    m_ssEntity << " 62"          << std::endl;
    //    m_ssEntity << "     0"       << std::endl;
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbCircle"   << std::endl;
    }
    m_ssEntity << " 10"       << std::endl;    // Centre X
    m_ssEntity << c[0]        << std::endl;    // X in WCS coordinates
    m_ssEntity << " 20"       << std::endl;
    m_ssEntity << c[1]        << std::endl;    // Y in WCS coordinates
    m_ssEntity << " 30"       << std::endl;
    m_ssEntity << c[2]        << std::endl;    // Z in WCS coordinates
    m_ssEntity << " 40"       << std::endl;    //
    m_ssEntity << radius      << std::endl;    // Radius

    if (m_version > 12) {
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbArc" << std::endl;
    }
    m_ssEntity << " 50"       << std::endl;
    m_ssEntity << start_angle << std::endl;    // Start angle
    m_ssEntity << " 51"       << std::endl;
    m_ssEntity << end_angle   << std::endl;    // End angle
}

void CDxfWrite::writeCircle(const double* c, double radius)
{
    m_ssEntity << "  0"       << std::endl;
    m_ssEntity << "CIRCLE"    << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"       << std::endl;    // Group code for layer name
    m_ssEntity << getLayerName()  << std::endl;    // Layer number
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbCircle"   << std::endl;
    }
    m_ssEntity << " 10"       << std::endl;    // Centre X
    m_ssEntity << c[0]        << std::endl;    // X in WCS coordinates
    m_ssEntity << " 20"       << std::endl;
    m_ssEntity << c[1]        << std::endl;    // Y in WCS coordinates
    //    m_ssEntity << " 30"       << std::endl;
    //    m_ssEntity << c[2]        << std::endl;    // Z in WCS coordinates
    m_ssEntity << " 40"       << std::endl;    //
    m_ssEntity << radius      << std::endl;    // Radius
}

void CDxfWrite::writeEllipse(const double* c,
                             double major_radius,
                             double minor_radius,
                             double rotation,
                             double start_angle,
                             double end_angle,
                             bool endIsCW)
{
    double m[3];
    m[2]=0;
    m[0] = major_radius * sin(rotation);
    m[1] = major_radius * cos(rotation);

    double ratio = minor_radius/major_radius;

    if (!endIsCW){                          //end is NOT CW from start
        double temp = start_angle;
        start_angle = end_angle;
        end_angle = temp;
    }
    m_ssEntity << "  0"       << std::endl;
    m_ssEntity << "ELLIPSE"   << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"       << std::endl;    // Group code for layer name
    m_ssEntity << getLayerName()  << std::endl;    // Layer number
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbEllipse"   << std::endl;
    }
    m_ssEntity << " 10"       << std::endl;    // Centre X
    m_ssEntity << c[0]        << std::endl;    // X in WCS coordinates
    m_ssEntity << " 20"       << std::endl;
    m_ssEntity << c[1]        << std::endl;    // Y in WCS coordinates
    m_ssEntity << " 30"       << std::endl;
    m_ssEntity << c[2]        << std::endl;    // Z in WCS coordinates
    m_ssEntity << " 11"       << std::endl;    //
    m_ssEntity << m[0]        << std::endl;    // Major X
    m_ssEntity << " 21"       << std::endl;
    m_ssEntity << m[1]        << std::endl;    // Major Y
    m_ssEntity << " 31"       << std::endl;
    m_ssEntity << m[2]        << std::endl;    // Major Z
    m_ssEntity << " 40"       << std::endl;    //
    m_ssEntity << ratio       << std::endl;    // Ratio
    //    m_ssEntity << "210"       << std::endl;    //extrusion dir??
    //    m_ssEntity << "0"         << std::endl;
    //    m_ssEntity << "220"       << std::endl;
    //    m_ssEntity << "0"         << std::endl;
    //    m_ssEntity << "230"       << std::endl;
    //    m_ssEntity << "1"         << std::endl;
    m_ssEntity << " 41"       << std::endl;
    m_ssEntity << start_angle << std::endl;    // Start angle (radians [0..2pi])
    m_ssEntity << " 42"       << std::endl;
    m_ssEntity << end_angle   << std::endl;    // End angle
}

//***************************
//writeSpline
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeSpline(const SplineDataOut &sd)
{
    m_ssEntity << "  0"          << std::endl;
    m_ssEntity << "SPLINE"       << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"          << std::endl;    // Group code for layer name
    m_ssEntity << getLayerName()     << std::endl;    // Layer name
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbSpline"   << std::endl;
    }
    m_ssEntity << "210"          << std::endl;
    m_ssEntity << "0"            << std::endl;
    m_ssEntity << "220"          << std::endl;
    m_ssEntity << "0"            << std::endl;
    m_ssEntity << "230"          << std::endl;
    m_ssEntity << "1"            << std::endl;

    m_ssEntity << " 70"          << std::endl;
    m_ssEntity << sd.flag        << std::endl;      //flags
    m_ssEntity << " 71"          << std::endl;
    m_ssEntity << sd.degree      << std::endl;
    m_ssEntity << " 72"          << std::endl;
    m_ssEntity << sd.knots       << std::endl;
    m_ssEntity << " 73"          << std::endl;
    m_ssEntity << sd.control_points   << std::endl;
    m_ssEntity << " 74"          << std::endl;
    m_ssEntity << 0              << std::endl;

    //    m_ssEntity << " 12"          << std::endl;
    //    m_ssEntity << sd.starttan.x  << std::endl;
    //    m_ssEntity << " 22"          << std::endl;
    //    m_ssEntity << sd.starttan.y  << std::endl;
    //    m_ssEntity << " 32"          << std::endl;
    //    m_ssEntity << sd.starttan.z  << std::endl;
    //    m_ssEntity << " 13"          << std::endl;
    //    m_ssEntity << sd.endtan.x    << std::endl;
    //    m_ssEntity << " 23"          << std::endl;
    //    m_ssEntity << sd.endtan.y    << std::endl;
    //    m_ssEntity << " 33"          << std::endl;
    //    m_ssEntity << sd.endtan.z    << std::endl;

    for (auto& k: sd.knot) {
        m_ssEntity << " 40"      << std::endl;
        m_ssEntity << k          << std::endl;
    }

    for (auto& w : sd.weight) {
        m_ssEntity << " 41"      << std::endl;
        m_ssEntity << w          << std::endl;
    }

    for (auto& c: sd.control) {
        m_ssEntity << " 10"      << std::endl;
        m_ssEntity << c.x        << std::endl;    // X in WCS coordinates
        m_ssEntity << " 20"      << std::endl;
        m_ssEntity << c.y        << std::endl;    // Y in WCS coordinates
        m_ssEntity << " 30"      << std::endl;
        m_ssEntity << c.z        << std::endl;    // Z in WCS coordinates
    }
    for (auto& f: sd.fit) {
        m_ssEntity << " 11"      << std::endl;
        m_ssEntity << f.x        << std::endl;    // X in WCS coordinates
        m_ssEntity << " 21"      << std::endl;
        m_ssEntity << f.y        << std::endl;    // Y in WCS coordinates
        m_ssEntity << " 31"      << std::endl;
        m_ssEntity << f.z        << std::endl;    // Z in WCS coordinates
    }
}

//***************************
//writeVertex
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeVertex(double x, double y, double z)
{
    m_ssEntity << "  0"          << std::endl;
    m_ssEntity << "VERTEX"       << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"          << std::endl;
    m_ssEntity << getLayerName()     << std::endl;
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbVertex"   << std::endl;
    }
    m_ssEntity << " 10"          << std::endl;
    m_ssEntity << x              << std::endl;
    m_ssEntity << " 20"          << std::endl;
    m_ssEntity << y              << std::endl;
    m_ssEntity << " 30"          << std::endl;
    m_ssEntity << z              << std::endl;
    m_ssEntity << " 70"          << std::endl;
    m_ssEntity << 0              << std::endl;
}

void CDxfWrite::writeText(const char* text,
                          const double* location1,
                          const double* location2,
                          const double height,
                          const int horizJust)
{
    putText(text,
            toVector3d(location1),
            toVector3d(location2),
            height,
            horizJust,
            m_ssEntity,
            getEntityHandle(),
            m_saveModelSpaceHandle);
}                                     

//***************************
//putText
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::putText(const char* text,
                        const Base::Vector3d& location1,
                        const Base::Vector3d& location2,
                        const double height,
                        const int horizJust,
                        std::ostringstream& outStream,
                        const std::string& handle,
                        const std::string& ownerHandle)
{
    (void) location2;

    outStream << "  0"          << std::endl;
    outStream << "TEXT"         << std::endl;
    outStream << "  5"      << std::endl;
    outStream << handle << std::endl;
    if (m_version > 12) {
        outStream << "330"      << std::endl;
        outStream << ownerHandle  << std::endl;
        outStream << "100"      << std::endl;
        outStream << "AcDbEntity"      << std::endl;
    }
    outStream << "  8"          << std::endl;
    outStream << getLayerName()     << std::endl;
    if (m_version > 12) {
        outStream << "100"          << std::endl;
        outStream << "AcDbText"     << std::endl;
    }
    //    outStream << " 39"          << std::endl;
    //    outStream << 0              << std::endl;     //thickness
    outStream << " 10"          << std::endl;     //first alignment point
    outStream << location1.x    << std::endl;
    outStream << " 20"          << std::endl;
    outStream << location1.y    << std::endl;
    outStream << " 30"          << std::endl;
    outStream << location1.z    << std::endl;
    outStream << " 40"          << std::endl;
    outStream << height         << std::endl;
    outStream << "  1"          << std::endl;
    outStream << text           << std::endl;
    //    outStream << " 50"          << std::endl;
    //    outStream << 0              << std::endl;    //rotation
    //    outStream << " 41"          << std::endl;
    //    outStream << 1              << std::endl;
    //    outStream << " 51"          << std::endl;
    //    outStream << 0              << std::endl;

    outStream << "  7"          << std::endl;
    outStream << "STANDARD"     << std::endl;    //style
    //    outStream << " 71"          << std::endl;  //default
    //    outStream << "0"            << std::endl;
    outStream << " 72"          << std::endl;
    outStream << horizJust      << std::endl;
    ////    outStream << " 73"          << std::endl;
    ////    outStream << "0"            << std::endl;
    outStream << " 11"          << std::endl;    //second alignment point
    outStream << location2.x    << std::endl;
    outStream << " 21"          << std::endl;
    outStream << location2.y    << std::endl;
    outStream << " 31"          << std::endl;
    outStream << location2.z    << std::endl;
    //    outStream << "210"          << std::endl;
    //    outStream << "0"            << std::endl;
    //    outStream << "220"          << std::endl;
    //    outStream << "0"            << std::endl;
    //    outStream << "230"          << std::endl;
    //    outStream << "1"            << std::endl;
    if (m_version > 12) {
        outStream << "100"          << std::endl;
        outStream << "AcDbText"     << std::endl;
    }
    
}

void CDxfWrite::putArrow(const Base::Vector3d& arrowPos,
                         const Base::Vector3d& barb1Pos,
                         const Base::Vector3d& barb2Pos,
                         std::ostringstream& outStream,
                         const std::string& handle,
                         const std::string& ownerHandle)
{
    outStream << "  0"          << std::endl;
    outStream << "SOLID"        << std::endl;
    outStream << "  5"          << std::endl;
    outStream << handle         << std::endl;
    if (m_version > 12) {
        outStream << "330"      << std::endl;
        outStream << ownerHandle << std::endl;
        outStream << "100"      << std::endl;
        outStream << "AcDbEntity"      << std::endl;
    }
    outStream << "  8"          << std::endl;
    outStream << "0"            << std::endl;
    outStream << " 62"          << std::endl;
    outStream << "     0"       << std::endl;
    if (m_version > 12) {
        outStream << "100"      << std::endl;
        outStream << "AcDbTrace" << std::endl;
    }
    outStream << " 10"          << std::endl;
    outStream << barb1Pos.x     << std::endl;
    outStream << " 20"          << std::endl;
    outStream << barb1Pos.y     << std::endl;
    outStream << " 30"          << std::endl;
    outStream << barb1Pos.z     << std::endl;
    outStream << " 11"          << std::endl;
    outStream << barb2Pos.x     << std::endl;
    outStream << " 21"          << std::endl;
    outStream << barb2Pos.y     << std::endl;
    outStream << " 31"          << std::endl;
    outStream << barb2Pos.z     << std::endl;
    outStream << " 12"          << std::endl;
    outStream << arrowPos.x     << std::endl;
    outStream << " 22"          << std::endl;
    outStream << arrowPos.y     << std::endl;
    outStream << " 32"          << std::endl;
    outStream << arrowPos.z     << std::endl;
    outStream << " 13"          << std::endl;
    outStream << arrowPos.x     << std::endl;
    outStream << " 23"          << std::endl;
    outStream << arrowPos.y     << std::endl;
    outStream << " 33"          << std::endl;
    outStream << arrowPos.z     << std::endl;
}

//***************************
//writeLinearDim
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
#define ALIGNED 0
#define HORIZONTAL 1
#define VERTICAL 2
void CDxfWrite::writeLinearDim(const double* textMidPoint,
                               const double* lineDefPoint,
                               const double* extLine1,
                               const double* extLine2,
                               const char* dimText,
                               int type)
{
    m_ssEntity << "  0"          << std::endl;
    m_ssEntity << "DIMENSION"    << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"          << std::endl;
    m_ssEntity << getLayerName()     << std::endl;
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbDimension"     << std::endl;
    }
    m_ssEntity << "  2"          << std::endl;
    m_ssEntity << "*" << getLayerName()     << std::endl;     // blockName
    m_ssEntity << " 10"          << std::endl;     //dimension line definition point
    m_ssEntity << lineDefPoint[0]    << std::endl;
    m_ssEntity << " 20"          << std::endl;
    m_ssEntity << lineDefPoint[1]    << std::endl;
    m_ssEntity << " 30"          << std::endl;
    m_ssEntity << lineDefPoint[2]    << std::endl;
    m_ssEntity << " 11"          << std::endl;     //text mid point
    m_ssEntity << textMidPoint[0]    << std::endl;
    m_ssEntity << " 21"          << std::endl;
    m_ssEntity << textMidPoint[1]    << std::endl;
    m_ssEntity << " 31"          << std::endl;
    m_ssEntity << textMidPoint[2]    << std::endl;
    if (type == ALIGNED) {
        m_ssEntity << " 70"          << std::endl;
        m_ssEntity << 1              << std::endl;    // dimType1 = Aligned
    }
    if ( (type == HORIZONTAL) ||
        (type == VERTICAL) ) {
        m_ssEntity << " 70"          << std::endl;
        m_ssEntity << 32             << std::endl;  // dimType0 = Aligned + 32 (bit for unique block)?
    }
    //    m_ssEntity << " 71"          << std::endl;    // not R12
    //    m_ssEntity << 1              << std::endl;    // attachPoint ??1 = topleft
    m_ssEntity << "  1"          << std::endl;
    m_ssEntity << dimText        << std::endl;
    m_ssEntity << "  3"          << std::endl;
    m_ssEntity << "STANDARD"     << std::endl;    //style
    //linear dims
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbAlignedDimension"     << std::endl;
    }
    m_ssEntity << " 13"          << std::endl;
    m_ssEntity << extLine1[0]    << std::endl;
    m_ssEntity << " 23"          << std::endl;
    m_ssEntity << extLine1[1]    << std::endl;
    m_ssEntity << " 33"          << std::endl;
    m_ssEntity << extLine1[2]    << std::endl;
    m_ssEntity << " 14"          << std::endl;
    m_ssEntity << extLine2[0]    << std::endl;
    m_ssEntity << " 24"          << std::endl;
    m_ssEntity << extLine2[1]    << std::endl;
    m_ssEntity << " 34"          << std::endl;
    m_ssEntity << extLine2[2]    << std::endl;
    if (m_version > 12) {
        if (type == VERTICAL) {
            m_ssEntity << " 50"          << std::endl;
            m_ssEntity << "90"     << std::endl;
        }
        if ( (type == HORIZONTAL) || (type == VERTICAL) ) {
            m_ssEntity << "100"          << std::endl;
            m_ssEntity << "AcDbRotatedDimension"     << std::endl;
        }
    }

    writeDimBlockPreamble();
    writeLinearDimBlock(textMidPoint, lineDefPoint, extLine1, extLine2, dimText, type);
    writeBlockTrailer();
}

//***************************
//writeAngularDim
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeAngularDim(const double* textMidPoint,
                                const double* lineDefPoint,
                                const double* startExt1,
                                const double* endExt1,
                                const double* startExt2,
                                const double* endExt2,
                                const char* dimText)
{
    m_ssEntity << "  0"          << std::endl;
    m_ssEntity << "DIMENSION"    << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"          << std::endl;
    m_ssEntity << getLayerName()     << std::endl;
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbDimension"     << std::endl;
    }
    m_ssEntity << "  2"          << std::endl;
    m_ssEntity << "*" << getLayerName()     << std::endl;     // blockName

    m_ssEntity << " 10"          << std::endl;
    m_ssEntity << endExt2[0]     << std::endl;
    m_ssEntity << " 20"          << std::endl;
    m_ssEntity << endExt2[1]     << std::endl;
    m_ssEntity << " 30"          << std::endl;
    m_ssEntity << endExt2[2]     << std::endl;

    m_ssEntity << " 11"          << std::endl;
    m_ssEntity << textMidPoint[0]  << std::endl;
    m_ssEntity << " 21"          << std::endl;
    m_ssEntity << textMidPoint[1]  << std::endl;
    m_ssEntity << " 31"          << std::endl;
    m_ssEntity << textMidPoint[2]  << std::endl;

    m_ssEntity << " 70"          << std::endl;
    m_ssEntity << 2             << std::endl;    // dimType 2 = Angular  5 = Angular 3 point
        // +32 for block?? (not R12)
    //    m_ssEntity << " 71"          << std::endl;    // not R12?  not required?
    //    m_ssEntity << 5              << std::endl;    // attachPoint 5 = middle
    m_ssEntity << "  1"          << std::endl;
    m_ssEntity << dimText        << std::endl;
    m_ssEntity << "  3"          << std::endl;
    m_ssEntity << "STANDARD"     << std::endl;    //style
    //angular dims
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDb2LineAngularDimension"     << std::endl;
    }
    m_ssEntity << " 13"           << std::endl;
    m_ssEntity << startExt1[0]    << std::endl;
    m_ssEntity << " 23"           << std::endl;
    m_ssEntity << startExt1[1]    << std::endl;
    m_ssEntity << " 33"           << std::endl;
    m_ssEntity << startExt1[2]    << std::endl;

    m_ssEntity << " 14"           << std::endl;
    m_ssEntity << endExt1[0]      << std::endl;
    m_ssEntity << " 24"           << std::endl;
    m_ssEntity << endExt1[1]      << std::endl;
    m_ssEntity << " 34"           << std::endl;
    m_ssEntity << endExt1[2]      << std::endl;

    m_ssEntity << " 15"           << std::endl;
    m_ssEntity << startExt2[0]    << std::endl;
    m_ssEntity << " 25"           << std::endl;
    m_ssEntity << startExt2[1]    << std::endl;
    m_ssEntity << " 35"           << std::endl;
    m_ssEntity << startExt2[2]    << std::endl;

    m_ssEntity << " 16"           << std::endl;
    m_ssEntity << lineDefPoint[0] << std::endl;
    m_ssEntity << " 26"           << std::endl;
    m_ssEntity << lineDefPoint[1] << std::endl;
    m_ssEntity << " 36"           << std::endl;
    m_ssEntity << lineDefPoint[2] << std::endl;
    writeDimBlockPreamble();
    writeAngularDimBlock(textMidPoint,
                         lineDefPoint,
                         startExt1,
                         endExt1,
                         startExt2,
                         endExt2,
                         dimText);
    writeBlockTrailer();
}

//***************************
//writeRadialDim
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeRadialDim(const double* centerPoint,
                               const double* textMidPoint,
                               const double* arcPoint,
                               const char* dimText)
{
    m_ssEntity << "  0"          << std::endl;
    m_ssEntity << "DIMENSION"    << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"          << std::endl;
    m_ssEntity << getLayerName()     << std::endl;
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbDimension"     << std::endl;
    }
    m_ssEntity << "  2"          << std::endl;
    m_ssEntity << "*" << getLayerName()     << std::endl;     // blockName
    m_ssEntity << " 10"          << std::endl;     // arc center point
    m_ssEntity << centerPoint[0] << std::endl;
    m_ssEntity << " 20"          << std::endl;
    m_ssEntity << centerPoint[1] << std::endl;
    m_ssEntity << " 30"          << std::endl;
    m_ssEntity << centerPoint[2] << std::endl;
    m_ssEntity << " 11"          << std::endl;     //text mid point
    m_ssEntity << textMidPoint[0]   << std::endl;
    m_ssEntity << " 21"          << std::endl;
    m_ssEntity << textMidPoint[1]   << std::endl;
    m_ssEntity << " 31"          << std::endl;
    m_ssEntity << textMidPoint[2]   << std::endl;
    m_ssEntity << " 70"          << std::endl;
    m_ssEntity << 4              << std::endl;    // dimType 4 = Radius
    //    m_ssEntity << " 71"          << std::endl;    // not R12
    //    m_ssEntity << 1              << std::endl;    // attachPoint 5 = middle center
    m_ssEntity << "  1"          << std::endl;
    m_ssEntity << dimText        << std::endl;
    m_ssEntity << "  3"          << std::endl;
    m_ssEntity << "STANDARD"     << std::endl;    //style
    //radial dims
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbRadialDimension"     << std::endl;
    }
    m_ssEntity << " 15"          << std::endl;
    m_ssEntity << arcPoint[0]    << std::endl;
    m_ssEntity << " 25"          << std::endl;
    m_ssEntity << arcPoint[1]    << std::endl;
    m_ssEntity << " 35"          << std::endl;
    m_ssEntity << arcPoint[2]    << std::endl;
    m_ssEntity << " 40"          << std::endl;   // leader length????
    m_ssEntity << 0              << std::endl;

    writeDimBlockPreamble();
    writeRadialDimBlock(centerPoint, textMidPoint, arcPoint, dimText);
    writeBlockTrailer();
}

//***************************
//writeDiametricDim
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeDiametricDim(const double* textMidPoint,
                                  const double* arcPoint1,
                                  const double* arcPoint2,
                                  const char* dimText)
{
    m_ssEntity << "  0"          << std::endl;
    m_ssEntity << "DIMENSION"    << std::endl;
    m_ssEntity << "  5"      << std::endl;
    m_ssEntity << getEntityHandle() << std::endl;
    if (m_version > 12) {
        m_ssEntity << "330"      << std::endl;
        m_ssEntity << m_saveModelSpaceHandle  << std::endl;
        m_ssEntity << "100"      << std::endl;
        m_ssEntity << "AcDbEntity"      << std::endl;
    }
    m_ssEntity << "  8"          << std::endl;
    m_ssEntity << getLayerName()     << std::endl;
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbDimension"     << std::endl;
    }
    m_ssEntity << "  2"          << std::endl;
    m_ssEntity << "*" << getLayerName()     << std::endl;     // blockName
    m_ssEntity << " 10"          << std::endl;
    m_ssEntity << arcPoint1[0]   << std::endl;
    m_ssEntity << " 20"          << std::endl;
    m_ssEntity << arcPoint1[1]   << std::endl;
    m_ssEntity << " 30"          << std::endl;
    m_ssEntity << arcPoint1[2]   << std::endl;
    m_ssEntity << " 11"          << std::endl;     //text mid point
    m_ssEntity << textMidPoint[0]   << std::endl;
    m_ssEntity << " 21"          << std::endl;
    m_ssEntity << textMidPoint[1]   << std::endl;
    m_ssEntity << " 31"          << std::endl;
    m_ssEntity << textMidPoint[2]   << std::endl;
    m_ssEntity << " 70"          << std::endl;
    m_ssEntity << 3              << std::endl;    // dimType 3 = Diameter
    //    m_ssEntity << " 71"          << std::endl;    // not R12
    //    m_ssEntity << 5              << std::endl;    // attachPoint 5 = middle center
    m_ssEntity << "  1"          << std::endl;
    m_ssEntity << dimText        << std::endl;
    m_ssEntity << "  3"          << std::endl;
    m_ssEntity << "STANDARD"     << std::endl;    //style
    //diametric dims
    if (m_version > 12) {
        m_ssEntity << "100"          << std::endl;
        m_ssEntity << "AcDbDiametricDimension"     << std::endl;
    }
    m_ssEntity << " 15"          << std::endl;
    m_ssEntity << arcPoint2[0]   << std::endl;
    m_ssEntity << " 25"          << std::endl;
    m_ssEntity << arcPoint2[1]   << std::endl;
    m_ssEntity << " 35"          << std::endl;
    m_ssEntity << arcPoint2[2]   << std::endl;
    m_ssEntity << " 40"          << std::endl;   // leader length????
    m_ssEntity << 0              << std::endl;

    writeDimBlockPreamble();
    writeDiametricDimBlock(textMidPoint, arcPoint1, arcPoint2, dimText);
    writeBlockTrailer();
}

//***************************
//writeDimBlockPreamble
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeDimBlockPreamble()
{
    if (m_version > 12) {
        std::string blockName("*");
        blockName += getLayerName();
        m_saveBlkRecordHandle = getBlkRecordHandle();
        addBlockName(blockName,m_saveBlkRecordHandle);
    }

    m_currentBlock = getBlockHandle();
    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "BLOCK"        << std::endl;
    m_ssBlock << "  5"      << std::endl;
    m_ssBlock << m_currentBlock << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"      << std::endl;
        m_ssBlock << m_saveBlkRecordHandle << std::endl;
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbEntity"      << std::endl;
    }
    m_ssBlock << "  8"          << std::endl;
    m_ssBlock << getLayerName() << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"          << std::endl;
        m_ssBlock << "AcDbBlockBegin"  << std::endl;
    }
    m_ssBlock << "  2"          << std::endl;
    m_ssBlock << "*" << getLayerName()     << std::endl;     // blockName
    m_ssBlock << " 70"          << std::endl;
    m_ssBlock << "   1"         << std::endl;
    m_ssBlock << " 10"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << " 20"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << " 30"          << std::endl;
    m_ssBlock << 0.0            << std::endl;
    m_ssBlock << "  3"          << std::endl;
    m_ssBlock << "*" << getLayerName()     << std::endl;     // blockName
    m_ssBlock << "  1"          << std::endl;
    m_ssBlock << " "            << std::endl;
}

//***************************
//writeBlockTrailer
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeBlockTrailer()
{
    m_ssBlock << "  0"    << std::endl;
    m_ssBlock << "ENDBLK" << std::endl;
    m_ssBlock << "  5"      << std::endl;
    m_ssBlock << getBlockHandle() << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"    << std::endl;
        m_ssBlock << m_saveBlkRecordHandle    << std::endl;
        m_ssBlock << "100"    << std::endl;
        m_ssBlock << "AcDbEntity"    << std::endl;
    }
    //    m_ssBlock << " 67"    << std::endl;
    //    m_ssBlock << "1"    << std::endl;
    m_ssBlock << "  8"    << std::endl;
    m_ssBlock << getLayerName() << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"    << std::endl;
        m_ssBlock << "AcDbBlockEnd"    << std::endl;
    }
}

//***************************
//writeLinearDimBlock
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeLinearDimBlock(const double* textMidPoint,
                                    const double* dimLine,
                                    const double* e1Start,
                                    const double* e2Start,
                                    const char* dimText,
                                    int type)
{
    Base::Vector3d e1S(e1Start[0],e1Start[1],e1Start[2]);
    Base::Vector3d e2S(e2Start[0],e2Start[1],e2Start[2]);
    Base::Vector3d dl(dimLine[0],dimLine[1],dimLine[2]);      //point on DimLine (somewhere!)
    Base::Vector3d perp = dl.DistanceToLineSegment(e2S,e1S);
    Base::Vector3d e1E = e1S - perp;
    Base::Vector3d e2E = e2S - perp;
    Base::Vector3d para = e1E - e2E;
    Base::Vector3d X(1.0,0.0,0.0);
    double angle = para.GetAngle(X);
    angle = angle * 180.0 / M_PI;
    if (type == ALIGNED) {
        //NOP
    }
    else if (type == HORIZONTAL) {
        double x = e1Start[0];
        double y = dimLine[1];
        e1E = Base::Vector3d(x, y, 0.0);
        x = e2Start[0];
        e2E = Base::Vector3d(x, y, 0.0);
        perp = Base::Vector3d(0, -1, 0);     //down
        para = Base::Vector3d(1, 0, 0);      //right
        if (dimLine[1] > e1Start[1]) {
            perp = Base::Vector3d(0, 1, 0);   //up 
        }
        if (e1Start[0] > e2Start[0]) {
            para = Base::Vector3d(-1, 0, 0);  //left
        }
        angle = 0;
    }
    else if (type == VERTICAL) {
        double x = dimLine[0];
        double y = e1Start[1];
        e1E = Base::Vector3d(x, y, 0.0);
        y = e2Start[1];
        e2E = Base::Vector3d(x, y, 0.0);
        perp = Base::Vector3d(1, 0, 0);
        para = Base::Vector3d(0, 1, 0);
        if (dimLine[0] < e1Start[0]) {
            perp = Base::Vector3d(-1, 0, 0);
        }
        if (e1Start[1] > e2Start[1]) {
            para = Base::Vector3d(0, -1, 0);
        }
        angle = 90;
    }

    double arrowLen = 5.0;             //magic number
    double arrowWidth = arrowLen/6.0/2.0;   //magic number calc!

    putLine(e2S, e2E, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);

    putLine(e1S, e1E, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);

    putLine(e1E, e2E, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);

    putText(dimText,
            toVector3d(textMidPoint),
            toVector3d(dimLine),
            3.5,
            1,
            m_ssBlock,
            getBlockHandle(),
            m_saveBlkRecordHandle);

    perp.Normalize();
    para.Normalize();
    Base::Vector3d arrowStart = e1E;
    Base::Vector3d barb1 = arrowStart + perp*arrowWidth - para*arrowLen;
    Base::Vector3d barb2 = arrowStart - perp*arrowWidth - para*arrowLen;
    putArrow(arrowStart, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);


    arrowStart = e2E;
    barb1 = arrowStart + perp*arrowWidth + para*arrowLen;
    barb2 = arrowStart - perp*arrowWidth + para*arrowLen;
    putArrow(arrowStart, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);
}

//***************************
//writeAngularDimBlock
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeAngularDimBlock(const double* textMidPoint,
                                     const double* lineDefPoint,
                                     const double* startExt1,
                                     const double* endExt1,
                                     const double* startExt2,
                                     const double* endExt2,
                                     const char* dimText)
{
    Base::Vector3d e1S(startExt1[0],startExt1[1],startExt1[2]);   //apex
    Base::Vector3d e2S(startExt2[0],startExt2[1],startExt2[2]);
    Base::Vector3d e1E(endExt1[0],endExt1[1],endExt1[2]);
    Base::Vector3d e2E(endExt2[0],endExt2[1],endExt2[2]);
    Base::Vector3d e1 = e1E - e1S;
    Base::Vector3d e2 = e2E - e2S;

    double startAngle = atan2(e2.y,e2.x);
    double endAngle = atan2(e1.y,e1.x);
    double span = fabs(endAngle - startAngle);
    double offset = span * 0.10;
    if (startAngle < 0) {
        startAngle += 2.0 * M_PI;
    }
    if (endAngle < 0) {
        endAngle += 2.0 * M_PI;
    }
    Base::Vector3d startOff(cos(startAngle + offset),sin(startAngle + offset),0.0);
    Base::Vector3d endOff(cos(endAngle - offset),sin(endAngle - offset),0.0);
    startAngle = startAngle * 180.0 / M_PI;
    endAngle = endAngle * 180.0 / M_PI;
    
    Base::Vector3d linePt(lineDefPoint[0],lineDefPoint[1],lineDefPoint[2]);
    double radius = (e2S - linePt).Length();

    m_ssBlock << "  0"          << std::endl;
    m_ssBlock << "ARC"          << std::endl;       //dimline arc
    m_ssBlock << "  5"          << std::endl;
    m_ssBlock << getBlockHandle() << std::endl;
    if (m_version > 12) {
        m_ssBlock << "330"      << std::endl;
        m_ssBlock << m_saveBlkRecordHandle << std::endl;
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbEntity"      << std::endl;
    }
    m_ssBlock << "  8"          << std::endl;
    m_ssBlock << "0"            << std::endl;
    //    m_ssBlock << " 62"          << std::endl;
    //    m_ssBlock << "     0"       << std::endl;
    if (m_version > 12) {
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbCircle" << std::endl;
    }
    m_ssBlock << " 10"          << std::endl;
    m_ssBlock << startExt2[0]   << std::endl;      //arc center
    m_ssBlock << " 20"          << std::endl;
    m_ssBlock << startExt2[1]   << std::endl;
    m_ssBlock << " 30"          << std::endl;
    m_ssBlock << startExt2[2]   << std::endl;
    m_ssBlock << " 40"          << std::endl;
    m_ssBlock << radius         << std::endl;      //radius
    if (m_version > 12) {
        m_ssBlock << "100"      << std::endl;
        m_ssBlock << "AcDbArc" << std::endl;
    }
    m_ssBlock << " 50"          << std::endl;
    m_ssBlock << startAngle     << std::endl;            //start angle
    m_ssBlock << " 51"          << std::endl;
    m_ssBlock << endAngle       << std::endl;            //end angle

    putText(dimText,
            toVector3d(textMidPoint),
            toVector3d(textMidPoint),
            3.5,
            1,
            m_ssBlock,
            getBlockHandle(),
            m_saveBlkRecordHandle);

    e1.Normalize();
    e2.Normalize();
    Base::Vector3d arrow1Start = e1S + e1 * radius;
    Base::Vector3d arrow2Start = e2S + e2 * radius;

    //wf: idk why the Tan pts have to be reversed.  something to do with CW angles in Dxf?
    Base::Vector3d endTan = e1S + (startOff * radius);
    Base::Vector3d startTan = e2S + (endOff * radius);
    Base::Vector3d tanP1 = (arrow1Start - startTan).Normalize();
    Base::Vector3d perp1(-tanP1.y,tanP1.x,tanP1.z); 
    Base::Vector3d tanP2 = (arrow2Start - endTan).Normalize();
    Base::Vector3d perp2(-tanP2.y,tanP2.x,tanP2.z); 
    double arrowLen = 5.0;                  //magic number
    double arrowWidth = arrowLen/6.0/2.0;   //magic number calc!

    Base::Vector3d barb1 = arrow1Start + perp1*arrowWidth - tanP1*arrowLen;
    Base::Vector3d barb2 = arrow1Start - perp1*arrowWidth - tanP1*arrowLen;

    putArrow(arrow1Start, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);

    barb1 = arrow2Start + perp2*arrowWidth - tanP2*arrowLen;
    barb2 = arrow2Start - perp2*arrowWidth - tanP2*arrowLen;

    putArrow(arrow2Start, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);
}

//***************************
//writeRadialDimBlock
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeRadialDimBlock(const double* centerPoint,
                                    const double* textMidPoint,
                                    const double* arcPoint,
                                    const char* dimText)
{
    putLine(toVector3d(centerPoint),
            toVector3d(arcPoint),
            m_ssBlock,
            getBlockHandle(),
            m_saveBlkRecordHandle);

    putText(dimText,
            toVector3d(textMidPoint),
            toVector3d(textMidPoint),
            3.5,
            1,
            m_ssBlock,
            getBlockHandle(),
            m_saveBlkRecordHandle);

    Base::Vector3d c(centerPoint[0],centerPoint[1],centerPoint[2]);
    Base::Vector3d a(arcPoint[0],arcPoint[1],arcPoint[2]);
    Base::Vector3d para = a - c;
    double arrowLen = 5.0;                  //magic number
    double arrowWidth = arrowLen/6.0/2.0;   //magic number calc!
    para.Normalize();
    Base::Vector3d perp(-para.y,para.x,para.z);
    Base::Vector3d arrowStart = a;
    Base::Vector3d barb1 = arrowStart + perp*arrowWidth - para*arrowLen;
    Base::Vector3d barb2 = arrowStart - perp*arrowWidth - para*arrowLen;

    putArrow(arrowStart, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);
}

//***************************
//writeDiametricDimBlock
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeDiametricDimBlock(const double* textMidPoint,
                                       const double* arcPoint1,
                                       const double* arcPoint2,
                                       const char* dimText)
{
    putLine(toVector3d(arcPoint1),
            toVector3d(arcPoint2),
            m_ssBlock,
            getBlockHandle(),
            m_saveBlkRecordHandle);

    putText(dimText,
            toVector3d(textMidPoint),
            toVector3d(textMidPoint),
            3.5,
            1,
            m_ssBlock,
            getBlockHandle(),
            m_saveBlkRecordHandle);

    Base::Vector3d a1(arcPoint1[0],arcPoint1[1],arcPoint1[2]);
    Base::Vector3d a2(arcPoint2[0],arcPoint2[1],arcPoint2[2]);
    Base::Vector3d para = a2 - a1;
    double arrowLen = 5.0;                  //magic number
    double arrowWidth = arrowLen/6.0/2.0;   //magic number calc!
    para.Normalize();
    Base::Vector3d perp(-para.y,para.x,para.z);
    Base::Vector3d arrowStart = a1;
    Base::Vector3d barb1 = arrowStart + perp*arrowWidth + para*arrowLen;
    Base::Vector3d barb2 = arrowStart - perp*arrowWidth + para*arrowLen;

    putArrow(arrowStart, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);

    arrowStart = a2;
    barb1 = arrowStart + perp*arrowWidth - para*arrowLen;
    barb2 = arrowStart - perp*arrowWidth - para*arrowLen;
    putArrow(arrowStart, barb1, barb2, m_ssBlock, getBlockHandle(), m_saveBlkRecordHandle);
}

//***************************
//writeBlocksSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeBlocksSection()
{
    if (m_version < 14) {
        std::stringstream ss;
        ss << "blocks1" << m_version << ".rub";
        std::string fileSpec = m_dataDir + ss.str();
        m_ofs << getPlateFile(fileSpec);
    }
    
    //write blocks content
    m_ofs << m_ssBlock.str();

    m_ofs << "  0"      << std::endl;
    m_ofs << "ENDSEC"   << std::endl;
}

//***************************
//writeEntitiesSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeEntitiesSection()
{
    std::stringstream ss;
    ss << "entities" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    m_ofs << getPlateFile(fileSpec);
    
    //write entities content
    m_ofs << m_ssEntity.str();
    

    m_ofs << "  0"      << std::endl;
    m_ofs << "ENDSEC"   << std::endl;
}

//***************************
//writeObjectsSection
//added by Wandererfan 2018 (wandererfan@gmail.com) for FreeCAD project
void CDxfWrite::writeObjectsSection()
{
    if (m_version < 14) {
        return;
    }
    std::stringstream ss;
    ss << "objects" << m_version << ".rub";
    std::string fileSpec = m_dataDir + ss.str();
    m_ofs << getPlateFile(fileSpec);
}

CDxfRead::CDxfRead(const char* filepath)
    : m_ifs(filepath)
{
    // start the file
    memset( m_str, '\0', sizeof(m_str) );
    memset( m_unused_line, '\0', sizeof(m_unused_line) );
    m_fail = false;
    m_ColorIndex = 0;
    m_eUnits = eMillimeters;
    m_measurement_inch = false;
    m_layer_name = "0";  // Default layer name
    memset( m_section_name, '\0', sizeof(m_section_name) );
    memset( m_block_name, '\0', sizeof(m_block_name) );
    m_ignore_errors = true;

    m_version = RUnknown;
    //m_CodePage = nullptr;
    //m_encoding = nullptr;

    if (!m_ifs)
        m_fail = true;
    else
        m_ifs.imbue(std::locale::classic());
}

CDxfRead::~CDxfRead()
{
    //delete m_CodePage;
    //delete m_encoding;
}

double CDxfRead::mm( double value ) const
{
    // re #6461
    // this if handles situation of malformed DXF file where
    // MEASUREMENT specifies English units, but
    // INSUNITS specifies millimeters or is not specified
    //(millimeters is our default)
    if (m_measurement_inch && (m_eUnits == eMillimeters)) {
        value *= 25.4;
    }

    switch (m_eUnits) {
    case eUnspecified:
        return (value * 1.0);  // We don't know any better.
    case eInches:
        return (value * 25.4);
    case eFeet:
        return (value * 25.4 * 12);
    case eMiles:
        return (value * 1609344.0);
    case eMillimeters:
        return (value * 1.0);
    case eCentimeters:
        return (value * 10.0);
    case eMeters:
        return (value * 1000.0);
    case eKilometers:
        return (value * 1000000.0);
    case eMicroinches:
        return (value * 25.4 / 1000.0);
    case eMils:
        return (value * 25.4 / 1000.0);
    case eYards:
        return (value * 3 * 12 * 25.4);
    case eAngstroms:
        return (value * 0.0000001);
    case eNanometers:
        return (value * 0.000001);
    case eMicrons:
        return (value * 0.001);
    case eDecimeters:
        return (value * 100.0);
    case eDekameters:
        return (value * 10000.0);
    case eHectometers:
        return (value * 100000.0);
    case eGigameters:
        return (value * 1000000000000.0);
    case eAstronomicalUnits:
        return (value * 149597870690000.0);
    case eLightYears:
        return (value * 9454254955500000000.0);
    case eParsecs:
        return (value * 30856774879000000000.0);
    default:
        return (value * 1.0);  // We don't know any better.
    } // End switch
} // End mm() method


bool CDxfRead::ReadLine()
{
    double s[3] = {0, 0, 0};
    double e[3] = {0, 0, 0};
    bool hidden = false;

    while (!m_ifs.eof()) {
        get_line();
        int n;

        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadLine()");
            return false;
        }

        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found, so finish with line
            ResolveColorIndex();
            OnReadLine(s, e, hidden);
            hidden = false;
            return true;

        case 6: // line style name follows
            get_line();
            if (m_str[0] == 'h' || m_str[0] == 'H') {
                hidden = true;
            }
            break;

        case 10:
            // start x
            get_line();
            ss.str(m_str);
            ss >> s[0];
            s[0] = mm(s[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // start y
            get_line();
            ss.str(m_str);
            ss >> s[1];
            s[1] = mm(s[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // start z
            get_line();
            ss.str(m_str);
            ss >> s[2];
            s[2] = mm(s[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 11:
            // end x
            get_line();
            ss.str(m_str);
            ss >> e[0];
            e[0] = mm(e[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 21:
            // end y
            get_line();
            ss.str(m_str);
            ss >> e[1];
            e[1] = mm(e[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 31:
            // end z
            get_line();
            ss.str(m_str);
            ss >> e[2];
            e[2] = mm(e[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }

    try {
        ResolveColorIndex();
        OnReadLine(s, e, false);
    }
    catch (...) {
        if (!IgnoreErrors()) {
            throw;  // Re-throw the exception.
        }
    }

    return false;
}

bool CDxfRead::ReadPoint()
{
    double s[3] = {0, 0, 0};

    while (!m_ifs.eof()) {
        get_line();
        int n;

        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadPoint()");
            return false;
        }

        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found, so finish with line
            ResolveColorIndex();
            OnReadPoint(s);
            return true;

        case 10:
            // start x
            get_line();
            ss.str(m_str);
            ss >> s[0];
            s[0] = mm(s[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // start y
            get_line();
            ss.str(m_str);
            ss >> s[1];
            s[1] = mm(s[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // start z
            get_line();
            ss.str(m_str);
            ss >> s[2];
            s[2] = mm(s[2]);
            if (ss.fail()) {
                return false;
            }
            break;

        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }

    try {
        ResolveColorIndex();
        OnReadPoint(s);
    }
    catch (...) {
        if (!IgnoreErrors()) {
            throw;  // Re-throw the exception.
        }
    }

    return false;
}

bool CDxfRead::ReadArc()
{
    double start_angle = 0.0;// in degrees
    double end_angle = 0.0;
    double radius = 0.0;
    double c[3] = {0,0,0}; // centre
    double z_extrusion_dir = 1.0;
    bool hidden = false;
    
    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadArc()");
            return false;
        }

        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found, so finish with arc
            ResolveColorIndex();
            OnReadArc(start_angle, end_angle, radius, c,z_extrusion_dir, hidden);
            hidden = false;
            return true;

        case 6: // line style name follows
            get_line();
            if (m_str[0] == 'h' || m_str[0] == 'H') {
                hidden = true;
            }
            break;

        case 10:
            // centre x
            get_line();
            ss.str(m_str);
            ss >> c[0];
            c[0] = mm(c[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // centre y
            get_line();
            ss.str(m_str);
            ss >> c[1];
            c[1] = mm(c[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // centre z
            get_line();
            ss.str(m_str);
            ss >> c[2];
            c[2] = mm(c[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 40:
            // radius
            get_line();
            ss.str(m_str);
            ss >> radius;
            radius = mm(radius);
            if (ss.fail()) {
                return false;
            }
            break;
        case 50:
            // start angle
            get_line();
            ss.str(m_str);
            ss >> start_angle;
            if (ss.fail()) {
                return false;
            }
            break;
        case 51:
            // end angle
            get_line();
            ss.str(m_str);
            ss >> end_angle;
            if (ss.fail()) {
                return false;
            }
            break;

        case 100:
        case 39:
        case 210:
        case 220:
            // skip the next line
            get_line();
            break;
        case 230:
            //Z extrusion direction for arc
            get_line();
            ss.str(m_str);
            ss >> z_extrusion_dir;
            if (ss.fail()) {
                return false;
            }
            break;

        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
    ResolveColorIndex();
    OnReadArc(start_angle, end_angle, radius, c, z_extrusion_dir, false);
    return false;
}

bool CDxfRead::ReadSpline()
{
    struct SplineData sd;
    sd.norm[0] = 0;
    sd.norm[1] = 0;
    sd.norm[2] = 1;
    sd.degree = 0;
    sd.knots = 0;
    sd.flag = 0;
    sd.control_points = 0;
    sd.fit_points = 0;

    double temp_double;

    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadSpline()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n) {
        case 0:
            // next item found, so finish with Spline
            ResolveColorIndex();
            OnReadSpline(sd);
            return true;
        case 210:
            // normal x
            get_line();
            ss.str(m_str);
            ss >> sd.norm[0];
            if (ss.fail()) {
                return false;
            }
            break;
        case 220:
            // normal y
            get_line();
            ss.str(m_str);
            ss >> sd.norm[1];
            if (ss.fail()) {
                return false;
            }
            break;
        case 230:
            // normal z
            get_line();
            ss.str(m_str);
            ss >> sd.norm[2];
            if (ss.fail()) {
                return false;
            }
            break;
        case 70:
            // flag
            get_line();
            ss.str(m_str);
            ss >> sd.flag;
            if (ss.fail()) {
                return false;
            }
            break;
        case 71:
            // degree
            get_line();
            ss.str(m_str);
            ss >> sd.degree;
            if (ss.fail()) {
                return false;
            }
            break;
        case 72:
            // knots
            get_line();
            ss.str(m_str);
            ss >> sd.knots;
            if (ss.fail()) {
                return false;
            }
            break;
        case 73:
            // control points
            get_line();
            ss.str(m_str);
            ss >> sd.control_points;
            if (ss.fail()) {
                return false;
            }
            break;
        case 74:
            // fit points
            get_line();
            ss.str(m_str);
            ss >> sd.fit_points;
            if (ss.fail()) {
                return false;
            }
            break;
        case 12:
            // starttan x
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.starttanx.push_back(temp_double);
            break;
        case 22:
            // starttan y
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.starttany.push_back(temp_double);
            break;
        case 32:
            // starttan z
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.starttanz.push_back(temp_double);
            break;
        case 13:
            // endtan x
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.endtanx.push_back(temp_double);
            break;
        case 23:
            // endtan y
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.endtany.push_back(temp_double);
            break;
        case 33:
            // endtan z
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.endtanz.push_back(temp_double);
            break;
        case 40:
            // knot
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.knot.push_back(temp_double);
            break;
        case 41:
            // weight
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.weight.push_back(temp_double);
            break;
        case 10:
            // control x
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.controlx.push_back(temp_double);
            break;
        case 20:
            // control y
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.controly.push_back(temp_double);
            break;
        case 30:
            // control z
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.controlz.push_back(temp_double);
            break;
        case 11:
            // fit x
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.fitx.push_back(temp_double);
            break;
        case 21:
            // fit y
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.fity.push_back(temp_double);
            break;
        case 31:
            // fit z
            get_line();
            ss.str(m_str);
            ss >> temp_double;
            temp_double = mm(temp_double);
            if (ss.fail()) {
                return false;
            }
            sd.fitz.push_back(temp_double);
            break;
        case 42:
        case 43:
        case 44:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
    ResolveColorIndex();
    OnReadSpline(sd);
    return false;
}


bool CDxfRead::ReadCircle()
{
    double radius = 0.0;
    double c[3] = {0,0,0}; // centre
    bool hidden = false;

    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadCircle()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found, so finish with Circle
            ResolveColorIndex();
            OnReadCircle(c, radius, hidden);
            hidden = false;
            return true;

        case 6: // line style name follows
            get_line();
            if (m_str[0] == 'h' || m_str[0] == 'H') {
                hidden = true;
            }
            break;

        case 10:
            // centre x
            get_line();
            ss.str(m_str);
            ss >> c[0];
            c[0] = mm(c[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // centre y
            get_line();
            ss.str(m_str);
            ss >> c[1];
            c[1] = mm(c[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // centre z
            get_line();
            ss.str(m_str);
            ss >> c[2];
            c[2] = mm(c[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 40:
            // radius
            get_line();
            ss.str(m_str);
            ss >> radius;
            radius = mm(radius);
            if (ss.fail()) {
                return false;
            }
            break;

        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
    ResolveColorIndex();
    OnReadCircle(c, radius, false);
    return false;
}

void CDxfRead::ReadText()
{
    ScopedCLocale _(LC_NUMERIC);

    DxfText text;
    while (!m_ifs.eof()) {
        get_line();
        const int n = stringToInt(m_str);
        switch (n) {
        case 0:
            ResolveColorIndex();
            {
                size_t pos = text.str.find("\\P", 0);
                while (pos != std::string::npos) {
                    text.str.replace(pos, 2, "\n");
                    pos = text.str.find("\\P", pos + 1);
                }

                text.str = this->toUtf8(text.str);
                OnReadText(text);
            }
            return;
        case 10:
            // centre x
            get_line();
            text.point[0] = mm(stringToDouble(m_str));
            break;
        case 20:
            // centre y
            get_line();
            text.point[1] = mm(stringToDouble(m_str));
            break;
        case 30:
            // centre z
            get_line();
            text.point[2] = mm(stringToDouble(m_str));
            break;
        case 40:
            // text height
            get_line();
            text.height = mm(stringToDouble(m_str)) * 25.4 / 72.0;
            break;
        case 50:
            // text rotation
            get_line();
            text.rotation = stringToDouble(m_str);
            break;
        case 3:
            // Additional text that goes before the type 1 text
            // Note that if breaking the text into type-3 records splits a UFT-8 encoding we do
            // the decoding after splicing the lines together. I'm not sure if this actually
            // occurs, but handling the text this way will treat this condition properly.
            get_line();
            text.str.append(m_str);
            break;
        case 1:
            // final text
            get_line();
            text.str.append(m_str);
            break;

        case 71: {
            // attachment point
            get_line();
            const int ap = stringToInt(m_str);
            if (ap >= 1 && ap <= 9) {
                text.attachPoint = static_cast<AttachmentPoint>(ap);
            }
        }
            break;

        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
}


bool CDxfRead::ReadEllipse()
{
    double c[3] = {0,0,0}; // centre
    double m[3] = {0,0,0}; //major axis point
    double ratio=0; //ratio of major to minor axis
    double start=0; //start of arc
    double end=0;  // end of arc

    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadEllipse()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found, so finish with Ellipse
            ResolveColorIndex();
            OnReadEllipse(c, m, ratio, start, end);
            return true;

        case 10:
            // centre x
            get_line();
            ss.str(m_str);
            ss >> c[0];
            c[0] = mm(c[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // centre y
            get_line();
            ss.str(m_str);
            ss >> c[1];
            c[1] = mm(c[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // centre z
            get_line();
            ss.str(m_str);
            ss >> c[2];
            c[2] = mm(c[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 11:
            // major x
            get_line();
            ss.str(m_str);
            ss >> m[0];
            m[0] = mm(m[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 21:
            // major y
            get_line();
            ss.str(m_str);
            ss >> m[1];
            m[1] = mm(m[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 31:
            // major z
            get_line();
            ss.str(m_str);
            ss >> m[2];
            m[2] = mm(m[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 40:
            // ratio
            get_line();
            ss.str(m_str);
            ss >> ratio;
            if (ss.fail()) {
                return false;
            }
            break;
        case 41:
            // start
            get_line();
            ss.str(m_str);
            ss >> start;
            if (ss.fail()) {
                return false;
            }
            break;
        case 42:
            // end
            get_line();
            ss.str(m_str);
            ss >> end;
            if (ss.fail()) {
                return false;
            }
            break;
        case 100:
        case 210:
        case 220:
        case 230:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
    ResolveColorIndex();
    OnReadEllipse(c, m, ratio, start, end);
    return false;
}

// TODO Remove this(refactoring of CDxfRead::ReadLwPolyLine()
static bool poly_prev_found = false;
static double poly_prev_x;
static double poly_prev_y;
static double poly_prev_z;
static bool poly_prev_bulge_found = false;
static double poly_prev_bulge;
static bool poly_first_found = false;
static double poly_first_x;
static double poly_first_y;
static double poly_first_z;

// TODO Remove this(refactoring of CDxfRead::ReadLwPolyLine()
static void
AddPolyLinePoint(CDxfRead* dxf_read, double x, double y, double z, bool bulge_found, double bulge)
{

    try {
        if (poly_prev_found) {
            bool arc_done = false;
            if (poly_prev_bulge_found) {
                double cot = 0.5 * ((1.0 / poly_prev_bulge) - poly_prev_bulge);
                double cx = ((poly_prev_x + x) - ((y - poly_prev_y) * cot)) / 2.0;
                double cy = ((poly_prev_y + y) + ((x - poly_prev_x) * cot)) / 2.0;
                double ps[3] = {poly_prev_x, poly_prev_y, poly_prev_z};
                double pe[3] = {x, y, z};
                double pc[3] = {cx, cy, (poly_prev_z + z)/2.0};
                dxf_read->OnReadArc(ps, pe, pc, poly_prev_bulge >= 0, false);
                arc_done = true;
            }

            if (!arc_done) {
                double s[3] = {poly_prev_x, poly_prev_y, poly_prev_z};
                double e[3] = {x, y, z};
                dxf_read->OnReadLine(s, e, false);
            }
        }

        poly_prev_found = true;
        poly_prev_x = x;
        poly_prev_y = y;
        poly_prev_z = z;
        if (!poly_first_found) {
            poly_first_x = x;
            poly_first_y = y;
            poly_first_z = z;
            poly_first_found = true;
        }
        poly_prev_bulge_found = bulge_found;
        poly_prev_bulge = bulge;
    }
    catch (...) {
        if (!dxf_read->IgnoreErrors()) {
            throw;  // Re-throw it.
        }
    }
}

// TODO Remove this(refactoring of CDxfRead::ReadLwPolyLine()
static void PolyLineStart()
{
    poly_prev_found = false;
    poly_first_found = false;
}

// TODO Reimplement this function(refactoring of CDxfRead::ReadLwPolyLine()
bool CDxfRead::ReadLwPolyLine()
{
    PolyLineStart();

    bool x_found = false;
    bool y_found = false;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    bool bulge_found = false;
    double bulge = 0.0;
    bool closed = false;
    int flags;
    bool next_item_found = false;

    while (!m_ifs.eof() && !next_item_found) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadLwPolyLine()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found
            ResolveColorIndex();
            if (x_found && y_found){
                // add point
                AddPolyLinePoint(this, x, y, z, bulge_found, bulge);
                bulge_found = false;
                x_found = false;
                y_found = false;
            }
            next_item_found = true;
            break;
        case 10:
            // x
            get_line();
            if (x_found && y_found) {
                // add point
                AddPolyLinePoint(this, x, y, z, bulge_found, bulge);
                bulge_found = false;
                x_found = false;
                y_found = false;
            }
            ss.str(m_str);
            ss >> x;
            x = mm(x);
            if (ss.fail()) {
                return false;
            }
            x_found = true;
            break;
        case 20:
            // y
            get_line();
            ss.str(m_str);
            ss >> y;
            y = mm(y);
            if (ss.fail()) {
                return false;
            }
            y_found = true;
            break;
        case 38:
            // elevation
            get_line();
            ss.str(m_str);
            ss >> z;
            z = mm(z);
            if (ss.fail()) {
                return false;
            }
            break;
        case 42:
            // bulge
            get_line();
            ss.str(m_str);
            ss >> bulge;
            if (ss.fail()) {
                return false;
            }
            bulge_found = true;
            break;
        case 70:
            // flags
            get_line();
            if (sscanf(m_str, "%d", &flags) != 1) {
                return false;
            }
            closed = ((flags & 1) != 0);
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }

    if (next_item_found) {
        if (closed && poly_first_found) {
            // repeat the first point
            ResolveColorIndex();
            AddPolyLinePoint(this, poly_first_x, poly_first_y, poly_first_z, false, 0.0);
        }
        return true;
    }

    return false;
}

bool CDxfRead::ReadVertex(DxfVertex* vertex)
{
    bool x_found = false;
    bool y_found = false;
    while (!m_ifs.eof()) {
        get_line();
        int n;
        if(sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadVertex()");
            return false;
        }

        switch (n){
        case 0:
            ResolveColorIndex();
            put_line(m_str);    // read one line too many.  put it back.
            return (x_found && y_found);
            break;
        case 10:
            // x
            get_line();
            vertex->point[0] = mm(stringToDouble(m_str));
            x_found = true;
            break;
        case 20:
            // y
            get_line();
            vertex->point[1] = mm(stringToDouble(m_str));
            y_found = true;
            break;
        case 30:
            // z
            get_line();
            vertex->point[2] = mm(stringToDouble(m_str));
            break;

        case 42: {
            // bulge
            get_line();
            const int bulge = stringToInt(m_str);
            if (bulge == 0)
                vertex->bulge = DxfVertex::Bulge::StraightSegment;
            else
                vertex->bulge = DxfVertex::Bulge::SemiCircle;
        }
            break;

        case 70:
            // flags
            get_line();
            vertex->flags = stringToUnsigned(m_str);
            break;

        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }

    return false;
}


bool CDxfRead::ReadPolyLine()
{
    ScopedCLocale _(LC_NUMERIC);
    DxfPolyline polyline;
    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadPolyLine()");
            return false;
        }

        switch (n) {
        case 0:
            // next item found
            ResolveColorIndex();
            get_line();
            if (!strcmp(m_str, "VERTEX")) {
                DxfVertex vertex;
                if (ReadVertex(&vertex))
                    polyline.vertices.push_back(vertex);
            }

            if (!strcmp(m_str, "SEQEND")) {
                OnReadPolyline(polyline);
                return true;
            }

            break;
        case 70:
            // flags
            get_line();
            polyline.flags = stringToUnsigned(m_str);
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }

    return false;
}

void CDxfRead::OnReadArc(double start_angle,
                         double end_angle,
                         double radius,
                         const double* c,
                         double z_extrusion_dir,
                         bool hidden)
{
    double s[3] = {0, 0, 0}, e[3] = {0, 0, 0}, temp[3] = {0, 0, 0};
    if (z_extrusion_dir == 1.0) {
        temp[0] = c[0];
        temp[1] = c[1];
        temp[2] = c[2];
        s[0] = c[0] + radius * cos(start_angle * M_PI / 180);
        s[1] = c[1] + radius * sin(start_angle * M_PI / 180);
        s[2] = c[2];
        e[0] = c[0] + radius * cos(end_angle * M_PI / 180);
        e[1] = c[1] + radius * sin(end_angle * M_PI / 180);
        e[2] = c[2];
    }
    else {
        temp[0] =-c[0];
        temp[1] =c[1];
        temp[2] =c[2];

        e[0] = -(c[0] + radius * cos(start_angle * M_PI/180));
        e[1] = (c[1] + radius * sin(start_angle * M_PI/180));
        e[2] = c[2];
        s[0] = -(c[0] + radius * cos(end_angle * M_PI/180));
        s[1] = (c[1] + radius * sin(end_angle * M_PI/180));
        s[2] = c[2];
    }
    OnReadArc(s, e, temp, true, hidden);
}

void CDxfRead::OnReadCircle(const double* c, double radius, bool hidden)
{
    double s[3];
    double start_angle = 0;
    s[0] = c[0] + radius * cos(start_angle * M_PI / 180);
    s[1] = c[1] + radius * sin(start_angle * M_PI / 180);
    s[2] = c[2];

    OnReadCircle(s,
                 c,
                 false,
                 hidden);  // false to change direction because otherwise the arc length is zero
}

void CDxfRead::OnReadEllipse(const double* c,
                             const double* m,
                             double ratio,
                             double start_angle,
                             double end_angle)
{
    double major_radius = sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
    double minor_radius = major_radius * ratio;

    // Since we only support 2d stuff, we can calculate the rotation from the major axis x and y
    // value only, since z is zero, major_radius is the vector length

    double rotation = atan2(m[1] / major_radius, m[0] / major_radius);


    OnReadEllipse(c, major_radius, minor_radius, rotation, start_angle, end_angle, true);
}


bool CDxfRead::ReadInsert()
{
    double c[3] = {0,0,0}; // coordinate
    double s[3] = {1,1,1}; // scale
    double rot = 0.0; // rotation
    char name[1024] = {0};

    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadInsert()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found
            ResolveColorIndex();
            OnReadInsert(c, s, name, rot * M_PI/180);
            return(true);
        case 10:
            // coord x
            get_line();
            ss.str(m_str);
            ss >> c[0];
            c[0] = mm(c[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // coord y
            get_line();
            ss.str(m_str);
            ss >> c[1];
            c[1] = mm(c[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // coord z
            get_line();
            ss.str(m_str);
            ss >> c[2];
            c[2] = mm(c[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 41:
            // scale x
            get_line();
            ss.str(m_str);
            ss >> s[0];
            if (ss.fail()) {
                return false;
            }
            break;
        case 42:
            // scale y
            get_line();
            ss.str(m_str);
            ss >> s[1];
            if (ss.fail()) {
                return false;
            }
            break;
        case 43:
            // scale z
            get_line();
            ss.str(m_str);
            ss >> s[2];
            if (ss.fail()) {
                return false;
            }
            break;
        case 50:
            // rotation
            get_line();
            ss.str(m_str);
            ss >> rot;
            if (ss.fail()) {
                return false;
            }
            break;
        case 2:
            // block name
            get_line();
            strcpy(name, m_str);
            break;
        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
    return false;
}


bool CDxfRead::ReadDimension()
{
    double s[3] = {0,0,0}; // startpoint
    double e[3] = {0,0,0}; // endpoint
    double p[3] = {0,0,0}; // dimpoint
    double rot = -1.0; // rotation

    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadDimension()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 0:
            // next item found
            ResolveColorIndex();
            OnReadDimension(s, e, p, rot * M_PI/180);
            return(true);
        case 13:
            // start x
            get_line();
            ss.str(m_str);
            ss >> s[0];
            s[0] = mm(s[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 23:
            // start y
            get_line();
            ss.str(m_str);
            ss >> s[1];
            s[1] = mm(s[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 33:
            // start z
            get_line();
            ss.str(m_str);
            ss >> s[2];
            s[2] = mm(s[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 14:
            // end x
            get_line();
            ss.str(m_str);
            ss >> e[0];
            e[0] = mm(e[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 24:
            // end y
            get_line();
            ss.str(m_str);
            ss >> e[1];
            e[1] = mm(e[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 34:
            // end z
            get_line();
            ss.str(m_str);
            ss >> e[2];
            e[2] = mm(e[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 10:
            // dimline x
            get_line();
            ss.str(m_str);
            ss >> p[0];
            p[0] = mm(p[0]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 20:
            // dimline y
            get_line();
            ss.str(m_str);
            ss >> p[1];
            p[1] = mm(p[1]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 30:
            // dimline z
            get_line();
            ss.str(m_str);
            ss >> p[2];
            p[2] = mm(p[2]);
            if (ss.fail()) {
                return false;
            }
            break;
        case 50:
            // rotation
            get_line();
            ss.str(m_str);
            ss >> rot;
            if (ss.fail()) {
                return false;
            }
            break;
        case 100:
        case 39:
        case 210:
        case 220:
        case 230:
            // skip the next line
            get_line();
            break;
        default:
            get_line();
            HandleCommonGroupCode(n);
            break;
        }
    }
    return false;
}


bool CDxfRead::ReadBlockInfo()
{
    while (!m_ifs.eof()) {
        get_line();
        int n;
        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadBlockInfo()");
            return false;
        }
        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n){
        case 2:
            // block name
            get_line();
            safe_strcpy(m_block_name, m_str);
            return true;
        case 3:
            // block name too???
            get_line();
            safe_strcpy(m_block_name, m_str);
            return true;
        default:
            // skip the next line
            get_line();
            break;
        }
    }
    return false;
}


void CDxfRead::get_line()
{
    if (m_unused_line[0] != '\0') {
        safe_strcpy(m_str, m_unused_line);
        memset(m_unused_line, '\0', sizeof(m_unused_line));
        return;
    }

    m_ifs.getline(m_str, 1024);

    char str[1024];
    size_t len = strlen(m_str);
    int j = 0;
    bool non_white_found = false;
    for (size_t i = 0; i < len; i++) {
        if (non_white_found || (m_str[i] != ' ' && m_str[i] != '\t')) {
            if (m_str[i] != '\r') {
                str[j] = m_str[i];
                j++;
            }
            non_white_found = true;
        }
    }
    str[j] = 0;
    safe_strcpy(m_str, str);
    ++m_line_nb;
}

void dxf_strncpy(char* dst, const char* src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
}

void CDxfRead::put_line(const char *value)
{
    dxf_strncpy( m_unused_line, value, sizeof(m_unused_line) );
}


bool CDxfRead::ReadUnits()
{
    get_line(); // Skip to next line.
    get_line(); // Skip to next line.
    int n = 0;
    if (sscanf(m_str, "%d", &n) == 1) {
        m_eUnits = eDxfUnits_t( n );
        return(true);
    } // End if - then
    else {
        this->ReportError_readInteger("DXF::ReadUnits()");
        return(false);
    }
}


bool CDxfRead::ReadLayer()
{
    std::string layername;
    ColorIndex_t colorIndex = -1;

    while (!m_ifs.eof()) {
        get_line();
        int n;

        if (sscanf(m_str, "%d", &n) != 1) {
            this->ReportError_readInteger("DXF::ReadLayer()");
            return false;
        }

        std::istringstream ss;
        ss.imbue(std::locale::classic());
        switch (n) {
        case 0: // next item found, so finish with line
            if (layername.empty()) {
                this->ReportError_readInteger("DXF::ReadLayer() - no layer name");
                return false;
            }
            m_layer_ColorIndex_map[layername] = colorIndex;
            return true;

        case 2: // Layer name follows
            get_line();
            layername = m_str;
            break;

        case 62:
            // layer color ; if negative, layer is off
            get_line();
            if (sscanf(m_str, "%d", &colorIndex) != 1) {
                return false;
            }
            break;

        case 6: // linetype name
        case 70: // layer flags
        case 100:
        case 290:
        case 370:
        case 390:
            // skip the next line
            get_line();
            break;
        default:
            // skip the next line
            get_line();
            break;
        }
    }
    return false;
}

bool CDxfRead::ReadVersion()
{
    static const std::vector<std::string> VersionNames = {
                                                          // This table is indexed by eDXFVersion_t - (ROlder+1)
                                                          "AC1006",
                                                          "AC1009",
                                                          "AC1012",
                                                          "AC1014",
                                                          "AC1015",
                                                          "AC1018",
                                                          "AC1021",
                                                          "AC1024",
                                                          "AC1027",
                                                          "AC1032"};

    assert(VersionNames.size() == RNewer - ROlder - 1);
    get_line();
    get_line();
    std::vector<std::string>::const_iterator first = VersionNames.cbegin();
    std::vector<std::string>::const_iterator last = VersionNames.cend();
    std::vector<std::string>::const_iterator found = std::lower_bound(first, last, m_str);
    if (found == last) {
        m_version = RNewer;
    }
    else if (*found == m_str) {
        m_version = (eDXFVersion_t)(std::distance(first, found) + (ROlder + 1));
    }
    else if (found == first) {
        m_version = ROlder;
    }
    else {
        m_version = RUnknown;
    }

    return ResolveEncoding();
}

bool CDxfRead::ReadDWGCodePage()
{
    get_line();
    get_line();
    m_CodePage = m_str;

    return ResolveEncoding();
}

bool CDxfRead::ResolveEncoding()
{
    //
    // See https://ezdxf.readthedocs.io/en/stable/dxfinternals/fileencoding.html#
    //

    if (m_version >= R2007) { // Note this does not include RUnknown, but does include RLater
        return this->setSourceEncoding("UTF8");
    }
    else {
        std::transform(m_CodePage.cbegin(), m_CodePage.cend(), m_CodePage.begin(), [](char c) {
            return std::toupper(c, std::locale::classic());
        });
        // ANSI_1252 by default if $DWGCODEPAGE is not set
        if (m_CodePage.empty())
            m_CodePage = "ANSI_1252";

        return this->setSourceEncoding(m_CodePage);
    }
}

void CDxfRead::HandleCommonGroupCode(int n)
{
    switch (n) {
    case 8:
        // layer name
        m_layer_name = m_str;
        break;
    case 62:
        // color index
        m_ColorIndex = stringToInt(m_str);
        break;
    }
}

void CDxfRead::DoRead(const bool ignore_errors /* = false */ )
{
    m_ignore_errors = ignore_errors;
    if (m_fail) {
        return;
    }

    get_line();

    while (!m_ifs.eof()) {
        m_ColorIndex = ColorBylayer; // Default

        if (!strcmp(m_str, "$INSUNITS" )) {
            if (!ReadUnits()) {
                return;
            }
            continue;
        } // End if - then

        if (!strcmp(m_str, "$MEASUREMENT" )) {
            get_line();
            get_line();
            int n = 1;
            if (sscanf(m_str, "%d", &n) == 1) {
                if (n == 0) {
                    m_measurement_inch = true;
                }
            }
            continue;
        } // End if - then
        
        if (!strcmp(m_str, "$ACADVER")) {
            if (!ReadVersion()) {
                return;
            }
            continue;
        }  // End if - then

        if (!strcmp(m_str, "$DWGCODEPAGE")) {
            if (!ReadDWGCodePage()) {
                return;
            }
            continue;
        }  // End if - then

        if (!strcmp(m_str, "0")) {
            get_line();
            if (!strcmp(m_str, "0"))
                get_line(); // Skip again

            if (!strcmp( m_str, "SECTION" )) {
                safe_strcpy(m_section_name, "");
                get_line();
                get_line();
                if (strcmp( m_str, "ENTITIES" )) {
                    safe_strcpy(m_section_name, m_str);
                }
                safe_strcpy(m_block_name, "");

            } // End if - then
            else if (!strcmp( m_str, "TABLE" )) {
                get_line();
                get_line();
            }

            else if (!strcmp( m_str, "LAYER" )) {
                //get_line();
                //get_line();
                if (!ReadLayer()) {
                    this->ReportError("DXF::DoRead() - Failed to read layer");
                    //return; Some objects or tables can have "LAYER" as name...
                }
                continue;
            }

            else if (!strcmp( m_str, "BLOCK" )) {
                if (!ReadBlockInfo()) {
                    this->ReportError("DXF::DoRead() - Failed to read block info");
                    return;
                }
                continue;
            } // End if - then

            else if (!strcmp( m_str, "ENDSEC" )) {
                safe_strcpy(m_section_name, "");
                safe_strcpy(m_block_name, "");
            } // End if - then
            else if (!strcmp(m_str, "LINE")) {
                if (!ReadLine()) {
                    this->ReportError("DXF::DoRead() - Failed to read line");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "ARC")) {
                if (!ReadArc()) {
                    this->ReportError("DXF::DoRead() - Failed to read arc");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "CIRCLE")) {
                if (!ReadCircle()) {
                    this->ReportError("DXF::DoRead() - Failed to read circle");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "MTEXT") || !strcmp(m_str, "TEXT")) {
                try {
                    ReadText();
                } catch (const std::runtime_error& err) {
                    this->ReportError("DXF::DoRead() - Failed to read text\nError: " + std::string(err.what()));
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "ELLIPSE")) {
                if (!ReadEllipse()) {
                    this->ReportError("DXF::DoRead() - Failed to read ellipse");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "SPLINE")) {
                if (!ReadSpline()) {
                    this->ReportError("DXF::DoRead() - Failed to read spline");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "LWPOLYLINE")) {
                if (!ReadLwPolyLine()) {
                    this->ReportError("DXF::DoRead() - Failed to read LW Polyline");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "POLYLINE")) {
                if (!ReadPolyLine()) {
                    this->ReportError("DXF::DoRead() - Failed to read Polyline");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "POINT")) {
                if (!ReadPoint()) {
                    this->ReportError("DXF::DoRead() - Failed to read Point");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "INSERT")) {
                if (!ReadInsert()) {
                    this->ReportError("DXF::DoRead() - Failed to read Insert");
                    return;
                }
                continue;
            }
            else if (!strcmp(m_str, "DIMENSION")) {
                if (!ReadDimension()) {
                    this->ReportError("DXF::DoRead() - Failed to read Dimension");
                    return;
                }
                continue;
            }
        }

        get_line();
    }
    AddGraphics();
}


void  CDxfRead::ResolveColorIndex()
{

    if (m_ColorIndex == ColorBylayer)  // if color = layer color, replace by color from layer
    {
        m_ColorIndex = m_layer_ColorIndex_map[m_layer_name];
    }
}

void CDxfRead::ReportError_readInteger(const char* context)
{
    std::string msg;
    if (context) {
        msg += context;
        msg += " - ";
    }

    msg += "Failed to read integer from '";
    msg += m_str;
    msg += "'";
    this->ReportError(msg.c_str());
}

std::streamsize CDxfRead::gcount() const
{
    return m_ifs.gcount();
}

std::string CDxfRead::LayerName() const
{
    std::string result;

    if (strlen(m_section_name) > 0) {
        result.append(m_section_name);
        result.append(" ");
    }

    if (strlen(m_block_name) > 0) {
        result.append(m_block_name);
        result.append(" ");
    }

    if (!m_layer_name.empty()) {
        result.append(m_layer_name);
    }

    return(result);
}
