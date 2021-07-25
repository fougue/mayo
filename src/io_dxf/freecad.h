#pragma once

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <limits>

// Implements required FreeCad classes/functions for the sake of dxf.h,cpp and io_dxf.h,cpp

namespace Base {

class Vector3d {
public:
    double x;
    double y;
    double z;

    Vector3d() = default;
    Vector3d(double ax, double ay, double az) : x(ax), y(ay), z(az) {}
    Vector3d operator-(const Vector3d& other) const {
        return Vector3d(this->x - other.x, this->y - other.y, this->z - other.z);
    }
    Vector3d operator+(const Vector3d& other) const {
        return Vector3d(this->x + other.x, this->y + other.y, this->z + other.z);
    }
    Vector3d operator*(double k) const {
        return Vector3d(this->x * k, this->y * k, this->z * k);
    }
    double operator*(const Vector3d& other) const {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }

    inline Vector3d& Normalize();
    inline double Length() const { return std::sqrt((x * x) + (y * y) + (z * z)); }
    inline double GetAngle(const Vector3d& rcVect) const;
    inline Vector3d DistanceToLineSegment(const Vector3d& rclP1, const Vector3d& rclP2) const;
};

template<class T> T clamp (T num, T lower, T upper) {
    return std::max<T>(std::min<T>(upper,num),lower);
}

inline double DistanceP2(const Vector3d& v1, const Vector3d& v2)
{
    const double x = v1.x - v2.x;
    const double y = v1.y - v2.y;
    const double z = v1.z - v2.z;
    return x * x + y * y + z * z;
}

Vector3d& Vector3d::Normalize()
{
    double fLen = Length ();
    if (fLen != 0.0 && fLen != 1.0) { // Suspicious
        x /= fLen;
        y /= fLen;
        z /= fLen;
    }

    return *this;
}

double Vector3d::GetAngle(const Vector3d& rcVect) const
{
    double len1 = Length();
    double len2 = rcVect.Length();
    if (len1 <= DBL_EPSILON || len2 <= DBL_EPSILON)
        return std::numeric_limits<double>::quiet_NaN(); // division by zero

    double dot = (*this) * rcVect;
    dot /= len1;
    dot /= len2;

    if (dot <= -1.0)
        return 3.141592653589793;
    else if (dot >= 1.0)
        return 0.0;

    return std::acos(dot);
}

Vector3d Vector3d::DistanceToLineSegment(const Vector3d& rclP1, const Vector3d& rclP2) const
{
    double len2 = Base::DistanceP2(rclP1, rclP2);
    if (len2 <= DBL_EPSILON)
        return rclP1;

    Vector3d p2p1 = rclP2-rclP1;
    Vector3d pXp1 = *this-rclP1;
    double dot = pXp1 * p2p1;
    double t = clamp<double>(dot/len2, 0, 1);
    Vector3d dist = p2p1 * t - pXp1;
    return dist;
}

} // namespace Base
