/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInformationUnsignedLongKey.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkInformationUnsignedLongKey - Key for unsigned long values in vtkInformation.
// .SECTION Description
// vtkInformationUnsignedLongKey is used to represent keys for unsigned long values
// in vtkInformation.

#ifndef __vtkInformationUnsignedLongKey_h
#define __vtkInformationUnsignedLongKey_h

#include "vtkInformationKey.h"

class VTK_FILTERING_EXPORT vtkInformationUnsignedLongKey : public vtkInformationKey
{
public:
  vtkTypeRevisionMacro(vtkInformationUnsignedLongKey,vtkInformationKey);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkInformationUnsignedLongKey(const char* name, const char* location);
  ~vtkInformationUnsignedLongKey();

  // Description:
  // Get/Set the value associated with this key in the given
  // information object.
  void Set(vtkInformation* info, unsigned long);
  unsigned long Get(vtkInformation* info);
  int Has(vtkInformation* info);

  // Description:
  // Copy the entry associated with this key from one information
  // object to another.  If there is no entry in the first information
  // object for this key, the value is removed from the second.
  virtual void Copy(vtkInformation* from, vtkInformation* to);

private:
  vtkInformationUnsignedLongKey(const vtkInformationUnsignedLongKey&);  // Not implemented.
  void operator=(const vtkInformationUnsignedLongKey&);  // Not implemented.
};

#endif