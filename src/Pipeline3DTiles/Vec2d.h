#ifndef __VEC2D_H__
#define __VEC2D_H__

#include <math.h>
#include <float.h>

class Vec2d
{
public:
    double _v[2];

    /** Constructor that sets all components of the vector to zero */
    Vec2d() { _v[0] = 0.0; _v[1] = 0.0; }

    Vec2d(double x, double y) { _v[0] = x; _v[1] = y; }

    inline Vec2d(const Vec2d& vec) { _v[0] = vec._v[0]; _v[1] = vec._v[1]; }

    inline bool operator == (const Vec2d& v) const { return _v[0] == v._v[0] && _v[1] == v._v[1]; }

    inline bool operator != (const Vec2d& v) const { return _v[0] != v._v[0] || _v[1] != v._v[1]; }

    inline bool operator <  (const Vec2d& v) const
    {
        if (_v[0] < v._v[0]) return true;
        else if (_v[0] > v._v[0]) return false;
        else return (_v[1] < v._v[1]);
    }

    inline double* ptr() { return _v; }
    inline const double* ptr() const { return _v; }

    inline void set(double x, double y) { _v[0] = x; _v[1] = y; }

    inline double& operator [] (int i) { return _v[i]; }
    inline double operator [] (int i) const { return _v[i]; }

    inline double& x() { return _v[0]; }
    inline double& y() { return _v[1]; }

    inline double x() const { return _v[0]; }
    inline double y() const { return _v[1]; }

    /** Returns true if all components have values that are not NaN. */
    inline bool valid() const { return !isNaN(); }
    /** Returns true if at least one component has value NaN. */
    inline bool isNaN() const { return isnan(_v[0]) || isnan(_v[1]); }

    /** Dot product. */
    inline double operator * (const Vec2d& rhs) const
    {
        return _v[0] * rhs._v[0] + _v[1] * rhs._v[1];
    }

    /** Multiply by scalar. */
    inline const Vec2d operator * (double rhs) const
    {
        return Vec2d(_v[0] * rhs, _v[1] * rhs);
    }

    /** Unary multiply by scalar. */
    inline Vec2d& operator *= (double rhs)
    {
        _v[0] *= rhs;
        _v[1] *= rhs;
        return *this;
    }

    /** Divide by scalar. */
    inline const Vec2d operator / (double rhs) const
    {
        return Vec2d(_v[0] / rhs, _v[1] / rhs);
    }

    /** Unary divide by scalar. */
    inline Vec2d& operator /= (double rhs)
    {
        _v[0] /= rhs;
        _v[1] /= rhs;
        return *this;
    }

    /** Binary vector add. */
    inline const Vec2d operator + (const Vec2d& rhs) const
    {
        return Vec2d(_v[0] + rhs._v[0], _v[1] + rhs._v[1]);
    }

    /** Unary vector add. Slightly more efficient because no temporary
      * intermediate object.
    */
    inline Vec2d& operator += (const Vec2d& rhs)
    {
        _v[0] += rhs._v[0];
        _v[1] += rhs._v[1];
        return *this;
    }

    /** Binary vector subtract. */
    inline const Vec2d operator - (const Vec2d& rhs) const
    {
        return Vec2d(_v[0] - rhs._v[0], _v[1] - rhs._v[1]);
    }

    /** Unary vector subtract. */
    inline Vec2d& operator -= (const Vec2d& rhs)
    {
        _v[0] -= rhs._v[0];
        _v[1] -= rhs._v[1];
        return *this;
    }

    /** Negation operator. Returns the negative of the Vec2d. */
    inline const Vec2d operator - () const
    {
        return Vec2d(-_v[0], -_v[1]);
    }

    /** Length of the vector = sqrt( vec . vec ) */
    inline double length() const
    {
        return sqrt(_v[0] * _v[0] + _v[1] * _v[1]);
    }

    /** Length squared of the vector = vec . vec */
    inline double length2(void) const
    {
        return _v[0] * _v[0] + _v[1] * _v[1];
    }

    /** Normalize the vector so that it has length unity.
      * Returns the previous length of the vector.
    */
    inline double normalize()
    {
        double norm = Vec2d::length();
        if (norm > 0.0)
        {
            double inv = 1.0 / norm;
            _v[0] *= inv;
            _v[1] *= inv;
        }
        return(norm);
    }

};    // end of class Vec2d


/** multiply by vector components. */
inline Vec2d componentMultiply(const Vec2d& lhs, const Vec2d& rhs)
{
    return Vec2d(lhs[0] * rhs[0], lhs[1] * rhs[1]);
}

/** divide rhs components by rhs vector components. */
inline Vec2d componentDivide(const Vec2d& lhs, const Vec2d& rhs)
{
    return Vec2d(lhs[0] / rhs[0], lhs[1] / rhs[1]);
}

#endif