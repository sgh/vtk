/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRayCastImageDisplayHelper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkRayCastImageDisplayHelper.h"
#include "vtkVolumeRenderingFactory.h"

vtkCxxRevisionMacro(vtkRayCastImageDisplayHelper, "1.1");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkRayCastImageDisplayHelper);
//----------------------------------------------------------------------------

vtkRayCastImageDisplayHelper* vtkRayCastImageDisplayHelper::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret=vtkVolumeRenderingFactory::CreateInstance("vtkRayCastImageDisplayHelper");
  return (vtkRayCastImageDisplayHelper*)ret;
}


// Construct a new vtkRayCastImageDisplayHelper with default values
vtkRayCastImageDisplayHelper::vtkRayCastImageDisplayHelper()
{
}

// Destruct a vtkRayCastImageDisplayHelper - clean up any memory used
vtkRayCastImageDisplayHelper::~vtkRayCastImageDisplayHelper()
{
}

void vtkRayCastImageDisplayHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}