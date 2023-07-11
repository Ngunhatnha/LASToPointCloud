#include <stdio.h>
#include <iostream>
#include "EllipsoidModel.h"
//using namespace osg;
void CesiumEllipsoidModel::convertLatLongHeightToXYZ(double latitude, double longitude, double height,
	double& X, double& Y, double& Z) const
{
	// for details on maths see http://www.colorado.edu/geography/gcraft/notes/datum/gif/llhxyz.gif
	double sin_latitude = sin(latitude);
	double cos_latitude = cos(latitude);
	double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);
	X = (N + height) * cos_latitude * cos(longitude);
	Y = (N + height) * cos_latitude * sin(longitude);
	Z = (N * (1 - _eccentricitySquared) + height) * sin_latitude;
}


void CesiumEllipsoidModel::convertXYZToLatLongHeight(double X, double Y, double Z,
	double& latitude, double& longitude, double& height) const
{
	// http://www.colorado.edu/geography/gcraft/notes/datum/gif/xyzllh.gif
	double p = sqrt(X * X + Y * Y);
	double theta = atan2(Z * _radiusEquator, (p * _radiusPolar));
	double eDashSquared = (_radiusEquator * _radiusEquator - _radiusPolar * _radiusPolar) /
		(_radiusPolar * _radiusPolar);

	double sin_theta = sin(theta);
	double cos_theta = cos(theta);

	latitude = atan((Z + eDashSquared * _radiusPolar * sin_theta * sin_theta * sin_theta) /
		(p - _eccentricitySquared * _radiusEquator * cos_theta * cos_theta * cos_theta));
	longitude = atan2(Y, X);

	double sin_latitude = sin(latitude);
	double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude * sin_latitude);

	height = p / cos(latitude) - N;
}
void CesiumEllipsoidModel::computerWoldToLocalTransformFromLatLonHeight(double latitude, double longitude, double height, Matrixd& worldTolocal, Matrixd& localToWorld) const
{
		double sin_lamda = sin(longitude);
		double cos_lamda = cos(longitude);
		double sin_phi = sin(latitude);
		double cos_phi = cos(latitude);
		// row 0
		worldTolocal(0, 0) = -sin_lamda;
		worldTolocal(0, 1) = cos_lamda;
		worldTolocal(0, 2) = 0.0;
		worldTolocal(0, 3) = 0.0;
		// row 1
		worldTolocal(1, 0) = -sin_phi * cos_lamda;
		worldTolocal(1, 1) = -sin_phi * sin_lamda;
		worldTolocal(1, 2) = cos_phi;
		worldTolocal(1, 3) = 0.0;
		// row 2
		worldTolocal(2, 0) = cos_phi * cos_lamda;
		worldTolocal(2, 1) = cos_phi * sin_lamda;
		worldTolocal(2, 2) = sin_phi;
		worldTolocal(2, 3) = 0.0;
		// row 3
		worldTolocal(3, 0) = 0.0;
		worldTolocal(3, 1) = 0.0;
		worldTolocal(3, 2) = 0.0;
		worldTolocal(3, 3) = 1.0;

		/////////// localToWorld
		localToWorld(0, 0) = -sin_lamda;
		localToWorld(0, 1) = -sin_phi * cos_lamda;
		localToWorld(0, 2) = cos_phi * cos_lamda;
		localToWorld(0, 3) = 0.0;

		localToWorld(1, 0) = cos_lamda;
		localToWorld(1, 1) = -sin_phi * sin_lamda;
		localToWorld(1, 2) = cos_phi * sin_lamda;
		localToWorld(1, 3) = 0.0;

		localToWorld(2, 0) = 0;
		localToWorld(2, 1) = cos_phi;
		localToWorld(2, 2) = sin_phi;
		localToWorld(2, 3) = 0.0;

		localToWorld(3, 0) = 0;
		localToWorld(3, 1) = 0;
		localToWorld(3, 2) = 0;
		localToWorld(3, 3) = 1.0;

}
void CesiumEllipsoidModel::computeLocalToWorldTransformFromLatLongHeight(double latitude, double longitude, double height, Matrixd& localToWorld) const
{
	double X, Y, Z;
	convertLatLongHeightToXYZ(latitude, longitude, height, X, Y, Z);

	localToWorld.makeTranslate(X, Y, Z);
	computeCoordinateFrame(latitude, longitude, localToWorld);
}

