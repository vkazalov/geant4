//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// $Id: G4Cons.cc,v 1.34 2004/12/10 16:22:37 gcosmo Exp $
// GEANT4 tag $Name: geant4-07-00-cand-05 $
//
// class G4Cons
//
// Implementation for G4Cons class
//
// History:
//
// 26.07.04 V.Grichine: bugs fixed in   Distance ToIn(p,v):dIn=dOut=0 in 3/10^8
// 23.01.04 V.Grichine: bugs fixed in   Distance ToIn(p,v)
// 26.06.02 V.Grichine: bugs fixed in   Distance ToIn(p,v)
// 05.10.00 V.Grichine: bugs fixed in   Distance ToIn(p,v)
// 17.08.00 V.Grichine: if one and only one Rmin=0, it'll be 1e3*kRadTolerance 
// 08.08.00 V.Grichine: more stable roots 2-equation in DistanceToOut(p,v,...)
// 06.03.00 V.Grichine: modifications in DistanceToOut(p,v,...) 
// 18.11.99 V.Grichine: side = kNull initialisation in DistanceToOut(p,v,...)
// 28.04.99 V.Grichine: bugs fixed in  Distance ToOut(p,v,...) and  
//                      Distance ToIn(p,v)
// 09.10.98 V.Grichine: modifications in Distance ToOut(p,v,...)
// 13.09.96 V.Grichine: final modifications to commit
// ~1994    P.Kent: main part of geometry functions
// --------------------------------------------------------------------

#include "G4Cons.hh"

#include "G4VoxelLimits.hh"
#include "G4AffineTransform.hh"

#include "G4VPVParameterisation.hh"

#include "meshdefs.hh"

#include "G4VGraphicsScene.hh"
#include "G4Polyhedron.hh"
#include "G4NURBS.hh"
#include "G4NURBSbox.hh"

////////////////////////////////////////////////////////////////////////
//
// Private enum: Not for external use - used by distanceToOut

enum ESide {kNull,kRMin,kRMax,kSPhi,kEPhi,kPZ,kMZ};

// used by normal

enum ENorm {kNRMin,kNRMax,kNSPhi,kNEPhi,kNZ};

///////////////////////////////////////////////////////////////////////
//
// Destructor

G4Cons::~G4Cons()
{
}

//////////////////////////////////////////////////////////////////////////
//
// constructor - check parameters, convert angles so 0<sphi+dpshi<=2_PI
//               - note if pDPhi>2PI then reset to 2PI

G4Cons::G4Cons( const G4String& pName,
                      G4double  pRmin1, G4double pRmax1,
                      G4double  pRmin2, G4double pRmax2,
                      G4double pDz,
                      G4double pSPhi, G4double pDPhi)
  : G4CSGSolid(pName)
{
  // Check z-len

  if ( pDz > 0 )
    fDz = pDz ;
  else
  {
    G4cerr << "ERROR - G4Cons()::G4Cons(): " << GetName() << G4endl
           << "        Negative Z half-length ! - "
           << pDz << G4endl;
    G4Exception("G4Cons::G4Cons()", "InvalidSetup",
                FatalException, "Invalid Z half-length.");
  }

  // Check radii

  if ( pRmin1 < pRmax1 && pRmin2 < pRmax2 && pRmin1 >= 0 && pRmin2 >= 0 )
  {

    fRmin1 = pRmin1 ; 
    fRmax1 = pRmax1 ;
    fRmin2 = pRmin2 ; 
    fRmax2 = pRmax2 ;
    if( (pRmin1 == 0.0 && pRmin2 > 0.0) ) fRmin1 = 1e3*kRadTolerance ; 
    if( (pRmin2 == 0.0 && pRmin1 > 0.0) ) fRmin2 = 1e3*kRadTolerance ; 
  }
  else
  {
    G4cerr << "ERROR - G4Cons()::G4Cons(): " << GetName() << G4endl
           << "        Invalide values for radii ! - "
           << "        pRmin1 = " << pRmin1 << ", pRmin2 = " << pRmin2
           << ", pRmax1 = " << pRmax1 << ", pRmax2 = " << pRmax2 << G4endl;
    G4Exception("G4Cons::G4Cons()", "InvalidSetup",
                FatalException, "Invalid radii.") ;
  }

  // Check angles

  if ( pDPhi >= twopi )
  {
    fDPhi=twopi;
    fSPhi=0;
  }
  else
  {
    if ( pDPhi > 0 ) fDPhi = pDPhi ;
    else
    {
      G4cerr << "ERROR - G4Cons()::G4Cons(): " << GetName() << G4endl
             << "        Negative delta-Phi ! - "
             << pDPhi << G4endl;
      G4Exception("G4Cons::G4Cons()", "InvalidSetup",
                  FatalException, "Invalid pDPhi.") ;
    }

    // Ensure pSPhi in 0-2PI or -2PI-0 range if shape crosses 0

    if ( pSPhi < 0 ) fSPhi = twopi - std::fmod(std::fabs(pSPhi),twopi) ;
    else             fSPhi = std::fmod(pSPhi,twopi) ;
      
    if (fSPhi + fDPhi > twopi) fSPhi -= twopi ;
  }
}

/////////////////////////////////////////////////////////////////////
//
// Return whether point inside/outside/on surface

EInside G4Cons::Inside(const G4ThreeVector& p) const
{
  G4double r2, rl, rh, pPhi, tolRMin, tolRMax ;
  EInside in;

  if (std::fabs(p.z()) > fDz + kCarTolerance*0.5 ) return in = kOutside;
  else if(std::fabs(p.z()) >= fDz - kCarTolerance*0.5 )   in = kSurface;
  else                                               in = kInside;

  r2 = p.x()*p.x() + p.y()*p.y() ;
  rl = 0.5*(fRmin2*(p.z() + fDz) + fRmin1*(fDz - p.z()))/fDz ;
  rh = 0.5*(fRmax2*(p.z()+fDz)+fRmax1*(fDz-p.z()))/fDz;

  tolRMin = rl - kRadTolerance*0.5 ;
  if ( tolRMin < 0 ) tolRMin = 0 ;
  tolRMax = rh + kRadTolerance*0.5 ;

  if ( r2 < tolRMin*tolRMin || r2 > tolRMax*tolRMax ) return in = kOutside;

  if (rl) tolRMin = rl + kRadTolerance*0.5 ;
  else    tolRMin = 0.0 ;
  tolRMax         = rh - kRadTolerance*0.5 ;
      
  if (in == kInside) // else it's kSurface already
  {
    if (r2 < tolRMin*tolRMin || r2 >= tolRMax*tolRMax) in = kSurface;
  }
  if ( ( fDPhi < twopi - kAngTolerance ) &&
       ( (p.x() != 0.0 ) || (p.y() != 0.0) ) )
  {
    pPhi = std::atan2(p.y(),p.x()) ;

    if ( pPhi < fSPhi - kAngTolerance*0.5  )             pPhi += twopi ; 
    else if ( pPhi > fSPhi + fDPhi + kAngTolerance*0.5 ) pPhi -= twopi; 
    
    if ( (pPhi < fSPhi - kAngTolerance*0.5) ||          
         (pPhi > fSPhi + fDPhi + kAngTolerance*0.5) )   return in = kOutside;
      
    else if (in == kInside)  // else it's kSurface anyway already
    {
        if ( (pPhi < fSPhi + kAngTolerance*0.5) || 
             (pPhi > fSPhi + fDPhi - kAngTolerance*0.5) )  in = kSurface ;
    }
  }
  else if( fDPhi < twopi - kAngTolerance )  in = kSurface ;

  return in ;
}

/////////////////////////////////////////////////////////////////////////
//
// Dispatch to parameterisation for replication mechanism dimension
// computation & modification.

void G4Cons::ComputeDimensions(      G4VPVParameterisation* p,
                               const G4int                  n,
                               const G4VPhysicalVolume*     pRep    )
{
  p->ComputeDimensions(*this,n,pRep) ;
}


///////////////////////////////////////////////////////////////////////////
//
// Calculate extent under transform and specified limit

