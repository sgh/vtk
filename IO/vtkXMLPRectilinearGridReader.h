/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXMLPRectilinearGridReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkXMLPRectilinearGridReader - Read PVTK XML RectilinearGrid files.
// .SECTION Description
// vtkXMLPRectilinearGridReader reads the PVTK XML RectilinearGrid
// file format.  This reads the parallel format's summary file and
// then uses vtkXMLRectilinearGridReader to read data from the
// individual RectilinearGrid piece files.  Streaming is supported.
// The standard extension for this reader's file format is "pvtr".

// .SECTION See Also
// vtkXMLRectilinearGridReader

#ifndef __vtkXMLPRectilinearGridReader_h
#define __vtkXMLPRectilinearGridReader_h

#include "vtkXMLPStructuredDataReader.h"

class vtkRectilinearGrid;

class VTK_IO_EXPORT vtkXMLPRectilinearGridReader : public vtkXMLPStructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkXMLPRectilinearGridReader,vtkXMLPStructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkXMLPRectilinearGridReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkRectilinearGrid *output);
  vtkRectilinearGrid *GetOutput();
  vtkRectilinearGrid *GetOutput(int idx);
  
protected:
  vtkXMLPRectilinearGridReader();
  ~vtkXMLPRectilinearGridReader();
  
  vtkRectilinearGrid* GetPieceInput(int index);
  
  const char* GetDataSetName();
  void SetOutputExtent(int* extent);
  void GetPieceInputExtent(int index, int* extent);
  int ReadPrimaryElement(vtkXMLDataElement* ePrimary);
  void SetupOutputData();
  int ReadPieceData();
  vtkXMLDataReader* CreatePieceReader();
  void CopySubCoordinates(int* inBounds, int* outBounds, int* subBounds,
                          vtkDataArray* inArray, vtkDataArray* outArray);
  virtual int FillOutputPortInformation(int, vtkInformation*);

  // The PCoordinates element with coordinate information.
  vtkXMLDataElement* PCoordinatesElement;

private:
  vtkXMLPRectilinearGridReader(const vtkXMLPRectilinearGridReader&);  // Not implemented.
  void operator=(const vtkXMLPRectilinearGridReader&);  // Not implemented.
};

#endif
