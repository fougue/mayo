/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <utility>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>
class Adaptor3d_Curve;
class gp_Ax1;

namespace Mayo::GeomUtils {

double normalizedU(const Adaptor3d_Curve& curve, double u);

gp_Pnt d0(const Adaptor3d_Curve& curve, double u);
gp_Vec d1(const Adaptor3d_Curve& curve, double u);
std::pair<gp_Pnt, gp_Vec> d0d1(const Adaptor3d_Curve& curve, double u);

// Detects if transformation matrix 'trsf' contains scaling
bool hasScaling(const gp_Trsf& trsf);

gp_Trsf makeTranslation(const gp_Vec& v);
gp_Trsf makeTranslation(const gp_Pnt& p1, const gp_Pnt& p2);

gp_Trsf makeRotation(const gp_Ax1& ax1, double angle_rad);

} // namespace Mayo::GeomUtils
