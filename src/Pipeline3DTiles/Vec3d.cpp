#include <limits>
#include <math.h>
#include "Vec3d.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implement  methods.
//
#define SET_ROW(row, v1, v2, v3, v4 )    \
    _mat[(row)][0] = (v1); \
    _mat[(row)][1] = (v2); \
    _mat[(row)][2] = (v3); \
    _mat[(row)][3] = (v4);
#define INNER_PRODUCT(a,b,r,c) \
     ((a)._mat[r][0] * (b)._mat[0][c]) \
    +((a)._mat[r][1] * (b)._mat[1][c]) \
    +((a)._mat[r][2] * (b)._mat[2][c]) \
    +((a)._mat[r][3] * (b)._mat[3][c])
#define QX  q._v[0]
#define QY  q._v[1]
#define QZ  q._v[2]
#define QW  q._v[3]
/// constants ------------------------------------------------------------------
const Vec3d& Vec3d::ZERO()
{
    static const Vec3d k(0.0,0.0,0.0);
    return k;
}


const Vec3d& Vec3d::HALF()
{
    static const Vec3d k(0.5,0.5,0.5);
    return k;
}


const Vec3d& Vec3d::ONE()
{
    static const Vec3d k(1.0,1.0,1.0);
    return k;
}

void Matrixd::makeIdentity()
{
	SET_ROW(0, 1, 0, 0, 0)
		SET_ROW(1, 0, 1, 0, 0)
		SET_ROW(2, 0, 0, 1, 0)
		SET_ROW(3, 0, 0, 0, 1)
}
void Matrixd::makeTranslate(double x, double y, double z)
{
	    SET_ROW(0, 1, 0, 0, 0)
		SET_ROW(1, 0, 1, 0, 0)
		SET_ROW(2, 0, 0, 1, 0)
		SET_ROW(3, x, y, z, 1)
}
void Matrixd::makeTranslate(Vec3d& v)
{
    makeTranslate(v[0],v[1],v[2]);
}
void Matrixd::getTranslate(Vec3d& v) const
{
    v[0] = _mat[3][0];
    v[1] = _mat[3][1];
    v[2] = _mat[3][2];
}

void Matrixd::makeScale(double x, double y, double z)
{
       SET_ROW(0, x, 0, 0, 0)
        SET_ROW(1, 0, y, 0, 0)
        SET_ROW(2, 0, 0, z, 0)
        SET_ROW(3, 0, 0, 0, 1)
}
void Matrixd::mult(const Matrixd& lhs, const Matrixd& rhs)
{
    if (&lhs == this)
    {
        postMult(rhs);
        return;
    }
    if (&rhs == this)
    {
        preMult(lhs);
        return;
    }

    // PRECONDITION: We assume neither &lhs nor &rhs == this
    // if it did, use preMult or postMult instead
    _mat[0][0] = INNER_PRODUCT(lhs, rhs, 0, 0);
    _mat[0][1] = INNER_PRODUCT(lhs, rhs, 0, 1);
    _mat[0][2] = INNER_PRODUCT(lhs, rhs, 0, 2);
    _mat[0][3] = INNER_PRODUCT(lhs, rhs, 0, 3);
    _mat[1][0] = INNER_PRODUCT(lhs, rhs, 1, 0);
    _mat[1][1] = INNER_PRODUCT(lhs, rhs, 1, 1);
    _mat[1][2] = INNER_PRODUCT(lhs, rhs, 1, 2);
    _mat[1][3] = INNER_PRODUCT(lhs, rhs, 1, 3);
    _mat[2][0] = INNER_PRODUCT(lhs, rhs, 2, 0);
    _mat[2][1] = INNER_PRODUCT(lhs, rhs, 2, 1);
    _mat[2][2] = INNER_PRODUCT(lhs, rhs, 2, 2);
    _mat[2][3] = INNER_PRODUCT(lhs, rhs, 2, 3);
    _mat[3][0] = INNER_PRODUCT(lhs, rhs, 3, 0);
    _mat[3][1] = INNER_PRODUCT(lhs, rhs, 3, 1);
    _mat[3][2] = INNER_PRODUCT(lhs, rhs, 3, 2);
    _mat[3][3] = INNER_PRODUCT(lhs, rhs, 3, 3);
}
void Matrixd::preMult(const Matrixd& other)
{
    // brute force method requiring a copy
    //Matrix_implementation tmp(other* *this);
    // *this = tmp;

    // more efficient method just use a value_type[4] for temporary storage.
    double t[4];
    for (int col = 0; col < 4; ++col) {
        t[0] = INNER_PRODUCT(other, *this, 0, col);
        t[1] = INNER_PRODUCT(other, *this, 1, col);
        t[2] = INNER_PRODUCT(other, *this, 2, col);
        t[3] = INNER_PRODUCT(other, *this, 3, col);
        _mat[0][col] = t[0];
        _mat[1][col] = t[1];
        _mat[2][col] = t[2];
        _mat[3][col] = t[3];
    }

}