G4bool G4Cons::CalculateExtent( const EAxis              pAxis,
              const G4VoxelLimits&     pVoxelLimit,
              const G4AffineTransform& pTransform,
                    G4double&          pMin,
                    G4double&          pMax  ) const
{
  if ( !pTransform.IsRotated() && 
        fDPhi == twopi && fRmin1 == 0 && fRmin2 == 0 )
  {
    // Special case handling for unrotated solid cones
    // Compute z/x/y mins and maxs for bounding box respecting limits,
    // with early returns if outside limits. Then switch() on pAxis,
    // and compute exact x and y limit for x/y case
      
    G4double xoffset, xMin, xMax ;
    G4double yoffset, yMin, yMax ;
    G4double zoffset, zMin, zMax ;

    G4double diff1, diff2, maxDiff, newMin, newMax, RMax ;
    G4double xoff1, xoff2, yoff1, yoff2 ;
      
    zoffset = pTransform.NetTranslation().z();
    zMin    = zoffset - fDz ;
    zMax    = zoffset + fDz ;

    if (pVoxelLimit.IsZLimited())
    {
      if( zMin > pVoxelLimit.GetMaxZExtent() + kCarTolerance || 
          zMax < pVoxelLimit.GetMinZExtent() - kCarTolerance   )
      {
        return false ;
      }
      else
      {
        if ( zMin < pVoxelLimit.GetMinZExtent() )
        {
          zMin = pVoxelLimit.GetMinZExtent() ;
        }
        if ( zMax > pVoxelLimit.GetMaxZExtent() )
        {
          zMax = pVoxelLimit.GetMaxZExtent() ;
        }
      }
    }
    xoffset = pTransform.NetTranslation().x() ;
    RMax    = (fRmax2 >= fRmax1) ?  zMax : zMin  ;                  
    xMax    = xoffset + (fRmax1 + fRmax2)*0.5 + 
              (RMax - zoffset)*(fRmax2 - fRmax1)/(2*fDz) ;
    xMin    = 2*xoffset-xMax ;

    if (pVoxelLimit.IsXLimited())
    {
      if ( xMin > pVoxelLimit.GetMaxXExtent() + kCarTolerance || 
           xMax < pVoxelLimit.GetMinXExtent() - kCarTolerance    )
      {
        return false ;
      }
      else
      {
        if ( xMin < pVoxelLimit.GetMinXExtent() )
        {
          xMin = pVoxelLimit.GetMinXExtent() ;
        }
        if ( xMax > pVoxelLimit.GetMaxXExtent() )
        {
          xMax=pVoxelLimit.GetMaxXExtent() ;
        }
      }
    }
    yoffset = pTransform.NetTranslation().y() ;
    yMax    = yoffset + (fRmax1 + fRmax2)*0.5 + 
              (RMax - zoffset)*(fRmax2 - fRmax1)/(2*fDz) ;
    yMin    = 2*yoffset-yMax ;
    RMax    = yMax - yoffset ;  // = max radius due to Zmax/Zmin cuttings

    if (pVoxelLimit.IsYLimited())
    {
      if ( yMin > pVoxelLimit.GetMaxYExtent() + kCarTolerance || 
           yMax < pVoxelLimit.GetMinYExtent() - kCarTolerance     )
      {
        return false ;
      }
      else
      {
        if ( yMin < pVoxelLimit.GetMinYExtent() )
        {
          yMin = pVoxelLimit.GetMinYExtent() ;
        }
        if ( yMax > pVoxelLimit.GetMaxYExtent() )
        {
          yMax = pVoxelLimit.GetMaxYExtent() ;
        }
      }
    }    
    switch (pAxis) // Known to cut cones
    {
      case kXAxis:
        yoff1 = yoffset - yMin ;
        yoff2 = yMax - yoffset ;

        if (yoff1 >= 0 && yoff2 >= 0) // Y limits cross max/min x => no change
        {
          pMin = xMin ;
          pMax = xMax ;
        }
        else
        {
          // Y limits don't cross max/min x => compute max delta x,
          // hence new mins/maxs
         
          diff1   = std::sqrt(RMax*RMax - yoff1*yoff1) ;
          diff2   = std::sqrt(RMax*RMax - yoff2*yoff2) ;
          maxDiff = (diff1>diff2) ? diff1:diff2 ;
          newMin  = xoffset - maxDiff ;
          newMax  = xoffset + maxDiff ;
          pMin    = ( newMin < xMin ) ? xMin : newMin  ;
          pMax    = ( newMax > xMax) ? xMax : newMax ;
        } 
      break ;

      case kYAxis:
        xoff1 = xoffset - xMin ;
        xoff2 = xMax - xoffset ;

        if (xoff1 >= 0 && xoff2 >= 0 ) // X limits cross max/min y => no change
        {
          pMin = yMin ;
          pMax = yMax ;
        }
        else
        {
          // X limits don't cross max/min y => compute max delta y,
          // hence new mins/maxs

          diff1   = std::sqrt(RMax*RMax - xoff1*xoff1) ;
          diff2   = std::sqrt(RMax*RMax-xoff2*xoff2) ;
          maxDiff = (diff1 > diff2) ? diff1:diff2 ;
          newMin  = yoffset - maxDiff ;
          newMax  = yoffset + maxDiff ;
          pMin    = (newMin < yMin) ? yMin : newMin ;
          pMax    = (newMax > yMax) ? yMax : newMax ;
        }
      break ;

      case kZAxis:
        pMin = zMin ;
        pMax = zMax ;
      break ;
      
      default:
      break ;
    }
    pMin -= kCarTolerance ;
    pMax += kCarTolerance ;

    return true ;
  }
  else   // Calculate rotated vertex coordinates
  {
    G4int i, noEntries, noBetweenSections4 ;
    G4bool existsAfterClip = false ;
    G4ThreeVectorList* vertices = CreateRotatedVertices(pTransform) ;

    pMin = +kInfinity ;
    pMax = -kInfinity ;

    noEntries          = vertices->size() ;
    noBetweenSections4 = noEntries-4 ;
      
    for ( i = 0 ; i < noEntries ; i += 4 )
    {
      ClipCrossSection(vertices,i,pVoxelLimit,pAxis,pMin,pMax) ;
    }
    for ( i = 0 ; i < noBetweenSections4 ; i += 4 )
    {
      ClipBetweenSections(vertices,i,pVoxelLimit,pAxis,pMin,pMax) ;
    }    
    if ( pMin != kInfinity || pMax != -kInfinity )
    {
      existsAfterClip = true ;
        
      // Add 2*tolerance to avoid precision troubles

      pMin -= kCarTolerance ;
      pMax += kCarTolerance ;
    }
    else
    {
      // Check for case where completely enveloping clipping volume
      // If point inside then we are confident that the solid completely
      // envelopes the clipping volume. Hence set min/max extents according
      // to clipping volume extents along the specified axis.
       
      G4ThreeVector clipCentre(
      (pVoxelLimit.GetMinXExtent() + pVoxelLimit.GetMaxXExtent())*0.5,
      (pVoxelLimit.GetMinYExtent() + pVoxelLimit.GetMaxYExtent())*0.5,
      (pVoxelLimit.GetMinZExtent() + pVoxelLimit.GetMaxZExtent())*0.5  ) ;
        
      if (Inside(pTransform.Inverse().TransformPoint(clipCentre)) != kOutside)
      {
        existsAfterClip = true ;
        pMin            = pVoxelLimit.GetMinExtent(pAxis) ;
        pMax            = pVoxelLimit.GetMaxExtent(pAxis) ;
      }
    }
    delete vertices ;
    return existsAfterClip ;
  }
}

////////////////////////////////////////////////////////////////////////
//
// Return unit normal of surface closest to p
// - note if point on z axis, ignore phi divided sides
// - unsafe if point close to z axis a rmin=0 - no explicit checks

G4ThreeVector G4Cons::SurfaceNormal( const G4ThreeVector& p) const
{
  ENorm side ;
  G4ThreeVector norm ;
  G4double rho, phi ;
  G4double distZ, distRMin, distRMax, distSPhi, distEPhi, distMin ;
  G4double tanRMin, secRMin, pRMin, widRMin ;
  G4double tanRMax, secRMax, pRMax, widRMax ;

  distZ = std::fabs(std::fabs(p.z()) - fDz) ;
  rho   = std::sqrt(p.x()*p.x() + p.y()*p.y()) ;

  tanRMin  = (fRmin2 - fRmin1)*0.5/fDz ;
  secRMin  = std::sqrt(1 + tanRMin*tanRMin) ;
  pRMin    = rho - p.z()*tanRMin ;
  widRMin  = fRmin2 - fDz*tanRMin ;
  distRMin = std::fabs(pRMin - widRMin)/secRMin ;

  tanRMax  = (fRmax2 - fRmax1)*0.5/fDz ;
  secRMax  = std::sqrt(1+tanRMax*tanRMax) ;
  pRMax    = rho - p.z()*tanRMax ;
  widRMax  = fRmax2 - fDz*tanRMax ;
  distRMax = std::fabs(pRMax - widRMax)/secRMax ;
  
  if (distRMin < distRMax)  // First minimum
  {
    if (distZ < distRMin)
    {
      distMin = distZ ;
      side    = kNZ ;
    }
    else
    {
      distMin = distRMin ;
      side    = kNRMin ;
    }
  }
  else
  {
    if (distZ < distRMax)
    {
      distMin = distZ ;
      side    = kNZ ;
    }
    else
    {
      distMin = distRMax ;
      side    = kNRMax ;
    }
  }
  if ( fDPhi < twopi && rho )  // Protected against (0,0,z) 
  {
    phi = std::atan2(p.y(),p.x()) ;

    if (phi < 0) phi += twopi ;

    if (fSPhi < 0) distSPhi = std::fabs(phi - (fSPhi + twopi))*rho ;
    else           distSPhi = std::fabs(phi - fSPhi)*rho ;

    distEPhi = std::fabs(phi - fSPhi - fDPhi)*rho ;

    // Find new minimum

    if (distSPhi < distEPhi)
    {
      if (distSPhi < distMin) side = kNSPhi ;
    }
    else 
    {
      if (distEPhi < distMin) side = kNEPhi ;
    }
  }    
  switch (side)
  {
    case kNRMin:      // Inner radius
      rho *= secRMin ;
      norm = G4ThreeVector(-p.x()/rho,-p.y()/rho,tanRMin/secRMin) ;
      break ;
    case kNRMax:      // Outer radius
      rho *= secRMax ;
      norm = G4ThreeVector(p.x()/rho,p.y()/rho,-tanRMax/secRMax) ;
      break ;
    case kNZ:      // +/- dz
      if (p.z() > 0) norm = G4ThreeVector(0,0,1) ; 
      else           norm = G4ThreeVector(0,0,-1) ; 
      break ;
    case kNSPhi:
      norm = G4ThreeVector(std::sin(fSPhi),-std::cos(fSPhi),0) ;
      break ;
    case kNEPhi:
      norm=G4ThreeVector(-std::sin(fSPhi+fDPhi),std::cos(fSPhi+fDPhi),0) ;
      break ;
    default:
      DumpInfo();
      G4Exception("G4Cons::SurfaceNormal()", "Notification", JustWarning,
                  "Undefined side for valid surface normal to solid.") ;
      break ;    
  }
  return norm ;
}

