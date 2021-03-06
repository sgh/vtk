/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageIterator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageIterator - a simple image iterator
// .SECTION Description
// This is a simple image iterator that can be used to iterate over an 
// image. This should be used internally by Filter writers.

// .SECTION See also
// vtkImageData vtkImageProgressIterator

#ifndef __vtkImageIterator_h
#define __vtkImageIterator_h

#include "vtkSystemIncludes.h"
class vtkImageData;

template<class DType>
class vtkImageIterator 
{
public:        
  typedef DType *SpanIterator;
  
  // Description:
  // Create an image iterator fora given image data and a given extent
  vtkImageIterator(vtkImageData *id, int *ext);

  // Description:
  // Move the iterator to the next span
  void NextSpan();

  // Description:
  // Return an iterator (pointer) for the span
  SpanIterator BeginSpan()
    {
      return this->Pointer;
    }

  // Description:
  // Return an iterator (pointer) for the end of the span
  SpanIterator EndSpan()
    {
      return this->SpanEndPointer;
    }
    
  // Description:
  // tets if the end of the extent has been reached
  int IsAtEnd()
    {
    return (this->Pointer >= this->EndPointer);
    }

protected:
  DType *Pointer;
  DType *SpanEndPointer;
  DType *SliceEndPointer;
  DType *EndPointer;
  vtkIdType    Increments[3];
  vtkIdType    ContinuousIncrements[3];
};

#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkImageIterator.txx"
#endif 

#endif
