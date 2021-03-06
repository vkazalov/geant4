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
// $Id: G4LevelReader.cc 88407 2015-02-18 09:18:44Z vnivanch $
//
// -------------------------------------------------------------------
//
//      GEANT4 header file 
//
//      File name:     G4NucLevel
//
//      Author:        V.Ivanchenko (M.Kelsey reading method is used)
// 
//      Creation date: 4 January 2012
//
//      Modifications:
//
// -------------------------------------------------------------------

#include "G4LevelReader.hh"
#include "G4NucleiProperties.hh"
#include "G4NucLevel.hh"
#include "G4NuclearLevelData.hh"
#include "G4DeexPrecoParameters.hh"
#include "G4SystemOfUnits.hh"
#include "G4Pow.hh"
#include <vector>
#include <fstream>
#include <sstream>

G4String G4LevelReader::fFloatingLevels[] = {
  "-", "+X", "+Y", "+Z", "+U", "+V", "+W", "+R", "+S", "+T", "+A", "+B", "+C"};

G4LevelReader::G4LevelReader(G4NuclearLevelData* ptr) 
  : fData(ptr),fAlphaMax(9.0f+9),fVerbose(0),fLevelMax(632),fTransMax(80)
{
  fParam = fData->GetParameters();
  fTimeFactor = (G4float)(CLHEP::second/G4Pow::GetInstance()->logZ(2));
  char* directory = getenv("G4LEVELGAMMADATA");
  if(directory) {
    fDirectory = directory;
  } else {
    G4Exception("G4LevelReader()","had0707",FatalException,
		"Environment variable G4LEVELGAMMADATA is not defined");
    fDirectory = "";
  } 
  fFile = fDirectory + "/z100.a200";
  fPol = "  ";
  for(G4int i=0; i<10; ++i) { fICC[i] = 0.0f; }
  for(G4int i=0; i<nbufmax; ++i) { buffer[i] = ' '; }
  for(G4int i=0; i<nbuf1; ++i)   { buff1[i] = ' '; }
  for(G4int i=0; i<nbuf2; ++i)   { buff2[i] = ' '; }
  bufp[0] = bufp[1] = bufp[2] = ' ';

  fEnergy = fCurrEnergy = fTrEnergy = 0.0;
  fTime = fProb = fSpin = fAlpha = fRatio = fNorm1 = 0.0f;

  vTrans.resize(fTransMax,0);
  vRatio.resize(fTransMax,0.0f);
  vGammaCumProbability.resize(fTransMax,0.0f);
  vGammaProbability.resize(fTransMax,0.0f);
  vShellProbability.resize(fTransMax,nullptr);
  vMpRatio.resize(fTransMax,0.0f);

  vEnergy.resize(fLevelMax,0.0f);
  vSpin.resize(fLevelMax,0);
  vLevel.resize(fLevelMax,nullptr);
}

const G4LevelManager* 
G4LevelReader::CreateLevelManager(G4int Z, G4int A)
{
  std::ostringstream ss;
  ss << "/z" << Z << ".a" << A;
  G4String st =  G4String(ss.str());
  fFile = fDirectory + st;
  return MakeLevelManager(Z, A, fFile);
}