////////////////////////////////////////////////////////////////////////
//
// Calculate distance to shape from outside, along normalised vector
// - return kInfinity if no intersection, or intersection distance <= tolerance
//
// - Compute the intersection with the z planes 
//        - if at valid r, phi, return
//
// -> If point is outside cone, compute intersection with rmax1*0.5
//        - if at valid phi,z return
//        - if inside outer cone, handle case when on tolerant outer cone
//          boundary and heading inwards(->0 to in)
//
// -> Compute intersection with inner cone, taking largest +ve root
//        - if valid (in z,phi), save intersction
//
//    -> If phi segmented, compute intersections with phi half planes
//        - return smallest of valid phi intersections and
//          inner radius intersection
//
// NOTE:
// - Precalculations for phi trigonometry are Done `just in time'
// - `if valid' implies tolerant checking of intersection points
// - z, phi intersection from Tubs

G4double G4Cons::DistanceToIn( const G4ThreeVector& p,
                               const G4ThreeVector& v   ) const
{
  G4double snxt = kInfinity ;      // snxt = default return value

  G4bool seg ;        // true if segmented in phi
  G4double hDPhi,hDPhiOT,hDPhiIT,cosHDPhiOT=0.,cosHDPhiIT=0. ;
          // half dphi + outer tolerance
  G4double cPhi,sinCPhi=0.,cosCPhi=0. ;  // central phi

  G4double tanRMax,secRMax,rMaxAv,rMaxOAv ;  // Data for cones
  G4double tanRMin,secRMin,rMinAv,rMinIAv,rMinOAv ;
  G4double rout,rin ;

  G4double tolORMin,tolORMin2,tolIRMin,tolIRMin2 ; // `generous' radii squared
  G4double tolORMax2,tolIRMax,tolIRMax2 ;
  G4double tolODz,tolIDz ;

  G4double Dist,s,xi,yi,zi,ri=0.,rhoi2,cosPsi ; // Intersection point variables

  G4double t1,t2,t3,b,c,d ;    // Quadratic solver variables 
  G4double nt1,nt2,nt3 ;
  G4double Comp ;
  G4double cosSPhi,sinSPhi ;    // Trig for phi start intersect
  G4double ePhi,cosEPhi,sinEPhi ;  // for phi end intersect

  //
  // Set phi divided flag and precalcs
  //
  if (fDPhi < twopi)
  {
    seg        = true ;
    hDPhi      = 0.5*fDPhi ;    // half delta phi
    cPhi       = fSPhi + hDPhi ; ;
    hDPhiOT    = hDPhi + 0.5*kAngTolerance ;  // outers tol' half delta phi 
    hDPhiIT    = hDPhi - 0.5*kAngTolerance ;
    sinCPhi    = std::sin(cPhi) ;
    cosCPhi    = std::cos(cPhi) ;
    cosHDPhiOT = std::cos(hDPhiOT) ;
    cosHDPhiIT = std::cos(hDPhiIT) ;
  }
  else     seg = false ;

  // Cone Precalcs

  tanRMin = (fRmin2 - fRmin1)*0.5/fDz ;
  secRMin = std::sqrt(1.0 + tanRMin*tanRMin) ;
  rMinAv  = (fRmin1 + fRmin2)*0.5 ;

  if (rMinAv > kRadTolerance*0.5)
  {
    rMinOAv = rMinAv - kRadTolerance*0.5 ;
    rMinIAv = rMinAv + kRadTolerance*0.5 ;
  }
  else
  {
    rMinOAv = 0.0 ;
    rMinIAv = 0.0 ;
  }  
  tanRMax = (fRmax2 - fRmax1)*0.5/fDz ;
  secRMax = std::sqrt(1.0 + tanRMax*tanRMax) ;
  rMaxAv  = (fRmax1 + fRmax2)*0.5 ;
  rMaxOAv = rMaxAv + kRadTolerance*0.5 ;
   
  // Intersection with z-surfaces

  tolIDz = fDz - kCarTolerance*0.5 ;
  tolODz = fDz + kCarTolerance*0.5 ;

  if (std::fabs(p.z()) >= tolIDz)
  {
    if ( p.z()*v.z() < 0 )    // at +Z going in -Z or visa versa
    {
      s = (std::fabs(p.z()) - fDz)/std::fabs(v.z()) ;  // Z intersect distance

      if( s < 0.0 ) s = 0.0 ;                  // negative dist -> zero

      xi   = p.x() + s*v.x() ;  // Intersection coords
      yi   = p.y() + s*v.y() ;
      rhoi2 = xi*xi + yi*yi   ;

      // Check validity of intersection
      // Calculate (outer) tolerant radi^2 at intersecion

      if (v.z() > 0)
      {
        tolORMin  = fRmin1 - 0.5*kRadTolerance*secRMin ;
        tolIRMin  = fRmin1 + 0.5*kRadTolerance*secRMin ;
        tolIRMax  = fRmax1 - 0.5*kRadTolerance*secRMin ;
        tolORMax2 = (fRmax1 + 0.5*kRadTolerance*secRMax)*
                    (fRmax1 + 0.5*kRadTolerance*secRMax) ;
      }
      else
      {
        tolORMin  = fRmin2 - 0.5*kRadTolerance*secRMin ;
        tolIRMin  = fRmin2 + 0.5*kRadTolerance*secRMin ;
        tolIRMax  = fRmax2 - 0.5*kRadTolerance*secRMin ;
        tolORMax2 = (fRmax2 + 0.5*kRadTolerance*secRMax)*
                    (fRmax2 + 0.5*kRadTolerance*secRMax) ;
      }
      if ( tolORMin > 0 ) 
      {
        tolORMin2 = tolORMin*tolORMin ;
        tolIRMin2 = tolIRMin*tolIRMin ;
      }
      else                
      {
        tolORMin2 = 0.0 ;
        tolIRMin2 = 0.0 ;
      }
      if ( tolIRMax > 0 )   tolIRMax2 = tolIRMax*tolIRMax ;      
      else                  tolIRMax2 = 0.0 ;
      
      if (tolIRMin2 <= rhoi2 && rhoi2 <= tolIRMax2)
      {
  if ( seg && rhoi2 )
  {
    // Psi = angle made with central (average) phi of shape

    cosPsi = (xi*cosCPhi + yi*sinCPhi)/std::sqrt(rhoi2) ;

    if (cosPsi >= cosHDPhiIT) return s ;
  }
  else return s ;
      }
      /*
      else if (tolORMin2 <= rhoi2 && rhoi2 <= tolORMax2)
      {
  if ( seg && rhoi2 )
  {
    // Psi = angle made with central (average) phi of shape

    cosPsi = (xi*cosCPhi + yi*sinCPhi)/std::sqrt(rhoi2) ;

    if (cosPsi >= cosHDPhiIT) return s ;
  }
  else return s ;
      }
      */
    }
    else  // On/outside extent, and heading away  -> cannot intersect
    {
      return snxt ;  
    }
  }
    
// ----> Can not intersect z surfaces


// Intersection with outer cone (possible return) and
//                   inner cone (must also check phi)
//
// Intersection point (xi,yi,zi) on line x=p.x+t*v.x etc.
//
// Intersects with x^2+y^2=(a*z+b)^2
//
// where a=tanRMax or tanRMin
//       b=rMaxAv  or rMinAv
//
// (vx^2+vy^2-(a*vz)^2)t^2+2t(pxvx+pyvy-a*vz(a*pz+b))+px^2+py^2-(a*pz+b)^2=0 ;
//     t1                        t2                      t3  
//
//  \--------u-------/       \-----------v----------/ \---------w--------/
//

  t1   = 1.0 - v.z()*v.z() ;
  t2   = p.x()*v.x() + p.y()*v.y() ;
  t3   = p.x()*p.x() + p.y()*p.y() ;
  rin  = tanRMin*p.z() + rMinAv ;
  rout = tanRMax*p.z() + rMaxAv ;

  // Outer Cone Intersection
  // Must be outside/on outer cone for valid intersection

  nt1 = t1 - (tanRMax*v.z())*(tanRMax*v.z()) ;
  nt2 = t2 - tanRMax*v.z()*rout ;
  nt3 = t3 - rout*rout ;

  if (std::fabs(nt1) > kRadTolerance)  // Equation quadratic => 2 roots
  {
      b = nt2/nt1 ;
      c = nt3/nt1 ;
      d = b*b-c   ;
    if ( nt3 > rout*kRadTolerance*secRMax || rout < 0 )
    {
      // If outside real cone (should be rho-rout>kRadTolerance*0.5
      // NOT rho^2 etc) saves a std::sqrt() at expense of accuracy


      if (d >= 0)
      {
          
        if (rout < 0 && nt3 <= 0 )
        {
          // Inside `shadow cone' with -ve radius
          // -> 2nd root could be on real cone

          s = -b + std::sqrt(d) ;
        }
        else
        {
          if ( b <= 0 && c >= 0 ) // both >=0, try smaller root
          {
            s = -b - std::sqrt(d) ;
          }
          else
          {
            if ( c <= 0 ) // second >=0
            {
              s = -b + std::sqrt(d) ;
            }
            else  // both negative, travel away
            {
              return kInfinity ;
            }
          }
        }
        if ( s > 0 )  // If 'forwards'. Check z intersection
        {
          zi = p.z() + s*v.z() ;

          if (std::fabs(zi) <= tolODz)
          {
            // Z ok. Check phi intersection if reqd

            if ( ! seg )  return s ;
            else
            {
              xi     = p.x() + s*v.x() ;
              yi     = p.y() + s*v.y() ;
              ri     = rMaxAv + zi*tanRMax ;
              cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri ;

              if ( cosPsi >= cosHDPhiIT ) return s ;
            }
          }
        }                // end if (s>0)
      }
    }
    else
    {
      // Inside outer cone
      // check not inside, and heading through G4Cons (-> 0 to in)

      if ( t3  > (rin + kRadTolerance*0.5*secRMin)*
                 (rin + kRadTolerance*0.5*secRMin) && 
	   nt2 < 0                                 && 
	   d >= 0                                  && 
	   //  nt2 < -kCarTolerance*secRMax/2/fDz                  && 
           // t2 < std::sqrt(t3)*v.z()*tanRMax                          && 
	   // d > kCarTolerance*secRMax*(rout-b*tanRMax*v.z())/nt1 && 
           std::fabs(p.z()) <= tolIDz )
      {
        // Inside cones, delta r -ve, inside z extent

        if (seg)
        {
          cosPsi = (p.x()*cosCPhi + p.y()*sinCPhi)/std::sqrt(t3) ;

          if (cosPsi >= cosHDPhiIT) return 0.0 ;
        }
        else  return 0.0 ;
      }
    }
  }
  else  //  Single root case 
  {
    if ( std::fabs(nt2) > kRadTolerance )
    {
      s = -0.5*nt3/nt2 ;

      if ( s < 0 )   return kInfinity ;   // travel away
      else  // s >= 0,  If 'forwards'. Check z intersection
      {
        zi = p.z() + s*v.z() ;

        if (std::fabs(zi) <= tolODz && nt2 < 0)
        {
          // Z ok. Check phi intersection if reqd

          if ( ! seg ) return s ;
          else
          {
            xi     = p.x() + s*v.x() ;
            yi     = p.y() + s*v.y() ;
            ri     = rMaxAv + zi*tanRMax ;
            cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri ;

            if (cosPsi >= cosHDPhiIT) return s ;
          }
        }
      }
    }
    else  //    travel || cone surface from its origin
    {
      s = kInfinity ;
    }
  }

  // Inner Cone Intersection
  // o Space is divided into 3 areas:
  //   1) Radius greater than real inner cone & imaginary cone & outside
  //      tolerance
  //   2) Radius less than inner or imaginary cone & outside tolarance
  //   3) Within tolerance of real or imaginary cones
  //      - Extra checks needed for 3's intersections
  //        => lots of duplicated code

  if (rMinAv)
  {
    nt1 = t1 - (tanRMin*v.z())*(tanRMin*v.z()) ;
    nt2 = t2 - tanRMin*v.z()*rin ;
    nt3 = t3 - rin*rin ;
 
    if ( nt1 )
    {
      if ( nt3 > rin*kRadTolerance*secRMin )
      {
        // At radius greater than real & imaginary cones
        // -> 2nd root, with zi check

        b = nt2/nt1 ;
        c = nt3/nt1 ;
        d = b*b-c ;
        if (d >= 0)   // > 0
        {
          s = -b + std::sqrt(d) ;

          if ( s >= 0 )   // > 0
          {
            zi = p.z() + s*v.z() ;

            if ( std::fabs(zi) <= tolODz )
            {
              if ( seg )
              {
                xi     = p.x() + s*v.x() ;
                yi     = p.y() + s*v.y() ;
                ri     = rMinAv + zi*tanRMin ;
                cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri ;

                if (cosPsi >= cosHDPhiIT) snxt = s ; 
              }
              else return s ;
            }
          }
        }
      }
      else  if ( nt3 < -rin*kRadTolerance*secRMin )
      {
        // Within radius of inner cone (real or imaginary)
        // -> Try 2nd root, with checking intersection is with real cone
        // -> If check fails, try 1st root, also checking intersection is
        //    on real cone

        b = nt2/nt1 ;
        c = nt3/nt1 ;
        d = b*b - c ;

        if ( d >= 0 )  // > 0
        {
          s  = -b + std::sqrt(d) ;
          zi = p.z() + s*v.z() ;
          ri = rMinAv + zi*tanRMin ;

          if ( ri >= 0 )
          {
            if ( s >= 0 && std::fabs(zi) <= tolODz )  // s > 0
            {
              if ( seg )
              {
                xi     = p.x() + s*v.x() ;
                yi     = p.y() + s*v.y() ;
                cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri ;

                if (cosPsi >= cosHDPhiOT) snxt = s ; 
              }
              else  return s ;
            }
          }
          else
          {
            s  = -b - std::sqrt(d) ;
            zi = p.z() + s*v.z() ;
            ri = rMinAv + zi*tanRMin ;

            if ( s >= 0 && ri >= 0 && std::fabs(zi) <= tolODz ) // s>0
            {
              if ( seg )
              {
                xi     = p.x() + s*v.x() ;
                yi     = p.y() + s*v.y() ;
                cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri ;

                if (cosPsi >= cosHDPhiIT) snxt = s ;
              }
              else  return s ;
            }
          }
        }
      }
      else
      {
        // Within kRadTol*0.5 of inner cone (real OR imaginary)
        // ----> Check not travelling through (=>0 to in)
        // ----> if not:
        //    -2nd root with validity check

        if ( std::fabs(p.z()) <= tolODz )
        {
          if ( nt2 > 0 )
          {
            // Inside inner real cone, heading outwards, inside z range

            if ( seg )
            {
              cosPsi = (p.x()*cosCPhi + p.y()*sinCPhi)/std::sqrt(t3) ;

              if (cosPsi >= cosHDPhiIT)  return 0.0 ;
            }
            else  return 0.0 ;
          }
          else
          {
            // Within z extent, but not travelling through
            // -> 2nd root or kInfinity if 1st root on imaginary cone

            b = nt2/nt1 ;
            c = nt3/nt1 ;
            d = b*b - c ;

            if ( d >= 0 )   // > 0
            {
              s  = -b - std::sqrt(d) ;
              zi = p.z() + s*v.z() ;
              ri = rMinAv + zi*tanRMin ;

              if ( ri > 0 )   // 2nd root
              {
                s  = -b + std::sqrt(d) ;
                zi = p.z() + s*v.z() ;

                if ( s >= 0 && std::fabs(zi) <= tolODz )  // s>0
                {
                  if ( seg )
                  {
                    xi     = p.x() + s*v.x() ;
                    yi     = p.y() + s*v.y() ;
                    ri     = rMinAv + zi*tanRMin ;
                    cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri ;

                    if ( cosPsi >= cosHDPhiIT )  snxt = s ; 
                  }
                  else return s ;
                }
              }
              else  return kInfinity ;
            }
          }
        }
        else   // 2nd root
        {
          b = nt2/nt1 ;
          c = nt3/nt1 ;
          d = b*b - c ;

          if ( d > 0 )
          {  
            s  = -b + std::sqrt(d) ;
            zi = p.z() + s*v.z() ;

            if ( s >= 0 && std::fabs(zi) <= tolODz )  // s>0
            {
              if ( seg )
              {
                xi     = p.x() + s*v.x();
                yi     = p.y() + s*v.y();
                ri     = rMinAv + zi*tanRMin ;
                cosPsi = (xi*cosCPhi + yi*sinCPhi)/ri;

                if (cosPsi >= cosHDPhiIT) snxt = s ; 
              }
              else  return s ;
            }
          }
        }
      }
    }
  }

  // Phi segment intersection
  //
  // o Tolerant of points inside phi planes by up to kCarTolerance*0.5
  //
  // o NOTE: Large duplication of code between sphi & ephi checks
  //         -> only diffs: sphi -> ephi, Comp -> -Comp and half-plane
  //            intersection check <=0 -> >=0
  //         -> Should use some form of loop Construct

  if ( seg )
  {
    // First phi surface (`S'tarting phi)

    sinSPhi = std::sin(fSPhi) ;
    cosSPhi = std::cos(fSPhi) ;
    Comp    = v.x()*sinSPhi - v.y()*cosSPhi ;
                    
    if ( Comp < 0 )    // Component in outwards normal dirn
    {
      Dist = (p.y()*cosSPhi - p.x()*sinSPhi) ;

      if (Dist < kCarTolerance*0.5)
      {
        s = Dist/Comp ;

        if ( s < snxt )
        {
          if ( s < 0 ) s = 0.0 ;

          zi = p.z() + s*v.z() ;

          if ( std::fabs(zi) <= tolODz )
          {
            xi        = p.x() + s*v.x() ;
            yi        = p.y() + s*v.y() ;
            rhoi2     = xi*xi + yi*yi ;
            tolORMin2 = (rMinOAv + zi*tanRMin)*(rMinOAv + zi*tanRMin) ;
            tolORMax2 = (rMaxOAv + zi*tanRMax)*(rMaxOAv + zi*tanRMax) ;

            if ( rhoi2 >= tolORMin2 && rhoi2 <= tolORMax2 )
            {
              // z and r intersections good - check intersecting with
              // correct half-plane

              if ((yi*cosCPhi - xi*sinCPhi) <= 0 ) snxt = s ;
            }    
          }
        }
      }
    }    
    // Second phi surface (`E'nding phi)

    ePhi    = fSPhi + fDPhi ;
    sinEPhi = std::sin(ePhi) ;
    cosEPhi = std::cos(ePhi) ;
    Comp    = -(v.x()*sinEPhi - v.y()*cosEPhi) ;
        
    if ( Comp < 0 )   // Component in outwards normal dirn
    {
      Dist = -(p.y()*cosEPhi - p.x()*sinEPhi) ;
      if (Dist < kCarTolerance*0.5)
      {
        s = Dist/Comp ;

        if ( s < snxt )
        {
          if ( s < 0 )  s = 0.0 ;

          zi = p.z() + s*v.z() ;

          if (std::fabs(zi) <= tolODz)
          {
            xi        = p.x() + s*v.x() ;
            yi        = p.y() + s*v.y() ;
            rhoi2     = xi*xi + yi*yi ;
            tolORMin2 = (rMinOAv + zi*tanRMin)*(rMinOAv + zi*tanRMin) ;
            tolORMax2 = (rMaxOAv + zi*tanRMax)*(rMaxOAv + zi*tanRMax) ;

            if ( rhoi2 >= tolORMin2 && rhoi2 <= tolORMax2 )
            {
              // z and r intersections good - check intersecting with
              // correct half-plane

              if ( (yi*cosCPhi - xi*sinCPhi) >= 0.0 )  snxt = s ;
            }    
          }
        }
      }
    }
  }
  if (snxt < kCarTolerance*0.5) snxt = 0.;
  return snxt ;
}

