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
// $Id: G4GraphicsSystemList.hh,v 1.8 2003/06/16 17:14:02 gunter Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
// 
// John Allison  2nd April 1996

#ifndef G4GRAPHICSSYSTEMLIST_HH
#define G4GRAPHICSSYSTEMLIST_HH

#include <vector>
#include "G4VGraphicsSystem.hh"

class G4GraphicsSystemList: public std::vector<G4VGraphicsSystem*> {
public:
  void remove(G4VGraphicsSystem*);
};

typedef std::vector<G4VGraphicsSystem*>::iterator
        G4GraphicsSystemListIterator;
typedef std::vector<G4VGraphicsSystem*>::const_iterator
        G4GraphicsSystemListConstIterator;

#endif
