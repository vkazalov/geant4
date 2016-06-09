//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4HadronicProcessStore.hh,v 1.3.2.1 2009/03/03 11:45:23 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-02-patch-02 $
//
//
// -------------------------------------------------------------------
//
// GEANT4 Class header file
//
//
// File name:     G4HadronicProcessStore
//
// Author:        Vladimir Ivanchenko 
//
// Creation date: 09.05.2008
//
// Modifications:
//
//
// Class Description:
//

// -------------------------------------------------------------------
//

#ifndef G4HadronicProcessStore_h
#define G4HadronicProcessStore_h 1


#include "globals.hh"
#include "G4DynamicParticle.hh"
#include "G4HadronicProcess.hh"
#include "G4HadronicInteraction.hh"
#include "G4ParticleDefinition.hh"
#include "G4HadronicProcessType.hh"
#include <map>
#include <vector>

class G4Element;

class G4HadronicProcessStore
{

public:

  static G4HadronicProcessStore* Instance();

  ~G4HadronicProcessStore();

  void Clean();

  G4double GetInelasticCrossSectionPerVolume(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Material *material);

  G4double GetInelasticCrossSectionPerAtom(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Element *anElement);

  G4double GetInelasticCrossSectionPerIsotope(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    G4int Z, G4int A);

  G4double GetElasticCrossSectionPerVolume(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Material *material);

  G4double GetElasticCrossSectionPerAtom(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Element *anElement);

  G4double GetElasticCrossSectionPerIsotope(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    G4int Z, G4int A);

  G4double GetCaptureCrossSectionPerVolume(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Material *material);

  G4double GetCaptureCrossSectionPerAtom(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Element *anElement);

  G4double GetCaptureCrossSectionPerIsotope(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    G4int Z, G4int A);

  G4double GetFissionCrossSectionPerVolume(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Material *material);

  G4double GetFissionCrossSectionPerAtom(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Element *anElement);

  G4double GetFissionCrossSectionPerIsotope(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    G4int Z, G4int A);

  G4double GetChargeExchangeCrossSectionPerVolume(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Material *material);

  G4double GetChargeExchangeCrossSectionPerAtom(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    const G4Element *anElement);

  G4double GetChargeExchangeCrossSectionPerIsotope(
    const G4ParticleDefinition *aParticle,
    G4double kineticEnergy,
    G4int Z, G4int A);

  // register/deregister processes following G4HadronicProcess interface
  void Register(G4HadronicProcess*); 

  void RegisterParticle(G4HadronicProcess*,
			const G4ParticleDefinition*); 

  void RegisterInteraction(G4HadronicProcess*,
			   G4HadronicInteraction*); 

  void DeRegister(G4HadronicProcess*); 

  // register/deregister processes following only G4VProcess interface
  void RegisterExtraProcess(G4VProcess*); 

  void RegisterParticleForExtraProcess(G4VProcess*,
				       const G4ParticleDefinition*); 

  void DeRegisterExtraProcess(G4VProcess*); 

  void PrintInfo(const G4ParticleDefinition*); 

  void Dump(G4int level);

  void SetVerbose(G4int val);

  G4int GetVerbose();

  G4HadronicProcess* FindProcess(const G4ParticleDefinition*, 
				 G4HadronicProcessType subType);

private:

  // constructor
  G4HadronicProcessStore();

  // print process info
  void Print(G4int idxProcess, G4int idxParticle);

  static G4HadronicProcessStore* theInstance;

  typedef const G4ParticleDefinition* PD;
  typedef G4HadronicProcess* HP;
  typedef G4HadronicInteraction* HI;

  // hadronic processes following G4HadronicProcess interface
  std::vector<G4HadronicProcess*> process;
  std::vector<G4HadronicInteraction*> model;
  std::vector<G4String> modelName;
  std::vector<PD> particle;
  std::vector<G4int> wasPrinted;

  std::multimap<PD,HP> p_map;
  std::multimap<HP,HI> m_map;

  // hadronic processes following only G4VProcess interface
  std::vector<G4VProcess*> extraProcess;
  std::multimap<PD,G4VProcess*> ep_map;

  // counters and options
  G4int n_proc;
  G4int n_model;
  G4int n_part;
  G4int n_extra;

  G4int  verbose;
  G4bool buildTableStart;

  // cash
  HP   currentProcess;
  PD   currentParticle;

  G4DynamicParticle localDP;

};


#endif
