#ifndef __VEC3D_H__
#define __VEC3D_H__

#include <math.h>
#include <float.h>

class Vec3d
{
public:
    double _v[3];

    /** Constructor that sets all components of the vector to zero */
    Vec3d() { _v[0] = 0.0; _v[1] = 0.0; _v[2] = 0.0; }
    inline Vec3d(const Vec3d& vec) { _v[0] = vec._v[0]; _v[1] = vec._v[1]; _v[2] = vec._v[2]; }
    Vec3d(double x, double y, double z) { _v[0] = x; _v[1] = y; _v[2] = z; }
    inline double& operator [] (int i) { return _v[i]; }
    inline double operator [] (int i) const { return _v[i]; }

    inline double& x() { return _v[0]; }
    inline double& y() { return _v[1]; }
    inline double& z() { return _v[2]; }

    inline double getX() const { return _v[0]; }
    inline double getY() const { return _v[1]; }
    inline double getZ() const { return _v[2]; }



    inline double x() const { return _v[0]; }
    inline double y() const { return _v[1]; }
    inline double z() const { return _v[2]; }

    inline double* ptr() { return _v; }
    inline const double* ptr() const { return _v; }

    inline void set(double x, double y, double z)
    {
        _v[0] = x; _v[1] = y; _v[2] = z;
    }

    inline void set(const Vec3d& rhs)
    {
        _v[0] = rhs._v[0]; _v[1] = rhs._v[1]; _v[2] = rhs._v[2];
    }

    inline bool operator == (const Vec3d& v) const { return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2]; }

    inline bool operator != (const Vec3d& v) const { return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2]; }

    inline bool operator <  (const Vec3d& v) const
    {
        if (_v[0] < v._v[0]) return true;
        else if (_v[0] > v._v[0]) return false;
        else if (_v[1] < v._v[1]) return true;
        else if (_v[1] > v._v[1]) return false;
        else return (_v[2] < v._v[2]);
    }
    /** Dot product. */
    inline double operator * (const Vec3d& rhs) const
    {
        return _v[0] * rhs._v[0] + _v[1] * rhs._v[1] + _v[2] * rhs._v[2];
    }

    /** Cross product. */
    inline const Vec3d operator ^ (const Vec3d& rhs) const
    {
        return Vec3d(_v[1] * rhs._v[2] - _v[2] * rhs._v[1],
            _v[2] * rhs._v[0] - _v[0] * rhs._v[2],
            _v[0] * rhs._v[1] - _v[1] * rhs._v[0]);
    }
    /** Multiply by scalar. */
    inline const Vec3d operator * (double rhs) const
    {
        return Vec3d(_v[0] * rhs, _v[1] * rhs, _v[2] * rhs);
    }

    /** Unary multiply by scalar. */
    inline Vec3d& operator *= (double rhs)
    {
        _v[0] *= rhs;
        _v[1] *= rhs;
        _v[2] *= rhs;
        return *this;
    }

    /** Divide by scalar. */
    inline const Vec3d operator / (double rhs) const
    {
        return Vec3d(_v[0] / rhs, _v[1] / rhs, _v[2] / rhs);
    }

    /** Unary divide by scalar. */
    inline Vec3d& operator /= (double rhs)
    {
        _v[0] /= rhs;
        _v[1] /= rhs;
        _v[2] /= rhs;
        return *this;
    }

    /** Binary vector add. */
    inline const Vec3d operator + (const Vec3d& rhs) const
    {
        return Vec3d(_v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2]);
    }

    /** Unary vector add. Slightly more efficient because no temporary
      * intermediate object.
    */
    inline Vec3d& operator += (const Vec3d& rhs)
    {
        _v[0] += rhs._v[0];
        _v[1] += rhs._v[1];
        _v[2] += rhs._v[2];
        return *this;
    }

    /** Binary vector subtract. */
    inline const Vec3d operator - (const Vec3d& rhs) const
    {
        return Vec3d(_v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2]);
    }

    /** Unary vector subtract. */
    inline Vec3d& operator -= (const Vec3d& rhs)
    {
        _v[0] -= rhs._v[0];
        _v[1] -= rhs._v[1];
        _v[2] -= rhs._v[2];
        return *this;
    }

    /** Negation operator. Returns the negative of the Vec3d. */
    inline const Vec3d operator - () const
    {
        return Vec3d(-_v[0], -_v[1], -_v[2]);
    }

    /** Length of the vector = sqrt( vec . vec ) */
    inline double length() const
    {
        return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]);
    }

    /** Length squared of the vector = vec . vec */
    inline double length2() const
    {
        return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
    }

    /** Normalize the vector so that it has length unity.
      * Returns the previous length of the vector.
      * If the vector is zero length, it is left unchanged and zero is returned.
    */
    inline double normalize()
    {
        double norm = Vec3d::length();
        if (norm > 0.0)
        {
            double inv = 1.0 / norm;
            _v[0] *= inv;
            _v[1] *= inv;
            _v[2] *= inv;
        }
        return(norm);
    }
    static const Vec3d& ZERO();
    static const Vec3d& HALF();
    static const Vec3d& ONE();
};