//////////////////////////////////////////////////////////////////////////////
// 
// Calculate distance (<= actual) to closest surface of shape from outside
// - Calculate distance to z, radial planes
// - Only to phi planes if outside phi extent
// - Return 0 if point inside

G4double G4Cons::DistanceToIn(const G4ThreeVector& p) const
{
  G4double safe=0.0, rho, safeR1, safeR2, safeZ ;
  G4double tanRMin, secRMin, pRMin ;
  G4double tanRMax, secRMax, pRMax ;
  G4double phiC, cosPhiC, sinPhiC, safePhi, ePhi ;
  G4double cosPsi ;

  rho   = std::sqrt(p.x()*p.x() + p.y()*p.y()) ;
  safeZ = std::fabs(p.z()) - fDz ;

  if ( fRmin1 || fRmin2 )
  {
    tanRMin = (fRmin2 - fRmin1)*0.5/fDz ;
    secRMin = std::sqrt(1.0 + tanRMin*tanRMin) ;
    pRMin   = tanRMin*p.z() + (fRmin1 + fRmin2)*0.5 ;
    safeR1  = (pRMin - rho)/secRMin ;

    tanRMax = (fRmax2 - fRmax1)*0.5/fDz ;
    secRMax = std::sqrt(1.0 + tanRMax*tanRMax) ;
    pRMax   = tanRMax*p.z() + (fRmax1 + fRmax2)*0.5 ;
    safeR2  = (rho - pRMax)/secRMax ;

    if ( safeR1 > safeR2) safe = safeR1 ;
    else                  safe = safeR2 ;
  }
  else
  {
    tanRMax = (fRmax2 - fRmax1)*0.5/fDz ;
    secRMax = std::sqrt(1.0 + tanRMax*tanRMax) ;
    pRMax   = tanRMax*p.z() + (fRmax1 + fRmax2)*0.5 ;
    safe    = (rho - pRMax)/secRMax ;
  }
  if ( safeZ > safe ) safe = safeZ ;

  if ( fDPhi < twopi && rho )
  {
    phiC    = fSPhi + fDPhi*0.5 ;
    cosPhiC = std::cos(phiC) ;
    sinPhiC = std::sin(phiC) ;

    // Psi=angle from central phi to point

    cosPsi = (p.x()*cosPhiC + p.y()*sinPhiC)/rho ;

    if ( cosPsi < std::cos(fDPhi*0.5) ) // Point lies outside phi range
    {
      if ( (p.y()*cosPhiC - p.x()*sinPhiC) <= 0.0 )
      {
        safePhi = std::fabs(p.x()*std::sin(fSPhi) - p.y()*std::cos(fSPhi)) ;
      }
      else
      {
        ePhi    = fSPhi + fDPhi ;
        safePhi = std::fabs(p.x()*std::sin(ePhi) - p.y()*std::cos(ePhi)) ;
      }
      if ( safePhi > safe ) safe = safePhi ;
    }
  }
  if ( safe < 0.0 ) safe = 0.0 ;

  return safe ;
}

