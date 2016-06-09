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
// $Id: G4eIonisationSpectrum.hh,v 1.6 2003/12/09 15:36:40 gunter Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// -------------------------------------------------------------------
//
// GEANT4 Class file
//
//
// File name:     G4eIonisationSpectrum
//
// Author:        V.Ivanchenko (Vladimir.Ivantchenko@cern.ch)
// 
// Creation date: 27 September 2001
//
// Modifications: 
// 10.10.01 MGP             Revision to improve code quality and 
//                          consistency with design
// 29.11.01  V.Ivanchenko   Parametrisation is updated
// 30.05.02  VI             Add inline functions
//
// -------------------------------------------------------------------

// Class Description: 
// Provides various integration over delta-electron spectrum for e- 
// ionisation process. Spectrum is parametrised accourding to 
// EEDL database, details are described in the Physics Reference Manual
// Further documentation available from http://www.ge.infn.it/geant4/lowE

// -------------------------------------------------------------------

#ifndef G4EIONISATIONSPECTRUM_HH
#define G4EIONISATIONSPECTRUM_HH 1

#include "G4VEnergySpectrum.hh"
#include "G4eIonisationParameters.hh"
 
//class G4eIonisationParameters;
class G4DataVector;

class G4eIonisationSpectrum : public G4VEnergySpectrum
{

public:

  G4eIonisationSpectrum();

  ~G4eIonisationSpectrum();

  G4double Probability(G4int Z, G4double tMin, G4double tMax, 
		       G4double kineticEnergy, G4int shell,
		       const G4ParticleDefinition* pd=0) const;

  G4double AverageEnergy(G4int Z, G4double tMin, G4double tMax,
			 G4double kineticEnergy, G4int shell,
			 const G4ParticleDefinition* pd=0) const;

  G4double SampleEnergy(G4int Z, G4double tMin, G4double tMax,
			G4double kineticEnergy, G4int shell,
			const G4ParticleDefinition* pd=0) const;

  G4double MaxEnergyOfSecondaries(G4double kineticEnergy,
                                  G4int Z = 0,
				  const G4ParticleDefinition* pd=0) const;

  G4double Excitation(G4int Z, G4double e) const;
  
  void PrintData() const;

protected:

private:

  G4double IntSpectrum(G4double xMin, G4double xMax,
                         const G4DataVector& p) const; 
  
  G4double AverageValue(G4double xMin, G4double xMax,
			const G4DataVector& p) const; 
  
  G4double Function(G4double x, const G4DataVector& p) const; 
  
  // Hide copy constructor and assignment operator 
  G4eIonisationSpectrum(const  G4eIonisationSpectrum&);
  G4eIonisationSpectrum & operator = (const G4eIonisationSpectrum &right);
  
private:
  
  G4eIonisationParameters* theParam;
  G4double                 lowestE;
  G4double                 factor;
  G4int                    iMax;            
  G4int                    verbose;
};


inline G4double G4eIonisationSpectrum::Function(G4double x, 
				          const G4DataVector& p) const
{
  G4double f = 1.0 - p[0] - p[iMax]*x 
             + x*x*(1.0 - p[iMax] + (1.0/(1.0 - x) - p[iMax])/(1.0 - x))
             + 0.5*p[0]/x;

  return f;
} 

inline G4double G4eIonisationSpectrum::Excitation(G4int Z, G4double e) const 
{
  return theParam->Excitation(Z, e);
}


#endif
