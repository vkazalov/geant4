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
// $Id: F03EventAction.cc,v 1.4 2001/11/07 16:36:33 gcosmo Exp $
// GEANT4 tag $Name: geant4-05-01 $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include "F03EventAction.hh"

#include "F03RunAction.hh"

#include "F03CalorHit.hh"
#include "F03EventActionMessenger.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4VVisManager.hh"
#include "G4ios.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

F03EventAction::F03EventAction(F03RunAction* F03RA)
 : calorimeterCollID(-1), eventMessenger(0),
   runaction(F03RA), verboselevel(0),
   drawFlag("all"), printModulo(10000)
{
  eventMessenger = new F03EventActionMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

F03EventAction::~F03EventAction()
{
  delete eventMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void F03EventAction::BeginOfEventAction(const G4Event* evt)
{
 G4int evtNb = evt->GetEventID();
 if (evtNb%printModulo == 0) 
    G4cout << "\n---> Begin of Event: " << evtNb << G4endl;
     
  if(verboselevel>1)
    G4cout << "<<< Event  " << evtNb << " started." << G4endl;
    
  if (calorimeterCollID==-1)
    {
     G4SDManager * SDman = G4SDManager::GetSDMpointer();
     calorimeterCollID = SDman->GetCollectionID("CalCollection");
    } 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void F03EventAction::EndOfEventAction(const G4Event* evt)
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();

  if(pVVisManager)
  {
   G4TrajectoryContainer* trajectoryContainer = evt->GetTrajectoryContainer();
   G4int n_trajectories = 0;
   if (trajectoryContainer) n_trajectories = trajectoryContainer->entries();  
   for(G4int i=0; i<n_trajectories; i++) 
      { G4Trajectory* trj = (G4Trajectory *)((*(evt->GetTrajectoryContainer()))[i]);
        if (drawFlag == "all") trj->DrawTrajectory(50);
        else if ((drawFlag == "charged")&&(trj->GetCharge() != 0.))
                               trj->DrawTrajectory(50); 
      }
  }  

  if(verboselevel>0)
    G4cout << "<<< Event  " << evt->GetEventID() << " ended." << G4endl;
  
  
  //save rndm status
  if (runaction->GetRndmFreq() == 2)
    { 
     HepRandom::saveEngineStatus("endOfEvent.rndm");   
     G4int evtNb = evt->GetEventID();
     if (evtNb%printModulo == 0)
       { 
        G4cout << "\n---> End of Event: " << evtNb << G4endl;
        HepRandom::showEngineStatus();
       }
    }     
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4int F03EventAction::GetEventno()
{
  G4int evno = fpEventManager->GetConstCurrentEvent()->GetEventID() ;
  return evno ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void F03EventAction::setEventVerbose(G4int level)
{
  verboselevel = level ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....