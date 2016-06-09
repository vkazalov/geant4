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
// $Id: G4ViewParameters.hh,v 1.28.2.1 2009/03/13 09:02:57 gcosmo Exp $
// GEANT4 tag $Name: geant4-09-02-patch-02 $
//
// 
// John Allison  19th July 1996
//
// Class description
//
// View parameters and options.
//
// THE STANDARD VIEW AND ALL THAT.
//
// In GEANT4 visualization, we have the concept of a "Standard
// View".  This is the view when the complete set of objects being
// viewed is comfortably in view from any viewpoint.  It is defined by
// the "Bounding Sphere" of "visible" objects when initially
// registered in the scene, and by the View Parameters.
//
// There is also the "Standard Target Point", which is the centre of
// the Bounding Sphere (note that this belongs to the scene and is
// stored in the G4Scene object).  The "Current Target Point", defined
// relative to the Standard Target Point, is changed by the
// "dolly" and "zoom" commands, and can be reset to the Standard
// Target Point with the "/vis/viewer/reset" command.
//
// Also, the "Standard Camera Position" is the "Standard Camera
// Distance" along the Viewpoint Direction vector from the Standard
// Target Point.  The Standard Camera Distance is the radius of the
// Bounding Sphere divided by fFieldHalfAngle.  It is not stored
// explicitly because of the singularity at fFieldHalfAngle = 0,
// which implies parallel projection.
//
// Similarly, the "Current Camera Position" is the "Current Camera
// Distance" along the Viewpoint Direction vector from the Current
// Target Point.  The Current Camera Distance is given by the formulae
// below, but note that it can be negative, meaning that the camera
// has moved *beyond* the Current Target Point, which is
// conceptually possible, but which might give some problems when
// setting up the view matrix - see, for example, G4OpenGLView::SetView ().
//
// All viewers are expected to keep the "Up Vector" vertical.
//
// Finally, the view is magnified by the "Zoom Factor" which is
// reset to 1 by the "/vis/viewer/reset" command.
//
// The algorithms for calculating various useful quantities from the
// View Parameters, such as GetCameraDistance, are described below.

#ifndef G4VIEWPARAMETERS_HH
#define G4VIEWPARAMETERS_HH

#include <vector>
#include "G4Vector3D.hh"
#include "G4Point3D.hh"
#include "G4Plane3D.hh"
#include "G4VisAttributes.hh"
#include "G4VMarker.hh"

typedef std::vector<G4Plane3D> G4Planes;

class G4ViewParameters {

public: // With description

  enum DrawingStyle {
    wireframe,  // Draw edges    - no hidden line removal.
    hlr,        // Draw edges    - hidden lines removed.
    hsr,        // Draw surfaces - hidden surfaces removed.
    hlhsr       // Draw surfaces and edges - hidden removed.
  };

  enum RepStyle {
    polyhedron, // Use G4Polyhedron.
    nurbs       // Use G4NURBS.
  };

  enum CutawayMode {
    cutawayUnion,       // Union (addition) of result of each cutaway plane.
    cutawayIntersection // Intersection (multiplication) " .
  };

  friend std::ostream& operator << (std::ostream&,
				      const DrawingStyle&);

  friend std::ostream& operator << (std::ostream&,
				      const G4ViewParameters&);

  G4ViewParameters ();
  ~G4ViewParameters ();

  // Note: uses default assignment operator and copy constructor.

  G4bool operator != (const G4ViewParameters&) const;

