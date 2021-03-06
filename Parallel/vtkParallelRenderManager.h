/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelRenderManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  Copyright 2003 Sandia Corporation. Under the terms of Contract
  DE-AC04-94AL85000, there is a non-exclusive license for use of this work by
  or on behalf of the U.S. Government. Redistribution and use in source and
  binary forms, with or without modification, are permitted provided that this
  Notice and any statement of authorship are reproduced on all copies.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkParallelRenderManager - An object to control parallel rendering.
//
// .SECTION Description:
// vtkParallelRenderManager operates in multiple processes.  It provides
// proper renderers and render windows for performing the parallel
// rendering correctly.  It can also attach itself to render windows and
// propagate rendering events and camera views.
//
// This class is based on the vtkCompositeManager class, except that it can
// handle any type of parallel rendering.
//
// .SECTION Note:
// Many parallel rendering schemes do not correctly handle transparency.
// Unless otherwise documented, assume a sub class does not.
//
// .SECTION ToDo:
// Synchronization/barrier primitives.
//
// Query ranges of scalar values of objects in addition to the boundry in
// three-space
//

#ifndef __vtkParallelRenderManager_h
#define __vtkParallelRenderManager_h

#include "vtkObject.h"

class vtkRenderWindow;
class vtkRenderer;
class vtkUnsignedCharArray;
class vtkDoubleArray;
class vtkTimerLog;
class vtkMultiProcessController;

