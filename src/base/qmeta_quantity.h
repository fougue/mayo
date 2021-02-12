/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "quantity.h"
#include <QtCore/QMetaType>
Q_DECLARE_METATYPE(Mayo::QuantityLength)
Q_DECLARE_METATYPE(Mayo::QuantityMass)
Q_DECLARE_METATYPE(Mayo::QuantityTime)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::ElectricCurrent>)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::ThermodynamicTemperature>)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::AmountOfSubstance>)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::LuminousIntensity>)
Q_DECLARE_METATYPE(Mayo::QuantityAngle)
Q_DECLARE_METATYPE(Mayo::QuantityArea)
Q_DECLARE_METATYPE(Mayo::QuantityVolume)
Q_DECLARE_METATYPE(Mayo::QuantityVelocity)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::Acceleration>)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::Density>)
Q_DECLARE_METATYPE(Mayo::Quantity<Mayo::Unit::Pressure>)