class BoundingBox
{
public:
    /** Minimum extent. (Smallest X, Y, and Z values of all coordinates.) */
    Vec3d _min;
    /** Maximum extent. (Greatest X, Y, and Z values of all coordinates.) */
    Vec3d _max;

    /** Creates an uninitialized bounding box. */
    inline BoundingBox() :
        _min(DBL_MAX,
            DBL_MAX,
            DBL_MAX),
        _max(-DBL_MAX,
            -DBL_MAX,
            -DBL_MAX)
    {}

    inline BoundingBox(const BoundingBox& bb) :
        _min(bb._min),
        _max(bb._max)
    {}

    /** Creates a bounding box initialized to the given extents. */
    inline BoundingBox(double xmin, double ymin, double zmin,
        double xmax, double ymax, double zmax) :
        _min(xmin, ymin, zmin),
        _max(xmax, ymax, zmax) {}

    /** Creates a bounding box initialized to the given extents. */
    inline BoundingBox(const Vec3d& min, const Vec3d& max) :
        _min(min),
        _max(max) {}

    /** Clear the bounding box. Erases existing minimum and maximum extents. */
    inline void init()
    {
        _min.set(DBL_MAX,
            DBL_MAX,
            DBL_MAX);
        _max.set(-DBL_MAX,
            -DBL_MAX,
            -DBL_MAX);
    }

    inline bool operator == (const BoundingBox& rhs) const { return _min == rhs._min && _max == rhs._max; }
    inline bool operator != (const BoundingBox& rhs) const { return _min != rhs._min || _max != rhs._max; }

    /** Returns true if the bounding box extents are valid, false otherwise. */
    inline bool valid() const
    {
        return _max.x() >= _min.x() && _max.y() >= _min.y() && _max.z() >= _min.z();
    }

    /** Sets the bounding box extents. */
    inline void set(double xmin, double ymin, double zmin,
        double xmax, double ymax, double zmax)
    {
        _min.set(xmin, ymin, zmin);
        _max.set(xmax, ymax, zmax);
    }

    /** Sets the bounding box extents. */
    inline void set(const Vec3d& min, const Vec3d& max)
    {
        _min = min;
        _max = max;
    }


    inline double& xMin() { return _min.x(); }
    inline double xMin() const { return _min.x(); }

    inline double& yMin() { return _min.y(); }
    inline double yMin() const { return _min.y(); }

    inline double& zMin() { return _min.z(); }
    inline double zMin() const { return _min.z(); }

    inline double& xMax() { return _max.x(); }
    inline double xMax() const { return _max.x(); }

    inline double& yMax() { return _max.y(); }
    inline double yMax() const { return _max.y(); }

    inline double& zMax() { return _max.z(); }
    inline double zMax() const { return _max.z(); }

    /** Calculates and returns the bounding box center. */
    inline const Vec3d center() const
    {
        return (_min + _max) * 0.5;
    }

