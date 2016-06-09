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
// $Id: G4FTFCNeutronBuilder.cc,v 1.2.4.1 2009/08/11 15:11:24 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-02-patch-02 $
//
//---------------------------------------------------------------------------
//
// ClassName:   G4FTFCNeutronBuilder
//
// Author: 2002 J.P. Wellisch
//
// Modified:
// 30.03.2009 V.Ivanchenko create cross section by new
//
//----------------------------------------------------------------------------
//
#include "G4FTFCNeutronBuilder.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4NeutronInelasticCrossSection.hh"

G4FTFCNeutronBuilder::
G4FTFCNeutronBuilder(G4bool quasiElastic) 
{
  theMin = 4*GeV;
  theModel = new G4TheoFSGenerator;

  theStringModel = new G4FTFModel;
  theStringDecay = new G4ExcitedStringDecay(new G4LundStringFragmentation);
  theStringModel->SetFragmentationModel(theStringDecay);

  theCascade = new G4StringChipsParticleLevelInterface;

  theModel->SetHighEnergyGenerator(theStringModel);
  if (quasiElastic)
  {
     theQuasiElastic=new G4QuasiElasticChannel;
     theModel->SetQuasiElasticChannel(theQuasiElastic);
  } else 
  {  theQuasiElastic=0;}  

  theModel->SetTransport(theCascade);
  theModel->SetMinEnergy(theMin);
  theModel->SetMaxEnergy(100*TeV);
}

G4FTFCNeutronBuilder::~G4FTFCNeutronBuilder() 
{
  delete theCascade;
  delete theStringDecay;
  if ( theQuasiElastic ) delete theQuasiElastic;
  delete theStringModel;
  delete theModel;
}

void G4FTFCNeutronBuilder::
Build(G4HadronElasticProcess * )
{
}

void G4FTFCNeutronBuilder::
Build(G4HadronFissionProcess * )
{
}

void G4FTFCNeutronBuilder::
Build(G4HadronCaptureProcess * )
{
}

void G4FTFCNeutronBuilder::
Build(G4NeutronInelasticProcess * aP)
{
  theModel->SetMinEnergy(theMin);
  aP->AddDataSet(new G4NeutronInelasticCrossSection);  
  aP->RegisterMe(theModel);
}

 // 2002 by J.P. Wellisch
