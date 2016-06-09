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
// $Id: G4ConstantLevelDensityParameter.hh,v 1.1 2003/08/26 18:50:08 lara Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// Hadronic Process: Nuclear De-excitations (photon evaporation)
// by C. Dallapiccola (Nov 1998) 
//

#ifndef G4ConstantLevelDensityParameter_h
#define G4ConstantLevelDensityParameter_h 1

#include "G4VLevelDensityParameter.hh"

class G4ConstantLevelDensityParameter : public G4VLevelDensityParameter
{
public:
  G4ConstantLevelDensityParameter() :  EvapLevelDensityParameter(0.125*(1./MeV)) {};
  virtual ~G4ConstantLevelDensityParameter() {};

private:  
  G4ConstantLevelDensityParameter(const G4ConstantLevelDensityParameter &right);

  const G4ConstantLevelDensityParameter & operator=(const G4ConstantLevelDensityParameter &right);
  G4bool operator==(const G4ConstantLevelDensityParameter &right) const;
  G4bool operator!=(const G4ConstantLevelDensityParameter &right) const;
  
public:
  G4double LevelDensityParameter(const G4int A,const G4int /*Z*/,const G4double /*U*/) const 
    {return A * EvapLevelDensityParameter;}

private:

  const G4double EvapLevelDensityParameter;

};


#endif
