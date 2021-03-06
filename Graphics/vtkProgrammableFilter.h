/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProgrammableFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkProgrammableFilter - a user-programmable filter
// .SECTION Description
// vtkProgrammableFilter is a filter that can be programmed by the user.  To
// use the filter you define a function that retrieves input of the correct
// type, creates data, and then manipulates the output of the filter.  Using
// this filter avoids the need for subclassing - and the function can be
// defined in an interpreter wrapper language such as Tcl or Java.
//
// The trickiest part of using this filter is that the input and output
// methods are unusual and cannot be compile-time type checked. Instead, as a
// user of this filter it is your responsibility to set and get the correct
// input and output types.

// .SECTION Caveats
// The filter correctly manages modified time and network execution in most
// cases. However, if you change the definition of the filter function,
// you'll want to send a manual Modified() method to the filter to force it
// to reexecute.

// .SECTION See Also
// vtkProgrammablePointDataFilter vtkProgrammableSource

#ifndef __vtkProgrammableFilter_h
#define __vtkProgrammableFilter_h

#include "vtkDataSetAlgorithm.h"

class VTK_GRAPHICS_EXPORT vtkProgrammableFilter : public vtkDataSetAlgorithm
{
public:
  static vtkProgrammableFilter *New();
  vtkTypeRevisionMacro(vtkProgrammableFilter,vtkDataSetAlgorithm);

  // Description:
  // Specify the function to use to operate on the point attribute data. Note
  // that the function takes a single (void *) argument.
  void SetExecuteMethod(void (*f)(void *), void *arg);

  // Description:
  // Set the arg delete method. This is used to free user memory.
  void SetExecuteMethodArgDelete(void (*f)(void *));

  // Description:
  // Get the input as a concrete type. This method is typically used by the
  // writer of the filter function to get the input as a particular type (i.e.,
  // it essentially does type casting). It is the users responsibility to know
  // the correct type of the input data.
  vtkPolyData *GetPolyDataInput();

  // Description:
  // Get the input as a concrete type.
  vtkStructuredPoints *GetStructuredPointsInput();

  // Description:
  // Get the input as a concrete type.
  vtkStructuredGrid *GetStructuredGridInput();

  // Description:
  // Get the input as a concrete type.
  vtkUnstructuredGrid *GetUnstructuredGridInput();

  // Description:
  // Get the input as a concrete type.
  vtkRectilinearGrid *GetRectilinearGridInput();

protected:
  vtkProgrammableFilter();
  ~vtkProgrammableFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  void (*ExecuteMethod)(void *); //function to invoke
  void (*ExecuteMethodArgDelete)(void *);
  void *ExecuteMethodArg;
  
private:
  vtkProgrammableFilter(const vtkProgrammableFilter&);  // Not implemented.
  void operator=(const vtkProgrammableFilter&);  // Not implemented.
};

#endif