///////////////////////////////////////////////////////////////
//
// Calculate distance to surface of shape from `inside', allowing for tolerance
// - Only Calc rmax intersection if no valid rmin intersection

G4double G4Cons::DistanceToOut( const G4ThreeVector& p,
              const G4ThreeVector& v,
              const G4bool calcNorm,
                    G4bool *validNorm,
                    G4ThreeVector *n) const
{
  ESide side = kNull, sider = kNull, sidephi = kNull;

  G4double snxt,sr,sphi,pdist ;

  G4double tanRMax, secRMax, rMaxAv ;  // Data for outer cone
  G4double tanRMin, secRMin, rMinAv ;  // Data for inner cone

  G4double t1, t2, t3, rout, rin, nt1, nt2, nt3 ;
  G4double b, c, d, sr2, sr3 ;

  // Vars for intersection within tolerance

  ESide    sidetol ;
  G4double slentol = kInfinity ;

  // Vars for phi intersection:

  G4double sinSPhi, cosSPhi, ePhi, sinEPhi, cosEPhi ;
  G4double cPhi, sinCPhi, cosCPhi ;
  G4double pDistS, compS, pDistE, compE, sphi2, xi, yi, risec, vphi ;
  G4double zi, ri, deltaRoi2 ;

  // Z plane intersection

  if ( v.z() > 0.0 )
  {
    pdist = fDz - p.z() ;

    if (pdist > kCarTolerance*0.5)
    {
      snxt = pdist/v.z() ;
      side = kPZ ;
    }
    else
    {
      if (calcNorm)
      {
        *n         = G4ThreeVector(0,0,1) ;
        *validNorm = true ;
      }
      return snxt = 0.0 ;
    }
  }
  else if ( v.z() < 0.0 )
  {
    pdist = fDz + p.z() ;

    if ( pdist > kCarTolerance*0.5)
    {
      snxt = -pdist/v.z() ;
      side = kMZ ;
    }
    else
    {
      if ( calcNorm )
      {
        *n         = G4ThreeVector(0,0,-1) ;
        *validNorm = true ;
      }
      return snxt = 0.0 ;
    }
  }
  else     // Travel perpendicular to z axis
  {
    snxt = kInfinity ;    
    side = kNull ;
  }

  // Radial Intersections
  //
  // Intersection with outer cone (possible return) and
  //                   inner cone (must also check phi)
  //
  // Intersection point (xi,yi,zi) on line x=p.x+t*v.x etc.
  //
  // Intersects with x^2+y^2=(a*z+b)^2
  //
  // where a=tanRMax or tanRMin
  //       b=rMaxAv  or rMinAv
  //
  // (vx^2+vy^2-(a*vz)^2)t^2+2t(pxvx+pyvy-a*vz(a*pz+b))+px^2+py^2-(a*pz+b)^2=0 ;
  //     t1                        t2                      t3  
  //
  //  \--------u-------/       \-----------v----------/ \---------w--------/

  tanRMax = (fRmax2 - fRmax1)*0.5/fDz ;
  secRMax = std::sqrt(1.0 + tanRMax*tanRMax) ;
  rMaxAv  = (fRmax1 + fRmax2)*0.5 ;


  t1   = 1.0 - v.z()*v.z() ;      // since v normalised
  t2   = p.x()*v.x() + p.y()*v.y() ;
  t3   = p.x()*p.x() + p.y()*p.y() ;
  rout = tanRMax*p.z() + rMaxAv ;

  nt1 = t1 - (tanRMax*v.z())*(tanRMax*v.z()) ;
  nt2 = t2 - tanRMax*v.z()*rout ;
  nt3 = t3 - rout*rout ;

  if (v.z() > 0.0)
  {
    deltaRoi2 = snxt*snxt*t1 + 2*snxt*t2 + t3
                - fRmax2*(fRmax2 + kRadTolerance*secRMax);
  }
  else if ( v.z() < 0.0 )
  {
    deltaRoi2 = snxt*snxt*t1 + 2*snxt*t2 + t3
                - fRmax1*(fRmax1 + kRadTolerance*secRMax);
  }
  else deltaRoi2 = 1.0 ;

  if ( nt1 && deltaRoi2 > 0.0 )  
  {
    // Equation quadratic => 2 roots : second root must be leaving

    b = nt2/nt1 ;
    c = nt3/nt1 ;
    d = b*b - c ;

    if ( d >= 0 )
    {
      // Check if on outer cone & heading outwards
      // NOTE: Should use rho-rout>-kRadtolerance*0.5
        
      if (nt3 > -kRadTolerance*0.5 && nt2 >= 0 )
      {
        if (calcNorm)
        {
          risec      = std::sqrt(t3)*secRMax ;
          *validNorm = true ;
          *n         = G4ThreeVector(p.x()/risec,p.y()/risec,-tanRMax/secRMax) ;
        }
        return snxt=0 ;
      }
      else
      {
        sider = kRMax  ;
        sr    = -b - std::sqrt(d) ; // was +srqrt(d), vmg 28.04.99
        zi    = p.z() + sr*v.z() ;
        ri    = tanRMax*zi + rMaxAv ;
          
        if ( (ri >= 0) && (-kRadTolerance*0.5 <= sr) && 
                          ( sr <= kRadTolerance*0.5)     )
        {
          // An intersection within the tolerance
          //   we will Store it in case it is good -
          // 
          slentol = sr ;
          sidetol = kRMax ;
        }            
        if ( (ri < 0) || (sr < kRadTolerance*0.5) )
        {
          // Safety: if both roots -ve ensure that sr cannot `win'
          //         distance to out

          sr2 = -b + std::sqrt(d) ;
          zi  = p.z() + sr2*v.z() ;
          ri  = tanRMax*zi + rMaxAv ;

          if (ri >= 0 && sr2 > kRadTolerance*0.5)  sr = sr2 ;
          else
          {
            sr = kInfinity ;

            if(    (-kRadTolerance*0.5 <= sr2)
                && ( sr2 <= kRadTolerance*0.5)  )
            {
              // An intersection within the tolerance.
              // Storing it in case it is good.

              slentol = sr2 ;
              sidetol = kRMax ;
            }
          }
        }
      }
    }
    else
    {
      // No intersection with outer cone & not parallel
      // -> already outside, no intersection

      if ( calcNorm )
      {
        risec      = std::sqrt(t3)*secRMax ;
        *validNorm = true ;
        *n         = G4ThreeVector(p.x()/risec,p.y()/risec,-tanRMax/secRMax) ;
      }
      return snxt = 0.0 ;
    }
  }
  else if ( nt2 && deltaRoi2 > 0.0 )
  {
    // Linear case (only one intersection) => point outside outer cone

    if ( calcNorm )
    {
      risec      = std::sqrt(t3)*secRMax ;
      *validNorm = true ;
      *n         = G4ThreeVector(p.x()/risec,p.y()/risec,-tanRMax/secRMax) ;
    }
    return snxt = 0.0 ;
  }
  else
  {
    // No intersection -> parallel to outer cone
    // => Z or inner cone intersection

    sr = kInfinity ;
  }

  // Check possible intersection within tolerance

  if ( slentol <= kCarTolerance*0.5 )
  {
    // An intersection within the tolerance was found.  
    // We must accept it only if the momentum points outwards.  
    //
    // G4ThreeVector ptTol ;  // The point of the intersection  
    // ptTol= p + slentol*v ;
    // ri=tanRMax*zi+rMaxAv ;
    //
    // Calculate a normal vector,  as below

    xi    = p.x() + slentol*v.x() ;
    yi    = p.y() + slentol*v.y() ;
    risec = std::sqrt(xi*xi + yi*yi)*secRMax ;
    G4ThreeVector Normal = G4ThreeVector(xi/risec,yi/risec,-tanRMax/secRMax) ;

    if ( Normal.dot(v) > 0 )    // We will leave the Cone immediatelly
    {
      if ( calcNorm ) 
      {
        *n         = Normal.unit() ;
        *validNorm = true ;
      }
      return snxt = 0.0 ;
    }
    else // On the surface, but not heading out so we ignore this intersection
    {    //                                    (as it is within tolerance).  
      slentol = kInfinity ;
    }
  }

  // Inner Cone intersection

  if ( fRmin1 || fRmin2 )
  {
    tanRMin = (fRmin2 - fRmin1)*0.5/fDz ;
    nt1     = t1 - (tanRMin*v.z())*(tanRMin*v.z()) ;

    if ( nt1 )
    {
      secRMin = std::sqrt(1.0 + tanRMin*tanRMin) ;
      rMinAv  = (fRmin1 + fRmin2)*0.5 ;    
      rin     = tanRMin*p.z() + rMinAv ;
      nt2     = t2 - tanRMin*v.z()*rin ;
      nt3     = t3 - rin*rin ;
      
      // Equation quadratic => 2 roots : first root must be leaving

      b = nt2/nt1 ;
      c = nt3/nt1 ;
      d = b*b - c ;

      if (d >= 0.0 )
      {
        // NOTE: should be rho-rin<kRadTolerance*0.5,
        //       but using squared versions for efficiency

        if (nt3 < kRadTolerance*(rin + kRadTolerance*0.25)) 
        {
          if ( nt2 < 0.0 )
          {
            if (calcNorm)  *validNorm = false ;
            return          snxt      = 0.0 ;
          }
        }
        else
        {
          sr2 = -b - std::sqrt(d) ;
          zi  = p.z() + sr2*v.z() ;
          ri  = tanRMin*zi + rMinAv ;

          if( (ri >= 0.0) && (-kRadTolerance*0.5 <= sr2) && 
                             ( sr2 <= kRadTolerance*0.5)      )
          {
            // An intersection within the tolerance
            // storing it in case it is good.

            slentol = sr2 ;
            sidetol = kRMax ;
          }
          if( (ri<0) || (sr2 < kRadTolerance*0.5) )
          {
            sr3 = -b + std::sqrt(d) ;

            // Safety: if both roots -ve ensure that sr cannot `win'
            //         distancetoout

            if  ( sr3 > kCarTolerance*0.5 )
            {
              if( sr3 < sr )
              {
                zi = p.z() + sr3*v.z() ;
                ri = tanRMin*zi + rMinAv ;

                if ( ri >= 0.0 )
                {
                  sr=sr3 ;
                  sider=kRMin ;
                }
              } 
            }
            else if ( sr3 > -kCarTolerance*0.5 )
            {
              // Intersection in tolerance. Store to check if it's good

              slentol = sr3 ;
              sidetol = kRMin ;
            }
          }
          else if ( sr2 < sr && sr2 > kCarTolerance*0.5 )
          {
            sr    = sr2 ;
            sider = kRMin ;
          }
          else if (sr2 > -kCarTolerance*0.5)
          {
            // Intersection in tolerance. Store to check if it's good

            slentol = sr2 ;
            sidetol = kRMin ;
          }    
          if( slentol <= kCarTolerance*0.5  )
          {
            // An intersection within the tolerance was found. 
            // We must accept it only if  the momentum points outwards. 

            G4ThreeVector Normal ; 
            
            // Calculate a normal vector,  as below

            xi     = p.x() + slentol*v.x() ;
            yi     = p.y() + slentol*v.y() ;
            risec  = std::sqrt(xi*xi + yi*yi)*secRMin ;
            Normal = G4ThreeVector(xi/risec,yi/risec,-tanRMin/secRMin) ;
            
            if( Normal.dot(v) > 0 )
            {
              // We will leave the Cone immediatelly
              if( calcNorm ) 
              {
                *n         = Normal.unit() ;
                *validNorm = true ;
              }
              return snxt = 0.0 ;
            }
            else 
            { 
              // On the surface, but not heading out so we ignore this
              // intersection (as it is within tolerance). 

              slentol = kInfinity ;
            }        
          }
        }
      }
    }
  }

  // Linear case => point outside inner cone ---> outer cone intersect
  //
  // Phi Intersection
  
  if ( fDPhi < twopi )
  {
    sinSPhi = std::sin(fSPhi) ;
    cosSPhi = std::cos(fSPhi) ;
    ePhi    = fSPhi + fDPhi ;
    sinEPhi = std::sin(ePhi) ;
    cosEPhi = std::cos(ePhi) ;
    cPhi    = fSPhi + fDPhi*0.5 ;
    sinCPhi = std::sin(cPhi) ;
    cosCPhi = std::cos(cPhi) ;

    if ( p.x() || p.y() )   // Check if on z axis (rho not needed later)
    {
      // pDist -ve when inside

      pDistS = p.x()*sinSPhi - p.y()*cosSPhi ;
      pDistE = -p.x()*sinEPhi + p.y()*cosEPhi ;

      // Comp -ve when in direction of outwards normal

      compS = -sinSPhi*v.x() + cosSPhi*v.y() ;
      compE = sinEPhi*v.x() - cosEPhi*v.y() ;

      sidephi = kNull ;

      if (pDistS <= 0 && pDistE <= 0)
      {
        // Inside both phi *full* planes

        if (compS < 0)
        {
          sphi = pDistS/compS ;
          xi   = p.x() + sphi*v.x() ;
          yi   = p.y() + sphi*v.y() ;

          // Check intersecting with correct half-plane
          // (if not -> no intersect)

          if ( (yi*cosCPhi - xi*sinCPhi) >= 0.0 ) sphi = kInfinity ;
          else
          {
            sidephi = kSPhi ;

            if ( pDistS > -kCarTolerance*0.5 ) // Leave by sphi immediately
            {
              sphi = 0.0 ;
            }      
          }
        }
        else sphi = kInfinity ;

        if ( compE < 0.0 )
        {
          // Only check further if < starting phi intersection

          sphi2 = pDistE/compE ;

          if ( sphi2 < sphi )
          {
            xi = p.x() + sphi2*v.x() ;
            yi = p.y() + sphi2*v.y() ;

            // Check intersecting with correct half-plane 

            if ( (yi*cosCPhi - xi*sinCPhi) >= 0.0 )  // Leaving via ending phi
            {
              sidephi = kEPhi ;

              if ( pDistE <= -kCarTolerance*0.5 )  sphi = sphi2 ;
              else                                 sphi = 0.0   ;
            }
          }
        }        
      }
      else if ( pDistS >= 0 && pDistE >= 0 )
      {
        // Outside both *full* phi planes

        if ( pDistS <= pDistE )  sidephi = kSPhi ;
        else                     sidephi = kEPhi ;

        if ( fDPhi > pi )
        {
          if (compS < 0 && compE < 0) sphi = 0.0 ;
          else                        sphi = kInfinity ;
        }
        else
        {
          // if towards both >=0 then once inside (after error)
          // will remain inside

          if ( compS >= 0.0 && compE >= 0.0 ) sphi = kInfinity ;
          else                                sphi = 0.0 ;
        }        
      }
      else if ( pDistS > 0.0 && pDistE < 0.0 )
      {
        // Outside full starting plane, inside full ending plane

        if ( fDPhi > pi )
        {
          if ( compE < 0.0 )
          {
            sphi = pDistE/compE ;
            xi   = p.x() + sphi*v.x() ;
            yi   = p.y() + sphi*v.y() ;

            // Check intersection in correct half-plane
            // (if not -> not leaving phi extent)

            if ( (yi*cosCPhi - xi*sinCPhi) <= 0.0 )
            {
              sphi = kInfinity ;
            }
            else    // Leaving via Ending phi
            {
              sidephi = kEPhi  ;

              if ( pDistE > -kCarTolerance*0.5 ) sphi = 0.0 ;
            }
          }
          else sphi = kInfinity ;
        }
        else  // fDPhi <= pi
        {
          if (compS >= 0.0 )
          {
            if ( compE < 0.0 )
            {            
              sphi = pDistE/compE ;
              xi   = p.x() + sphi*v.x() ;
              yi   = p.y() + sphi*v.y() ;

              // Check intersection in correct half-plane
              // (if not -> remain in extent)

              if ( (yi*cosCPhi - xi*sinCPhi) <= 0.0 )
              {
                sphi = kInfinity ;
              }
              else  // otherwise leaving via Ending phi
              {
                sidephi = kEPhi ;
              }
            }
            else sphi = kInfinity ;
          }
          else  // leaving immediately by starting phi
          {
            sidephi = kSPhi ;
            sphi    = 0.0 ;
          }
        }
      }
      else
      {
        // Must be pDistS<0&&pDistE>0
        // Inside full starting plane, outside full ending plane

        if (fDPhi > pi)
        {
          if ( compS < 0.0 )
          {
            sphi = pDistS/compS ;
            xi   = p.x() + sphi*v.x() ;
            yi   = p.y() + sphi*v.y() ;

            // Check intersection in correct half-plane
            // (if not -> not leaving phi extent)

            if ( (yi*cosCPhi - xi*sinCPhi) >= 0.0 )  sphi = kInfinity ;
            else     // Leaving via Starting phi
            {
              sidephi = kSPhi  ; 
  
              if ( pDistS > -kCarTolerance*0.5 )  sphi = 0.0 ;
            }
          }
          else sphi = kInfinity ;
        }
        else  // fDPhi <= pi
        {
          if ( compE >= 0.0 )
          {
            if ( compS < 0.0 )
            {
                
              sphi = pDistS/compS ;
              xi   = p.x() + sphi*v.x() ;
              yi   = p.y() + sphi*v.y() ;

              // Check intersection in correct half-plane
              // (if not -> remain in extent)

              if ( (yi*cosCPhi - xi*sinCPhi) >= 0.0 )  sphi = kInfinity ;
              else // otherwise leaving via Starting phi
              {
                sidephi = kSPhi ;
              }
            }
            else sphi = kInfinity ;
          }
          else // CompE < 0, leaving immediately by ending
          {
            sidephi = kEPhi ;
            sphi    = 0.0 ;
          }
        }
      }    
    }
    else
    {
      // On z axis + travel not || to z axis -> if phi of vector direction
      // within phi of shape, Step limited by rmax, else Step =0

      vphi = std::atan2(v.y(),v.x()) ;

      if ( fSPhi < vphi && vphi < fSPhi + fDPhi ) sphi = kInfinity ;
      else
      {
        sidephi = kSPhi  ;   // arbitrary 
        sphi    = 0.0 ;
      }
    }      
    if ( sphi < snxt )  // Order intersecttions
    {
        snxt=sphi ;
        side=sidephi ;
    }
  }
  if ( sr < snxt )  // Order intersections
  {
    snxt = sr    ;
    side = sider ;
  }
  if (calcNorm)
  {
    switch(side)
    {
      case kRMax:
          // Note: returned vector not normalised
          // (divide by frmax for unit vector)
        xi         = p.x() + snxt*v.x() ;
        yi         = p.y() + snxt*v.y() ;
        risec      = std::sqrt(xi*xi + yi*yi)*secRMax ;
        *n         = G4ThreeVector(xi/risec,yi/risec,-tanRMax/secRMax) ;
        *validNorm = true ;
        break ;
      case kRMin:
        *validNorm=false ;  // Rmin is inconvex
        break ;
      case kSPhi:
        if ( fDPhi <= pi )
        {
          *n         = G4ThreeVector(std::sin(fSPhi),-std::cos(fSPhi),0) ;
          *validNorm = true ;
        }
        else   *validNorm = false ;
        break ;
      case kEPhi:
        if ( fDPhi <= pi )
        {
          *n         = G4ThreeVector(-std::sin(fSPhi+fDPhi),std::cos(fSPhi+fDPhi),0) ;
          *validNorm = true ;
        }
        else  *validNorm = false ;
        break ;
      case kPZ:
        *n         = G4ThreeVector(0,0,1) ;
        *validNorm = true ;
        break ;
      case kMZ:
        *n         = G4ThreeVector(0,0,-1) ;
        *validNorm = true ;
        break ;
      default:
        G4cout.precision(16) ;
        G4cout << G4endl ;
        DumpInfo();
        G4cout << "Position:"  << G4endl << G4endl ;
        G4cout << "p.x() = "   << p.x()/mm << " mm" << G4endl ;
        G4cout << "p.y() = "   << p.y()/mm << " mm" << G4endl ;
        G4cout << "p.z() = "   << p.z()/mm << " mm" << G4endl << G4endl ;
        G4cout << "pho at z = "   << std::sqrt( p.x()*p.x()+p.y()*p.y() )/mm << " mm"
               << G4endl << G4endl ;
        if( p.x() != 0. || p.x() != 0.)
        {
           G4cout << "point phi = "   << std::atan2(p.y(),p.x())/degree << " degree" 
                  << G4endl << G4endl ; 
        }
        G4cout << "Direction:" << G4endl << G4endl ;
        G4cout << "v.x() = "   << v.x() << G4endl ;
        G4cout << "v.y() = "   << v.y() << G4endl ;
        G4cout << "v.z() = "   << v.z() << G4endl<< G4endl ;
        G4cout << "Proposed distance :" << G4endl<< G4endl ;
        G4cout << "snxt = "    << snxt/mm << " mm" << G4endl << G4endl ;
        G4Exception("G4Cons::DistanceToOut(p,v,..)","Notification",JustWarning,
                    "Undefined side for valid surface normal to solid.") ;
        break ;
    }
  }
  if (snxt < kCarTolerance*0.5) snxt = 0.;
  return snxt ;
}

