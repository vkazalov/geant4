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
// $Id: G4UIcmdWithAnInteger.hh,v 1.6 2004/05/16 18:42:30 asaim Exp $
// GEANT4 tag $Name: geant4-07-00-cand-01 $
//
//

#ifndef G4UIcmdWithAnInteger_H
#define G4UIcmdWithAnInteger_H 1

#include "G4UIcommand.hh"

// class description:
//  A concrete class of G4UIcommand. The command defined by this class
// takes an integer value.
//  General information of G4UIcommand is given in G4UIcommand.hh.

class G4UIcmdWithAnInteger : public G4UIcommand
{
  public: // with description
    G4UIcmdWithAnInteger
    (const char * theCommandPath,G4UImessenger * theMessenger);
    //  Constructor. The command string with full path directory
    // and the pointer to the messenger must be given.
    static G4int GetNewIntValue(const char* paramString);
    //  Convert string which represents an integer to an integer.
    void SetParameterName(const char * theName,G4bool omittable,
                          G4bool currentAsDefault=false);
    //  Set the parameter name. The name is used by the range checking.
    //  If "omittable" is set as true, the user of this command can ommit
    // the value when he/she applies the command. If "omittable" is false,
    // the user must supply an integer value.
    //  "currentAsDefault" flag is valid only if "omittable" is true. If this
    // flag is true, the current value is used as the default value when the 
    // user ommit the parameter. If this flag is false, the value given by the 
    // next SetDefaultValue() method is used.
    void SetDefaultValue(G4int defVal);
    //  Set the default value of the parameter. This default value is used
    // when the user of this command ommits the parameter value, and
    // "ommitable" is true and "currentAsDefault" is false.
};

#endif