    /** Calculates and returns the bounding box radius. */
    inline double radius() const
    {
        return sqrt(radius2());
    }

    /** Calculates and returns the squared length of the bounding box radius.
      * Note, radius2() is faster to calculate than radius(). */
    inline double radius2() const
    {
        return 0.25 * ((_max - _min).length2());
    }

    /** Returns a specific corner of the bounding box.
      * pos specifies the corner as a number between 0 and 7.
      * Each bit selects an axis, X, Y, or Z from least- to
      * most-significant. Unset bits select the minimum value
      * for that axis, and set bits select the maximum. */
    inline const Vec3d corner(unsigned int pos) const
    {
        return Vec3d(pos & 1 ? _max.x() : _min.x(), pos & 2 ? _max.y() : _min.y(), pos & 4 ? _max.z() : _min.z());
    }

    /** Expands the bounding box to include the given coordinate.
      * If the box is uninitialized, set its min and max extents to v. */
    inline void expandBy(const Vec3d& v)
    {
        if (v.x() < _min.x()) _min.x() = v.x();
        if (v.x() > _max.x()) _max.x() = v.x();

        if (v.y() < _min.y()) _min.y() = v.y();
        if (v.y() > _max.y()) _max.y() = v.y();

        if (v.z() < _min.z()) _min.z() = v.z();
        if (v.z() > _max.z()) _max.z() = v.z();
    }

    /** Expands the bounding box to include the given coordinate.
      * If the box is uninitialized, set its min and max extents to
      * Vec3(x,y,z). */
    inline void expandBy(double x, double y, double z)
    {
        if (x < _min.x()) _min.x() = x;
        if (x > _max.x()) _max.x() = x;

        if (y < _min.y()) _min.y() = y;
        if (y > _max.y()) _max.y() = y;

        if (z < _min.z()) _min.z() = z;
        if (z > _max.z()) _max.z() = z;
    }

    /** Expands this bounding box to include the given bounding box.
      * If this box is uninitialized, set it equal to bb. */
    void expandBy(const BoundingBox& bb)
    {
        if (!bb.valid()) return;

        if (bb._min.x() < _min.x()) _min.x() = bb._min.x();
        if (bb._max.x() > _max.x()) _max.x() = bb._max.x();

        if (bb._min.y() < _min.y()) _min.y() = bb._min.y();
        if (bb._max.y() > _max.y()) _max.y() = bb._max.y();

        if (bb._min.z() < _min.z()) _min.z() = bb._min.z();
        if (bb._max.z() > _max.z()) _max.z() = bb._max.z();
    }

    /** Expands this bounding box to include the given sphere.
      * If this box is uninitialized, set it to include sh. */
    void expandBySphere(const Vec3d& center, double radius)
    {


        if (center.x() - radius < _min.x()) _min.x() = center.x() - radius;
        if (center.x() + radius > _max.x()) _max.x() = center.x() + radius;

        if (center.y() - radius < _min.y()) _min.y() = center.y() - radius;
        if (center.y() + radius > _max.y()) _max.y() = center.y() + radius;

        if (center.z() - radius < _min.z()) _min.z() = center.z() - radius;
        if (center.z() + radius > _max.z()) _max.z() = center.z() + radius;
    }
    inline double maximum(double a, double b) const
    {
        if (a > b)
            return a;
        return b;
    }
    inline double minimum(double a, double b) const
    {
        if (a < b)
            return a;
        return b;
    }

    /** Returns the intersection of this bounding box and the specified bounding box. */
    BoundingBox intersect(const BoundingBox& bb) const
    {
        return BoundingBox(maximum(xMin(), bb.xMin()), maximum(yMin(), bb.yMin()), maximum(zMin(), bb.zMin()),
            minimum(xMax(), bb.xMax()), minimum(yMax(), bb.yMax()), minimum(zMax(), bb.zMax()));

    }

