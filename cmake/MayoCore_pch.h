#pragma once

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
// Required by windows for M_PI definition
#  define _USE_MATH_DEFINES
#endif

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <cassert>
#include <cstdint>

#include <fmt/format.h>
#include <gsl/span>
#include <kdbindings/signal.h>
#include <magic_enum/magic_enum.hpp>

//#include <Standard_Handle.hxx>
//#include <TDF_Label.hxx>
//#include <TopoDS_Shape.hxx>
//#include <NCollection_Vector.hxx>
