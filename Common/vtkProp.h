/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProp.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkProp - abstract superclass for all actors, volumes and annotations
// .SECTION Description
// vtkProp is an abstract superclass for any objects that can exist in a
// rendered scene (either 2D or 3D). Instances of vtkProp may respond to
// various render methods (e.g., RenderOpaqueGeometry()). vtkProp also
// defines the API for picking, LOD manipulation, and common instance 
// variables that control visibility, picking, and dragging.
// .SECTION See Also
// vtkActor2D vtkActor vtkVolume vtkProp3D

#ifndef __vtkProp_h
#define __vtkProp_h

#include "vtkObject.h"

class vtkAssemblyPath;
class vtkAssemblyPaths;
class vtkMatrix4x4;
class vtkPropCollection;
class vtkViewport;
class vtkWindow;

class VTK_COMMON_EXPORT vtkProp : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkProp,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // For some exporters and other other operations we must be
  // able to collect all the actors or volumes. These methods
  // are used in that process.
  virtual void GetActors(vtkPropCollection *) {}
  virtual void GetActors2D(vtkPropCollection *) {}
  virtual void GetVolumes(vtkPropCollection *) {}

  // Description:
  // Set/Get visibility of this vtkProp.
  vtkSetMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkBooleanMacro(Visibility, int);

  // Description:
  // Set/Get the pickable instance variable.  This determines if the vtkProp
  // can be picked (typically using the mouse). Also see dragable.
  vtkSetMacro(Pickable,int);
  vtkGetMacro(Pickable,int);
  vtkBooleanMacro(Pickable,int);

  // Description:
  // Method fires PickEvent if the prop is picked.
  virtual void Pick();

  // Description:
  // Set/Get the value of the dragable instance variable. This determines if 
  // an Prop, once picked, can be dragged (translated) through space.
  // This is typically done through an interactive mouse interface.
  // This does not affect methods such as SetPosition, which will continue
  // to work.  It is just intended to prevent some vtkProp'ss from being
  // dragged from within a user interface.
  vtkSetMacro(Dragable,int);
  vtkGetMacro(Dragable,int);
  vtkBooleanMacro(Dragable,int);

  // Description:
  // Return the mtime of anything that would cause the rendered image to 
  // appear differently. Usually this involves checking the mtime of the 
  // prop plus anything else it depends on such as properties, textures
  // etc.
  virtual unsigned long GetRedrawMTime() {return this->GetMTime();}
  
  // Description:
  // Get the bounds for this Prop as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
  // in world coordinates. NULL means that the bounds are not defined.
  virtual double *GetBounds() {return NULL;}

  // Description:
  // Shallow copy of this vtkProp.
  virtual void ShallowCopy(vtkProp *prop);

  // Description:
  // vtkProp and its subclasses can be picked by subclasses of
  // vtkAbstractPicker (e.g., vtkPropPicker). The following methods interface
  // with the picking classes and return "pick paths". A pick path is a
  // hierarchical, ordered list of props that form an assembly.  Most often,
  // when a vtkProp is picked, its path consists of a single node (i.e., the
  // prop). However, classes like vtkAssembly and vtkPropAssembly can return
  // more than one path, each path being several layers deep. (See
  // vtkAssemblyPath for more information.)  To use these methods - first
  // invoke InitPathTraversal() followed by repeated calls to GetNextPath().
  // GetNextPath() returns a NULL pointer when the list is exhausted.
  virtual void InitPathTraversal();
  virtual vtkAssemblyPath *GetNextPath();
  virtual int GetNumberOfPaths() {return 1;}

  // Description:
  // These methods are used by subclasses to place a matrix (if any) in the
  // prop prior to rendering. Generally used only for picking. See vtkProp3D
  // for more information.
  virtual void PokeMatrix(vtkMatrix4x4 *vtkNotUsed(matrix)) {}
  virtual vtkMatrix4x4 *GetMatrix() {return NULL;}