    /** Return true if this bounding box intersects the specified bounding box. */
    bool intersects(const BoundingBox& bb) const
    {
        return maximum(xMin(), bb.xMin()) <= minimum(xMax(), bb.xMax()) &&
            maximum(yMin(), bb.yMin()) <= minimum(yMax(), bb.yMax()) &&
            maximum(zMin(), bb.zMin()) <= minimum(zMax(), bb.zMax());

    }

    /** Returns true if this bounding box contains the specified coordinate. */
    inline bool contains(const Vec3d& v) const
    {
        return valid() &&
            (v.x() >= _min.x() && v.x() <= _max.x()) &&
            (v.y() >= _min.y() && v.y() <= _max.y()) &&
            (v.z() >= _min.z() && v.z() <= _max.z());
    }

    /** Returns true if this bounding box contains the specified coordinate allowing for specific epsilon. */
    inline bool contains(const Vec3d& v, double epsilon) const
    {
        return valid() &&
            ((v.x() + epsilon) >= _min.x() && (v.x() - epsilon) <= _max.x()) &&
            ((v.y() + epsilon) >= _min.y() && (v.y() - epsilon) <= _max.y()) &&
            ((v.z() + epsilon) >= _min.z() && (v.z() - epsilon) <= _max.z());
    }
    inline Vec3d dimension()
    {
        return _max - _min;
    }
};
//////////////////////////
class Quat
{
public:
    double _v[4];    // a four-vector
    inline Quat() { _v[0] = 0.0; _v[1] = 0.0; _v[2] = 0.0; _v[3] = 1.0; }
    inline Quat(double x, double y, double z, double w)
    {
        _v[0] = x;
        _v[1] = y;
        _v[2] = z;
        _v[3] = w;
    }
    /// Length of the quaternion = sqrt( vec . vec )
    double length() const
    {
        return sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3]);
    }

    /// Length of the quaternion = vec . vec
    double length2() const
    {
        return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3];
    }
    void makeRotate(double angle, double x, double y, double z);
    void makeRotate(double angle, const Vec3d& vec);
    void makeRotate(double angle1, const Vec3d& axis1,
        double angle2, const Vec3d& axis2,
        double angle3, const Vec3d& axis3);
    /// Binary multiply
    inline const Quat operator*(const Quat& rhs) const
    {
        return Quat(rhs._v[3] * _v[0] + rhs._v[0] * _v[3] + rhs._v[1] * _v[2] - rhs._v[2] * _v[1],
            rhs._v[3] * _v[1] - rhs._v[0] * _v[2] + rhs._v[1] * _v[3] + rhs._v[2] * _v[0],
            rhs._v[3] * _v[2] + rhs._v[0] * _v[1] - rhs._v[1] * _v[0] + rhs._v[2] * _v[3],
            rhs._v[3] * _v[3] - rhs._v[0] * _v[0] - rhs._v[1] * _v[1] - rhs._v[2] * _v[2]);
    }

    /// Unary multiply
    inline Quat& operator*=(const Quat& rhs)
    {
        double x = rhs._v[3] * _v[0] + rhs._v[0] * _v[3] + rhs._v[1] * _v[2] - rhs._v[2] * _v[1];
        double y = rhs._v[3] * _v[1] - rhs._v[0] * _v[2] + rhs._v[1] * _v[3] + rhs._v[2] * _v[0];
        double z = rhs._v[3] * _v[2] + rhs._v[0] * _v[1] - rhs._v[1] * _v[0] + rhs._v[2] * _v[3];
        _v[3] = rhs._v[3] * _v[3] - rhs._v[0] * _v[0] - rhs._v[1] * _v[1] - rhs._v[2] * _v[2];

        _v[2] = z;
        _v[1] = y;
        _v[0] = x;

        return (*this);            // enable nesting
    }
};
//----------------------------------------------------
template <class T>
inline T SGL_ABS(T a)
{
    return (a >= 0 ? a : -a);
}

#ifndef SGL_SWAP
#define SGL_SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))
#endif