//////////////////////////////////////////////////////////////////
//
// Calculate distance (<=actual) to closest surface of shape from inside

G4double G4Cons::DistanceToOut(const G4ThreeVector& p) const
{
  G4double safe=0.0,rho,safeR1,safeR2,safeZ ;
  G4double tanRMin,secRMin,pRMin ;
  G4double tanRMax,secRMax,pRMax ;
  G4double safePhi,phiC,cosPhiC,sinPhiC,ePhi ;

#ifdef G4CSGDEBUG
  if( Inside(p) == kOutside )
  {
    G4cout.precision(16) ;
    G4cout << G4endl ;
    DumpInfo();
    G4cout << "Position:"  << G4endl << G4endl ;
    G4cout << "p.x() = "   << p.x()/mm << " mm" << G4endl ;
    G4cout << "p.y() = "   << p.y()/mm << " mm" << G4endl ;
    G4cout << "p.z() = "   << p.z()/mm << " mm" << G4endl << G4endl ;
    G4cout << "pho at z = "   << std::sqrt( p.x()*p.x()+p.y()*p.y() )/mm << " mm" 
           << G4endl << G4endl ;
    if( p.x() != 0. || p.x() != 0.)
    {
      G4cout << "point phi = "   << std::atan2(p.y(),p.x())/degree << " degree" 
             << G4endl << G4endl ; 
    }
    G4Exception("G4Cons::DistanceToOut(p)", "Notification", JustWarning, 
                "Point p is outside !?" );
  }
#endif

  rho = std::sqrt(p.x()*p.x() + p.y()*p.y()) ;
  safeZ = fDz - std::fabs(p.z()) ;

  if (fRmin1 || fRmin2)
  {
    tanRMin = (fRmin2 - fRmin1)*0.5/fDz ;
    secRMin = std::sqrt(1.0 + tanRMin*tanRMin) ;
    pRMin   = tanRMin*p.z() + (fRmin1 + fRmin2)*0.5 ;
    safeR1  = (rho - pRMin)/secRMin ;
  }
  else safeR1 = kInfinity ;

  tanRMax = (fRmax2 - fRmax1)*0.5/fDz ;
  secRMax = std::sqrt(1.0 + tanRMax*tanRMax) ;
  pRMax   = tanRMax*p.z() + (fRmax1+fRmax2)*0.5 ;
  safeR2  = (pRMax - rho)/secRMax ;

  if (safeR1 < safeR2) safe = safeR1 ;
  else                 safe = safeR2 ;
  if (safeZ < safe)    safe = safeZ  ;

  // Check if phi divided, Calc distances closest phi plane

  if (fDPhi < twopi)
  {
    // Above/below central phi of G4Cons?

    phiC    = fSPhi + fDPhi*0.5 ;
    cosPhiC = std::cos(phiC) ;
    sinPhiC = std::sin(phiC) ;

    if ( (p.y()*cosPhiC - p.x()*sinPhiC) <= 0 )
    {
      safePhi = -(p.x()*std::sin(fSPhi) - p.y()*std::cos(fSPhi)) ;
    }
    else
    {
      ePhi    = fSPhi + fDPhi ;
      safePhi = (p.x()*std::sin(ePhi) - p.y()*std::cos(ePhi)) ;
    }
    if (safePhi < safe) safe = safePhi ;
  }
  if ( safe < 0 ) safe = 0 ;
  return safe ;  
}

