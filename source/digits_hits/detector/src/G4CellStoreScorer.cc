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
// $Id: G4CellStoreScorer.cc,v 1.1 2003/10/03 10:10:56 gcosmo Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// ----------------------------------------------------------------------
// GEANT 4 class source file
//
// G4CellStoreScorer
//
// ----------------------------------------------------------------------

#include "G4CellStoreScorer.hh"

#include "G4VCellScorer.hh"

#include "G4Track.hh"
#include "G4GeometryCell.hh"
#include "G4Step.hh"
#include "G4GeometryCellStep.hh"
#include "G4VCellScorerStore.hh"

G4CellStoreScorer::G4CellStoreScorer(G4VCellScorerStore &csc) :
  fCellScorerStore(csc)
{}

G4CellStoreScorer::~G4CellStoreScorer()
{}


void G4CellStoreScorer::
Score(const G4Step &aStep, const G4GeometryCellStep &aPstep){
  G4Track *track = 0;
  track = aStep.GetTrack();
  
  if (track->GetTrackStatus()==fStopAndKill) {
    G4cout << "G4CellStoreScorer::Score:   track status is StopAndKill -> do nothing" << G4endl;
  }
  else { 
    // chose the cells to be scored
    G4GeometryCell pre_gCell(aPstep.GetPreGeometryCell()); 
    G4GeometryCell post_gCell(aPstep.GetPostGeometryCell()); 
    G4VCellScorer *post_cs = 0;
    post_cs = fCellScorerStore.GetCellScore(post_gCell);
    if (aPstep.GetCrossBoundary()) { 
      // entering post_gCell
      if (post_cs) {
	post_cs->ScoreAnEnteringStep(aStep, post_gCell);
      }
      // exiting pre_gCell
      G4VCellScorer *pre_cs = 0;
      pre_cs = fCellScorerStore.GetCellScore(pre_gCell);
      if (pre_cs) {
	pre_cs->ScoreAnExitingStep(aStep, pre_gCell);
      }
    } 
    else {
      // step in post_gCell
      if (post_cs) {
	post_cs->ScoreAnInVolumeStep(aStep, post_gCell);
      }
    }
  }
  
}