class Matrixd
{
public:
    inline Matrixd() { makeIdentity(); }
    inline Matrixd(const Matrixd& mat) { set(mat.ptr()); }
    ~Matrixd() {};
    void makeIdentity();
    inline double& operator()(int row, int col) { return _mat[row][col]; }
    inline double operator()(int row, int col) const { return _mat[row][col]; }

    inline void set(double const* const ptr)
    {
        double* local_ptr = (double*)_mat;
        for (int i = 0; i < 16; ++i) local_ptr[i] = (double)ptr[i];
    }
    double* ptr() { return (double*)_mat; }
    const double* ptr() const { return (const double*)_mat; }
    void makeTranslate(double x, double y, double z);
    void makeTranslate(Vec3d& v);
    void getTranslate(Vec3d& v) const;
    void makeScale(double x, double y, double z);
  
    inline Vec3d postMult(const Vec3d& v) const
    {
        double d = 1.0 / (_mat[3][0] * v.x() + _mat[3][1] * v.y() + _mat[3][2] * v.z() + _mat[3][3]);
        return Vec3d((_mat[0][0] * v.x() + _mat[0][1] * v.y() + _mat[0][2] * v.z() + _mat[0][3]) * d,
            (_mat[1][0] * v.x() + _mat[1][1] * v.y() + _mat[1][2] * v.z() + _mat[1][3]) * d,
            (_mat[2][0] * v.x() + _mat[2][1] * v.y() + _mat[2][2] * v.z() + _mat[2][3]) * d);
    }
    inline Vec3d preMult(const Vec3d& v) const
    {
        double d = 1.0f / (_mat[0][3] * v.x() + _mat[1][3] * v.y() + _mat[2][3] * v.z() + _mat[3][3]);
        return Vec3d((_mat[0][0] * v.x() + _mat[1][0] * v.y() + _mat[2][0] * v.z() + _mat[3][0]) * d,
            (_mat[0][1] * v.x() + _mat[1][1] * v.y() + _mat[2][1] * v.z() + _mat[3][1]) * d,
            (_mat[0][2] * v.x() + _mat[1][2] * v.y() + _mat[2][2] * v.z() + _mat[3][2]) * d);
    }

    inline Vec3d operator* (const Vec3d& v) const
    {
        return postMult(v);
    }
    inline Matrixd transpose() const
    {
        Matrixd mat;
        for (int col = 0; col < 4; col++)
        {
            for (int row = 0; row < 4; row++)
                mat(col, row) = _mat[row][col];
        }
        return mat;
    }
    void mult(const Matrixd& lhs, const Matrixd& rhs);
    void preMult(const Matrixd& other);
    void postMult(const Matrixd& other);
/******************************************
Matrix inversion technique:
Given a matrix mat, we want to invert it.
mat = [ r00 r01 r02 a
        r10 r11 r12 b
        r20 r21 r22 c
        tx  ty  tz  d ]
We note that this matrix can be split into three matrices.
mat = rot * trans * corr, where rot is rotation part, trans is translation part, and corr is the correction due to perspective (if any).
rot = [ r00 r01 r02 0
        r10 r11 r12 0
        r20 r21 r22 0
        0   0   0   1 ]
trans = [ 1  0  0  0
          0  1  0  0
          0  0  1  0
          tx ty tz 1 ]
corr = [ 1 0 0 px
         0 1 0 py
         0 0 1 pz
         0 0 0 s ]
where the elements of corr are obtained from linear combinations of the elements of rot, trans, and mat.
So the inverse is mat' = (trans * corr)' * rot', where rot' must be computed the traditional way, which is easy since it is only a 3x3 matrix.
This problem is simplified if [px py pz s] = [0 0 0 1], which will happen if mat was composed only of rotations, scales, and translations (which is common).  In this case, we can ignore corr entirely which saves on a lot of computations.
******************************************/
    /** Get the inverse matrix. */    
    bool getInverseMatrix(const Matrixd& mat);    
    void makeRotate(double angle1, const Vec3d& axis1,
        double angle2, const Vec3d& axis2,
        double angle3, const Vec3d& axis3);
    void setRotate(const Quat& q);
protected:
    double _mat[4][4];
};

#endif