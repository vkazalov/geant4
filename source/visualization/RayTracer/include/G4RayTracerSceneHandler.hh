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
// $Id: G4RayTracerSceneHandler.hh,v 1.6 2002/12/11 15:59:34 johna Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $

// John Allison  17th March 2000

#ifndef G4RAYTRACERSCENEHANDLER_HH
#define G4RAYTRACERSCENEHANDLER_HH

#include "G4VSceneHandler.hh"

class G4RayTracerSceneHandler: public G4VSceneHandler {

public:

  G4RayTracerSceneHandler(G4VGraphicsSystem& system,
			  const G4String& name = "");
  virtual ~G4RayTracerSceneHandler();

  void AddPrimitive(const G4Polyline&){}
  void AddPrimitive(const G4Text&){}
  void AddPrimitive(const G4Circle&){}
  void AddPrimitive(const G4Square&){}
  void AddPrimitive(const G4Polyhedron&){}
  void AddPrimitive(const G4NURBS&){}
  void AddPrimitive(const G4Polymarker&){}
  void AddPrimitive(const G4Scale&){}

  void AddThis(const G4Box&){}
  void AddThis(const G4Cons&){}
  void AddThis(const G4Tubs&){}
  void AddThis(const G4Trd&){}
  void AddThis(const G4Trap&){}
  void AddThis(const G4Sphere&){}
  void AddThis(const G4Para&){}
  void AddThis(const G4Torus&){}
  void AddThis(const G4Polycone&){}
  void AddThis(const G4Polyhedra&){}
  void AddThis(const G4VSolid&){}
  void AddThis(const G4VTrajectory&){}
  void AddThis(const G4VHit&){}

  static G4int GetSceneCount();

private:
  static G4int    fSceneIdCount;  // Counter for RayTracer scene handlers.
  static G4int    fSceneCount;    // No. of extanct scene handlers.
};

#endif