void CesiumEllipsoidModel::computeLocalToWorldTransformFromXYZ(double X, double Y, double Z, Matrixd& localToWorld) const
{
	double  latitude, longitude, height;
	convertXYZToLatLongHeight(X, Y, Z, latitude, longitude, height);

	localToWorld.makeTranslate(X, Y, Z);
	computeCoordinateFrame(latitude, longitude, localToWorld);
}

void CesiumEllipsoidModel::computeCoordinateFrame(double latitude, double longitude, Matrixd& localToWorld) const
{
	// Compute up vector
	Vec3d    up(cos(longitude) * cos(latitude), sin(longitude) * cos(latitude), sin(latitude));

	// Compute east vector
	Vec3d    east(-sin(longitude), cos(longitude), 0);

	// Compute north vector = outer product up x east
	Vec3d    north = up ^ east;

	// set matrix
	localToWorld(0, 0) = east[0];
	localToWorld(0, 1) = east[1];
	localToWorld(0, 2) = east[2];

	localToWorld(1, 0) = north[0];
	localToWorld(1, 1) = north[1];
	localToWorld(1, 2) = north[2];

	localToWorld(2, 0) = up[0];
	localToWorld(2, 1) = up[1];
	localToWorld(2, 2) = up[2];
}

Vec3d CesiumEllipsoidModel::computeLocalUpVector(double X, double Y, double Z) const
{
	// Note latitude is angle between normal to ellipsoid surface and XY-plane
	double  latitude;
	double  longitude;
	double  altitude;
	convertXYZToLatLongHeight(X, Y, Z, latitude, longitude, altitude);

	// Compute up vector
	return Vec3d(cos(longitude) * cos(latitude),
		sin(longitude) * cos(latitude),
		sin(latitude));
}
///////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------
// Class Constructor
//------------------------------------------------------------------------------------------------
CartographicConversions::CartographicConversions()
{
}

