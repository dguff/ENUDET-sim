#include <iostream>

#include "integrator.hh"
#include "GLT.hh"

#include <cmath>

integrator::integrator(double (&boundaries)[3][2], double (&origin)[3], double (&wallpoint)[3], bool RSb, double rsBin)
: rsed(
        boundaries[0][1] - boundaries[0][0],
        boundaries[1][1] - boundaries[1][0],
        boundaries[2][1] - boundaries[2][0],
        rsBin, rsBin, rsBin, // uniform RSED grid spacing [mm] (default 100)
        origin[0] - boundaries[0][0],
        origin[1] - boundaries[1][0],
        origin[2] - boundaries[2][0],
        mu_RS, mu_abs
      )
{
  // fboundaries = boundaries;
  // forigin = origin;
  // fwallpoint = wallpoint;

  forigin[0] = origin[0]; forigin[1] = origin[1]; forigin[2] = origin[2];
  fwallpoint[0] = wallpoint[0]; fwallpoint[1] = wallpoint[1]; fwallpoint[2] = wallpoint[2];

  r_det.xl = fboundaries[0][0] = boundaries[0][0]; r_det.xu = fboundaries[0][1] = boundaries[0][1];
  r_det.yl = fboundaries[1][0] = boundaries[1][0]; r_det.yu = fboundaries[1][1] = boundaries[1][1];
  r_det.zl = fboundaries[2][0] = boundaries[2][0]; r_det.zu = fboundaries[2][1] = boundaries[2][1];

  // fr = {};
  regions.reserve(11);

  rs = {};
  regions_RS.reserve(15);

  RSb_ = RSb;

  lambda_tot = lambda_abs + lambda_RS;

  mu_tot = mu_abs;
  if(RSb_) mu_tot = mu_abs + mu_RS;

}

// ======== GEOMETRY ======== // (2D surface model and helpers: see geometry.hh)

void integrator::SetBaseGeometry(const geometry& g)
{
  baseGeo = g;
  baseGeo.surfColl.resize(kNFaces);
}

// ========================== //

void integrator::SetWallPos( double x, double y, double z)
{
  fwallpoint[0] = x;
  fwallpoint[1] = y;
  fwallpoint[2] = z;
}

void integrator::SetOrigin( double x, double y, double z)
{
  forigin[0] = x;
  forigin[1] = y;
  forigin[2] = z;
}

void integrator::EnableRS( bool RS )
{
  RSb_ = RS;
  if(RSb_)
  {
    mu_tot = mu_abs + mu_RS;
  }else{
    mu_tot = mu_abs;
  }
}

void integrator::print_regions( const std::vector<region> &r )
{
  for(int i=0; i< r.size(); i++)
  {
    std::cout << "region " << i << ": " << r.at(i).xl << " "
                                        << r.at(i).xu << " " 
                                        << r.at(i).yl << " "
                                        << r.at(i).yu << " "
                                        << r.at(i).zl << " "
                                        << r.at(i).zu << "\n";                                 
  }
}

void integrator::computeRSED()
{
  rsed.computeRSEDgrid();
}

