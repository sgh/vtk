/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVectorText.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVectorText - create polygonal text
// .SECTION Description

// vtkVectorText generates vtkPolyData from an input text string. Besides the
// ASCII alphanumeric characters a-z, A-Z, 0-9, vtkVectorText also supports
// ASCII punctuation marks. (The supported ASCII character set are the codes
// (33-126) inclusive.) The only control character supported is the line feed
// character "\n", which advances to a new line.
//
// To use thie class, you normally couple it with a vtkPolyDataMapper and a
// vtkActor. In this case you would use the vtkActor's transformation methods
// to position, orient, and scale the text. You may also wish to use a
// vtkFollower to orient the text so that it always faces the camera.

// .SECTION See Also
// vtkTextMapper vtkCaptionActor2D

#ifndef __vtkVectorText_h
#define __vtkVectorText_h

#include "vtkPolyDataAlgorithm.h"

class VTK_HYBRID_EXPORT vtkVectorText : public vtkPolyDataAlgorithm 
{
public:
  static vtkVectorText *New();
  vtkTypeRevisionMacro(vtkVectorText,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the text to be drawn.
  vtkSetStringMacro(Text);
  vtkGetStringMacro(Text);

protected:
  vtkVectorText();
  ~vtkVectorText();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  char *Text;
  char *Letters[127];

private:
  vtkVectorText(const vtkVectorText&);  // Not implemented.
  void operator=(const vtkVectorText&);  // Not implemented.
};

#endif
