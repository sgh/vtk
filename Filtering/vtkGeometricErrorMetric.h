/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGeometricErrorMetric.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGeometricErrorMetric - Objects that compute
// geometry-based error during cell tessellation.
// .SECTION Description
// It is a concrete error metric, based on a geometric criterium:
// the variation of the edge from a straight line
//
// See Also
// vtkGenericCellTessellator, vtkGenericSubdivisionErrorMetric

#ifndef __vtkGeometricErrorMetric_h
#define __vtkGeometricErrorMetric_h

#include "vtkGenericSubdivisionErrorMetric.h"

class vtkGenericAttributeCollection;
class vtkGenericAdaptorCell;
class vtkGenericDataSet;

class VTK_FILTERING_EXPORT vtkGeometricErrorMetric : public vtkGenericSubdivisionErrorMetric
{
public:
  // Description:
  // Construct the tessellator.
  static vtkGeometricErrorMetric *New();
  
  // Description:
  // Standard VTK type and error macros.
  vtkTypeRevisionMacro(vtkGeometricErrorMetric,vtkGenericSubdivisionErrorMetric);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Specify the number of component to interpolate
  // This is for internal use only
  vtkGetMacro(GeometricTolerance, double);
 
  // Description:
  // Set the geometric accuracy with an absolute value.
  // This is the geometric object-based accuracy.
  // Subdivision will be required if the square distance between the real
  // point and the interpolated point is greater than `value'. For instance
  // 0.01 will give better result than 0.1.
  // \pre positive_value: value>0
  void SetAbsoluteGeometricTolerance(double value);
  
  // Description:
  // Set the geometric accuracy with a value relative to the length of the
  // bounding box of the dataset. Internally compute the absolute tolerance.
  // For instance 0.01 will give better result than 0.1.
  // \pre valid_range_value: value>0 && value<1
  // \pre ds_exists: ds!=0
  void SetRelativeGeometricTolerance(double value,
                                     vtkGenericDataSet *ds);
  
  // Description:
  // Does the edge need to be subdivided according to the distance between
  // the line passing through its endpoints and the mid point?
  // The edge is defined by its `leftPoint' and its `rightPoint'.
  // `leftPoint', `midPoint' and `rightPoint' have to be initialized before
  // calling NeedEdgeSubdivision().
  // Their format is global coordinates, parametric coordinates and
  // point centered attributes: xyx rst abc de...
  // \pre leftPoint_exists: leftPoint!=0
  // \pre midPoint_exists: midPoint!=0
  // \pre rightPoint_exists: rightPoint!=0
  // \pre valid_size: sizeof(leftPoint)=sizeof(midPoint)=sizeof(rightPoint)
  //          =GetAttributeCollection()->GetNumberOfPointCenteredComponents()+6
  int NeedEdgeSubdivision(double *leftPoint,
                          double *midPoint,
                          double *rightPoint);

protected:
  vtkGeometricErrorMetric();
  virtual ~vtkGeometricErrorMetric();
  
  // Description:
  // Square distance between a straight line (defined by points x and y)
  // and a point z. Property: if x and y are equal, the line is a point and
  // the result is the square distance between points x and z.
  double Distance2LinePoint(double x[3],
                            double y[3],
                            double z[3]);
  
  double GeometricTolerance;
  
private:
  vtkGeometricErrorMetric(const vtkGeometricErrorMetric&);  // Not implemented.
  void operator=(const vtkGeometricErrorMetric&);  // Not implemented.
};

#endif