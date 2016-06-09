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
// $Id: G4HESigmaMinusInelastic.hh,v 1.11 2004/04/22 20:50:20 dwright Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
//
// G4 Gheisha High Energy model class -- header file
// H. Fesefeldt, RWTH Aachen 23-October-1996
// Last modified: 10-December-1996

// A prototype of the Gheisha High Energy collision model.

#ifndef G4HESigmaMinusInelastic_h
#define G4HESigmaMinusInelastic_h 1

// Class description:
// High energy parameterized model for Sigma- inelastic scattering.  This
// class is responsible for producing the final state of the interaction and
// is typically valid for incident Sigma- energies above 20 GeV.  This
// physics may be invoked by registering an instance of the class with
// G4SigmaMinusInelasticProcess in the user's physics list.
//
// This class is derived from G4HEInelastic which in turn is derived from
// G4HadronicInteraction.

// Class Description - End

#include "G4HEInelastic.hh"

class G4HESigmaMinusInelastic : public G4HEInelastic  
{
 public:  // with description
        G4HESigmaMinusInelastic() : G4HEInelastic()
           {
              theMinEnergy =  20*GeV;
              theMaxEnergy = 10*TeV;
              MAXPART      = 2048;
              verboseLevel = 0; 
           }

        ~G4HESigmaMinusInelastic(){ };
         
        G4int vecLength;
        
        G4HadFinalState * ApplyYourself( const G4HadProjectile &aTrack, G4Nucleus &targetNucleus );

        G4int  GetNumberOfSecondaries()
               { return vecLength; }         

        void   FirstIntInCasSigmaMinus(G4bool &inElastic, const G4double availableEnergy,
                                       G4HEVector pv[],
                                       G4int &vecLen, 
                                       G4HEVector incidentParticle,
                                       G4HEVector targetParticle,
                                       const G4double atomicWeight);
};
#endif                     
                                         

