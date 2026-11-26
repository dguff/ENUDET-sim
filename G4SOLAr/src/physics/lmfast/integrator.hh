#ifndef INTEGRATOR_HH
#define INTEGRATOR_HH

#include <stdlib.h>
#include <math.h>
#include <vector>

#include "RSED.hh"
#include "geometry.hh" // 2D surface geometry model (EFace, surface, geometry, face helpers, SegmentGeometry)

struct region
  {
    double xl, xu, yl, yu, zl, zu;
  };

class integrator{

  public:
  // rsBin: uniform RSED grid spacing [mm] (same for all 3 axes), default 100
  integrator(double (&boundaries)[3][2], double (&origin)[3], double (&wallpoint)[3], bool RSb, double rsBin = 100.);
  ~integrator(){};

  //geometry (2D surfaces, see geometry.hh)
  geometry& GetBaseGeometry(){return baseGeo;}
  geometry& GetWorkGeometry(){return workGeo;}
  void SetBaseGeometry(const geometry& g); // set base geometry, enforce kNFaces collections

  //RSED
  void computeRSED();

  // test method
  double computeTestSurfaceVisibility();
    double integrateSurface( int face, const surface &surf );
    double integrateFromSurface( const double (&point)[3] );
    double integrateFromSurfaceSegment( int face, const surface &surf, const double (&point)[3] );

  double integrateSurfacePoint( const double (&point)[3] );

  // integration hierarchy: upper calls lower
  double integrateFromVolume( const double (&point)[3] );
  double integrateRegionWP( const region &r, const double (&point)[3] );
  double integrateRegion( const region &r, const double (&point)[3] );
  double integrateRS( const double (&point)[3] );

  // integration regions computations
  void compute_quadrants();
  void compute_octants( const region &r, std::vector<region> &regOut, const double (&point)[3] );
  void compute_regions( const region &r, std::vector<region> &regOut, const double (&point)[3] );

  void SetWallPos( double x, double y, double z);
  void SetOrigin ( double x, double y, double z);

  void EnableRS( bool RS );

  void test_octants( const double (&point)[3] );
  void print_regions( const std::vector<region> &r );

  private:

  double fboundaries[3][2];
  double forigin[3];
  double fwallpoint[3];
  double fsurfacepoint[3];

  //geometry
  geometry baseGeo;  // imported geometry (simplified detector: kNFaces surface collections, fixed face order)
  geometry workGeo;  // segmented geometry to compute surface visibilities

  region r_det;

  // region fr;
  region rs;

  std::vector<region> regions; // integration regions [n][coord][limits]
  std::vector<region> regions_RS; // integration regions [n][coord][limits]

  bool RSb_;

  double lambda_abs = 1.0/2800;
  double lambda_RS  = 1.0/1000;
    double lambda_tot;

  double mu_abs = 1.0/2800;
  double mu_RS  = 1.0/1000;
    double mu_tot;

  RSED rsed;
};

#endif
