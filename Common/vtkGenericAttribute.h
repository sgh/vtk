/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGenericAttribute.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGenericAttribute - abstract class defined API for attribute data
// .SECTION Description
// vtkGenericAttribute is an abstract class that defines an API for attribute
// data. Attribute data is data associated with the topology or geometry of
// a dataset (i.e., points, cells, etc.). vtkGenericAttribute is part of the
// adaptor framework (see GenericFiltering/README.html).
//
// vtkGenericAttribute provides a more general interface to attribute data
// than its counterpart vtkDataArray (which assumes a linear, contiguous
// array). It adopts an iterator interface, and allows attributes to be
// associated with points, edges, faces, or edges.

#ifndef __vtkGenericAttribute_h
#define __vtkGenericAttribute_h

#include "vtkObject.h"

class vtkGenericCellIterator;
class vtkGenericPointIterator;

enum
{
  vtkPointCentered,
  vtkCellCentered,
  vtkBoundaryCentered
};

class VTK_COMMON_EXPORT vtkGenericAttribute : public vtkObject
{
 public:
  vtkTypeRevisionMacro(vtkGenericAttribute,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the attribute. (e.g. "velocity")
  // \post result_may_not_exist: result!=0 || result==0
  virtual const char *GetName() = 0;

  // Description:
  // Dimension of the attribute. (1 for scalar, 3 for velocity)
  // \post positive_result: result>=0
  virtual int GetNumberOfComponents() = 0;

  // Description:
  // Is the attribute centered either on points, cells or boundaries?
  // \post valid_result: (result==vtkPointCentered) ||
  //            (result==vtkCellCentered) || (result==vtkBoundaryCentered)
  virtual int GetCentering() = 0;
  
  // Description:
  // Type of the attribute: int, float, double
  // \post valid_result: (result==VTK_INT)||(result==VTK_FLOAT)
  virtual int GetType() = 0;

  // Description:
  // Number of tuples.
  // \post valid_result: result>=0
  virtual vtkIdType GetSize() = 0;

  // Description:
  // Size in kilobytes taken by the attribute.
  virtual unsigned long GetActualMemorySize() = 0;

  // Description:
  // Range of the attribute component `component'. It returns double, even if
  // GetType()==VTK_INT.
  // NOT THREAD SAFE
  // \pre valid_component: (component>=0)&&(component<GetNumberOfComponents())
  // \post result_exists: result!=0
  virtual double *GetRange(int component=0) = 0;
  
  // Description:
  // Range of the attribute component `component'.
  // THREAD SAFE
  // \pre valid_component: (component>=0)&&(component<GetNumberOfComponents())
  virtual void GetRange(int component,
                        double range[2]) = 0;
  
  // Description:
  // Euclidean Norm.
  // \post positive_result: result>=0
  virtual double GetNorm()=0;
  
  // Description:
  // Attribute at all points of cell `c'.
  // \pre c_exists: c!=0
  // \pre c_valid: !c->IsAtEnd()
  // \post result_exists: result!=0
  // \post valid_result: sizeof(result)==GetNumberOfComponents()*c->GetCell()->GetNumberOfPoints()
  virtual double *GetTuple(vtkGenericCellIterator *c) = 0;

  // Description:
  // Put attribute at all points of cell `c' in `tuple'.
  // \pre c_exists: c!=0
  // \pre c_valid: !c->IsAtEnd()
  // \pre tuple_exists: tuple!=0
  // \pre valid_tuple: sizeof(tuple)>=GetNumberOfComponents()*c->GetCell()->GetNumberOfPoints()
  virtual void GetTuple(vtkGenericCellIterator *c, double *tuple) = 0;

  // Description:
  // Value of the attribute at position `p'.
  // \pre p_exists: p!=0
  // \pre p_valid: !p->IsAtEnd()
  // \post result_exists: result!=0
  // \post valid_result_size: sizeof(result)==GetNumberOfComponents()
  virtual double *GetTuple(vtkGenericPointIterator *p) = 0;
  
  // Description:
  // Put the value of the attribute at position `p' into `tuple'.
  // \pre p_exists: p!=0
  // \pre p_valid: !p->IsAtEnd()
  // \pre tuple_exists: tuple!=0
  // \pre valid_tuple_size: sizeof(tuple)>=GetNumberOfComponents()
  virtual void GetTuple(vtkGenericPointIterator *p, double *tuple) = 0;
  
  // Description:
  // Put component `i' of the attribute at all points of cell `c' in `values'.
  // \pre valid_component: (i>=0) && (i<GetNumberOfComponents())
  // \pre c_exists: c!=0
  // \pre c_valid: !c->IsAtEnd()
  // \pre values_exist: values!=0
  // \pre valid_values: sizeof(values)>=c->GetCell()->GetNumberOfPoints()
  virtual void GetComponent(int i,vtkGenericCellIterator *c, double *values) = 0;

  // Description:
  // Value of the component `i' of the attribute at position `p'.
  // \pre valid_component: (i>=0) && (i<GetNumberOfComponents())
  // \pre p_exists: p!=0
  // \pre p_valid: !p->IsAtEnd()
  virtual double GetComponent(int i,vtkGenericPointIterator *p) = 0;
  
  // Description:
  // Recursive duplication of `other' in `this'.
  // \pre other_exists: other!=0
  // \pre not_self: other!=this
  virtual void DeepCopy(vtkGenericAttribute *other) = 0;
  
  // Description:
  // Update `this' using fields of `other'.
  // \pre other_exists: other!=0
  // \pre not_self: other!=this
  virtual void ShallowCopy(vtkGenericAttribute *other) = 0;
  
private:
  vtkGenericAttribute(const vtkGenericAttribute&);  // Not implemented.
  void operator=(const vtkGenericAttribute&);  // Not implemented.
};

#endif