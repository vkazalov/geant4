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
// $Id: G4FissionParameters.hh,v 1.1 2003/08/26 18:37:02 lara Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara (Oct 1998) 
//



#ifndef G4FissionParameters_h
#define G4FissionParameters_h 1

#include "globals.hh"


class G4FissionParameters 
{
public:
  // Only available constructor
  G4FissionParameters(const G4int A, const G4int Z, const G4double ExEnergy, const G4double FissionBarrier);

  ~G4FissionParameters() {};  

private:  
  // Default constructor
  G4FissionParameters() {};

  // Copy constructor
  G4FissionParameters(const G4FissionParameters &right);

  const G4FissionParameters & operator=(const G4FissionParameters &right);
  G4bool operator==(const G4FissionParameters &right) const;
  G4bool operator!=(const G4FissionParameters &right) const;
  
public:

  inline G4double GetA1(void) const { return A1; }
  inline G4double GetA2(void) const { return A2; }

  inline G4double GetAs(void) const { return As; }
  inline G4double GetSigma1(void) const { return Sigma1; }
  inline G4double GetSigma2(void) const { return Sigma2; }
  inline G4double GetSigmaS(void) const { return SigmaS; }
  inline G4double GetW(void) const { return w; }

private:

  // Mean numbers of the corresponding Gaussians for assymmetric
  // fission
  static const G4double A1;
  static const G4double A2;

  // Mean number for symmetric fission
  G4double As;

  // Dispersions of the corresponding Gaussians for assymmetric
  // fission
  G4double Sigma1;
  G4double Sigma2;

  // Dispersion for symmetric fission
  G4double SigmaS;

  // Weight which determines the relative contribution of symmetric
  // and assymmetric components
  G4double w;


};


#endif
