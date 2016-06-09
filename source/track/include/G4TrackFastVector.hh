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
// $Id: G4TrackFastVector.hh,v 1.4 2001/07/11 10:08:37 gunter Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// 
// ------------------------------------------------------------
//	GEANT 4 class header file 
//
//	History: first implementation, based on object model of
//	8 Mar 1997, H.Kurashige
// ------------------------------------------------------------

#ifndef G4TrackFastVector_h
#define G4TrackFastVector_h 1

class G4Track;
#include "G4FastVector.hh"

const G4int G4TrackFastVectorSize = 512;
typedef G4FastVector<G4Track,G4TrackFastVectorSize> G4TrackFastVector;
//  Contains pointers to G4Track objects which are
//  generated by either primary or secondary interaction.

#endif