const G4LevelManager* 
G4LevelReader::MakeLevelManager(G4int Z, G4int A, const G4String& filename)
{
  vEnergy.resize(1,0.0f);
  vSpin.resize(1,0);
  vLevel.resize(1,nullptr);

  vTrans.clear();
  vGammaCumProbability.clear();
  vGammaProbability.clear();
  vShellProbability.clear();
  vMpRatio.clear();

  std::ifstream infile(filename, std::ios::in);

  // file is not opened
  if (!infile.is_open()) {
    if (fVerbose > 0) {
      G4cout << " G4LevelReader: fail open file for Z= " 
	     << Z << " A= " << A 
	     << " <" << filename << ">" <<  G4endl;
    }

  } else {

    G4bool allLevels = fParam->StoreAllLevels();
    if (fVerbose > 0) {
      G4cout << "G4LevelReader: open file for Z= " 
	     << Z << " A= " << A 
	     << " <" << filename << ">" <<  G4endl;
    }
    // read line by line
    G4bool end  = false;
    G4bool next = true;
    G4int nline = -1;
    G4int  spin;
    G4int  index(0);
    fCurrEnergy = DBL_MAX;
    do {
      fPol = "  ";
      ++nline;
      G4bool isOK = (ReadDataItem(infile,fEnergy)   &&
		     ReadDataItem(infile,fTrEnergy) &&
		     ReadDataItem(infile,fProb)     &&
		     ReadDataItem(infile,fPol)      &&
		     ReadDataItem(infile,fTime)     &&
		     ReadDataItem(infile,fSpin)     &&
		     ReadDataItem(infile,fAlpha));
      /*
      G4cout << fEnergy << " " << fTrEnergy << " " << fProb << " " 
	     << fPol << " " << fTime << " " << fSpin << " " << fAlpha 
	     << " " << isOK << " " << fVerbose << G4endl;
      */
      fEnergy   *= CLHEP::keV;
      fTrEnergy *= CLHEP::keV;

      if(isOK) {
	for(G4int i=0; i<10; ++i) { 
	  isOK = (isOK && (ReadDataItem(infile,fICC[i])));
	}
      }
      if(!isOK) {
	end = true; 
	next = false; 
      }
      if(fVerbose > 1) {
	G4cout << "#Line " << nline << " " << fEnergy << " " << fTrEnergy 
	       << " " << fProb << " " <<  fPol << " " << fTime << " " 
	       << fSpin << " " << fAlpha << G4endl;
	G4cout << "      ";
	for(G4int i=0; i<10; ++i) { G4cout << fICC[i] << " "; }
	G4cout << G4endl;
      }
      // end of nuclear level data
      if(end || fEnergy > fCurrEnergy) {
	size_t nn = vTrans.size();
	if(nn > 0) {
	  --nn;
	  if(fVerbose > 1) {
	    G4cout << "Reader: new level E= " << fEnergy 
		   << " Ntransitions= " << nn+1 << " fNorm1= " << fNorm1 
		   << G4endl;
	  } 
	  if(fNorm1 > 0.0f) { 
	    fNorm1 = 1.0f/fNorm1; 
	    for(size_t i=0; i<nn; ++i) {
	      vGammaCumProbability[i]  *= fNorm1;
	      if(fVerbose > 2) {
		G4cout << "Probabilities[" << i 
		       << "]= " << vGammaCumProbability[i]
		       << " idxTrans= " << index
		       << G4endl;
	      }
	    }
	    vGammaCumProbability[nn]  = 1.0f;
	    if(fVerbose > 2) {
	      G4cout << "Probabilities[" << nn << "]= " 
		     << vGammaCumProbability[nn]
		     << " IdxTrans= " << index
		     << G4endl;
	    }
	    vLevel.push_back(new G4NucLevel(vTrans.size(), fTime,
					    vTrans,
					    vGammaCumProbability,
					    vGammaProbability,
					    vMpRatio,
					    vShellProbability));
	    //case of X-level
	  } else {
            spin += 100000;
            fTime = 0.0f;
	    vLevel.push_back(nullptr);
	  }
	} else {
	  vLevel.push_back(nullptr);
	}
	vSpin.push_back(spin);      
	vTrans.clear();
	vGammaCumProbability.clear();
	vGammaProbability.clear();
	vShellProbability.clear();
	vMpRatio.clear();
        if(!end) { next = true; }
      }
      G4float ener = (G4float)fEnergy;
      // begin nuclear level data
      if(next) {
	if(fVerbose > 1) {
	  G4cout << "== Reader: begin of new level E= " << fEnergy 
		 << "  Prob= " << fProb << G4endl;
	} 
	// protection for bad level energy
	size_t nn = vEnergy.size();
	if(0 < nn && vEnergy[nn-1] > ener) { 
	  ener = vEnergy[nn-1]; 
	} 
	vEnergy.push_back(ener);
        fCurrEnergy = fEnergy;
	fTime *= fTimeFactor;
	if(fSpin > 20.0f) { fSpin = 0.0; }
        spin = (G4int)(fSpin+fSpin);
	fNorm1 = 0.0f;
	next = false;
      } 
      // continue filling level data
      if(!end) {
        if(fProb > 0.0f) {
	  // by default transition to a ground state
          G4float efinal = (G4float)std::max(fEnergy - fTrEnergy,0.0);
          G4float elevel = 0.0f;
          size_t  idxLevel = 0;
	  G4int tnum = 0;
	  // do not check initial energy
	  size_t nn = vEnergy.size();
	  static const G4float x_energy = (G4float)(0.1*CLHEP::eV);
	  if(1 < nn) {
	    G4float ediffMin = (G4float)fEnergy;
	    for(size_t i=0; i<nn-1; ++i) {
              G4float ediff = std::abs(efinal - vEnergy[i]);
	      /*
	      G4cout << "Elevel[" << i << "]= " << vEnergy[i] 
		     << " Efinal= " << efinal
		     << " Ediff= " << ediff
		     << " EdiffMin= " << ediffMin << G4endl;
	      */
	      if(ediff < ediffMin) {
		ediffMin = ediff;
		elevel   = vEnergy[i];
		idxLevel = i;
                if(ediff <= x_energy) { break; }
	      }
	    }
	    if(std::abs(vEnergy[nn-1] - elevel) < x_energy) { tnum = 1; }
	  }
	  G4float x = 1.0f + fAlpha;
	  fNorm1 += x*fProb;
	  vGammaCumProbability.push_back(fNorm1);
	  vGammaProbability.push_back(1.0f/x);
	  vMpRatio.push_back(0.0f);
	  vTrans.push_back(tnum + idxLevel*10000);
	  if(allLevels && fAlpha > 0.0f) {
	    vShellProbability.push_back(NormalizedICCProbability(Z));
	  } else {
	    vShellProbability.push_back(nullptr);
	  }
	}
      }
      if(nline > 10000) {
	G4cout << "G4LevelReader: Line #" << nline << " Z= " << Z << " A= "
	       << " this file is too long - stop loading" << G4endl;
	end = true;
      }
      // Loop checking, 05-Aug-2015, Vladimir Ivanchenko
    } while (!end);
    infile.close();
  }

  G4LevelManager* man = nullptr;
  if(vEnergy.size() >= 2) {
    man = new G4LevelManager(vEnergy.size(),vEnergy,vSpin,vLevel); 
    if(fVerbose > 0) {
      G4cout << "=== Reader: new manager for Z= " << Z << " A= " << A 
	     << " Nlevels= " << vEnergy.size() << " E[0]= " 
	     << vEnergy[0]/CLHEP::MeV << " MeV   Emax= " 
	     << man->MaxLevelEnergy()/CLHEP::MeV << " MeV "
	     << " S: " <<  vEnergy.size() 
	     << " " << vSpin.size() << " " << vLevel.size() 
	     << G4endl;
    }
  } 
  return man;
}

