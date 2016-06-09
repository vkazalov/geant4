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
#include "globals.hh"
#include "G4ios.hh"
#include "G4XAnnihilationChannel.hh"
#include "G4KineticTrack.hh"
#include "G4ParticleDefinition.hh"
#include "G4ResonanceWidth.hh"
#include "G4ResonancePartialWidth.hh"
#include "G4PhysicsVector.hh"
#include "G4PartialWidthTable.hh"

G4XAnnihilationChannel::G4XAnnihilationChannel(): resonance(0)
{ }

G4XAnnihilationChannel::G4XAnnihilationChannel(const G4ParticleDefinition* resDefinition,
					       const G4ResonanceWidth& resWidths,
					       const G4ResonancePartialWidth& resPartWidths,
					       const G4String& partWidthLabel) 
  : resonance(resDefinition)
{ 
  // Get the tabulated mass-dependent widths for the resonance
  G4String resName = resonance->GetParticleName();
  // cout << "HPW "<<resName<<endl;
  G4String shortName = theNames.ShortName(resName);
  // cout << "HPW "<<shortName<<endl;
  // cout << "HPW "<<partWidthLabel<<endl;

  widthTable = resWidths.MassDependentWidth(shortName);
  partWidthTable = resPartWidths.MassDependentWidth(partWidthLabel);

  // As a first approximation the model is assumed to be valid over 
  // the entire energy range
  lowLimit = 0.;
  highLimit = DBL_MAX;
}


G4XAnnihilationChannel::~G4XAnnihilationChannel()
{
  delete widthTable;
  widthTable = 0;
  delete partWidthTable;
  partWidthTable = 0;
 }


G4bool G4XAnnihilationChannel::operator==(const G4XAnnihilationChannel &right) const
{
  return (this == (G4XAnnihilationChannel *) &right);
}


G4bool G4XAnnihilationChannel::operator!=(const G4XAnnihilationChannel &right) const
{
  return (this != (G4XAnnihilationChannel *) &right);
}


G4double G4XAnnihilationChannel::CrossSection(const G4KineticTrack& trk1, 
					      const G4KineticTrack& trk2) const
{
  G4double sigma = 0.;
  G4double eCM = (trk1.Get4Momentum() + trk2.Get4Momentum()).mag();

  G4ParticleDefinition* def1 = trk1.GetDefinition();
  G4ParticleDefinition* def2 = trk2.GetDefinition();

  G4int J1 = def1->GetPDGiSpin();
  G4int J2 = def2->GetPDGiSpin();
  G4double m1 = def1->GetPDGMass();
  G4double m2 = def2->GetPDGMass();

  G4int JRes = resonance->GetPDGiSpin();
  G4double mRes = resonance->GetPDGMass();

  G4double branch = Branch(trk1,trk2);
  G4double width = VariableWidth(trk1,trk2);
  G4double cleb = NormalizedClebsch(trk1,trk2);

  G4double s = eCM * eCM;
  if (s == 0.) throw G4HadronicException(__FILE__, __LINE__, "G4XAnnihilationChannel::CrossSection - eCM = 0");

  G4double pCM = std::sqrt((s-(m1+m2)*(m1+m2))*(s-(m1-m2)*(m1-m2))/(4.*s));

  sigma = ( (JRes + 1.) / ( (J1 + 1) * (J2 + 1) ) 
	    * pi / (pCM * pCM) * branch * width * width / 
	    ( (eCM - mRes) * (eCM - mRes) + width * width / 4.0) * cleb * hbarc_squared);

//   G4cout << "SS " << branch<<" "<<sigma<<" "
//          << J1 <<" "
// 	 <<J2<<" "
// 	 <<m1<<" "
// 	 <<m2<<" "
// 	 <<JRes<<" "
// 	 <<mRes<<" "
// 	 <<wRes<<" "
// 	 <<width<<" "
// 	 <<cleb<<" "
// 	 <<G4endl;
  return sigma;
}


G4String G4XAnnihilationChannel::Name() const
{
  G4String name("XAnnihilationChannelCrossSection");
  return name;
}



G4bool G4XAnnihilationChannel::IsValid(G4double e) const
{
  G4bool answer = InLimits(e,lowLimit,highLimit);

  return answer;
}


const G4double G4XAnnihilationChannel::Branch(const G4KineticTrack& trk1, 
					      const G4KineticTrack& trk2) const
{
  G4double w=VariableWidth(trk1,trk2);
  if(w==0) return 0;
  return VariablePartialWidth(trk1,trk2) / VariableWidth(trk1,trk2);
}

const G4double G4XAnnihilationChannel::VariableWidth(const G4KineticTrack& trk1, 
						     const G4KineticTrack& trk2) const
{
  // actual production width of resonance, depending on available energy.

  G4double width = resonance->GetPDGWidth();
  G4bool dummy = false;
  G4double sqrtS = (trk1.Get4Momentum() + trk2.Get4Momentum()).mag();
  if (widthTable != 0) 
    {
      width = widthTable->GetValue(sqrtS,dummy);
    }
  return width;
}


const G4double G4XAnnihilationChannel::VariablePartialWidth(const G4KineticTrack& trk1, 
							    const G4KineticTrack& trk2) const
{
  // Calculate mass dependent partial width of resonance, 
  // based on UrQMD tabulations

  G4double width(0);

  if (partWidthTable != 0)
  {
    G4double sqrtS = 0;
    G4bool dummy = false;
    sqrtS = (trk1.Get4Momentum() + trk2.Get4Momentum()).mag();
    width = partWidthTable->GetValue(sqrtS,dummy);
  }
  else
  {
    width = resonance->GetPDGWidth();
  }
  return width;
}


const G4double G4XAnnihilationChannel::NormalizedClebsch(const G4KineticTrack& trk1, 
							 const G4KineticTrack& trk2) const
{
  G4double cleb = 0.;
  G4ParticleDefinition* def1 = trk1.GetDefinition();
  G4ParticleDefinition* def2 = trk2.GetDefinition();

  G4int iso31 = def1->GetPDGiIsospin3();
  G4int iso32 = def2->GetPDGiIsospin3();
  G4int iso3 = iso31 + iso32;
  G4int iso1 = def1->GetPDGiIsospin();
  G4int iso2 = def2->GetPDGiIsospin();

  G4int isoRes = resonance->GetPDGiIsospin();
  
  if (isoRes < iso3) return 0.;
  if ((iso1*iso2) == 0) return 1.;

  cleb = clebsch.NormalizedClebschGordan(isoRes,iso3,iso1,iso2,iso31,iso32);

  // Special case: particle-antiparticle, charge-conjugated states have the same weight
  G4String type1 = def1->GetParticleType();
  G4String type2 = def2->GetParticleType();
  G4int anti = def1->GetPDGEncoding() * def2->GetPDGEncoding();
  G4int strangeness = resonance->GetQuarkContent(3) + resonance->GetAntiQuarkContent(3);
  if ( ((type1 == "baryon" && type2 == "baryon") ||(type1 == "meson" && type2 == "meson")) &&
       anti < 0 && strangeness == 0) 
    {
      if (def1->GetPDGEncoding() != -(def2->GetPDGEncoding())) cleb = 0.5 * cleb;
    }
       
  return cleb;
}