  // Get and Is functions.
        DrawingStyle     GetDrawingStyle         () const;
        G4bool           IsAuxEdgeVisible        () const;
        RepStyle         GetRepStyle             () const;
        G4bool           IsCulling               () const;
        G4bool           IsCullingInvisible      () const;
        G4bool           IsDensityCulling        () const;
        G4double         GetVisibleDensity       () const;
        G4bool           IsCullingCovered        () const;
        G4bool           IsSection               () const;
  const G4Plane3D&       GetSectionPlane         () const;
        G4bool           IsCutaway               () const;
        CutawayMode      GetCutawayMode          () const;
  const G4Planes&        GetCutawayPlanes        () const;
        G4bool           IsExplode               () const;
        G4double         GetExplodeFactor        () const;
  const G4Point3D&       GetExplodeCentre        () const;
        G4int            GetNoOfSides            () const;
  const G4Vector3D&      GetViewpointDirection   () const;
  const G4Vector3D&      GetUpVector             () const;
        G4double         GetFieldHalfAngle       () const;
        G4double         GetZoomFactor           () const;
  const G4Vector3D&      GetScaleFactor          () const;
  const G4Point3D&       GetCurrentTargetPoint   () const;
        G4double         GetDolly                () const;
        G4bool           GetLightsMoveWithCamera () const;
  const G4Vector3D&      GetLightpointDirection  () const;  // Relative...
        G4Vector3D&      GetActualLightpointDirection  ();  // Actual...
  // ... depending on GetLightsMoveWithCamera.
  const G4VisAttributes* GetDefaultVisAttributes () const;
  const G4VisAttributes* GetDefaultTextVisAttributes () const;
  const G4VMarker&       GetDefaultMarker        () const;
        G4double         GetGlobalMarkerScale    () const;
        G4double         GetGlobalLineWidthScale () const;
        G4bool           IsMarkerNotHidden       () const;
        unsigned int     GetWindowSizeHintX      () const;
        unsigned int     GetWindowSizeHintY      () const;
        G4int            GetWindowAbsoluteLocationHintX (G4int) const;
        G4int            GetWindowAbsoluteLocationHintY (G4int) const;
        G4int            GetWindowLocationHintX  () const;
        G4int            GetWindowLocationHintY  () const;
  const G4String&        GetXGeometryString      () const;
  // GetXGeometryString is intended to be parsed by XParseGeometry.
  // It contains the size information, as in GetWindowSizeHint, but
  // may also contain the window position, e.g., "600x600-0+200.  The
  // viewer should use this in preference to GetWindowSizeHint, since
  // it contains more information.  (The size information in
  // GetXGeometryString and GetWindowSizeHint is guaranteed to be
  // identical.)
  bool IsWindowSizeHintX () const;
  bool IsWindowSizeHintY () const;
  bool IsWindowLocationHintX () const;
  bool IsWindowLocationHintY () const;

        G4bool           IsAutoRefresh           () const;
  const G4Colour&        GetBackgroundColour     () const;
        G4bool           IsPicking               () const;

  // Here Follow functions to evaluate the above algorithms as a
  // function of the radius of the Bounding Sphere of the object being
  // viewed.  Call them in the order given - for efficiency, later
  // functions depend on the results of earlier ones (Store the
  // results of earlier functions in your own temporary variables -
  // see, for example, G4OpenGLView::SetView ().)
  G4double GetCameraDistance  (G4double radius) const;
  G4double GetNearDistance    (G4double cameraDistance, G4double radius) const;
  G4double GetFarDistance     (G4double cameraDistance,
			       G4double nearDistance, G4double radius) const;
  G4double GetFrontHalfHeight (G4double nearDistance, G4double radius) const;

  // Set, Add, Multiply, Increment, Unset and Clear functions.
  void SetDrawingStyle         (G4ViewParameters::DrawingStyle style);
  void SetAuxEdgeVisible       (G4bool);
  void SetRepStyle             (G4ViewParameters::RepStyle style);
  void SetCulling              (G4bool);
  void SetCullingInvisible     (G4bool);
  void SetDensityCulling       (G4bool);
  void SetVisibleDensity       (G4double visibleDensity);
  void SetCullingCovered       (G4bool);
  void SetSectionPlane         (const G4Plane3D& sectionPlane);
  void UnsetSectionPlane       ();
  void SetCutawayMode          (CutawayMode);
  void AddCutawayPlane         (const G4Plane3D& cutawayPlane);
  void ChangeCutawayPlane      (size_t index, const G4Plane3D& cutawayPlane);
  void ClearCutawayPlanes      ();
  void SetExplodeFactor        (G4double explodeFactor);
  void UnsetExplodeFactor      ();
  void SetExplodeCentre        (const G4Point3D& explodeCentre);
  G4int SetNoOfSides           (G4int nSides);  // Returns actual number set.
  void SetViewpointDirection   (const G4Vector3D& viewpointDirection);
  // Calls the following to get lightpoint direction right too.
  void SetViewAndLights        (const G4Vector3D& viewpointDirection);
  // Also sets lightpoint direction according to G4bool fLightsMoveWithCamera.
  void SetUpVector             (const G4Vector3D& upVector);
  void SetFieldHalfAngle       (G4double fieldHalfAngle);
  void SetZoomFactor           (G4double zoomFactor);
  void MultiplyZoomFactor      (G4double zoomFactorMultiplier);
  void SetScaleFactor          (const G4Vector3D& scaleFactor);
  void MultiplyScaleFactor     (const G4Vector3D& scaleFactorMultiplier);
  void SetCurrentTargetPoint   (const G4Point3D& currentTargetPoint);
  void SetDolly                (G4double dolly);
  void IncrementDolly          (G4double dollyIncrement);
  void SetLightpointDirection  (const G4Vector3D& lightpointDirection);
  void SetLightsMoveWithCamera (G4bool moves);
  void SetPan                  (G4double right, G4double up);
  void IncrementPan            (G4double right, G4double up);
  // Increment currentTarget point perpendicular to viewpoint direction.
  void IncrementPan            (G4double right, G4double up, G4double forward);
  // Increment currentTarget point also along viewpoint direction.
  void SetDefaultVisAttributes (const G4VisAttributes&);
  void SetDefaultTextVisAttributes (const G4VisAttributes&);
  void SetDefaultMarker        (const G4VMarker& defaultMarker);
  void SetGlobalMarkerScale    (G4double globalMarkerScale);
  void SetGlobalLineWidthScale (G4double globalLineWidthScale);
  void SetMarkerHidden         ();
  void SetMarkerNotHidden      ();
  void SetWindowSizeHint       (G4int xHint, G4int yHint);
  void SetWindowLocationHint   (G4int xHint, G4int yHint);
  void SetXGeometryString      (const G4String&);
  void SetAutoRefresh          (G4bool);
  void SetBackgroundColour     (const G4Colour&);
  void SetPicking              (G4bool);