class VTK_PARALLEL_EXPORT vtkParallelRenderManager : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkParallelRenderManager, vtkObject);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Builds a vtkRenderWindow compatible with this render manager.  The
  // user program is responsible for registering the render window with the
  // SetRenderWindow method and calling Delete.  It is not advisable to use
  // a parallel render manager with a render window that was not built with
  // this method.
  virtual vtkRenderWindow *MakeRenderWindow();

  // Description:
  // Builds a vtkRenderer compatible with this render manager.  (Should we
  // also register it?)  The user program is responsible for calling
  // Delete.  It is not advisable to use a parallel render manager with a
  // renderer that was not built with this method.
  virtual vtkRenderer *MakeRenderer();

  // Description:
  // Set/Get the RenderWindow to use for compositing.
  // We add a start and end observer to the window.
  vtkGetObjectMacro(RenderWindow, vtkRenderWindow);
  virtual void SetRenderWindow(vtkRenderWindow *renWin);

  // Description:
  // Set/Get the vtkMultiProcessController which will handle communications
  // for the parallel rendering.
  vtkGetObjectMacro(Controller, vtkMultiProcessController);
  virtual void SetController(vtkMultiProcessController *controller);

  // Description:
  // This method sets the piece and number of pieces for each
  // actor with a polydata mapper.
  virtual void InitializePieces();

  // Description:
  // Make all rendering windows not viewable set as off screen rendering.
  // To make all renderwindows on screen rendering again, call
  // OffScreenRenderingOff on all the render windows.  This class assumes
  // the window on root node is the only one viewable.  Subclasses should
  // change this as necessary.
  virtual void InitializeOffScreen();

  // Description:
  // Initializes the RMIs and then, if on root node, starts the interactor
  // on the attached render window.  Otherwise, starts processing RMIs.
  // When the interactor returns, it breaks the RMI listening on all other
  // processors.
  virtual void StartInteractor();

  // Description:
  // If on node other than root, starts serving RMI requests for parallel
  // renders.
  virtual void StartServices();

  // Description:
  // If on root node, stops the RMI processing on all service nodes.
  virtual void StopServices();

  // Description:
  // Callbacks that initialize and finish rendering and other tasks.
  virtual void StartRender();
  virtual void EndRender();
  virtual void SatelliteStartRender();
  virtual void SatelliteEndRender();
  virtual void RenderRMI();
  virtual void ResetCamera(vtkRenderer *ren);
  virtual void ResetCameraClippingRange(vtkRenderer *ren);
  virtual void ComputeVisiblePropBoundsRMI();

  virtual void InitializeRMIs();

  // Description:
  // Resets the camera of each renderer contained in the RenderWindow.
  // Should only be called in the "root" process, and all remote processes
  // must be processing RMIs for this method to complete.
  virtual void ResetAllCameras();

  // Description:
  // Calculates the bounds by gathering information from all processes.
  virtual void ComputeVisiblePropBounds(vtkRenderer *ren, double bounds[6]);

  // Description:
  // Turns on/off parallel rendering.  When on (the default) the object
  // responds to render events of the attached window, propagates the
  // render event to other processors, and otherwise enables the parallel
  // rendering process.
  vtkSetMacro(ParallelRendering, int);
  vtkGetMacro(ParallelRendering, int);
  vtkBooleanMacro(ParallelRendering, int);

  // Description:
  // Turns on/off render event propagation.  When on (the default) and
  // ParallelRendering is on, process 0 will send an RMI call to all remote
  // processes to perform a synchronized render.  When off, render must be
  // manually called on each process.
  vtkSetMacro(RenderEventPropagation, int);
  vtkGetMacro(RenderEventPropagation, int);
  vtkBooleanMacro(RenderEventPropagation, int);

  // Description:
  // This is used for tiled display rendering.  When data has been
  // duplicated on all processes, then we do not need to compositing.
  // Cameras and renders are still propagated though.
  vtkSetMacro(UseCompositing, int);
  vtkGetMacro(UseCompositing, int);
  vtkBooleanMacro(UseCompositing, int);

  // Description:
  // Set/Get the reduction factor (for sort-last based parallel renderers).
  // The size of rendered image is divided by the reduction factor and then
  // is blown up to the size of the current vtkRenderWindow.  Setting
  // higher reduction factors enables shorter image transfer times (which
  // is often the bottleneck) but will greatly reduce image quality.  A
  // reduction factor of 2 or greater should only be used for intermediate
  // images in interactive applications.  A reduction factor of 1 (or less)
  // will result in no change in image quality.  A parallel render manager
  // may ignore the image reduction factor if it will result in little or
  // no performance enhancements (eg. it does not do image space
  // manipulations).
  virtual void SetImageReductionFactor(double factor);
  vtkGetMacro(ImageReductionFactor, double);

  vtkSetMacro(MaxImageReductionFactor, double);
  vtkGetMacro(MaxImageReductionFactor, double);

  // Description:
  // Sets the ReductionFactor based on the given desired update rate and
  // the rendering metrics taken from the last time UpdateServerInfo was
  // called.  Note that if AutoReductionFactor is on, this function is called
  // with the desired update rate of the render window automatically.
  virtual void SetImageReductionFactorForUpdateRate(double DesiredUpdateRate);

  // Description:
  // If on, the ReductionFactor is automatically adjusted to best meet the
  // the DesiredUpdateRate in the current RenderWindow based on metrics
  // from the last render.
  vtkSetMacro(AutoImageReductionFactor, int);
  vtkGetMacro(AutoImageReductionFactor, int);
  vtkBooleanMacro(AutoImageReductionFactor, int);

  // Description:
  // Get rendering metrics.
  vtkGetMacro(RenderTime, double);
  vtkGetMacro(ImageProcessingTime, double);

  // Description:
  // If on (the default), the result of any image space manipulations are
  // written back to the render window frame buffer.  If off, the image
  // stored in the frame buffer may not be correct.  Either way, the
  // correct frame buffer images may be read with
  // vtkParallelRenderManager::GetPixelData.  Turning WriteBackImages off
  // may result in a speedup if the render window is not visible to the user
  // and images are read back for further processing or transit.
  vtkSetMacro(WriteBackImages, int);
  vtkGetMacro(WriteBackImages, int);
  vtkBooleanMacro(WriteBackImages, int);

  // Description:
  // If on (the default), when the ImageReductionFactor is greater than 1
  // and WriteBackImages is on, the image will be magnified to fill the
  // entire render window.
  vtkSetMacro(MagnifyImages, int);
  vtkGetMacro(MagnifyImages, int);
  vtkBooleanMacro(MagnifyImages, int);

//BTX
  enum { NEAREST, LINEAR };
