#ifndef __IFC_READER_ELLIPSOLIDMODEL_H
#define __IFC_READER_ELLIPSOLIDMODEL_H
#include "Vec3d.h"

class CesiumEllipsoidModel
    {
    public:
        double WGS_84_RADIUS_EQUATOR;
        double WGS_84_RADIUS_POLAR;
        /** WGS_84 is a common representation of the earth's spheroid */
        CesiumEllipsoidModel()
        {
            WGS_84_RADIUS_EQUATOR = 6378137.0;
            //WGS_84_RADIUS_POLAR = 6378137.0;
            WGS_84_RADIUS_POLAR = 6356752.3142;
            _radiusEquator = WGS_84_RADIUS_EQUATOR;
            _radiusPolar = WGS_84_RADIUS_POLAR;
            computeCoefficients();
        }

        //CesiumEllipsoidModel(const CesiumEllipsoidModel& et,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
        //    Object(et,copyop),
        //    _radiusEquator(et._radiusEquator),
        //    _radiusPolar(et._radiusPolar) { computeCoefficients(); }

        //META_Object(osg,CesiumEllipsoidModel);

        void setRadiusEquator(double radius) { _radiusEquator = radius; computeCoefficients(); }
        double getRadiusEquator() const { return _radiusEquator; }

        void setRadiusPolar(double radius) { _radiusPolar = radius; computeCoefficients(); }
        double getRadiusPolar() const { return _radiusPolar; }

        void convertLatLongHeightToXYZ(double latitude, double longitude, double height,
            double& X, double& Y, double& Z) const;

        void convertXYZToLatLongHeight(double X, double Y, double Z,
            double& latitude, double& longitude, double& height) const;

        void computeLocalToWorldTransformFromLatLongHeight(double latitude, double longitude, double height, Matrixd& localToWorld) const;        
        void computerWoldToLocalTransformFromLatLonHeight(double latitude, double longitude, double height, Matrixd& worldTolocal, Matrixd& localToWorld) const;
        
        void computeLocalToWorldTransformFromXYZ(double X, double Y, double Z, Matrixd& localToWorld) const;

        void computeCoordinateFrame(double latitude, double longitude, Matrixd& localToWorld) const;

        Vec3d computeLocalUpVector(double X, double Y, double Z) const;

        // Convenience method for determining if CesiumEllipsoidModel is a stock WGS84 ellipsoid
        bool isWGS84() const { return(_radiusEquator == WGS_84_RADIUS_EQUATOR && _radiusPolar == WGS_84_RADIUS_POLAR); }

        // Compares two CesiumEllipsoidModel by comparing critical internal values.
        // Ignores _eccentricitySquared since it's just a cached value derived from
        // the _radiusEquator and _radiusPolar members.
        friend bool operator == (const CesiumEllipsoidModel& e1, const CesiumEllipsoidModel& e2) { return(e1._radiusEquator == e2._radiusEquator && e1._radiusPolar == e2._radiusPolar); }


    protected:

        void computeCoefficients()
        {
            double flattening = (_radiusEquator - _radiusPolar) / _radiusEquator;
            _eccentricitySquared = 2 * flattening - flattening * flattening;
        }

        double _radiusEquator;
        double _radiusPolar;
        double _eccentricitySquared;

    };
    /** CoordinateFrame encapsulates the orientation of east, north and up.*/
    typedef Matrixd CoordinateFrame;
    ///////////////////////////////////////////////////////////////////////////////
    typedef double array_type[3];
    typedef double matrix_type[3][3];

    /*! Primary class for CartographicConversions */
    class CartographicConversions {

    public:
        CartographicConversions();
        ~CartographicConversions();

        /*! Convert to/from ENU/LLA (requires reference LLA) */
        bool enu2lla(double lla[3], const double enu[3], const double ref_lla[3]);
        bool lla2enu(double enu[3], const double lla[3], const double ref_lla[3]);

        /*! Convert to/from ECEF/LLA */
        bool xyz2lla(double lla[3], const double xyz[3]);
        bool lla2xyz(double xyz[3], const double lla[3]);

        /*! Convert to/from ENU/ECEF (requires reference LLA) */
        bool enu2xyz(double xyz[3], const double enu[3], const double ref_lla[3]);
        bool xyz2enu(double enu[3], const double xyz[3], const double ref_lla[3]);

        /*! Convert velocities (or delta positions) to/from ENU/ECEF (requires reference LLA) */
        void enu2xyz_vel(double xyz_vel[3], const double enu_vel[3], const double ref_lla[3]);
        void xyz2enu_vel(double enu_vel[3], const double xyz_vel[3], const double ref_lla[3]);

        /*! Convert position/velocity covariance to/from ENU/ECEF (requires reference LLA) */
        void enu2xyz_cov(double xyz_Cov[3][3], const double enu_Cov[3][3], const double ref_lla[3]);
        void xyz2enu_cov(double enu_Cov[3][3], const double xyz_Cov[3][3], const double ref_lla[3]);

        void enu2xyz_cov(double xyz_cov[9], const double enu_cov[9], const double ref_lla[3]);
        void xyz2enu_cov(double enu_cov[9], const double xyz_cov[9], const double ref_lla[3]);

    private:

        /*! Rotation matrix about a given axis */
        void rot(double R[3][3], const double angle, const int axis);

        /*! Rotation matrix from ECEF to ENU frame */
        void rot3d(double R[3][3], const double reflat, const double reflon);

        /*! Multiply 3x3 matrix times another 3x3 matrix C=AB */
        void matrixMultiply(double C[3][3], const double A[3][3], const double B[3][3]);

        /*! Multiply 3x3 matrix times a 3x1 vector c=Ab */
        void matrixMultiply(double c[3], const double A[3][3], const double b[3]);

        /*! Transpose a 3x3 matrix At = A' */
        void transposeMatrix(double At[3][3], const double A[3][3]);

    };
#endif
