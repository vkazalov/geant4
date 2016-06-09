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
//
// $Id: G4OpenGLStoredQtViewer.cc,v 1.20.2.1 2009/03/13 09:02:57 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-02-patch-02 $
//
//
// Class G4OpenGLStoredQtViewer : a class derived from G4OpenGLQtViewer and
//                                G4OpenGLStoredViewer.

#ifdef G4VIS_BUILD_OPENGLQT_DRIVER

#include "G4OpenGLStoredQtViewer.hh"

#include "G4ios.hh"

//#include <qmouseevent.h>
#include <qevent.h> // include <qcontextmenuevent.h>

G4OpenGLStoredQtViewer::G4OpenGLStoredQtViewer
(G4OpenGLStoredSceneHandler& sceneHandler,
 const G4String&  name):
  G4VViewer (sceneHandler, sceneHandler.IncrementViewCount (), name),
  G4OpenGLViewer (sceneHandler),
  G4OpenGLQtViewer (sceneHandler),
  G4OpenGLStoredViewer (sceneHandler)             // FIXME : gerer le pb du parent !
{

  //set true to picking
  fVP.SetPicking(true);
#if QT_VERSION < 0x040000
  setFocusPolicy(QWidget::StrongFocus); // enable keybord events
#else
  setFocusPolicy(Qt::StrongFocus); // enable keybord events
#endif
  hasToRepaint =false;

  if (fViewId < 0) return;  // In case error in base class instantiation.
}

G4OpenGLStoredQtViewer::~G4OpenGLStoredQtViewer() {
  makeCurrent();
  // this is connect to the Dialog for deleting it properly
  // when close event.
  //   ((QDialog*)window())->reject();
}

void G4OpenGLStoredQtViewer::Initialise() {
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::Initialise 1\n");
#endif
  readyToPaint = false;
  CreateMainWindow (this,QString(fName));
  CreateFontLists ();
  
  readyToPaint = true;
}

void G4OpenGLStoredQtViewer::initializeGL () {

  InitializeGLView ();

#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::InitialiseGL () 1\n");
#endif

  // clear the buffers and window.
  ClearView ();
  FinishView ();
   
  glDepthFunc (GL_LEQUAL);
  glDepthMask (GL_TRUE);

  if (fSceneHandler.GetScene() == 0) {
    hasToRepaint =false;
  } else {
    hasToRepaint =true;
  }

#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::InitialiseGL  END\n");
#endif
}


void G4OpenGLStoredQtViewer::DrawView () {
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::DrawView  VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n");
#endif
  // That's no the same logic as Immediate Viewer, I don't know why...
  // But if I send updateGL here, we go here :
  //  updateQWidget -> paintGL -> ComputeView
  // whih is not the same as ComputeView Directly
  // And we loose the redraw of things !
  
  ComputeView();
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::DrawView  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
#endif
}

