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
// $Id: G4DAWNFILE.hh,v 1.8 2001/07/27 22:32:55 johna Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// Satoshi TANAKA
// DAWNFILE driver factory.

#ifndef G4DAWNFILE_HH
#define G4DAWNFILE_HH

#include "G4VGraphicsSystem.hh"

	//----- prototype
class G4VSceneHandler   ;

	//----------------------------//
	//----- class G4DAWNFILE -----// 
	//----------------------------//
class G4DAWNFILE: public G4VGraphicsSystem {

public:
  G4DAWNFILE ();
  virtual ~G4DAWNFILE ();
  G4VSceneHandler* CreateSceneHandler (const G4String& name = "");
  G4VViewer*  CreateViewer  (G4VSceneHandler&, const G4String& name = "");

private:


};

#endif
