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
// $Id: G4TrajectoryContainer.cc,v 1.4 2004/06/11 14:11:21 gcosmo Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//

#include "G4TrajectoryContainer.hh"
G4Allocator<G4TrajectoryContainer> aTrajectoryContainerAllocator;

G4TrajectoryContainer::G4TrajectoryContainer()
{ vect = new TrajectoryVector; }

G4TrajectoryContainer::~G4TrajectoryContainer()
{ clearAndDestroy();
  delete vect; }

G4int G4TrajectoryContainer::operator==(const G4TrajectoryContainer& right) const
{ return (this==&right); }
G4int G4TrajectoryContainer::operator!=(const G4TrajectoryContainer& right) const
{ return (this!=&right); }