void Matrixd::postMult(const Matrixd& other)
{
    // brute force method requiring a copy
    //Matrix_implementation tmp(*this * other);
    // *this = tmp;

    // more efficient method just use a value_type[4] for temporary storage.
    double t[4];
    for (int row = 0; row < 4; ++row)
    {
        t[0] = INNER_PRODUCT(*this, other, row, 0);
        t[1] = INNER_PRODUCT(*this, other, row, 1);
        t[2] = INNER_PRODUCT(*this, other, row, 2);
        t[3] = INNER_PRODUCT(*this, other, row, 3);
        SET_ROW(row, t[0], t[1], t[2], t[3])
    }
}
#undef INNER_PRODUCT
bool Matrixd::getInverseMatrix(const Matrixd& mat)
{
    if (&mat == this) {
        Matrixd tm(mat);
        return getInverseMatrix(tm);
    }

    unsigned int indxc[4], indxr[4], ipiv[4];
    unsigned int i, j, k, l, ll;
    unsigned int icol = 0;
    unsigned int irow = 0;
    double temp, pivinv, dum, big;

    // copy in place this may be unnecessary
    *this = mat;

    for (j = 0; j < 4; j++) ipiv[j] = 0;

    for (i = 0; i < 4; i++)
    {
        big = 0.0;
        for (j = 0; j < 4; j++)
            if (ipiv[j] != 1)
                for (k = 0; k < 4; k++)
                {
                    if (ipiv[k] == 0)
                    {
                        if (SGL_ABS(operator()(j, k)) >= big)
                        {
                            big = SGL_ABS(operator()(j, k));
                            irow = j;
                            icol = k;
                        }
                    }
                    else if (ipiv[k] > 1)
                        return false;
                }
        ++(ipiv[icol]);
        if (irow != icol)
            for (l = 0; l < 4; l++) SGL_SWAP(operator()(irow, l),
                operator()(icol, l),
                temp);

        indxr[i] = irow;
        indxc[i] = icol;
        if (operator()(icol, icol) == 0)
            return false;

        pivinv = 1.0 / operator()(icol, icol);
        operator()(icol, icol) = 1;
        for (l = 0; l < 4; l++) operator()(icol, l) *= pivinv;
        for (ll = 0; ll < 4; ll++)
            if (ll != icol)
            {
                dum = operator()(ll, icol);
                operator()(ll, icol) = 0;
                for (l = 0; l < 4; l++) operator()(ll, l) -= operator()(icol, l) * dum;
            }
    }
    for (int lx = 4; lx > 0; --lx)
    {
        if (indxr[lx - 1] != indxc[lx - 1])
            for (k = 0; k < 4; k++) SGL_SWAP(operator()(k, indxr[lx - 1]),
                operator()(k, indxc[lx - 1]), temp);
    }

    return true;
}
void Matrixd::setRotate(const Quat& q)
{
    double length2 = q.length2();
    if (fabs(length2) <= std::numeric_limits<double>::min())
    {
        _mat[0][0] = 0.0; _mat[1][0] = 0.0; _mat[2][0] = 0.0;
        _mat[0][1] = 0.0; _mat[1][1] = 0.0; _mat[2][1] = 0.0;
        _mat[0][2] = 0.0; _mat[1][2] = 0.0; _mat[2][2] = 0.0;
    }
    else
    {
        double rlength2;
        // normalize quat if required.
        // We can avoid the expensive sqrt in this case since all 'coefficients' below are products of two q components.
        // That is a square of a square root, so it is possible to avoid that
        if (length2 != 1.0)
        {
            rlength2 = 2.0 / length2;
        }
        else
        {
            rlength2 = 2.0;
        }

        // Source: Gamasutra, Rotating Objects Using Quaternions
        //
        //http://www.gamasutra.com/features/19980703/quaternions_01.htm

        double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

        // calculate coefficients
        x2 = rlength2 * QX;
        y2 = rlength2 * QY;
        z2 = rlength2 * QZ;

        xx = QX * x2;
        xy = QX * y2;
        xz = QX * z2;

        yy = QY * y2;
        yz = QY * z2;
        zz = QZ * z2;

        wx = QW * x2;
        wy = QW * y2;
        wz = QW * z2;

        // Note.  Gamasutra gets the matrix assignments inverted, resulting
        // in left-handed rotations, which is contrary to OpenGL and OSG's
        // methodology.  The matrix assignment has been altered in the next
        // few lines of code to do the right thing.
        // Don Burns - Oct 13, 2001
        _mat[0][0] = 1.0 - (yy + zz);
        _mat[1][0] = xy - wz;
        _mat[2][0] = xz + wy;


        _mat[0][1] = xy + wz;
        _mat[1][1] = 1.0 - (xx + zz);
        _mat[2][1] = yz - wx;

        _mat[0][2] = xz - wy;
        _mat[1][2] = yz + wx;
        _mat[2][2] = 1.0 - (xx + yy);
    }

#if 0
    _mat[0][3] = 0.0;
    _mat[1][3] = 0.0;
    _mat[2][3] = 0.0;

    _mat[3][0] = 0.0;
    _mat[3][1] = 0.0;
    _mat[3][2] = 0.0;
    _mat[3][3] = 1.0;
#endif
}
void Matrixd::makeRotate(double angle1, const Vec3d& axis1,
    double angle2, const Vec3d& axis2,
    double angle3, const Vec3d& axis3)
{
    makeIdentity();
    Quat quat;
    quat.makeRotate(angle1, axis1,
        angle2, axis2,
        angle3, axis3);
    setRotate(quat);
}
///////////////////////////
/// Set the elements of the Quat to represent a rotation of angle
/// (radians) around the axis (x,y,z)
void Quat::makeRotate(double angle, double x, double y, double z)
{
    const double epsilon = 0.0000001;

    double length = sqrt(x * x + y * y + z * z);
    if (length < epsilon)
    {
        // ~zero length axis, so reset rotation to zero.
        *this = Quat();
        return;
    }

    double inversenorm = 1.0 / length;
    double coshalfangle = cos(0.5 * angle);
    double sinhalfangle = sin(0.5 * angle);

    _v[0] = x * sinhalfangle * inversenorm;
    _v[1] = y * sinhalfangle * inversenorm;
    _v[2] = z * sinhalfangle * inversenorm;
    _v[3] = coshalfangle;
}
void Quat::makeRotate(double angle, const Vec3d& vec)
{
    makeRotate(angle, vec[0], vec[1], vec[2]);
}
void Quat::makeRotate(double angle1, const Vec3d& axis1,
    double angle2, const Vec3d& axis2,
    double angle3, const Vec3d& axis3)
{
    Quat q1; q1.makeRotate(angle1, axis1);
    Quat q2; q2.makeRotate(angle2, axis2);
    Quat q3; q3.makeRotate(angle3, axis3);

    *this = q1 * q2 * q3;
}