//BTX  
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // All concrete subclasses must be able to render themselves.
  // There are three key render methods in vtk and they correspond
  // to three different points in the rendering cycle. Any given
  // prop may implement one or more of these methods. 
  // The first method is intended for rendering all opaque geometry. The
  // second method is intended for rendering all translucent geometry. Most
  // volume rendering mappers draw their results during this second method.
  // The last method is to render any 2D annotation or overlays.
  // Each of these methods return an integer value indicating
  // whether or not this render method was applied to this data. 
  virtual int RenderOpaqueGeometry(      vtkViewport *) { return 0; }
  virtual int RenderTranslucentGeometry( vtkViewport *) { return 0; }
  virtual int RenderOverlay(             vtkViewport *) { return 0; }

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *) {}

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // The EstimatedRenderTime may be used to select between different props,
  // for example in LODProp it is used to select the level-of-detail.
  // The value is returned in seconds. For simple geometry the accuracy may
  // not be great due to buffering. For ray casting, which is already
  // multi-resolution, the current resolution of the image is factored into
  // the time. We need the viewport for viewing parameters that affect timing.
  // The no-arguments version simply returns the value of the variable with
  // no estimation.
  virtual double GetEstimatedRenderTime( vtkViewport * )
    { return this->EstimatedRenderTime; }
  virtual double GetEstimatedRenderTime(){ return this->EstimatedRenderTime; }
  
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // This method is used by, for example, the vtkLODProp3D in order to
  // initialize the estimated render time at start-up to some user defined
  // value.
  virtual void SetEstimatedRenderTime(double t) 
    {this->EstimatedRenderTime = t; this->SavedEstimatedRenderTime = t;}
    
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // When the EstimatedRenderTime is first set to 0.0 (in the
  // SetAllocatedRenderTime method) the old value is saved. This
  // method is used to restore that old value should the render be
  // aborted.
  virtual void RestoreEstimatedRenderTime()
    { this->EstimatedRenderTime = this->SavedEstimatedRenderTime; }
  
  
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // This method is intended to allow the renderer to add to the
  // EstimatedRenderTime in props that require information that
  // the renderer has in order to do this. For example, props
  // that are rendered with a ray casting method do not know
  // themselves how long it took for them to render. We don't want to
  // cause a this->Modified() when we set this value since it is not
  // really a modification to the object. (For example, we don't want
  // to rebuild matrices at every render because the estimated render time
  // is changing)
  virtual void AddEstimatedRenderTime(double t, vtkViewport *vtkNotUsed(vp))
    {this->EstimatedRenderTime+=t;}

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // The renderer may use the allocated rendering time to determine
  // how to render this actor. Therefore it might need the information
  // provided in the viewport.
  // A side effect of this method is to reset the EstimatedRenderTime to
  // 0.0. This way, each of the ways that this prop may be rendered can
  // be timed and added together into this value.
  virtual void SetAllocatedRenderTime(double t, vtkViewport *vtkNotUsed(v)) 
    {
    this->AllocatedRenderTime = t;
    this->SavedEstimatedRenderTime = this->EstimatedRenderTime;
    this->EstimatedRenderTime = 0.0;
    }

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  vtkGetMacro(AllocatedRenderTime, double);

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Get/Set the multiplier for the render time. This is used
  // for culling and is a number between 0 and 1. It is used
  // to create the allocated render time value.
  void SetRenderTimeMultiplier( double t ) { this->RenderTimeMultiplier = t; }
  vtkGetMacro(RenderTimeMultiplier, double);

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Used to construct assembly paths and perform part traversal.
  virtual void BuildPaths(vtkAssemblyPaths *paths, vtkAssemblyPath *path);

  // Description:
  // Get the number of consumers
  vtkGetMacro(NumberOfConsumers,int);
  
  // Description:
  // Add or remove or get or check a consumer, 
  void AddConsumer(vtkObject *c);
  void RemoveConsumer(vtkObject *c);
  vtkObject *GetConsumer(int i);
  int IsConsumer(vtkObject *c);

//ETX

protected:
  vtkProp();
  ~vtkProp();

  int Visibility;
  int Pickable;
  int Dragable;

  double AllocatedRenderTime;
  double EstimatedRenderTime;
  double SavedEstimatedRenderTime;
  double RenderTimeMultiplier;

  // how many consumers does this object have
  int NumberOfConsumers;
  vtkObject **Consumers;

  // support multi-part props and access to paths of prop
  // stuff that follows is used to build the assembly hierarchy
  vtkAssemblyPaths *Paths;
  
private:
  vtkProp(const vtkProp&);  // Not implemented.
  void operator=(const vtkProp&);  // Not implemented.
};

#endif