//------------------------------------------------------------------------------------------------
// Class Destructor
//------------------------------------------------------------------------------------------------
CartographicConversions::~CartographicConversions()
{
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::enu2lla [Public]  --- convert from (East,North,Up) to (Lat,Long,Alt)
//------------------------------------------------------------------------------------------------
bool CartographicConversions::enu2lla(double lla[3], const double enu[3], const double ref_lla[3]) {

    double ref_xyz[3], diff_xyz[3], xyz[3], R[3][3], Rt[3][3];

    // First, calculate the xyz of reflat, reflon, refalt
    if (!lla2xyz(ref_xyz, ref_lla))
        return 0;

    rot3d(R, ref_lla[0], ref_lla[1]);

    transposeMatrix(Rt, R);

    matrixMultiply(diff_xyz, Rt, enu);

    xyz[0] = diff_xyz[0] + ref_xyz[0];
    xyz[1] = diff_xyz[1] + ref_xyz[1];
    xyz[2] = diff_xyz[2] + ref_xyz[2];

    if (!xyz2lla(lla, xyz))
        return 0;

    return 1;
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::lla2enu [Public]  --- convert from (Lat,Long,Alt) to (East,North,Up)
//------------------------------------------------------------------------------------------------
bool CartographicConversions::lla2enu(double enu[3], const double lla[3], const double ref_lla[3]) {

    double xyz[3];

    if (!lla2xyz(xyz, lla))
        return 0;

    if (!xyz2enu(enu, xyz, ref_lla))
        return 0;

    return 1;
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::xyz2lla [Public]  --- convert from (ECEF X, ECEF Y, ECEF Z) to (Lat,Long,Alt)
//------------------------------------------------------------------------------------------------
bool CartographicConversions::xyz2lla(double lla[3], const double xyz[3]) {

    //This dual-variable iteration seems to be 7 or 8 times faster than
    //a one-variable (in latitude only) iteration.  AKB 7/17/95

    double A_EARTH = 6378137.0;
    double flattening = 1.0 / 298.257223563;
    double NAV_E2 = (2.0 - flattening) * flattening; // also e^2
    double rad2deg = 180.0 / M_PI;

    if ((xyz[0] == 0.0) & (xyz[1] == 0.0)) {
        lla[1] = 0.0;
    }
    else {
        lla[1] = atan2(xyz[1], xyz[0]) * rad2deg;
    }

    if ((xyz[0] == 0.0) & (xyz[1] == 0.0) & (xyz[2] == 0.0))
    {
        //printf("{\"Status\":\"Warning\",\"xyz2lla\":\"WGS xyz at center of earth\"}\n");        
        return 0;
    }
    else {
        // Make initial lat and alt guesses based on spherical earth.
        double rhosqrd = xyz[0] * xyz[0] + xyz[1] * xyz[1];
        double rho = sqrt(rhosqrd);
        double templat = atan2(xyz[2], rho);
        double tempalt = sqrt(rhosqrd + xyz[2] * xyz[2]) - A_EARTH;
        double rhoerror = 1000.0;
        double zerror = 1000.0;

        int iter = 0; // number of iterations

        //      %  Newton's method iteration on templat and tempalt makes
        //      %   the residuals on rho and z progressively smaller.  Loop
        //      %   is implemented as a 'while' instead of a 'do' to simplify
        //      %   porting to MATLAB

        while ((abs(rhoerror) > 1e-6) | (abs(zerror) > 1e-6)) {
            double slat = sin(templat);
            double clat = cos(templat);
            double q = 1.0 - NAV_E2 * slat * slat;
            double r_n = A_EARTH / sqrt(q);
            double drdl = r_n * NAV_E2 * slat * clat / q; // d(r_n)/d(latitutde)

            rhoerror = (r_n + tempalt) * clat - rho;
            zerror = (r_n * (1.0 - NAV_E2) + tempalt) * slat - xyz[2];

            //          %             --                               -- --      --
            //          %             |  drhoerror/dlat  drhoerror/dalt | |  a  b  |
            //                        % Find Jacobian           |                       |=|        |
            //          %             |   dzerror/dlat    dzerror/dalt  | |  c  d  |
            //          %             --                               -- --      --

            double aa = drdl * clat - (r_n + tempalt) * slat;
            double bb = clat;
            double cc = (1.0 - NAV_E2) * (drdl * slat + r_n * clat);
            double dd = slat;

            //Apply correction = inv(Jacobian)*errorvector

            double invdet = 1.0 / (aa * dd - bb * cc);
            templat = templat - invdet * (+dd * rhoerror - bb * zerror);
            tempalt = tempalt - invdet * (-cc * rhoerror + aa * zerror);

            iter++;

            if (iter > 20) {
                //printf("{\"Status\":\"Warning\",\"xyz2lla\":\"could not converge\"}\n");                
                return 0;
            }
        }

        lla[0] = templat * rad2deg;
        lla[2] = tempalt;
    }
    return 1;
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::lla2xyz [Public]  --- convert from (Lat,Long,Alt) to (ECEF X, ECEF Y, ECEF Z)
//------------------------------------------------------------------------------------------------
bool CartographicConversions::lla2xyz(double xyz[3], const double lla[3]) {

    if ((lla[0] < -90.0) | (lla[0] > +90.0) | (lla[1] < -180.0) | (lla[1] > +360.0)) 
    {
        //printf("{\"Status\":\"Warning\",\"xyz2lla\":\"WGS lat or WGS lon out of range\"}\n");        
        return 0;
    }

    double A_EARTH = 6378137.0;
    double flattening = 1.0 / 298.257223563;
    double NAV_E2 = (2.0 - flattening) * flattening; // also e^2
    double deg2rad = M_PI / 180.0;

    double slat = sin(lla[0] * deg2rad);
    double clat = cos(lla[0] * deg2rad);
    double r_n = A_EARTH / sqrt(1.0 - NAV_E2 * slat * slat);
    xyz[0] = (r_n + lla[2]) * clat * cos(lla[1] * deg2rad);
    xyz[1] = (r_n + lla[2]) * clat * sin(lla[1] * deg2rad);
    xyz[2] = (r_n * (1.0 - NAV_E2) + lla[2]) * slat;

    return 1;
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::enu2xyz [Public]  --- convert from (East,North,Up) to (ECEF X, ECEF Y, ECEF Z)
//------------------------------------------------------------------------------------------------
bool CartographicConversions::enu2xyz(double xyz[3], const double enu[3], const double ref_lla[3]) {

    double lla[3];

    // first enu2lla
    if (!enu2lla(lla, enu, ref_lla))
        return 0;

    // then lla2xyz
    if (!lla2xyz(xyz, lla))
        return 0;

    return 1;
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::xyz2enu [Public]  --- convert from (ECEF X, ECEF Y, ECEF Z) to (East,North,Up)
//------------------------------------------------------------------------------------------------
bool CartographicConversions::xyz2enu(double enu[3], const double xyz[3], const double ref_lla[3]) {

    double ref_xyz[3], diff_xyz[3], R[3][3];

    // First, calculate the xyz of reflat, reflon, refalt
    if (!lla2xyz(ref_xyz, ref_lla))
        return 0;

    //Difference xyz from reference point
    diff_xyz[0] = xyz[0] - ref_xyz[0];
    diff_xyz[1] = xyz[1] - ref_xyz[1];
    diff_xyz[2] = xyz[2] - ref_xyz[2];

    rot3d(R, ref_lla[0], ref_lla[1]);

    matrixMultiply(enu, R, diff_xyz);

    return 1;
}

//--------------------------------------------------------------------------------------------------------------
// CartographicConversions::xyz2enu_vel [Public]  --- convert velocities from (ECEF X, ECEF Y, ECEF Z) to (East,North,Up)
//--------------------------------------------------------------------------------------------------------------
void CartographicConversions::xyz2enu_vel(double enu_vel[3], const double xyz_vel[3], const double ref_lla[3]) {

    double R[3][3];

    rot3d(R, ref_lla[0], ref_lla[1]);

    matrixMultiply(enu_vel, R, xyz_vel);

}

//--------------------------------------------------------------------------------------------------------------
// CartographicConversions::enu2xyz_vel [Public]  --- convert velocities from (East,North,Up) to (ECEF X, ECEF Y, ECEF Z)
//--------------------------------------------------------------------------------------------------------------
void CartographicConversions::enu2xyz_vel(double xyz_vel[3], const double enu_vel[3], const double ref_lla[3]) {

    double R[3][3], Rt[3][3];

    rot3d(R, ref_lla[0], ref_lla[1]);

    transposeMatrix(Rt, R);

    matrixMultiply(xyz_vel, Rt, enu_vel);

}

//--------------------------------------------------------------------------------------------------------------------------------
// CartographicConversions::xyz2enu_cov [Public]  --- convert position/velocity covariance from (ECEF X, ECEF Y, ECEF Z) to (East,North,Up)
//--------------------------------------------------------------------------------------------------------------------------------
void CartographicConversions::xyz2enu_cov(double enu_Cov[3][3], const double xyz_Cov[3][3], const double ref_lla[3]) {

    double R[3][3], Rt[3][3], Tmp[3][3];

    rot3d(R, ref_lla[0], ref_lla[1]);

    transposeMatrix(Rt, R);

    matrixMultiply(Tmp, xyz_Cov, Rt); // Tmp = xyz_cov*R'

    matrixMultiply(enu_Cov, R, Tmp); // enu_cov = R*xyz_cov*R'

}

void CartographicConversions::xyz2enu_cov(double enu_cov[9], const double xyz_cov[9], const double ref_lla[3]) {

    double xyz_Cov[3][3], enu_Cov[3][3];

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            xyz_Cov[i][j] = xyz_cov[3 * i + j];

    double R[3][3], Rt[3][3], Tmp[3][3];

    rot3d(R, ref_lla[0], ref_lla[1]);

    transposeMatrix(Rt, R);

    matrixMultiply(Tmp, xyz_Cov, Rt);

    matrixMultiply(enu_Cov, R, Tmp);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            enu_cov[3 * i + j] = enu_Cov[i][j];

}

//--------------------------------------------------------------------------------------------------------------------------------
// CartographicConversions::enu2xyz_cov [Public]  --- convert position/velocity covariance from (East,North,Up) to (ECEF X, ECEF Y, ECEF Z)
//--------------------------------------------------------------------------------------------------------------------------------
void CartographicConversions::enu2xyz_cov(double xyz_Cov[3][3], const double enu_Cov[3][3], const double ref_lla[3]) {

    double R[3][3], Rt[3][3], Tmp[3][3];

    rot3d(R, ref_lla[0], ref_lla[1]);

    transposeMatrix(Rt, R);

    matrixMultiply(Tmp, enu_Cov, R);

    matrixMultiply(xyz_Cov, Rt, Tmp);

}

void CartographicConversions::enu2xyz_cov(double xyz_cov[9], const double enu_cov[9], const double ref_lla[3]) {

    double xyz_Cov[3][3], enu_Cov[3][3];

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            enu_Cov[i][j] = enu_cov[3 * i + j];

    double R[3][3], Rt[3][3], Tmp[3][3];

    rot3d(R, ref_lla[0], ref_lla[1]);

    transposeMatrix(Rt, R);

    matrixMultiply(Tmp, enu_Cov, R);

    matrixMultiply(xyz_Cov, Rt, Tmp);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            xyz_cov[3 * i + j] = xyz_Cov[i][j];
}

//--------------------------------------------------------------------------------------------
// CartographicConversions::enu2xyz [Private]  --- return the 3D rotation matrix to/from ECEF/ENU frame
//--------------------------------------------------------------------------------------------
void CartographicConversions::rot3d(double R[3][3], const double reflat, const double reflon)
{
    double R1[3][3], R2[3][3];
    rot(R1, 90 + reflon, 3);
    rot(R2, 90 - reflat, 1);
    matrixMultiply(R, R2, R1);
}

//------------------------------------------------------------------------------------------------
// CartographicConversions::matrixMultiply [Private]  --- Multiply 3x3 matrix times another 3x3 matrix C=AB
//------------------------------------------------------------------------------------------------
void CartographicConversions::matrixMultiply(double C[3][3], const double A[3][3], const double B[3][3]) {

    C[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0];
    C[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1];
    C[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2];
    C[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0];
    C[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1];
    C[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2];
    C[2][0] = A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0];
    C[2][1] = A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1];
    C[2][2] = A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2];

}

//------------------------------------------------------------------------------------------------
// CartographicConversions::matrixMultiply [Private]  --- Multiply 3x3 matrix times a 3x1 vector c=Ab
//------------------------------------------------------------------------------------------------
void CartographicConversions::matrixMultiply(double c[3], const double A[3][3], const double b[3]) {

    c[0] = A[0][0] * b[0] + A[0][1] * b[1] + A[0][2] * b[2];
    c[1] = A[1][0] * b[0] + A[1][1] * b[1] + A[1][2] * b[2];
    c[2] = A[2][0] * b[0] + A[2][1] * b[1] + A[2][2] * b[2];

}

//------------------------------------------------------------------------------------------------
// CartographicConversions::transposeMatrix [Private]  --- transpose a 3x3 matrix At = A'
//------------------------------------------------------------------------------------------------
void CartographicConversions::transposeMatrix(double At[3][3], const double A[3][3]) {

    At[0][0] = A[0][0];
    At[0][1] = A[1][0];
    At[0][2] = A[2][0];
    At[1][0] = A[0][1];
    At[1][1] = A[1][1];
    At[1][2] = A[2][1];
    At[2][0] = A[0][2];
    At[2][1] = A[1][2];
    At[2][2] = A[2][2];

}

//------------------------------------------------------------------------------------------------
// CartographicConversions::rot [Private]  --- rotation matrix
//------------------------------------------------------------------------------------------------
void CartographicConversions::rot(double R[3][3], const double angle, const int axis) {

    double cang = cos(angle * M_PI / 180);
    double sang = sin(angle * M_PI / 180);

    if (axis == 1) {
        R[0][0] = 1;
        R[0][1] = 0;
        R[0][2] = 0;
        R[1][0] = 0;
        R[2][0] = 0;
        R[1][1] = cang;
        R[2][2] = cang;
        R[1][2] = sang;
        R[2][1] = -sang;
    }
    else if (axis == 2) {
        R[0][1] = 0;
        R[1][0] = 0;
        R[1][1] = 1;
        R[1][2] = 0;
        R[2][1] = 0;
        R[0][0] = cang;
        R[2][2] = cang;
        R[0][2] = -sang;
        R[2][0] = sang;
    }
    else if (axis == 3) {
        R[2][0] = 0;
        R[2][1] = 0;
        R[2][2] = 1;
        R[0][2] = 0;
        R[1][2] = 0;
        R[0][0] = cang;
        R[1][1] = cang;
        R[1][0] = -sang;
        R[0][1] = sang;
    }
}

