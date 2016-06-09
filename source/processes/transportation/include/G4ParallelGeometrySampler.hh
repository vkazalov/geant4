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
// $Id: G4ParallelGeometrySampler.hh,v 1.7 2003/11/26 14:51:48 gcosmo Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// ----------------------------------------------------------------------
// Class G4ParallelGeometrySampler
//
// Class description:
//
// Class description:
// This class inherits from G4VSampler. It is used for scoring and 
// importance sampling in a parallel geometry.
// See also the description in G4VSampler.

// Author: Michael Dressel (Michael.Dressel@cern.ch)
// ----------------------------------------------------------------------
#ifndef G4ParallelGeometrySampler_hh
#define G4ParallelGeometrySampler_hh G4ParallelGeometrySampler_hh

#include "G4Types.hh"
#include "G4VSampler.hh"
#include "G4ParallelWorld.hh"
#include "G4VSamplerConfigurator.hh"

class G4ParallelTransportConfigurator;
class G4PScoreConfigurator;
class G4PImportanceConfigurator;
class G4WeightCutOffConfigurator;
class G4VGCellFinder;
class G4PWeightWindowConfigurator;
class G4WeightWindowStore;

class G4ParallelGeometrySampler : public G4VSampler
{

public:  // with description
 
  G4ParallelGeometrySampler(G4VPhysicalVolume &worldvolume,
                            const G4String &particlename);
  virtual ~G4ParallelGeometrySampler();

  virtual void PrepareScoring(G4VScorer *Scorer);
  virtual void PrepareImportanceSampling(G4VIStore *istore,
                                 const G4VImportanceAlgorithm 
                                 *ialg);
  virtual void PrepareWeightRoulett(G4double wsurvive, 
                            G4double wlimit,
                            G4double isource);
  
  virtual void PrepareWeightWindow(G4VWeightWindowStore *wwstore,
                                   G4VWeightWindowAlgorithm *wwAlg,
                                   G4PlaceOfAction placeOfAction);


  virtual void Configure();

  virtual void ClearSampling();
  virtual G4bool IsConfigured() const;
  
private:

  G4ParallelGeometrySampler(const G4ParallelGeometrySampler &);
  G4ParallelGeometrySampler &
  operator=(const G4ParallelGeometrySampler &);

private:

  G4String fParticleName;
  G4ParallelWorld fParallelWorld;
  G4ParallelTransportConfigurator *fParallelTransportConfigurator;
  G4PImportanceConfigurator *fPImportanceConfigurator;
  G4PScoreConfigurator *fPScoreConfigurator;
  G4VGCellFinder *fGCellFinder;
  G4WeightCutOffConfigurator *fWeightCutOffConfigurator;
  G4VIStore *fIStore;
  G4PWeightWindowConfigurator *fPWeightWindowConfigurator;
  G4VWeightWindowStore *fWWStore;
  G4bool fIsConfigured;
  G4Configurators fConfigurators;
};
  
#endif