G4bool G4LevelReader::ReadData(std::istringstream& stream, G4double& x)
{
  stream >> x;
  return stream.fail() ? false : true;
}

G4bool G4LevelReader::ReadDataItem(std::istream& dataFile, G4double& x)
{
  x = 0.0;
  for(G4int i=0; i<nbufmax; ++i) { buffer[i] = ' '; }
  G4bool okay = true;
  dataFile >> buffer;
  if(dataFile.fail()) { okay = false; }
  else { x = strtod(buffer, 0); }

  return okay;
}

G4bool G4LevelReader::ReadDataItem(std::istream& dataFile, G4float& x)
{
  x = 0.0f;
  for(G4int i=0; i<nbuf1; ++i) { buff1[i] = ' '; }
  G4bool okay = true;
  dataFile >> buff1;
  if(dataFile.fail()) { okay = false; }
  else { x = atof(buff1); }

  return okay;
}

G4bool G4LevelReader::ReadDataItem(std::istream& dataFile, G4int& ix)
{
  ix = 0;
  for(G4int i=0; i<nbuf2; ++i) { buff2[i] = ' '; }
  G4bool okay = true;
  dataFile >> buff2;
  if(dataFile.fail()) { okay = false; }
  else { ix = atoi(buff2); }

  return okay;
}