////////////////////////////////////////////////////////////////////////////
//
// Create a List containing the transformed vertices
// Ordering [0-3] -fDz cross section
//          [4-7] +fDz cross section such that [0] is below [4],
//                                             [1] below [5] etc.
// Note:
//  Caller has deletion resposibility
//  Potential improvement: For last slice, use actual ending angle
//                         to avoid rounding error problems.

G4ThreeVectorList*
G4Cons::CreateRotatedVertices(const G4AffineTransform& pTransform) const
{
  G4ThreeVectorList* vertices ;
  G4ThreeVector vertex0, vertex1, vertex2, vertex3 ;
  G4double meshAngle, meshRMax1, meshRMax2, crossAngle;
  G4double cosCrossAngle, sinCrossAngle, sAngle ;
  G4double rMaxX1, rMaxX2, rMaxY1, rMaxY2, rMinX1, rMinX2, rMinY1, rMinY2 ;
  G4int crossSection, noCrossSections ;

  // Compute no of cross-sections necessary to mesh cone
    
  noCrossSections = G4int(fDPhi/kMeshAngleDefault) + 1 ;

  if (noCrossSections < kMinMeshSections)
  {
    noCrossSections = kMinMeshSections ;
  }
  else if (noCrossSections > kMaxMeshSections)
  {
    noCrossSections = kMaxMeshSections ;
  }
  meshAngle = fDPhi/(noCrossSections - 1) ;

  // G4double RMax = (fRmax2 >= fRmax1) ? fRmax2 : fRmax1  ;

  meshRMax1 = fRmax1/std::cos(meshAngle*0.5) ;
  meshRMax2 = fRmax2/std::cos(meshAngle*0.5) ;

  // If complete in phi, set start angle such that mesh will be at RMax
  // on the x axis. Will give better extent calculations when not rotated.

  if (fDPhi == twopi && fSPhi == 0.0 )
  {
    sAngle = -meshAngle*0.5 ;
  }
  else
  {
    sAngle = fSPhi ;
  } 
  vertices = new G4ThreeVectorList();
  vertices->reserve(noCrossSections*4) ;

  if (vertices)
  {
    for (crossSection = 0 ; crossSection < noCrossSections ; crossSection++)
    {
      // Compute coordinates of cross section at section crossSection

      crossAngle    = sAngle + crossSection*meshAngle ;
      cosCrossAngle = std::cos(crossAngle) ;
      sinCrossAngle = std::sin(crossAngle) ;

      rMaxX1 = meshRMax1*cosCrossAngle ;
      rMaxY1 = meshRMax1*sinCrossAngle ;
      rMaxX2 = meshRMax2*cosCrossAngle ;
      rMaxY2 = meshRMax2*sinCrossAngle ;
        
      // G4double RMin = (fRmin2 <= fRmin1) ? fRmin2 : fRmin1  ;

      rMinX1 = fRmin1*cosCrossAngle ;
      rMinY1 = fRmin1*sinCrossAngle ;
      rMinX2 = fRmin2*cosCrossAngle ;
      rMinY2 = fRmin2*sinCrossAngle ;
        
      vertex0 = G4ThreeVector(rMinX1,rMinY1,-fDz) ;
      vertex1 = G4ThreeVector(rMaxX1,rMaxY1,-fDz) ;
      vertex2 = G4ThreeVector(rMaxX2,rMaxY2,+fDz) ;
      vertex3 = G4ThreeVector(rMinX2,rMinY2,+fDz) ;

      vertices->push_back(pTransform.TransformPoint(vertex0)) ;
      vertices->push_back(pTransform.TransformPoint(vertex1)) ;
      vertices->push_back(pTransform.TransformPoint(vertex2)) ;
      vertices->push_back(pTransform.TransformPoint(vertex3)) ;
    }
  }
  else
  {
    DumpInfo();
    G4Exception("G4Cons::CreateRotatedVertices()",
                "FatalError", FatalException,
                "Error in allocation of vertices. Out of memory !");
  }
  return vertices ;
}

