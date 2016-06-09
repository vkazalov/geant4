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
// $Id: G4Na25GEMProbability.cc,v 1.2 2003/11/03 17:53:04 hpw Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara (Nov 1999)
//


#include "G4Na25GEMProbability.hh"

G4Na25GEMProbability::G4Na25GEMProbability() :
  G4GEMProbability(25,11,5.0/2.0) // A,Z,Spin
{

    ExcitEnergies.push_back(89.53*keV);
    ExcitSpins.push_back(3.0/2.0);
    ExcitLifetimes.push_back(5.13e-3*picosecond);

    ExcitEnergies.push_back(1069.32*keV);
    ExcitSpins.push_back(1.0/2.0);
    ExcitLifetimes.push_back(1.6*picosecond);

    ExcitEnergies.push_back(2202.0*keV);
    ExcitSpins.push_back(3.0/2.0);
    ExcitLifetimes.push_back(17.0e-3*picosecond);

    ExcitEnergies.push_back(2417.0*keV);
    ExcitSpins.push_back(7.0/2.0);
    ExcitLifetimes.push_back(0.14*picosecond);

    ExcitEnergies.push_back(2788.0*keV);
    ExcitSpins.push_back(3.0/2.0);
    ExcitLifetimes.push_back(14.0e-3*picosecond);

    ExcitEnergies.push_back(2914.0*keV);
    ExcitSpins.push_back(3.0/2.0);
    ExcitLifetimes.push_back(14.0e-3*picosecond);

    ExcitEnergies.push_back(3687.0*keV);
    ExcitSpins.push_back(1.0/2.0);
    ExcitLifetimes.push_back(14.0e-3*picosecond);

    ExcitEnergies.push_back(3995.0*keV);
    ExcitSpins.push_back(1.0/2.0);
    ExcitLifetimes.push_back(14.0e-3*picosecond);

}


G4Na25GEMProbability::G4Na25GEMProbability(const G4Na25GEMProbability &) : G4GEMProbability()
{
  throw G4HadronicException(__FILE__, __LINE__, "G4Na25GEMProbability::copy_constructor meant to not be accessable");
}




const G4Na25GEMProbability & G4Na25GEMProbability::
operator=(const G4Na25GEMProbability &)
{
  throw G4HadronicException(__FILE__, __LINE__, "G4Na25GEMProbability::operator= meant to not be accessable");
  return *this;
}


G4bool G4Na25GEMProbability::operator==(const G4Na25GEMProbability &) const
{
  return false;
}

G4bool G4Na25GEMProbability::operator!=(const G4Na25GEMProbability &) const
{
  return true;
}