G4bool G4LevelReader::ReadDataItem(std::istream& dataFile, G4String& x)
{
  G4bool okay = true;
  bufp[0] = bufp[1] = ' ';
  dataFile >> bufp;
  if(dataFile.fail()) { okay = false; }
  else { x = G4String(bufp, 2); }

  return okay;
}

const std::vector<G4float>* G4LevelReader::NormalizedICCProbability(G4int Z)
{
  std::vector<G4float>* vec = nullptr;
  G4int LL = 3;
  G4int M = 5;
  G4int N = 1;
  if(Z <= 27) {
    M = N = 0;
    if(Z <= 4) {
      LL = 1;
    } else if(Z <= 6) {
      LL = 2;
    } else if(Z <= 10) {
    } else if(Z <= 12) {
      M = 1;
    } else if(Z <= 17) {
      M = 2;
    } else if(Z == 18) {
      M = 3;
    } else if(Z <= 20) {
      M = 3;
      N = 1;
    } else {
      M = 4;
      N = 1;
    }
    if(LL < 3) { for(G4int i=LL+1; i<=4; ++i) { fICC[i] = 0.0f; } }
    if(M < 5)  { for(G4int i=M+4;  i<=8; ++i) { fICC[i] = 0.0f; } }
    if(N < 1)  { fICC[9] = 0.0f; }
  }
  G4float norm = 0.0f;
  for(G4int i=0; i<10; ++i) {
    norm += fICC[i]; 
    fICC[i] = norm;
  }
  if(norm == 0.0f && fAlpha > fAlphaMax) {
    fICC[9] = norm = 1.0f;
  } 
  if(norm > 0.0f) {
    norm = 1.0f/norm;
    vec = new std::vector<G4float>;
    G4float x;
    for(G4int i=0; i<10; ++i) {
      x = fICC[i]*norm;
      if(x > 0.995f || 9 == i) {
	vec->push_back(1.0f);
	break;
      } 
      vec->push_back(x); 
    }
    if (fVerbose > 2) {
      G4int prec = G4cout.precision(3);
      G4cout << "# InternalConv: ";
      G4int nn = vec->size();
      for(G4int i=0; i<nn; ++i) { G4cout << " " << (*vec)[i]; }
      G4cout << G4endl;
      G4cout.precision(prec);
    }
  }
  return vec;
}

const G4LevelManager* 
G4LevelReader::CreateLevelManagerNEW(G4int Z, G4int A)
{
  std::ostringstream ss;
  ss << "/correlated_gamma/z" << Z << ".a" << A;
  G4String st = G4String(ss.str());
  fFile = fDirectory + st;
  std::ifstream infile(fFile, std::ios::in);

  // file is not opened
  if (!infile.is_open()) {
    if(fVerbose > 0) {
      G4ExceptionDescription ed;
      ed << " for Z= " << Z << " A= " << A  
	 << " <" << fFile << "> is not opened!"; 
      G4Exception("G4LevelReader::CreateLevelManagerNEW(..)","had014",
		  JustWarning, ed, "");
    }
    return nullptr;
  }
  if (fVerbose > 0) {
    G4cout << "G4LevelReader: open file for Z= " 
	   << Z << " A= " << A 
	   << " <" << fFile << ">" <<  G4endl;
  }
  return LevelManager(Z, A, 0, infile);
}

const G4LevelManager* 
G4LevelReader::MakeLevelManagerNEW(G4int Z, G4int A,
				   const G4String& filename)
{
  std::ifstream infile(filename, std::ios::in);

  // file is not opened
  if (!infile.is_open()) {
    G4ExceptionDescription ed;
    ed << " for Z= " << Z << " A= " << A  
       << " data file <" << filename << "> is not opened!";
    G4Exception("G4LevelReader::MakeLevelManagerNEW(..)","had014",
		JustWarning, ed, "Check G4LEVELGAMMADATA");
    return nullptr;
  }
  if (fVerbose > 0) {
    G4cout << "G4LevelReader: open file for Z= " 
	   << Z << " A= " << A 
	   << " <" << filename << ">" <<  G4endl;
  }
  return LevelManager(Z, A, 0, infile);
}