//ETX

  // Description:
  // Sets the method used to magnify images.  Nearest simply replicates
  // each pixel enough times to fill the image.  Linear performs linear
  // interpolation between the pixels.
  virtual void SetMagnifyImageMethod(int method);
  vtkGetMacro(MagnifyImageMethod, int);
  void SetMagnifyImageMethodToNearest() {
    this->SetMagnifyImageMethod(NEAREST);
  }
  void SetMagnifyImageMethodToLinear() {
    this->SetMagnifyImageMethod(LINEAR);
  }

  // Description:
  // The most appropriate way to retrieve full size image data after a
  // render.  Will work regardless of whether WriteBackImages or
  // MagnifyImage is on or off.  The data returned may be a shallow copy of
  // an internal array.  Therefore, the data may be invalid after the next
  // render or if the ParallelRenderManager is destroyed.
  virtual void GetPixelData(vtkUnsignedCharArray *data);
  virtual void GetPixelData(int x1, int y1, int x2, int y2,
          vtkUnsignedCharArray *data);

  // Description:
  // The most appropriate way to retrieve reduced size image data after a
  // render.  Will work regardless of whether WriteBackImages or
  // MagnifyImage is on or off.  The data returned may be a shallow copy of
  // an internal array.  Therefore, the data may be invalid after the next
  // render or if the ParallelRenderManager is destroyed.
  virtual void GetReducedPixelData(vtkUnsignedCharArray *data);
  virtual void GetReducedPixelData(int x1, int y1, int x2, int y2,
           vtkUnsignedCharArray *data);

  // Description:
  // Returns the full image size calculated at the last render.
  vtkGetVector2Macro(FullImageSize, int);
  // Description:
  // Returns the reduced image size calculated at the last render.
  vtkGetVector2Macro(ReducedImageSize, int);

  // Description:
  // Given the x and y size of the render windows, reposition them
  // in a tile of n columns.
  void TileWindows(int xsize, int ysize, int nColumns);

  // Description:
  // Get/Set if all Images must use RGBA instead of RGB. By default,
  // this flag is on.
  vtkSetMacro(UseRGBA, int);
  vtkGetMacro(UseRGBA, int);

//BTX
  enum Tags {
    RENDER_RMI_TAG=34532,
    COMPUTE_VISIBLE_PROP_BOUNDS_RMI_TAG=54636,
    WIN_INFO_INT_TAG=87834,
    WIN_INFO_DOUBLE_TAG=87835,
    REN_INFO_INT_TAG=87836,
    REN_INFO_DOUBLE_TAG=87837,
    LIGHT_INFO_DOUBLE_TAG=87838,
    REN_ID_TAG=58794,
    BOUNDS_TAG=23543
  };

  virtual void CheckForAbortRender() {}
  virtual int CheckForAbortComposite() {return 0;}  
//ETX

// Disable warnings about qualifiers on return types.
#if defined(_COMPILER_VERSION)
# pragma set woff 3303
#endif
#if defined(__INTEL_COMPILER)
# pragma warning (push)
# pragma warning (disable:858)
#endif

#ifdef VTK_WORKAROUND_WINDOWS_MANGLE
# define StartServiceA StartService
# define StartServiceW StartService
#endif

  // Description:
  // @deprecated Replaced by vtkParallelRenderManager::StartServices()
  // as of VTK 5.0.
  VTK_LEGACY(virtual void const StartService());

#ifdef VTK_WORKAROUND_WINDOWS_MANGLE
# undef StartServiceW
# undef StartServiceA
  //BTX
  VTK_LEGACY(virtual void const StartServiceA());
  VTK_LEGACY(virtual void const StartServiceW());
  //ETX
#endif

// Reset disabled warning about qualifiers on return types.
#if defined(__INTEL_COMPILER)
# pragma warning (pop)
#endif
#if defined(_COMPILER_VERSION)
# pragma reset woff 3303
#endif

