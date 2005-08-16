/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMPEG2Writer.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMPEG2Writer - Writes MPEG2 Movie files.
// .SECTION Description
// vtkMPEG2Writer writes Movie files. The data type
// of the file is unsigned char regardless of the input type.
// .SECTION See Also
// vtkGenericMovieWriter vtkAVIWriter

#ifndef __vtkMPEG2Writer_h
#define __vtkMPEG2Writer_h

#include "vtkGenericMovieWriter.h"

class vtkMPEG2WriterHelper;

class VTK_IO_EXPORT vtkMPEG2Writer : public vtkGenericMovieWriter
{
public:
  static vtkMPEG2Writer *New();
  vtkTypeRevisionMacro(vtkMPEG2Writer,vtkGenericMovieWriter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // These methods start writing an Movie file, write a frame to the file
  // and then end the writing process.
  void Start();
  void Write();
  void End();
  
  // Description:
  // Set/Get the input object from the image pipeline.
  virtual void SetInput(vtkImageData *input);
  virtual vtkImageData *GetInput();

protected:
  vtkMPEG2Writer();
  ~vtkMPEG2Writer();

  vtkMPEG2WriterHelper *MPEG2WriterHelper;

private:
  vtkMPEG2Writer(const vtkMPEG2Writer&); // Not implemented
  void operator=(const vtkMPEG2Writer&); // Not implemented
};

#endif