const G4LevelManager* 
G4LevelReader::LevelManager(G4int Z, G4int A, G4int nlev,
			    std::ifstream& infile)
{
  G4bool allLevels = fParam->StoreAllLevels();

  static const G4double fkev = CLHEP::keV;
  G4int nlevels = (0 == nlev) ? fLevelMax : nlev;
  if(fVerbose > 0) {
    G4cout << "## New isotope Z= " << Z << "  A= " << A;
    if(nlevels < fLevelMax) { G4cout << " Nlevels= " << nlevels; }
    G4cout << G4endl;
  }
  if(nlevels > fLevelMax) {
    fLevelMax = nlevels;
    vEnergy.resize(fLevelMax,0.0f);
    vSpin.resize(fLevelMax,0);
    vLevel.resize(fLevelMax,nullptr);
  }
  G4int ntrans(0), i1, i, k;
  G4int i2;    // Level number at which transition ends
  G4int tnum;  // Multipolarity index
  G4String xf("  ");
  G4float x, ener, tener;

  for(i=0; i<nlevels; ++i) {
    infile >> i1 >> xf;    // Level number and floating level
    //G4cout << "New line: i1= " << i1 << "  xf= <" << xf << "> " << G4endl;
    if(infile.eof()) {
      if(fVerbose > 1) { 
	G4cout << "### End of file Z= " << Z << " A= " << A 
	       << " Nlevels= " << i << G4endl;
      }
      break;
    }
    if(i1 != i) {
      G4ExceptionDescription ed;
      ed << " G4LevelReader: wrong data file for Z= " << Z << " A= " << A 
	 << " level #" << i << " has index " << i1 << G4endl;
      G4Exception("G4LevelReader::LevelManager(..)","had014",
		  FatalException, ed, "Check G4LEVELGAMMADATA");
    }

    if(!(ReadDataItem(infile,ener) &&
	 ReadDataItem(infile,fTime)   &&
	 ReadDataItem(infile,fSpin)   &&
	 ReadDataItem(infile,ntrans))) {
      if(fVerbose > 1) { 
	G4cout << "### End of file Z= " << Z << " A= " << A 
	       << " Nlevels= " << i << G4endl;
      }
      break;
    }
    fTime = std::max(fTime, 0.0f); 
    ener *= fkev;
    for(k=0; k<nfloting; ++k) {
      if(xf == fFloatingLevels[k]) {
	break;
      }
    }
    // if a previous level has not transitions it may be ignored
    if(0 < i) {
      // protection
      if(ener < vEnergy[i-1]) {
	G4cout << "### G4LevelReader: broken level " << i
	       << " E(MeV)= " << ener << " < " << vEnergy[i-1]
	       << " for isotope Z= " << Z << " A= " 
	       << A << " level energy increased" << G4endl; 
	ener = vEnergy[i-1];
      }
    }
    vEnergy[i] = ener;
    fTime     *= fTimeFactor;
    if(fSpin > 20.0f) { fSpin = 0.0f; }
    vSpin[i]   = (G4int)(100 + fSpin + fSpin) + k*100000;
    if(fVerbose > 1) {
      G4cout << "   Level #" << i1 << " E(MeV)= " << ener/CLHEP::MeV
	     << "  LTime(s)= " << fTime << " 2S= " << vSpin[i]
	     << "  meta= " << vSpin[i]/100000 << " idx= " << i  
	     << " ntr= " << ntrans << G4endl;
    }
    vLevel[i] = nullptr;
    if(ntrans > 0) {

      // there are transitions
      if(ntrans > fTransMax) {
	fTransMax = ntrans;
	vTrans.resize(fTransMax);
	vRatio.resize(fTransMax);
	vGammaCumProbability.resize(fTransMax);
	vGammaProbability.resize(fTransMax);
	vShellProbability.resize(fTransMax);
	vMpRatio.resize(fTransMax);
      }
      fNorm1 = 0.0f;
      for(G4int j=0; j<ntrans; ++j) {
       
	if(!(ReadDataItem(infile,i2)        &&
	     ReadDataItem(infile,tener) &&
	     ReadDataItem(infile,fProb)     &&
	     ReadDataItem(infile,tnum)      &&
	     ReadDataItem(infile,vRatio[j]) &&
	     ReadDataItem(infile,fAlpha))) {
	  //infile >>i2 >> tener >> fProb >> vTrans[j] >> fRatio >> fAlpha; 
	  //if(infile.fail()) { 
	  if(fVerbose > 0) { 
	    G4cout << "### Fail to read transition j= " << j 
		   << "  Z= " << Z << " A= " << A << G4endl; 
	  }
	  break;
	}
        if(i2 >= i) {
	  G4cout << "### G4LevelReader: broken transition " << j 
		 << " from level " << i << " to " << i2
		 << " for isotope Z= " << Z << " A= " 
		 << A << " - use ground level" << G4endl; 
          i2 = 0;
	}
	vTrans[j] = i2*10000 + tnum;
        if(fAlpha < fAlphaMax) {
	  x = 1.0f + fAlpha;
	  fNorm1 += x*fProb;
	  vGammaCumProbability[j] = fNorm1;
	  vGammaProbability[j] = 1.0f/x;
	} else {
	  // only internal conversion case - no gamma conversion at all
	  fNorm1 += fProb;
	  vGammaCumProbability[j] = fNorm1;
	  vGammaProbability[j] = 0.0f;
	} 
	vShellProbability[j] = nullptr;
	if(fVerbose > 1) { 
	  G4int prec = G4cout.precision(4);
	  G4cout << "### Transition #" << j << " to level " << i2 
		 << " i2= " << i2 <<  " Etrans(MeV)= " << tener*fkev
		 << "  fProb= " << fProb << " MultiP= " << tnum
		 << "  fMpRatio= " << fRatio << " fAlpha= " << fAlpha 
		 << G4endl;
	  G4cout.precision(prec);
	}
	if(fAlpha > 0.0f) {
	  for(k=0; k<10; ++k) {
	    //infile >> fICC[k];
	    if(!ReadDataItem(infile,fICC[k])) {
	      //if(infile.fail()) { 
	      if(fVerbose > 0) { 
		G4cout << "### Fail to read convertion coeff k= " << k 
		       << " for transition j= " << j 
		       << "  Z= " << Z << " A= " << A << G4endl; 
	      }
	      break;
	    }
	  }
	  if(allLevels) { 
	    vShellProbability[j] = NormalizedICCProbability(Z);
            if(!vShellProbability[j]) { vGammaProbability[j] = 1.0f; }
	  }
	}
      }
      if(0.0f < fNorm1) { fNorm1 = 1.0f/fNorm1; } 
      G4int nt = ntrans - 1;      
      for(k=0; k<nt; ++k) {
	vGammaCumProbability[k] *= fNorm1;
	if(fVerbose > 2) {
	  G4cout << "Probabilities[" << k 
		 << "]= " << vGammaCumProbability[k]
	       << "  " << vGammaProbability[k]
		 << " idxTrans= " << vTrans[k]/10000
		 << G4endl;
	}
      }
      vGammaCumProbability[nt] = 1.0f;
      if(fVerbose > 2) {
	G4cout << "Probabilities[" << nt << "]= " 
	       << vGammaCumProbability[nt]
	       << "  " << vGammaProbability[nt]
	       << " IdxTrans= " << vTrans[nt]/10000
	       << G4endl;
      }
      if(fVerbose > 1) {       
	G4cout << "   New G4NucLevel:  Ntrans= " << ntrans  
	       << " Time(ns)= " << fTime << G4endl; 
      }
      vLevel[i] = new G4NucLevel((size_t)ntrans, fTime,
				 vTrans,
				 vGammaCumProbability,
				 vGammaProbability,
				 vMpRatio,
				 vShellProbability);
    }
  }
  G4LevelManager* lman = nullptr;
  if(1 < i) { 
    lman = new G4LevelManager((size_t)i,vEnergy,vSpin,vLevel);
    if(fVerbose > 0) {
      G4cout << "=== Reader: new manager for Z= " << Z << " A= " << A 
	     << " Nlevels= " << i << " E[0]= " 
	     << vEnergy[0]/CLHEP::MeV << " MeV   E1= " 
	     << vEnergy[i-1]/CLHEP::MeV << " MeV "
	     << G4endl;
    }
  }

  return lman;
}