void G4OpenGLStoredQtViewer::ComputeView () {

#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::ComputeView %d %d   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n",fWinSize_x, fWinSize_y);
#endif
  makeCurrent();
  G4ViewParameters::DrawingStyle style = GetViewParameters().GetDrawingStyle();

  //Make sure current viewer is attached and clean...

  //See if things have changed from last time and remake if necessary...
  // The fNeedKernelVisit flag might have been set by the user in
  // /vis/viewer/rebuild, but if not, make decision and set flag only
  // if necessary...
  if (!fNeedKernelVisit) {
    KernelVisitDecision ();
  }
  G4bool kernelVisitWasNeeded = fNeedKernelVisit; // Keep (ProcessView resets).
  ProcessView ();
   

  if(style!=G4ViewParameters::hlr &&
     haloing_enabled) {
#ifdef G4DEBUG_VIS_OGL
    printf("G4OpenGLStoredQtViewer::ComputeView DANS LE IF\n");
#endif

    HaloingFirstPass ();
    DrawDisplayLists ();
    glFlush ();

    HaloingSecondPass ();

    DrawDisplayLists ();
    FinishView ();

  } else {
     
    // If kernel visit was needed, drawing and FinishView will already
    // have been done, so...
    if (!kernelVisitWasNeeded) {
#ifdef G4DEBUG_VIS_OGL
      printf("**************************  G4OpenGLStoredQtViewer::ComputeView Don't need kernel Visit \n");
#endif
      DrawDisplayLists ();
      FinishView ();
    } else {
#ifdef G4DEBUG_VIS_OGL
      printf("**************************  G4OpenGLStoredQtViewer::ComputeView need kernel Visit \n");
#endif
      // However, union cutaways are implemented in DrawDisplayLists, so make
      // an extra pass...
      if (fVP.IsCutaway() &&
          fVP.GetCutawayMode() == G4ViewParameters::cutawayUnion) {
        ClearView();
        DrawDisplayLists ();
        FinishView ();
#ifdef G4DEBUG_VIS_OGL
        printf("***************************  CASE 4 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n");
#endif
      } else { // ADD TO AVOID KernelVisit=1 and nothing to display
        DrawDisplayLists ();
        FinishView ();
      }
    }
  }

  if (isRecording()) {
    savePPMToTemp();
  }

#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::ComputeView %d %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n",fWinSize_x, fWinSize_y);
#endif
  hasToRepaint =true;
}


/**
   - Lors du resize de la fenetre, on doit non pas redessiner le detecteur, mais aussi les evenements
*/
void G4OpenGLStoredQtViewer::resizeGL(
                                      int aWidth
                                      ,int aHeight)
{  
  // Set new size, it will be update when next Repaint()->SetView() called
  fWinSize_x = aWidth;
  fWinSize_y = aHeight;
  hasToRepaint = true;
}


void G4OpenGLStoredQtViewer::paintGL()
{
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::paintGL ??\n");
#endif
  if (!readyToPaint) {
    readyToPaint= true;
    return;
  }
  // DO NOT RESIZE IF SIZE HAS NOT CHANGE :
  //    WHEN CLICK ON THE FRAME FOR EXAMPLE
  //    EXECEPT WHEN MOUSE MOVE EVENT
  if ( !hasToRepaint) {
    if (((fWinSize_x == (unsigned int)width())) &&(fWinSize_y == (unsigned int) height())) {
      return;
    }
  }
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::paintGL VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV ready %d\n",readyToPaint);
#endif

  SetView();
          
  ClearView (); //ok, put the background correct
  ComputeView();
     
  hasToRepaint =false;
     
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLStoredQtViewer::paintGL ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ready %d\n",readyToPaint);
#endif
}

void G4OpenGLStoredQtViewer::mousePressEvent(QMouseEvent *event)
{
  G4MousePressEvent(event);
}

void G4OpenGLStoredQtViewer::keyPressEvent (QKeyEvent * event) 
{
  G4keyPressEvent(event);
}

void G4OpenGLStoredQtViewer::wheelEvent (QWheelEvent * event) 
{
  G4wheelEvent(event);
}

/**
 * This function was build in order to make a zoom on double clic event.
 * It was think to build a rubberband on the zoom area, but never work fine
 */
void G4OpenGLStoredQtViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
  G4MouseDoubleClickEvent();
}

void G4OpenGLStoredQtViewer::mouseReleaseEvent(QMouseEvent *event)
{
  G4MouseReleaseEvent();
}

void G4OpenGLStoredQtViewer::mouseMoveEvent(QMouseEvent *event)
{
  G4MouseMoveEvent(event);
}


void G4OpenGLStoredQtViewer::contextMenuEvent(QContextMenuEvent *e)
{
  G4manageContextMenuEvent(e);
}

void G4OpenGLStoredQtViewer::updateQWidget() {
  hasToRepaint= true;
  updateGL();
  hasToRepaint= false;
}

void G4OpenGLStoredQtViewer::ShowView (
) 
//////////////////////////////////////////////////////////////////////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
{
#if QT_VERSION < 0x040000
  setActiveWindow();
#else
  activateWindow();
#endif
  updateQWidget();
}

#endif