//////////////////////////////////////////////////////////////////////////
//
// GetEntityType

G4GeometryType G4Cons::GetEntityType() const
{
  return G4String("G4Cons");
}

//////////////////////////////////////////////////////////////////////////
//
// Stream object contents to an output stream

std::ostream& G4Cons::StreamInfo(std::ostream& os) const
{
  os << "-----------------------------------------------------------\n"
     << "    *** Dump for solid - " << GetName() << " ***\n"
     << "    ===================================================\n"
     << " Solid type: G4Cons\n"
     << " Parameters: \n"
     << "   inside  -fDz radius: "  << fRmin1/mm << " mm \n"
     << "   outside -fDz radius: "  << fRmax1/mm << " mm \n"
     << "   inside  +fDz radius: "  << fRmin2/mm << " mm \n"
     << "   outside +fDz radius: "  << fRmax2/mm << " mm \n"
     << "   half length in Z   : "  << fDz/mm << " mm \n"
     << "   starting angle of segment: " << fSPhi/degree << " degrees \n"
     << "   delta angle of segment   : " << fDPhi/degree << " degrees \n"
     << "-----------------------------------------------------------\n";

  return os;
}

//////////////////////////////////////////////////////////////////////////
//
// Methods for visualisation

void G4Cons::DescribeYourselfTo (G4VGraphicsScene& scene) const
{
  scene.AddThis (*this);
}

G4Polyhedron* G4Cons::CreatePolyhedron () const
{
  return new G4PolyhedronCons(fRmin1,fRmax1,fRmin2,fRmax2,fDz,fSPhi,fDPhi);
}

G4NURBS* G4Cons::CreateNURBS () const
{
  G4double RMax = (fRmax2 >= fRmax1) ? fRmax2 : fRmax1 ;
  return new G4NURBSbox (RMax, RMax, fDz);       // Box for now!!!
}

//
//
/////////////////////////////// End of G4Cons.cc file //////////////////////