  void PrintDifferences (const G4ViewParameters& v) const;

private:
  G4int ParseGeometry ( const char *string, G4int *x, G4int *y, unsigned int *width, unsigned int *height);
  G4int ReadInteger(char *string, char **NextString);

  G4int fNoValue;
  G4int fXValue; // XValue set for XGeometry
  G4int fYValue; // YValue set for XGeometry
  G4int fWidthValue; // WidthValue set for XGeometry
  G4int fHeightValue; // HeightValue set for XGeometry
  G4int fAllValues; // AllValues are set for XGeometry
  G4int fXNegative; // XValue is from left for XGeometry
  G4int fYNegative; // YValue is from top for XGeometry
  G4int fGeometryMask; // Mask for ParseGeometry

  DrawingStyle fDrawingStyle;    // Drawing style.
  G4bool       fAuxEdgeVisible;  // Auxiliary edge visibility.
  RepStyle     fRepStyle;        // Representation style.
  G4bool       fCulling;         // Culling requested.
  G4bool       fCullInvisible;   // Cull (don't Draw) invisible objects.
  G4bool       fDensityCulling;  // Density culling requested.  If so...
  G4double     fVisibleDensity;  // ...density lower than this not drawn.
  G4bool       fCullCovered;     // Cull daughters covered by opaque mothers.
  G4bool       fSection;         // Section drawing requested (DCUT in GEANT3).
  G4Plane3D    fSectionPlane;    // Cut plane for section drawing (DCUT).
  CutawayMode  fCutawayMode;     // Cutaway mode.
  G4Planes     fCutawayPlanes;   // Set of planes used for cutaway.
  G4double     fExplodeFactor;   // Explode along radius by this factor...
  G4Point3D    fExplodeCentre;   // ...about this centre.
  G4int        fNoOfSides;       // ...if polygon approximates circle.
  G4Vector3D   fViewpointDirection;
  G4Vector3D   fUpVector;        // Up vector.  (Warning: MUST NOT be parallel
                                 // to fViewpointDirection!)
  G4double     fFieldHalfAngle;  // Radius / camara distance, 0 for parallel.
  G4double     fZoomFactor;      // Magnification relative to Standard View.
  G4Vector3D   fScaleFactor;     // (Non-uniform) scale/magnification factor.
  G4Point3D    fCurrentTargetPoint;  // Relative to standard target point.
  G4double     fDolly;           // Distance towards current target point.
  G4bool       fLightsMoveWithCamera;
  G4Vector3D   fRelativeLightpointDirection;
  // i.e., rel. to object or camera accoding to G4bool fLightsMoveWithCamera.
  G4Vector3D   fActualLightpointDirection;
  G4VisAttributes fDefaultVisAttributes;
  G4VisAttributes fDefaultTextVisAttributes;
  G4VMarker    fDefaultMarker;
  G4double     fGlobalMarkerScale;
  G4double     fGlobalLineWidthScale;
  G4bool       fMarkerNotHidden;
  // True if transients are to be drawn and not hidden by
  // hidden-line-hidden-surface removal algorithms, e.g., z-buffer
  // testing; false if they are to be hidden-line-hidden-surface
  // removed.
  G4int        fWindowSizeHintX; // Size hints for pixel-based window systems.
  G4int        fWindowSizeHintY;
  G4int        fWindowLocationHintX; // Location hints for pixel-based window systems.
  G4int        fWindowLocationHintY;
  G4bool       fWindowLocationHintXNegative; //  Reference of location hints for pixel-based window systems.
  G4bool       fWindowLocationHintYNegative;
  G4String     fXGeometryString; // If non-null, geometry string for X Windows.
  G4bool       fAutoRefresh;     // ...after change of view parameters.
  G4Colour     fBackgroundColour;
  G4bool       fPicking;         // Request picking.
};

#include "G4ViewParameters.icc"

#endif