void integrator::compute_quadrants()
{
  region r = {};

  // std::cout << "wallpoint xyz = " << fwallpoint[0] << ", " << fwallpoint[1] << ", " << fwallpoint[2] << "\n";

  if( fwallpoint[0]==fboundaries[0][0] || fwallpoint[0]== fboundaries[0][1])
  {
    // same for all 4 quadrants
    r.xl = fboundaries[0][0]; r.xu = fboundaries[0][1];

    // different for each quadrant
    //1
    r.yl = fboundaries[1][0]; r.yu = fwallpoint[1];
    r.zl = fboundaries[2][0]; r.zu = fwallpoint[2];

    if( forigin[1] < fwallpoint[1] && forigin[2] < fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //2
    r.zl = fwallpoint[2]; r.zu = fboundaries[2][1];

    if( forigin[1] < fwallpoint[1] && forigin[2] > fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //3
    r.yl = fwallpoint[1]; r.yu = fboundaries[1][1];
    r.zl = fboundaries[2][0]; r.zu = fwallpoint[2];

    if( forigin[1] > fwallpoint[1] && forigin[2] < fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //4
    r.zl = fwallpoint[2]; r.zu = fboundaries[2][1];

    if( forigin[1] > fwallpoint[1] && forigin[2] > fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }
  }
  else if( fwallpoint[1]==fboundaries[1][0] || fwallpoint[1]== fboundaries[1][1])
  {
    r.yl = fboundaries[1][0]; r.yu = fboundaries[1][1];

    // different for each quadrant
    //1
    r.xl = fboundaries[0][0]; r.xu = fwallpoint[0];
    r.zl = fboundaries[2][0]; r.zu = fwallpoint[2];

    if( forigin[0] < fwallpoint[0] && forigin[2] < fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //2
    r.zl = fwallpoint[2]; r.zu = fboundaries[2][1];

    if( forigin[0] < fwallpoint[0] && forigin[2] > fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //3
    r.xl = fwallpoint[0]; r.xu = fboundaries[0][1];
    r.zl = fboundaries[2][0]; r.zu = fwallpoint[2];

    if( forigin[0] > fwallpoint[0] && forigin[2] < fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //4
    r.zl = fwallpoint[2]; r.zu = fboundaries[2][1];

    if( forigin[0] > fwallpoint[0] && forigin[2] > fwallpoint[2])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

  }
  else if( fwallpoint[2]==fboundaries[2][0] || fwallpoint[2]== fboundaries[2][1])
  {
    r.zl = fboundaries[2][0]; r.zu = fboundaries[2][1];

    // different for each quadrant
    //1
    r.yl = fboundaries[1][0]; r.yu = fwallpoint[1];
    r.xl = fboundaries[0][0]; r.xu = fwallpoint[0];

    if( forigin[1] < fwallpoint[1] && forigin[0] < fwallpoint[0])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //2
    r.xl = fwallpoint[0]; r.zu = fboundaries[0][1];

    if( forigin[1] < fwallpoint[1] && forigin[0] > fwallpoint[0])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //3
    r.yl = fwallpoint[1]; r.yu = fboundaries[1][1];
    r.xl = fboundaries[0][0]; r.xu = fwallpoint[0];

    if( forigin[1] > fwallpoint[1] && forigin[0] < fwallpoint[0])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }

    //4
    r.xl = fwallpoint[0]; r.zu = fboundaries[0][1];

    if( forigin[1] > fwallpoint[1] && forigin[0] > fwallpoint[0])
    {
      compute_octants(r, regions, forigin);
    }else{
      regions.push_back(r);
    }
  }
  else
  {
    std::cout << "point not on surface!\n";
    exit(1);
  }
}


void integrator::compute_octants( const region &r, std::vector<region> &regOut, const double (&point)[3] )
{
  region ro;
    ro.xl = r.xl; ro.xu = r.xu;
    ro.yl = r.yl; ro.yu = r.yu;
    ro.zl = r.zl; ro.zu = r.zu;

  // 1
  if( point[0] != r.xl && point[1] != r.yl && point[2] != r.zl )
  {
    ro.xu = point[0];
    ro.yu = point[1];
    ro.zu = point[2];
    regOut.push_back(ro);
  }  

  // 2
  ro.zl = point[2]; ro.zu = r.zu;
  regOut.push_back(ro);

  // 3
  ro.yl = point[2]; ro.yu = r.yu;
  regOut.push_back(ro);

  // 4
  ro.zl = r.zl; ro.zu = point[2];
  regOut.push_back(ro);

  // 5
  ro.xl = point[0]; ro.xu = r.xu;
  regOut.push_back(ro);

  // 6
  ro.zl = point[2]; ro.zu = r.zu;
  regOut.push_back(ro);

  // 7
  ro.yl = r.yl; ro.yu = point[1];
  regOut.push_back(ro);

  // 8
  ro.zl = r.zl; ro.zu = point[2];
  regOut.push_back(ro);
}

void integrator::compute_regions( const region &r, std::vector<region> &regOut, const double (&point)[3] )
{
  region ro;

  double xs[3] = { r.xl, point[0], r.xu };
  double ys[3] = { r.yl, point[1], r.yu };
  double zs[3] = { r.zl, point[2], r.zu };

  for(int ix=0; ix<2; ix++)
  {
    for(int iy=0; iy<2; iy++)
    {
      for(int iz=0; iz<2; iz++)
      {
        ro.xl = xs[ix]; ro.xu = xs[ix+1];
        ro.yl = ys[iy]; ro.yu = ys[iy+1];
        ro.zl = zs[iz]; ro.zu = zs[iz+1];

        if( ro.xl != ro.xu &&
            ro.yl != ro.yu &&
            ro.zl != ro.zu)
        {
          regOut.push_back(ro);
        }
      }
    }
  }
  
}

void integrator::test_octants( const double (&point)[3])
{
  compute_octants(r_det, regions_RS, forigin);

  for(int reg=0; reg<8; reg++)
  {
    if( regions_RS.at(reg).xl < point[0] && regions_RS.at(reg).xu > point[0] &&
        regions_RS.at(reg).yl < point[1] && regions_RS.at(reg).yu < point[1] &&
        regions_RS.at(reg).zl < point[2] && regions_RS.at(reg).zu < point[2] )
    {
      region subr;
        subr.xl = regions_RS.at(reg).xl; subr.xu = regions_RS.at(reg).xu;
        subr.yl = regions_RS.at(reg).yl; subr.yu = regions_RS.at(reg).yu;
        subr.zl = regions_RS.at(reg).zl; subr.zu = regions_RS.at(reg).zu;
      regions_RS.erase(regions_RS.begin()+reg);
      compute_octants(subr, regions_RS, point);
      break;
    }
  }

  for(int i=0; i< regions_RS.size(); i++)
  {
    std::cout << "region " << i << ": " << regions_RS.at(i).xl << " "
                                        << regions_RS.at(i).xu << " " 
                                        << regions_RS.at(i).yl << " "
                                        << regions_RS.at(i).yu << " "
                                        << regions_RS.at(i).zl << " "
                                        << regions_RS.at(i).zu << "\n";                                 
  }

}

double integrator::integrateRS( const double (&point)[3] )
{

  // heaviest part... implement a cache?
  //  function is always the same but sampling is different each time...
  //
  //  before computing, search in cache for an existing point (x,y,z) within tolerance
  //    tolerance could be dependent on distance from origin (and walls maybe)
  //  if point is not in cache, compute and add
  //
  //    might interpolate between points in cache if close enough?
  //    check the error introduced by the cache...

  // compute_octants(r_det, regions_RS, forigin);
  compute_regions(r_det, regions_RS, forigin);

  for(int reg=0; reg<regions_RS.size(); reg++)
  {
    if( regions_RS.at(reg).xl < point[0] && regions_RS.at(reg).xu > point[0] &&
        regions_RS.at(reg).yl < point[1] && regions_RS.at(reg).yu < point[1] &&
        regions_RS.at(reg).zl < point[2] && regions_RS.at(reg).zu < point[2] )
    {
      region subr;
        subr.xl = regions_RS.at(reg).xl; subr.xu = regions_RS.at(reg).xu;
        subr.yl = regions_RS.at(reg).yl; subr.yu = regions_RS.at(reg).yu;
        subr.zl = regions_RS.at(reg).zl; subr.zu = regions_RS.at(reg).zu;
      regions_RS.erase(regions_RS.begin()+reg);
      // compute_octants(subr, regions_RS, point);
      compute_regions(subr, regions_RS, point);
      break;
    }
  }

  double result = 0;

  for(int i=0; i<regions_RS.size(); i++)
  {
    result += integrateRegion(regions_RS.at(i), point);
  }

  // std::cout << "result RS = " << result << "\n";

  regions_RS.clear();

  double r, dx, dy, dz;

  dx = point[0] - forigin[0];
  dy = point[1] - forigin[1];
  dz = point[2] - forigin[2];

  r = sqrt( dx*dx + dy*dy + dz*dz );

  // result += ((lambda_RS*lambda_tot*lambda_tot)/(8*M_PI))*exp(-lambda_tot*r);

  if(r<80) r=80;
  result += ((mu_RS)/(4*M_PI*r*r))*exp(-mu_tot*r);

  return result;
}

double integrator::integrateRegion( const region &reg, const double (&point)[3] )
{
  // double GLT[3][2] =
  //   {
  //     { -0.7745966692414834, 0.5555555555555552 },
  //     {  0.0000000000000000, 0.8888888888888888 },
  //     {  0.7745966692414834, 0.5555555555555552 }
  //   };

  double h[3];
    double &hx = h[0];
    double &hy = h[1];
    double &hz = h[2];

  hx = 0.5 * (reg.xu - reg.xl);
  hy = 0.5 * (reg.yu - reg.yl);
  hz = 0.5 * (reg.zu - reg.zl);

  double cx = 0.5 * (reg.xu + reg.xl);
  double cy = 0.5 * (reg.yu + reg.yl);
  double cz = 0.5 * (reg.zu + reg.zl);

  // select number of Gauss-Legendre nodes for integration
  int nodes[3] = { 1, 1, 1 };
  for(int n=0; n<3; n++)
  {
    if( h[n] > 10000 )
    {
      nodes[n] = 11 - 2;
    }
    else if( h[n] > 5000 )
    {
      nodes[n] = 9 - 2;
    }
    else if( h[n] > 2000 )
    {
      nodes[n] = 7 - 2;
    }
    else if( h[n] > 1000 )
    {
      nodes[n] = 5 - 2;
    }
  }

  double sum = 0.0;

  double RS, r, dx, dy, dz, r2, dx2, dy2, dz2;

  for (int i = 0; i < nodes[0]+2; ++i)
  {
    double x = hx * GLT.at(nodes[0])[i][0] + cx;
    dx = x - forigin[0];
    dx2 = x - point[0];

    for (int j = 0; j < nodes[1]+2; ++j)
    {
      double y = hy * GLT.at(nodes[1])[j][0] + cy;
      dy = y - forigin[1];
      dy2 = y - point[1];

      for (int k = 0; k < nodes[2]+2; ++k)
      {
        double z = hz * GLT.at(nodes[2])[k][0] + cz;
        dz = z - forigin[2];
        dz2 = z - point[2];

        r  = sqrt( dx*dx + dy*dy + dz*dz );
        r2 = sqrt( dx2*dx2 + dy2*dy2 + dz2*dz2 );

        // RS = ((1/(4*M_PI*r*r))*lambda_RS*exp(-(lambda_RS+lambda_abs)*(r) ) );

        // RS = ((lambda_RS*lambda_tot*lambda_tot)/(8*M_PI))*((lambda_RS*lambda_tot*lambda_tot)/(8*M_PI))*exp(-2*lambda_tot*r);
        if(r<80)  r=80;
        if(r2<80) r2=80;
        RS = ((mu_RS)/(4*M_PI*r*r))*((mu_RS)/(4*M_PI*r2*r2))*exp(-mu_tot*(r+r2));

        // r2 = sqrt( dx2*dx2 + dy2*dy2 + dz2*dz2 );

        // RS = RS + RS * ((lambda_RS*lambda_tot*lambda_tot/(8*M_PI))*exp(-lambda_tot*r2) );
        // std::cout << "r = " << r << "\tr2 = " << r2 << "\n";

        // if(r2!=0)
        // {
        //   // RS = RS + RS * ((1/(4*M_PI*r2*r2))*lambda_RS*exp(-(lambda_RS+lambda_abs)*(r2) ));
        //   // std::cout << "RS2 = " << RS2 << "\n";
        // }

        sum +=  GLT.at(nodes[0])[i][1]
              * GLT.at(nodes[1])[j][1]
              * GLT.at(nodes[2])[k][1]
              * RS;
        
        // std::cout << "sum = " << sum << "\n";
        // if( isinf(RS) )
        // {
        //   // std::cout << "DLRS = " << DLRS << "\n";
        //   std::cout << "r = " << r << "\n";
        //   std::cout << "RS sampling point x, y, z = " << x << ", " << y << ", " << z << "\n";
        //   std::cout << "point x, y, z = " << point[0] << ", " << point[1] << ", " << point[2] << "\n";
        //   std::cout << " dx2, dy2, dz2 = " << dx2 << ", " << dy2 << ", " << dz2 << "\n";
        //   std::cout << "r2 = " << r2 << "\n";
        //   // std::cout << "RS sampling point x, y, z = " << x << ", " << y << ", " << z << "\n";
        // }
      }
    }
  }

  // std::cout << "sum = " << hx * hy * hz * sum << "\n";
  return hx * hy * hz * sum;
}

double integrator::integrateFromVolume( const double (&point)[3] )
{
  // std::cout << "integrating for wall position (x, y, z) = (" << fwallpoint[0] << ", " << fwallpoint[1] << ", " << fwallpoint[2] << ")\n"; 
  // compute_quadrants();

  double result = 0;

  if(RSb_)
  {
    compute_regions(r_det, regions, point);

    for(int reg=0; reg<regions.size(); reg++)
    {
      if( regions.at(reg).xl <= forigin[0] && regions.at(reg).xu >= forigin[0] &&
          regions.at(reg).yl <= forigin[1] && regions.at(reg).yu >= forigin[1] &&
          regions.at(reg).zl <= forigin[2] && regions.at(reg).zu >= forigin[2] )
      {
        region subr;
          subr.xl = regions.at(reg).xl; subr.xu = regions.at(reg).xu;
          subr.yl = regions.at(reg).yl; subr.yu = regions.at(reg).yu;
          subr.zl = regions.at(reg).zl; subr.zu = regions.at(reg).zu;
        regions.erase(regions.begin()+reg);
        // compute_octants(subr, regions, forigin);
        compute_regions(subr, regions, forigin);
        break;
      }
    }

    // print_regions(regions);

    for(int i=0; i<regions.size(); i++)
    {
      // double ir = integrateRegionWP(regions.at(i));
      // std::cout << "ir WP = " << ir << "\n";
      // result += ir;
      result += integrateRegionWP(regions.at(i), point);
    }

    regions.clear();
  }

  // add direct light
  double dx, dy, dz, r;
    dx = point[0] - forigin[0];
    dy = point[1] - forigin[1];
    dz = point[2] - forigin[2];

  r = sqrt( dx*dx + dy*dy + dz*dz );

  // result += ( 1 + lambda_tot*r + lambda_tot*lambda_tot*r*r/2 ) * exp(-lambda_tot*r);
  // result += exp(-lambda_tot*r) / (4*M_PI*r*r);
  // if(r<100) r=100;
  result += exp(-mu_tot*r) / (4*M_PI*r*r);
  // std::cout << "DL = " << ( 1 + lambda_tot*r + lambda_tot*lambda_tot*r*r/2 ) * exp(-lambda_tot*r) << "\n";
  return result;

}


double integrator::integrateRegionWP( const region &reg, const double (&wallpoint)[3] )
{
  // double GLT[5][2] =
  // {
  //   { -0.9061798459386640, 0.2369268850561890 },
  //   { -0.5384693101056831, 0.4786286704993665 },
  //   {  0.0000000000000000, 0.5688888888888889 },
  //   {  0.5384693101056831, 0.4786286704993665 },
  //   {  0.9061798459386640, 0.2369268850561890 }
  // };

  // double GLT[3][2] =
  // {
  //   { -0.7745966692414834, 0.5555555555555552 },
  //   {  0.0000000000000000, 0.8888888888888888 },
  //   {  0.7745966692414834, 0.5555555555555552 }
  // };

  double h[3];
    double &hx = h[0];
    double &hy = h[1];
    double &hz = h[2];

  hx = 0.5 * (reg.xu - reg.xl);
  hy = 0.5 * (reg.yu - reg.yl);
  hz = 0.5 * (reg.zu - reg.zl);

  double cx = 0.5 * (reg.xu + reg.xl);
  double cy = 0.5 * (reg.yu + reg.yl);
  double cz = 0.5 * (reg.zu + reg.zl);

  // select number of Gauss-Legendre nodes for integration
  int nodes[3] = { 1, 1, 1 };
  for(int n=0; n<3; n++)
  {
    if( h[n] > 10000 )
    {
      nodes[n] = 11 - 2;
    }
    else if( h[n] > 5000 )
    {
      nodes[n] = 9 - 2;
    }
    else if( h[n] > 2000 )
    {
      nodes[n] = 7 - 2;
    }
    else if( h[n] > 1000 )
    {
      nodes[n] = 5 - 2;
    }
  }

  double sum = 0.0;

  double DLRS, r, dx, dy, dz, r2, dx2, dy2, dz2;

  double point[3];
    double &x = point[0];
    double &y = point[1];
    double &z = point[2];

  for (int i = 0; i < nodes[0]+2; ++i)
  {
    x = hx * GLT.at(nodes[0])[i][0] + cx;
    dx2 = x - wallpoint[0];

    for (int j = 0; j < nodes[1]+2; ++j)
    {
      y = hy * GLT.at(nodes[1])[j][0] + cy;
      dy2 = y - wallpoint[1];

      for (int k = 0; k < nodes[2]+2; ++k)
      {
        z = hz * GLT.at(nodes[2])[k][0] + cz;
        dz2 = z - wallpoint[2];

        r2 = sqrt( dx2*dx2 + dy2*dy2 + dz2*dz2 );

        // std::cout << "RS sampling point x, y, z = " << x << ", " << y << ", " << z << "\n";

        // DLRS = integrateRS(point) * ((1/(4*M_PI*r2*r2))*(exp(-lambda_abs*r2 ) - (1/(4*M_PI*r2*r2)*(4*M_PI*r2*r2))*lambda_RS*exp(-(lambda_abs+lambda_RS)*r2)) );

        // DLRS = integrateRS(point) * ((1/(4*M_PI*r2*r2))*(exp(-lambda_abs*r2 ) - (1/(4*M_PI*r2*r2)*(4*M_PI*r2*r2))*lambda_RS*exp(-(lambda_abs+lambda_RS)*r2)) );

        // DLRS = integrateRS(point) * ( 1 + lambda_tot*r2 + lambda_tot*lambda_tot*r2*r2/2 ) * exp(-lambda_tot*r2);
        // DLRS = integrateRS(point) * exp(-lambda_tot*r2) / (4*M_PI*r2*r2);
        // if(r2<100) r2=100;
        // DLRS = integrateRS(point) * exp(-mu_tot*r2) / (4*M_PI*r2*r2);

        // if(rsed.GetRSEDat(point[0] - r_det.xl, point[1] - r_det.yl, point[2] - r_det.zl) == 0) std::cout << "\nproblem!!!!";
        // std::cout << "computing RSED at: [ " << point[0] - r_det.xl << ", " << point[1] - r_det.yl << ", "<< point[2] - r_det.zl << " ]\n";

        DLRS = rsed.GetRSEDat(point[0] - r_det.xl, point[1] - r_det.yl, point[2] - r_det.zl) * exp(-mu_tot*r2) / (4*M_PI*r2*r2);

        // if( isinf(DLRS) )
        // {
          // std::cout << "DLRS = " << DLRS << "\n";
        //   // std::cout << "r = " << r << "\n";
        //   // std::cout << "r = " << r2 << "\n";
        //   std::cout << "RS sampling point x, y, z = " << x << ", " << y << ", " << z << "\n";
        // }

        sum +=  GLT.at(nodes[0])[i][1]
              * GLT.at(nodes[1])[j][1]
              * GLT.at(nodes[2])[k][1]
              * DLRS;
      }
    }
  }
  // std::cout << " hx, xy, xz, sum = " << hx << ", " << hy << ", " << hz << ", " << sum << "\n";

  return hx * hy * hz * sum ;
}


double integrator::integrateSurface( int face, const surface &surf )
{
  // integrate over the 2D patch on the given box face:
  // GL nodes in (u, v) are expanded to 3D points on the face

  double point[3];

  // determine GL nodes and integrate over them

  double h[2];
  double ha = 0.5 * (surf.uMax - surf.uMin);
  double hb = 0.5 * (surf.vMax - surf.vMin);
  h[0] = ha; h[1] = hb;
  double ca = 0.5 * (surf.uMax + surf.uMin);
  double cb = 0.5 * (surf.vMax + surf.vMin);

  // select number of Gauss-Legendre nodes for integration
  int nodes[2] = { 1, 1 };
  for(int n=0; n<2; n++)
  {
    if( h[n] > 10000 )
    {
      nodes[n] = 11 - 2;
    }
    else if( h[n] > 5000 )
    {
      nodes[n] = 9 - 2;
    }
    else if( h[n] > 2000 )
    {
      nodes[n] = 7 - 2;
    }
    else if( h[n] > 1000 )
    {
      nodes[n] = 5 - 2;
    }
  }

  double sum = 0.0;

  for (int i = 0; i < nodes[0]+2; ++i)
  {
    const double u = ha * GLT.at(nodes[0])[i][0] + ca;

    for (int j = 0; j < nodes[1]+2; ++j)
    {
      const double v = hb * GLT.at(nodes[1])[j][0] + cb;

      ExpandSurfacePoint(fboundaries, face, u, v, point);

      // NOTE (SLArFLSNewModel integration): the reflected-light wall term
      // integrateFromSurface(point) is DISABLED here — as written it is too
      // computationally heavy: it re-runs SegmentGeometry over the whole
      // geometry and re-evaluates integrateFromVolume (incl. the RSED lookups)
      // per segment Gauss node inside every outer Gauss node, i.e. O(n^2) per
      // emission point. TODO: re-enable in a viable form (see
      // LMFast-integration-notes.md §6 performance TODO).
      double totalVisibility = integrateFromVolume( point ) /*+ integrateFromSurface( point )*/ ;

      sum +=  GLT.at(nodes[0])[i][1]
            * GLT.at(nodes[1])[j][1]
            * totalVisibility;
    }
  }
  // std::cout << " hx, xy, xz, sum = " << hx << ", " << hy << ", " << hz << ", " << sum << "\n";

  return ha * hb * sum ;
}

double integrator::integrateSurfacePoint( const double (&point)[3] )
{
  double totalVisibility = integrateFromVolume( point ) /*+ integrateFromSurface( point )*/ ;
  return totalVisibility;
}

double integrator::integrateFromSurface( const double (&point)[3] )
{
  // segment the geometry for the given point
  // for each surface in the working geometry
  // determine GL nodes, compute their visibility and integrate over them

  // segmentation
  SegmentGeometry(baseGeo, workGeo, fboundaries, forigin, point);
  // PrintGeometry( workGeo, fboundaries );

  double visibility = 0;

  for( int f=0; f<(int)workGeo.surfColl.size(); f++)
  {
    if( f==kFaceYMin || f==kFaceYMax ) continue; // skip faces (no shifter)
    for( int j=0; j<(int)workGeo.surfColl.at(f).surf.size(); j++ )
    {
      // std::cout << "\tSurface number " << j << "\n";
      visibility += integrateFromSurfaceSegment( f, workGeo.surfColl.at(f).surf.at(j), point );
    }
  }

  return visibility;
}

double integrator::integrateFromSurfaceSegment( int face, const surface &surf, const double (&fromPoint)[3] )
{
  // integrate over the 2D patch on the given box face:
  // GL nodes in (u, v) are expanded to 3D points on the face

  double point[3];

  // determine GL nodes and integrate over them

  double h[2];
  double ha = 0.5 * (surf.uMax - surf.uMin);
  double hb = 0.5 * (surf.vMax - surf.vMin);
  h[0] = ha; h[1] = hb;
  double ca = 0.5 * (surf.uMax + surf.uMin);
  double cb = 0.5 * (surf.vMax + surf.vMin);

  // select number of Gauss-Legendre nodes for integration
  int nodes[2] = { 1, 1 };
  for(int n=0; n<2; n++)
  {
    if( h[n] > 10000 )
    {
      nodes[n] = 11 - 2;
    }
    else if( h[n] > 5000 )
    {
      nodes[n] = 9 - 2;
    }
    else if( h[n] > 2000 )
    {
      nodes[n] = 7 - 2;
    }
    else if( h[n] > 1000 )
    {
      nodes[n] = 5 - 2;
    }
  }

  double sum = 0.0;

  double visibility, r;

  for (int i = 0; i < nodes[0]+2; ++i)
  {
    const double u = ha * GLT.at(nodes[0])[i][0] + ca;

    for (int j = 0; j < nodes[1]+2; ++j)
    {
      const double v = hb * GLT.at(nodes[1])[j][0] + cb;

      ExpandSurfacePoint(fboundaries, face, u, v, point);

      r = sqrt( (point[0]-fromPoint[0])*(point[0]-fromPoint[0]) + (point[1]-fromPoint[1])*(point[1]-fromPoint[1]) + (point[2]-fromPoint[2])*(point[2]-fromPoint[2]) );

      visibility = integrateFromVolume( point ) / (4*M_PI*r*r) ;

      sum +=  GLT.at(nodes[0])[i][1]
            * GLT.at(nodes[1])[j][1]
            * visibility;
    }
  }
  // std::cout << " hx, xy, xz, sum = " << hx << ", " << hy << ", " << hz << ", " << sum << "\n";

  return ha * hb * sum ;
}

/*
double integrator::integrateSurfaceWP( const surface &surf )
{
  // determine GL nodes and integrate over them

  double h[2];
    double &ha = h[0];
    double &hb = h[1];

  ha = 0.5 * (reg.au - reg.al);
  hb = 0.5 * (reg.bu - reg.bl);
  double ca = 0.5 * (reg.au + reg.al);
  double cb = 0.5 * (reg.bu + reg.bl);

  // select number of Gauss-Legendre nodes for integration
  int nodes[2] = { 1, 1 };
  for(int n=0; n<2; n++)
  {
    if( h[n] > 10000 )
    {
      nodes[n] = 11 - 2;
    }
    else if( h[n] > 5000 )
    {
      nodes[n] = 9 - 2;
    }
    else if( h[n] > 2000 )
    {
      nodes[n] = 7 - 2;
    }
    else if( h[n] > 1000 )
    {
      nodes[n] = 5 - 2;
    }
  }

  double sum = 0.0;

  double DLRS, r, da, db, r2, da2, db2;

  double point[2];
    double &a = point[0];
    double &b = point[1];

  for (int i = 0; i < nodes[0]+2; ++i)
  {
    a = ha * GLT.at(nodes[0])[i][0] + ca;
    da2 = a - fwallpoint[1]; //change!

    for (int j = 0; j < nodes[1]+2; ++j)
    {
      b = hb * GLT.at(nodes[1])[j][0] + cb;
      db2 = b - fwallpoint[2]; //change!

      r2 = sqrt( da2*da2 + db2*db2 + dc2*dc2 );

      // std::cout << "RS sampling point x, y, z = " << x << ", " << y << ", " << z << "\n";

      // DLRS = integrateRS(point) * ((1/(4*M_PI*r2*r2))*(exp(-lambda_abs*r2 ) - (1/(4*M_PI*r2*r2)*(4*M_PI*r2*r2))*lambda_RS*exp(-(lambda_abs+lambda_RS)*r2)) );

      // DLRS = integrateRS(point) * ((1/(4*M_PI*r2*r2))*(exp(-lambda_abs*r2 ) - (1/(4*M_PI*r2*r2)*(4*M_PI*r2*r2))*lambda_RS*exp(-(lambda_abs+lambda_RS)*r2)) );

      // DLRS = integrateRS(point) * ( 1 + lambda_tot*r2 + lambda_tot*lambda_tot*r2*r2/2 ) * exp(-lambda_tot*r2);
      // DLRS = integrateRS(point) * exp(-lambda_tot*r2) / (4*M_PI*r2*r2);
      // if(r2<100) r2=100;
      // DLRS = integrateRS(point) * exp(-mu_tot*r2) / (4*M_PI*r2*r2);

      // if(rsed.GetRSEDat(point[0] - r_det.xl, point[1] - r_det.yl, point[2] - r_det.zl) == 0) std::cout << "\nproblem!!!!";
      // std::cout << "computing RSED at: [ " << point[0] - r_det.xl << ", " << point[1] - r_det.yl << ", "<< point[2] - r_det.zl << " ]\n";

      DLRS = rsed.GetRSEDat(point[0] - r_det.xl, point[1] - r_det.yl, point[2] - r_det.zl) * exp(-mu_tot*r2) / (4*M_PI*r2*r2);

      // if( isinf(DLRS) )
      // {
        // std::cout << "DLRS = " << DLRS << "\n";
      //   // std::cout << "r = " << r << "\n";
      //   // std::cout << "r = " << r2 << "\n";
      //   std::cout << "RS sampling point x, y, z = " << x << ", " << y << ", " << z << "\n";
      // }

      sum +=  GLT.at(nodes[0])[i][1]
            * GLT.at(nodes[1])[j][1]
            * DLRS;
    }
  }
  // std::cout << " hx, xy, xz, sum = " << hx << ", " << hy << ", " << hz << ", " << sum << "\n";

  return hx * hy * hz * sum ;
}
*/

double integrator::computeTestSurfaceVisibility()
{
  // > determine GL nodes for surface visibility integration
    // hardcoded 3x3 for now, could be (probably is) a problem for some light origin positions

  
  // > compute visibility on each node
  //  > integrate direct + RS light
  //  > integrate indirect light from the detector surfaces
  //   > segment det surface
  //   > determine GL nodes for each surface segment
  //   > compute visibility an each node
  //   > multiply by light propagation between the two nodes (take into account variables for reflectivity or other properties (but NO RS at these wl))

  if (baseGeo.surfColl.empty() || baseGeo.surfColl.at(0).surf.empty())
    return 0.;

  double visibility = integrateSurface( 0, baseGeo.surfColl.at(0).surf.at(0) );

  return visibility;
}