#ifndef INTEGRATOR_HH
#define INTEGRATOR_HH

#include <stdlib.h>
#include <math.h>
#include <vector>

#include "RSED.hh"

struct region
  {
    double xl, xu, yl, yu, zl, zu;
  };

struct surface
  {
    double xl, xu, yl, yu, zl, zu;
  };

struct surfaceNormal
  {
    double x, y, z;
  };

struct surfaceCollection  // i.e. detector faces
  {
    std::vector<surface> surf;
    // surfaceNormal normal;
  };

struct geometry
  {
    std::vector<surfaceCollection> surfColl;
  };

class integrator{

  public:
  integrator(double (&boundaries)[3][2], double (&origin)[3], double (&wallpoint)[3], bool RSb );
  ~integrator(){};

  //geometry
  void ImportGeometry(); // from where? G4? .json? // from raw to base
  void SetGeometry(); // set base

  geometry& GetBaseGeometry(){return baseGeo;}
  geometry& GetWorkGeometry(){return workGeo;}

  void PrintGeometry( geometry &geo );
    void PrintSurface( surface surf );

  // ?? GetGeometry();
  void SegmentGeometry( const double (&point)[3] ); // from base to work (for each wallpoint?)
    void UpdateGeometry(); // modify workGeo to reduce cost?

  //  GetDet(int detNum);
  double GetDetVis( int detNum );

  //RSED
  void computeRSED();

  // test method
  double computeTestSurfaceVisibility();
    double integrateSurface( const surface &surf);
    double integrateFromSurface( const double (&point)[3] );
    double integrateFromSurfaceSegment( const surface &surf, const double (&point)[3] );

  double integrateSurfacePoint( const double (&point)[3] );

  // integration hierarchy: upper calls lower
  double integrateWallSurface();
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
  // ??&? rawGeo;   // imported geometry (G4, .json, ...)
  geometry baseGeo;  // elaborated geometry (simplified, base for computations)
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