protected:
  vtkParallelRenderManager();
  ~vtkParallelRenderManager();

  vtkRenderWindow *RenderWindow;
  vtkMultiProcessController *Controller;

  // Description:
  // The "root" node's process id.  This is the node which is listening for
  // and propagating new render events from the RenderWindow.  All
  // processes on the controller must have the same value.  This value must
  // be set before SetRenderWindow method is called.  In the constructor or
  // the SetController methods are good places.  By default this is set to
  // 0.
  int RootProcessId;

  int ObservingRenderWindow;
  int ObservingRenderer;
  int ObservingAbort;

  unsigned long StartRenderTag;
  unsigned long EndRenderTag;
  unsigned long ResetCameraTag;
  unsigned long ResetCameraClippingRangeTag;
  unsigned long AbortRenderCheckTag;

  double ImageReductionFactor;
  double MaxImageReductionFactor;
  int AutoImageReductionFactor;

  int WriteBackImages;
  int MagnifyImages;
  int MagnifyImageMethod;

  int UseRGBA;

  int FullImageSize[2];
  int ReducedImageSize[2];

  vtkUnsignedCharArray *FullImage;
  vtkUnsignedCharArray *ReducedImage;

  int FullImageUpToDate;
  int ReducedImageUpToDate;
  int RenderWindowImageUpToDate;

  vtkDoubleArray *Viewports;

  int Lock;
  int ParallelRendering;
  int RenderEventPropagation;
  int UseCompositing;

  vtkTimerLog *Timer;

  double RenderTime;
  double ImageProcessingTime;

  // Description:
  // Used by SetImageReductionFactorForUpdateRate to smooth transitions
  // transitions between image reduction factors.
  double AverageTimePerPixel;

  // Description:
  // Used to synchronize rendering information per frame.
  virtual void SendWindowInformation() {}
  virtual void ReceiveWindowInformation() {}
  virtual void SendRendererInformation(vtkRenderer *) {};
  virtual void ReceiveRendererInformation(vtkRenderer *) {};

  // Description:
  // Here is a good place to handle processing of data before and after
  // render.
  virtual void PreRenderProcessing() = 0;
  virtual void PostRenderProcessing() = 0;

  // Description:
  // Called in satellites to set the render window size to the current
  // FullImageSize and ReducedImageSize (or vice versa)
  virtual void SetRenderWindowSize();

  // Description:
  // Called by ComputeVisiblePropBoundsRMI to get the bounds of a local
  // renderer.  Override this method if the true bounds are different than
  // those reported by the renderer.
  virtual void LocalComputeVisiblePropBounds(vtkRenderer *ren, double bounds[6]);

  // Description:
  // When called, fills FullImage.
  virtual void MagnifyReducedImage();

  // Description:
  // Write the full image back to the RenderWindow.
  virtual void WriteFullImage();

  // Description:
  // Reads in the reduced image from the RenderWindow.
  virtual void ReadReducedImage();

  // Description:
  // Returns 1 if the RenderWindow's last image is in the front buffer, 0
  // if it is in the back.
  virtual int LastRenderInFrontBuffer();

  // Description:
  // Select buffer to read from / render into.
  virtual int ChooseBuffer();
  
  // Description:
  // Sets the current render window's pixel data.
  virtual void SetRenderWindowPixelData(vtkUnsignedCharArray *pixels,
          const int pixelDimensions[2]);

//BTX
  struct RenderWindowInfoInt
  {
    int FullSize[2];
    int ReducedSize[2];
    int NumberOfRenderers;
    int UseCompositing;
  };

  struct RenderWindowInfoDouble
  {
    double ImageReductionFactor;
    double DesiredUpdateRate;
  };
  
  struct RendererInfoInt
  {
    int NumberOfLights;
  };

  struct RendererInfoDouble
  {
    double Viewport[4];
    double CameraPosition[3];
    double CameraFocalPoint[3];
    double CameraViewUp[3];
    double WindowCenter[2];
    double CameraClippingRange[2];
    double CameraViewAngle;
    double Background[3];
    double ParallelScale;
  };
  
  struct LightInfoDouble
  {
    double Position[3];
    double FocalPoint[3];
    double Type;
  };

  static const int WIN_INFO_INT_SIZE;
  static const int WIN_INFO_DOUBLE_SIZE;
  static const int REN_INFO_INT_SIZE;
  static const int REN_INFO_DOUBLE_SIZE;
  static const int LIGHT_INFO_DOUBLE_SIZE;
//ETX

  int AddedRMIs;
private:
  vtkParallelRenderManager(const vtkParallelRenderManager &); //Not implemented
  void operator=(const vtkParallelRenderManager &);  //Not implemented
};

#endif //__vtkParalleRenderManager_h
