/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageData.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageData.h"

#include "vtkBitArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkExtentTranslator.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkIntArray.h"
#include "vtkLargeInteger.h"
#include "vtkLine.h"
#include "vtkLongArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPixel.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkShortArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkVertex.h"
#include "vtkVoxel.h"

vtkCxxRevisionMacro(vtkImageData, "1.1");
vtkStandardNewMacro(vtkImageData);

//----------------------------------------------------------------------------
vtkImageData::vtkImageData()
{
  int idx;
  
  this->Vertex = vtkVertex::New();
  this->Line = vtkLine::New();
  this->Pixel = vtkPixel::New();
  this->Voxel = vtkVoxel::New();
  
  this->DataDescription = VTK_EMPTY;
  
  for (idx = 0; idx < 3; ++idx)
    {
    this->Dimensions[idx] = 0;
    this->Increments[idx] = 0;
    this->Origin[idx] = 0.0;
    this->Spacing[idx] = 1.0;
    }
  int extent[6] = {0, -1, 0, -1, 0, -1};
  this->Information->Set(vtkDataObject::DATA_EXTENT_TYPE(), VTK_3D_EXTENT);
  this->Information->Set(vtkDataObject::DATA_EXTENT(), extent, 6);

  this->NumberOfScalarComponents = 1;

  // Making the default double for structured points.
  this->ScalarType = VTK_VOID;
  this->SetScalarType(VTK_DOUBLE);

}

//----------------------------------------------------------------------------
vtkImageData::~vtkImageData()
{
  this->Vertex->Delete();
  this->Line->Delete();
  this->Pixel->Delete();
  this->Voxel->Delete();
}

//----------------------------------------------------------------------------
// Copy the geometric and topological structure of an input structured points 
// object.
void vtkImageData::CopyStructure(vtkDataSet *ds)
{
  vtkImageData *sPts=(vtkImageData *)ds;
  this->Initialize();

  int i;
  for (i=0; i<3; i++)
    {
    this->Dimensions[i] = sPts->Dimensions[i];
    this->Spacing[i] = sPts->Spacing[i];
    this->Origin[i] = sPts->Origin[i];
    }
  this->Information->Set(vtkDataObject::DATA_EXTENT(),
                         sPts->Information->Get(vtkDataObject::DATA_EXTENT()),
                         6);
  this->NumberOfScalarComponents = sPts->NumberOfScalarComponents;
  this->ScalarType = sPts->ScalarType;
  this->DataDescription = sPts->DataDescription;
  this->CopyInformation(sPts);
}

//----------------------------------------------------------------------------
void vtkImageData::Initialize()
{
  this->Superclass::Initialize();
  this->SetDimensions(0,0,0);
}

//----------------------------------------------------------------------------
// Graphics filters reallocate every execute.  Image filters try to reuse
// the scalars.
void vtkImageData::PrepareForNewData()
{
  // free everything but the scalars
  vtkDataArray *scalars = this->GetPointData()->GetScalars();
  if (scalars)
    {
    scalars->Register(this);
    }
  this->Initialize();
  if (scalars)
    {
    this->GetPointData()->SetScalars(scalars);
    scalars->UnRegister(this);
    }
}


//----------------------------------------------------------------------------

// The input data object must be of type vtkImageData or a subclass!
void vtkImageData::CopyTypeSpecificInformation( vtkDataObject *data )
{
  vtkImageData *image = (vtkImageData *)data;

  // Copy the generic stuff
  this->CopyInformation( data );
  
  // Now do the specific stuff
  this->SetOrigin( image->GetOrigin() );
  this->SetSpacing( image->GetSpacing() );
  this->SetScalarType( image->GetScalarType() );
  this->SetNumberOfScalarComponents( image->GetNumberOfScalarComponents() );
}

//----------------------------------------------------------------------------

unsigned long vtkImageData::GetEstimatedMemorySize()
{
  vtkLargeInteger size; 
  int             idx;
  int             *uExt; 
  unsigned long   lsize;

  // Start with the number of scalar components
  size = (unsigned long)(this->GetNumberOfScalarComponents());

  // Multiply by the number of bytes per scalar
  switch (this->GetScalarType())
    {
    case VTK_DOUBLE:
      size *= sizeof(double);
      break;
    case VTK_FLOAT:
      size *= sizeof(float);
      break;
    case VTK_INT:
      size *= sizeof(int);
      break;
    case VTK_UNSIGNED_INT:
      size *= sizeof(unsigned int);
      break;
    case VTK_LONG:
      size *= sizeof(long);
      break;
    case VTK_UNSIGNED_LONG:
      size *= sizeof(unsigned long);
      break;
    case VTK_SHORT:
      size *= sizeof(short);
      break;
    case VTK_UNSIGNED_SHORT:
      size *= sizeof(unsigned short);
      break;
    case VTK_UNSIGNED_CHAR:
      size *= sizeof(unsigned char);
      break;
    case VTK_CHAR:
      size *= sizeof(char);
      break;
    case VTK_BIT:
      size = size / 8;
      break;
    default:
      vtkWarningMacro(<< "GetExtentMemorySize: "
        << "Cannot determine input scalar type");
    }  

  // Multiply by the number of scalars.
  uExt = this->GetUpdateExtent();
  for (idx = 0; idx < 3; ++idx)
    {
    size = size*(uExt[idx*2+1] - uExt[idx*2] + 1);
    }

  // In case the extent is set improperly, set the size to 0
  if (size < 0)
    {
    vtkWarningMacro("Oops, size should not be negative.");
    size = 0;
    }

  // Convert from double bytes to unsigned long kilobytes
  size = size >> 10;
  lsize = size.CastToUnsignedLong();
  return lsize;
}

//----------------------------------------------------------------------------

vtkCell *vtkImageData::GetCell(vtkIdType cellId)
{
  vtkCell *cell = NULL;
  int loc[3];
  vtkIdType idx, npts;
  int iMin, iMax, jMin, jMax, kMin, kMax;
  int *dims = this->GetDimensions();
  int d01 = dims[0]*dims[1];
  double x[3];
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int extent[6];
  this->GetExtent(extent);

  iMin = iMax = jMin = jMax = kMin = kMax = 0;
  
  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
    vtkErrorMacro("Requesting a cell from an empty image.");
    return NULL;
    }
  
  switch (this->DataDescription)
    {
    case VTK_EMPTY: 
      //cell = this->EmptyCell;
      return NULL;

    case VTK_SINGLE_POINT: // cellId can only be = 0
      cell = this->Vertex;
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (dims[0]-1);
      jMax = jMin + 1;
      cell = this->Pixel;
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (dims[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (dims[1]-1);
      kMax = kMin + 1;
      cell = this->Pixel;
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (dims[0]-1);
      kMax = kMin + 1;
      cell = this->Pixel;
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (dims[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (dims[0] - 1)) % (dims[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((dims[0] - 1) * (dims[1] - 1));
      kMax = kMin + 1;
      cell = this->Voxel;
      break;
    }

  // Extract point coordinates and point ids
  // Ids are relative to extent min.
  npts = 0;
  for (loc[2]=kMin; loc[2]<=kMax; loc[2]++)
    {
    x[2] = origin[2] + (loc[2]+extent[4]) * spacing[2]; 
    for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
      {
      x[1] = origin[1] + (loc[1]+extent[2]) * spacing[1]; 
      for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
        {
        x[0] = origin[0] + (loc[0]+extent[0]) * spacing[0]; 

        idx = loc[0] + loc[1]*dims[0] + loc[2]*d01;
        cell->PointIds->SetId(npts,idx);
        cell->Points->SetPoint(npts++,x);
        }
      }
    }

  return cell;
}

//----------------------------------------------------------------------------
void vtkImageData::GetCell(vtkIdType cellId, vtkGenericCell *cell)
{
  vtkIdType npts, idx;
  int loc[3];
  int iMin, iMax, jMin, jMax, kMin, kMax;
  int *dims = this->GetDimensions();
  int d01 = dims[0]*dims[1];
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  double x[3];
  int extent[6];
  this->GetExtent(extent);

  iMin = iMax = jMin = jMax = kMin = kMax = 0;
  
  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
    vtkErrorMacro("Requesting a cell from an empty image.");
    cell->SetCellTypeToEmptyCell();
    return;
    }
  
  switch (this->DataDescription)
    {
    case VTK_EMPTY: 
      cell->SetCellTypeToEmptyCell();
      return;

    case VTK_SINGLE_POINT: // cellId can only be = 0
      cell->SetCellTypeToVertex();
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (dims[0]-1);
      jMax = jMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (dims[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (dims[1]-1);
      kMax = kMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (dims[0]-1);
      kMax = kMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (dims[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (dims[0] - 1)) % (dims[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((dims[0] - 1) * (dims[1] - 1));
      kMax = kMin + 1;
      cell->SetCellTypeToVoxel();
      break;
    }

  // Extract point coordinates and point ids
  for (npts=0,loc[2]=kMin; loc[2]<=kMax; loc[2]++)
    {
    x[2] = origin[2] + (loc[2]+extent[4]) * spacing[2]; 
    for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
      {
      x[1] = origin[1] + (loc[1]+extent[2]) * spacing[1]; 
      for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
        {
        x[0] = origin[0] + (loc[0]+extent[0]) * spacing[0]; 

        idx = loc[0] + loc[1]*dims[0] + loc[2]*d01;
        cell->PointIds->SetId(npts,idx);
        cell->Points->SetPoint(npts++,x);
        }
      }
    }
}


//----------------------------------------------------------------------------
// Fast implementation of GetCellBounds().  Bounds are calculated without
// constructing a cell.
void vtkImageData::GetCellBounds(vtkIdType cellId, double bounds[6])
{
  int loc[3], iMin, iMax, jMin, jMax, kMin, kMax;
  double x[3];
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int *dims = this->GetDimensions();
  int extent[6];
  this->GetExtent(extent);

  iMin = iMax = jMin = jMax = kMin = kMax = 0;
  
  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
    vtkErrorMacro("Requesting cell bounds from an empty image.");
    bounds[0] = bounds[1] = bounds[2] = bounds[3] 
      = bounds[4] = bounds[5] = 0.0;
    return;
    }
  
  switch (this->DataDescription)
    {
    case VTK_EMPTY:
      return;

    case VTK_SINGLE_POINT: // cellId can only be = 0
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (dims[0]-1);
      jMax = jMin + 1;
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (dims[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (dims[1]-1);
      kMax = kMin + 1;
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (dims[0]-1);
      kMax = kMin + 1;
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (dims[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (dims[0] - 1)) % (dims[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((dims[0] - 1) * (dims[1] - 1));
      kMax = kMin + 1;
      break;
    }


  // carefully compute the bounds
  if (kMax >= kMin && jMax >= jMin && iMax >= iMin)
    {
    bounds[0] = bounds[2] = bounds[4] =  VTK_DOUBLE_MAX;
    bounds[1] = bounds[3] = bounds[5] = -VTK_DOUBLE_MAX;
  
    // Extract point coordinates
    for (loc[2]=kMin; loc[2]<=kMax; loc[2]++)
      {
      x[2] = origin[2] + (loc[2]+extent[4]) * spacing[2]; 
      bounds[4] = (x[2] < bounds[4] ? x[2] : bounds[4]);
      bounds[5] = (x[2] > bounds[5] ? x[2] : bounds[5]);
      }
    for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
      {
      x[1] = origin[1] + (loc[1]+extent[2]) * spacing[1]; 
      bounds[2] = (x[1] < bounds[2] ? x[1] : bounds[2]);
      bounds[3] = (x[1] > bounds[3] ? x[1] : bounds[3]);
      }
    for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
      {
      x[0] = origin[0] + (loc[0]+extent[0]) * spacing[0]; 
      bounds[0] = (x[0] < bounds[0] ? x[0] : bounds[0]);
      bounds[1] = (x[0] > bounds[1] ? x[0] : bounds[1]);
      }
    }
  else
    {
    vtkMath::UninitializeBounds(bounds);
    }
}

//----------------------------------------------------------------------------
double *vtkImageData::GetPoint(vtkIdType ptId)
{
  static double x[3];
  int i, loc[3];
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int *dims = this->GetDimensions();
  int extent[6];
  this->GetExtent(extent);

  x[0] = x[1] = x[2] = 0.0;
  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
    vtkErrorMacro("Requesting a point from an empty image.");
    return x;
    }

  switch (this->DataDescription)
    {
    case VTK_EMPTY: 
      return x;

    case VTK_SINGLE_POINT: 
      loc[0] = loc[1] = loc[2] = 0;
      break;

    case VTK_X_LINE:
      loc[1] = loc[2] = 0;
      loc[0] = ptId;
      break;

    case VTK_Y_LINE:
      loc[0] = loc[2] = 0;
      loc[1] = ptId;
      break;

    case VTK_Z_LINE:
      loc[0] = loc[1] = 0;
      loc[2] = ptId;
      break;

    case VTK_XY_PLANE:
      loc[2] = 0;
      loc[0] = ptId % dims[0];
      loc[1] = ptId / dims[0];
      break;

    case VTK_YZ_PLANE:
      loc[0] = 0;
      loc[1] = ptId % dims[1];
      loc[2] = ptId / dims[1];
      break;

    case VTK_XZ_PLANE:
      loc[1] = 0;
      loc[0] = ptId % dims[0];
      loc[2] = ptId / dims[0];
      break;

    case VTK_XYZ_GRID:
      loc[0] = ptId % dims[0];
      loc[1] = (ptId / dims[0]) % dims[1];
      loc[2] = ptId / (dims[0]*dims[1]);
      break;
    }

  for (i=0; i<3; i++)
    {
    x[i] = origin[i] + (loc[i]+extent[i*2]) * spacing[i];
    }

  return x;
}

//----------------------------------------------------------------------------
vtkIdType vtkImageData::FindPoint(double x[3])
{
  int i, loc[3];
  double d;
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int *dims = this->GetDimensions();
  int extent[6];
  this->GetExtent(extent);

  //
  //  Compute the ijk location
  //
  for (i=0; i<3; i++) 
    {
    d = x[i] - origin[i];
    loc[i] = (int) ((d / spacing[i]) + 0.5);
    if ( loc[i] < extent[i*2] || loc[i] > extent[i*2+1] )
      {
      return -1;
      } 
    // since point id is relative to the first point actually stored
    loc[i] -= extent[i*2];
    }
  //
  //  From this location get the point id
  //
  return loc[2]*dims[0]*dims[1] + loc[1]*dims[0] + loc[0];
  
}

//----------------------------------------------------------------------------
vtkIdType vtkImageData::FindCell(double x[3], vtkCell *vtkNotUsed(cell), 
                                 vtkGenericCell *vtkNotUsed(gencell),
                                 vtkIdType vtkNotUsed(cellId), 
                                  double vtkNotUsed(tol2), 
                                  int& subId, double pcoords[3], 
                                  double *weights)
{
  return
    this->FindCell( x, (vtkCell *)NULL, 0, 0.0, subId, pcoords, weights );
}

//----------------------------------------------------------------------------
vtkIdType vtkImageData::FindCell(double x[3], vtkCell *vtkNotUsed(cell), 
                                 vtkIdType vtkNotUsed(cellId),
                                 double vtkNotUsed(tol2), 
                                 int& subId, double pcoords[3], double *weights)
{
  int loc[3];
  int *dims = this->GetDimensions();

  if ( this->ComputeStructuredCoordinates(x, loc, pcoords) == 0 )
    {
    return -1;
    }

  vtkVoxel::InterpolationFunctions(pcoords,weights);

  //
  //  From this location get the cell id
  //
  subId = 0;
  return loc[2] * (dims[0]-1)*(dims[1]-1) +
         loc[1] * (dims[0]-1) + loc[0];
}

//----------------------------------------------------------------------------
vtkCell *vtkImageData::FindAndGetCell(double x[3],
                                      vtkCell *vtkNotUsed(cell),
                                      vtkIdType vtkNotUsed(cellId),
                                      double vtkNotUsed(tol2), int& subId, 
                                      double pcoords[3], double *weights)
{
  int i, j, k, loc[3];
  vtkIdType npts, idx;
  int *dims = this->GetDimensions();
  vtkIdType d01 = dims[0]*dims[1];
  double xOut[3];
  int iMax = 0;
  int jMax = 0;
  int kMax = 0;;
  vtkCell *cell = NULL;
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int extent[6];
  this->GetExtent(extent);

  if ( this->ComputeStructuredCoordinates(x, loc, pcoords) == 0 )
    {
    return NULL;
    }

  //
  // Get the parametric coordinates and weights for interpolation
  //
  switch (this->DataDescription)
    {
    case VTK_EMPTY:
      return NULL;

    case VTK_SINGLE_POINT: // cellId can only be = 0
      vtkVertex::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1];
      kMax = loc[2];
      cell = this->Vertex;
      break;

    case VTK_X_LINE:
      vtkLine::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1];
      kMax = loc[2];
      cell = this->Line;
      break;

    case VTK_Y_LINE:
      vtkLine::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1] + 1;
      kMax = loc[2];
      cell = this->Line;
      break;

    case VTK_Z_LINE:
      vtkLine::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1];
      kMax = loc[2] + 1;
      cell = this->Line;
      break;

    case VTK_XY_PLANE:
      vtkPixel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1] + 1;
      kMax = loc[2];
      cell = this->Pixel;
      break;

    case VTK_YZ_PLANE:
      vtkPixel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1] + 1;
      kMax = loc[2] + 1;
      cell = this->Pixel;
      break;

    case VTK_XZ_PLANE:
      vtkPixel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1];
      kMax = loc[2] + 1;
      cell = this->Pixel;
      break;

    case VTK_XYZ_GRID:
      vtkVoxel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1] + 1;
      kMax = loc[2] + 1;
      cell = this->Voxel;
      break;
    }

  npts = 0;
  for (k = loc[2]; k <= kMax; k++)
    {
    xOut[2] = origin[2] + k * spacing[2]; 
    for (j = loc[1]; j <= jMax; j++)
      {
      xOut[1] = origin[1] + j * spacing[1]; 
      // make idx relative to the extent not the whole extent
      idx = loc[0]-extent[0] + (j-extent[2])*dims[0]
        + (k-extent[4])*d01;
      for (i = loc[0]; i <= iMax; i++, idx++)
        {
        xOut[0] = origin[0] + i * spacing[0]; 

        cell->PointIds->SetId(npts,idx);
        cell->Points->SetPoint(npts++,xOut);
        }
      }
    }
  subId = 0;

  return cell;
}

//----------------------------------------------------------------------------
int vtkImageData::GetCellType(vtkIdType vtkNotUsed(cellId))
{
  switch (this->DataDescription)
    {
    case VTK_EMPTY: 
      return VTK_EMPTY_CELL;

    case VTK_SINGLE_POINT: 
      return VTK_VERTEX;

    case VTK_X_LINE: case VTK_Y_LINE: case VTK_Z_LINE:
      return VTK_LINE;

    case VTK_XY_PLANE: case VTK_YZ_PLANE: case VTK_XZ_PLANE:
      return VTK_PIXEL;

    case VTK_XYZ_GRID:
      return VTK_VOXEL;

    default:
      vtkErrorMacro(<<"Bad data description!");
      return VTK_EMPTY_CELL;
    }
}

//----------------------------------------------------------------------------
void vtkImageData::ComputeBounds()
{
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int extent[6];
  this->GetExtent(extent);
  
  if ( extent[0] > extent[1] || 
       extent[2] > extent[3] ||
       extent[4] > extent[5] )
    {
    vtkMath::UninitializeBounds(this->Bounds);
    return;
    }
  this->Bounds[0] = origin[0] + (extent[0] * spacing[0]);
  this->Bounds[2] = origin[1] + (extent[2] * spacing[1]);
  this->Bounds[4] = origin[2] + (extent[4] * spacing[2]);

  this->Bounds[1] = origin[0] + (extent[1] * spacing[0]);
  this->Bounds[3] = origin[1] + (extent[3] * spacing[1]);
  this->Bounds[5] = origin[2] + (extent[5] * spacing[2]);
}

//----------------------------------------------------------------------------
// Given structured coordinates (i,j,k) for a voxel cell, compute the eight 
// gradient values for the voxel corners. The order in which the gradient
// vectors are arranged corresponds to the ordering of the voxel points. 
// Gradient vector is computed by central differences (except on edges of 
// volume where forward difference is used). The scalars s are the scalars
// from which the gradient is to be computed. This method will treat 
// only 3D structured point datasets (i.e., volumes).
void vtkImageData::GetVoxelGradient(int i, int j, int k, vtkDataArray *s, 
                                    vtkDataArray *g)
{
  double gv[3];
  int ii, jj, kk, idx=0;

  for ( kk=0; kk < 2; kk++)
    {
    for ( jj=0; jj < 2; jj++)
      {
      for ( ii=0; ii < 2; ii++)
        {
        this->GetPointGradient(i+ii, j+jj, k+kk, s, gv);
        g->SetTuple(idx++, gv);
        }
      } 
    }
}

//----------------------------------------------------------------------------
// Given structured coordinates (i,j,k) for a point in a structured point 
// dataset, compute the gradient vector from the scalar data at that point. 
// The scalars s are the scalars from which the gradient is to be computed.
// This method will treat structured point datasets of any dimension.
void vtkImageData::GetPointGradient(int i,int j,int k, vtkDataArray *s, 
                                    double g[3])
{
  int *dims=this->GetDimensions();
  double *ar=this->GetSpacing();
  vtkIdType ijsize=dims[0]*dims[1];
  double sp, sm;

  // x-direction
  if ( dims[0] == 1 )
    {
    g[0] = 0.0;
    }
  else if ( i == 0 )
    {
    sp = s->GetComponent(i+1 + j*dims[0] + k*ijsize, 0);
    sm = s->GetComponent(i + j*dims[0] + k*ijsize, 0);
    g[0] = (sm - sp) / ar[0];
    }
  else if ( i == (dims[0]-1) )
    {
    sp = s->GetComponent(i + j*dims[0] + k*ijsize,0);
    sm = s->GetComponent(i-1 + j*dims[0] + k*ijsize,0);
    g[0] = (sm - sp) / ar[0];
    }
  else
    {
    sp = s->GetComponent(i+1 + j*dims[0] + k*ijsize,0);
    sm = s->GetComponent(i-1 + j*dims[0] + k*ijsize,0);
    g[0] = 0.5 * (sm - sp) / ar[0];
    }

  // y-direction
  if ( dims[1] == 1 )
    {
    g[1] = 0.0;
    }
  else if ( j == 0 )
    {
    sp = s->GetComponent(i + (j+1)*dims[0] + k*ijsize,0);
    sm = s->GetComponent(i + j*dims[0] + k*ijsize,0);
    g[1] = (sm - sp) / ar[1];
    }
  else if ( j == (dims[1]-1) )
    {
    sp = s->GetComponent(i + j*dims[0] + k*ijsize,0);
    sm = s->GetComponent(i + (j-1)*dims[0] + k*ijsize,0);
    g[1] = (sm - sp) / ar[1];
    }
  else
    {
    sp = s->GetComponent(i + (j+1)*dims[0] + k*ijsize,0);
    sm = s->GetComponent(i + (j-1)*dims[0] + k*ijsize,0);
    g[1] = 0.5 * (sm - sp) / ar[1];
    }

  // z-direction
  if ( dims[2] == 1 )
    {
    g[2] = 0.0;
    }
  else if ( k == 0 )
    {
    sp = s->GetComponent(i + j*dims[0] + (k+1)*ijsize,0);
    sm = s->GetComponent(i + j*dims[0] + k*ijsize,0);
    g[2] = (sm - sp) / ar[2];
    }
  else if ( k == (dims[2]-1) )
    {
    sp = s->GetComponent(i + j*dims[0] + k*ijsize,0);
    sm = s->GetComponent(i + j*dims[0] + (k-1)*ijsize,0);
    g[2] = (sm - sp) / ar[2];
    }
  else
    {
    sp = s->GetComponent(i + j*dims[0] + (k+1)*ijsize,0);
    sm = s->GetComponent(i + j*dims[0] + (k-1)*ijsize,0);
    g[2] = 0.5 * (sm - sp) / ar[2];
    }
}

//----------------------------------------------------------------------------
// Set dimensions of structured points dataset.
void vtkImageData::SetDimensions(int i, int j, int k)
{
  this->SetExtent(0, i-1, 0, j-1, 0, k-1);
}

//----------------------------------------------------------------------------
// Set dimensions of structured points dataset.
void vtkImageData::SetDimensions(int dim[3])
{
  this->SetExtent(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
}


// streaming change: ijk is in extent coordinate system.
//----------------------------------------------------------------------------
// Convenience function computes the structured coordinates for a point x[3].
// The voxel is specified by the array ijk[3], and the parametric coordinates
// in the cell are specified with pcoords[3]. The function returns a 0 if the
// point x is outside of the volume, and a 1 if inside the volume.
int vtkImageData::ComputeStructuredCoordinates(double x[3], int ijk[3], 
                                               double pcoords[3])
{
  int i;
  double d, doubleLoc;
  double *origin = this->GetOrigin();
  double *spacing = this->GetSpacing();
  int *dims = this->GetDimensions();
  int extent[6];
  this->GetExtent(extent);
  
  //
  //  Compute the ijk location
  //
  for (i=0; i<3; i++) 
    {
    d = x[i] - origin[i];
    doubleLoc = d / spacing[i];
    // Floor for negtive indexes.
    ijk[i] = (int) (floor(doubleLoc));
    if ( ijk[i] >= extent[i*2] && ijk[i] < extent[i*2 + 1] )
      {
      pcoords[i] = doubleLoc - (double)ijk[i];
      }

    else if ( ijk[i] < extent[i*2] || ijk[i] > extent[i*2+1] ) 
      {
      return 0;
      } 

    else //if ( ijk[i] == extent[i*2+1] )
      {
      if (dims[i] == 1)
        {
        pcoords[i] = 0.0;
        }
      else
        {
        ijk[i] -= 1;
        pcoords[i] = 1.0;
        }
      }

    }
  return 1;
}


//----------------------------------------------------------------------------
void vtkImageData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  int idx;
  int *dims = this->GetDimensions();
  int extent[6];
  this->GetExtent(extent);
  
  os << indent << "ScalarType: " << this->ScalarType << endl;
  os << indent << "NumberOfScalarComponents: " << 
    this->NumberOfScalarComponents << endl;
  os << indent << "Spacing: (" << this->Spacing[0] << ", "
                               << this->Spacing[1] << ", "
                               << this->Spacing[2] << ")\n";
  os << indent << "Origin: (" << this->Origin[0] << ", "
                              << this->Origin[1] << ", "
                              << this->Origin[2] << ")\n";
  os << indent << "Dimensions: (" << dims[0] << ", "
                                  << dims[1] << ", "
                                  << dims[2] << ")\n";
  os << indent << "Increments: (" << this->Increments[0] << ", "
                                  << this->Increments[1] << ", "
                                  << this->Increments[2] << ")\n";
  os << indent << "Extent: (" << extent[0];
  for (idx = 1; idx < 6; ++idx)
    {
    os << ", " << extent[idx];
    }
  os << ")\n";
  os << indent << "WholeExtent: (" << this->WholeExtent[0];
  for (idx = 1; idx < 6; ++idx)
    {
    os << ", " << this->WholeExtent[idx];
    }
  os << ")\n";
}


//----------------------------------------------------------------------------
// Should we split up cells, or just points.  It does not matter for now.
// Extent of structured data assumes points.
void vtkImageData::SetUpdateExtent(int piece, int numPieces, int ghostLevel)
{
  int ext[6];
  
  this->UpdateInformation();
  this->GetWholeExtent(ext);
  this->ExtentTranslator->SetWholeExtent(ext);
  this->ExtentTranslator->SetPiece(piece);
  this->ExtentTranslator->SetNumberOfPieces(numPieces);
  this->ExtentTranslator->SetGhostLevel(ghostLevel);
  this->ExtentTranslator->PieceToExtent();
  this->SetUpdateExtent(this->ExtentTranslator->GetExtent());

  this->UpdatePiece = piece;
  this->UpdateNumberOfPieces = numPieces;
  this->UpdateGhostLevel = ghostLevel;
}

//----------------------------------------------------------------------------

void vtkImageData::SetNumberOfScalarComponents(int num)
{
  this->NumberOfScalarComponents = num;
  this->ComputeIncrements();
}

int *vtkImageData::GetIncrements()
{
  // Make sure the increments are up to date. The filter bypass and update
  // mechanism make it tricky to update the increments anywhere other than here
  this->ComputeIncrements();

  return this->Increments;
}

void vtkImageData::GetIncrements(int &incX, int &incY, int &incZ)
{
  // Make sure the increments are up to date. The filter bypass and update
  // mechanism make it tricky to update the increments anywhere other than here
  this->ComputeIncrements();

  incX = this->Increments[0];
  incY = this->Increments[1];
  incZ = this->Increments[2];
}

void vtkImageData::GetIncrements(int inc[3])
{
  // Make sure the increments are up to date. The filter bypass and update
  // mechanism make it tricky to update the increments anywhere other than here
  this->ComputeIncrements();

  inc[0] = this->Increments[0];
  inc[1] = this->Increments[1];
  inc[2] = this->Increments[2];
}


//----------------------------------------------------------------------------
void vtkImageData::GetContinuousIncrements(int extent[6], int &incX,
                                           int &incY, int &incZ)
{
  int e0, e1, e2, e3;
  
  incX = 0;
  int selfExtent[6];
  this->GetExtent(selfExtent);

  e0 = extent[0];
  if (e0 < selfExtent[0])
    {
    e0 = selfExtent[0];
    }
  e1 = extent[1];
  if (e1 > selfExtent[1])
    {
    e1 = selfExtent[1];
    }
  e2 = extent[2];
  if (e2 < selfExtent[2])
    {
    e2 = selfExtent[2];
    }
  e3 = extent[3];
  if (e3 > selfExtent[3])
    {
    e3 = selfExtent[3];
    }

  // Make sure the increments are up to date
  this->ComputeIncrements();

  incY = this->Increments[1] - (e1 - e0 + 1)*this->Increments[0];
  incZ = this->Increments[2] - (e3 - e2 + 1)*this->Increments[1];
}


//----------------------------------------------------------------------------
// This method computes the increments from the MemoryOrder and the extent.
void vtkImageData::ComputeIncrements()
{
  int idx;
  int inc = this->GetNumberOfScalarComponents();
  int extent[6];
  this->GetExtent(extent);

  for (idx = 0; idx < 3; ++idx)
    {
    this->Increments[idx] = inc;
    inc *= (extent[idx*2+1] - extent[idx*2] + 1);
    }
}

//----------------------------------------------------------------------------
double vtkImageData::GetScalarComponentAsDouble(int x, int y, int z, int comp)
{
  void *ptr;
  
  if (comp >= this->GetNumberOfScalarComponents() || comp < 0)
    {
    vtkErrorMacro("Bad component index " << comp);
    return 0.0;
    }
  
  ptr = this->GetScalarPointer(x, y, z);
  
  if (ptr == NULL)
    {
    // error message will be generated by get scalar pointer
    return 0.0;
    }
  
  switch (this->ScalarType)
    {
    case VTK_DOUBLE:
      return *(((double *)ptr) + comp);
    case VTK_FLOAT:
      return *(((float *)ptr) + comp);
    case VTK_INT:
      return (double)(*(((int *)ptr) + comp));
    case VTK_UNSIGNED_INT:
      return (double)(*(((unsigned int *)ptr) + comp));
    case VTK_LONG:
      return (double)(*(((long *)ptr) + comp));
    case VTK_UNSIGNED_LONG:
      return (double)(*(((unsigned long *)ptr) + comp));
    case VTK_SHORT:
      return (double)(*(((short *)ptr) + comp));
    case VTK_UNSIGNED_SHORT:
      return (double)(*(((unsigned short *)ptr) + comp));
    case VTK_UNSIGNED_CHAR:
      return (double)(*(((unsigned char *)ptr) + comp));
    case VTK_CHAR:
      return (double)(*(((char *)ptr) + comp));
    }

  vtkErrorMacro("Unknown Scalar type");
  return 0.0;
}

//----------------------------------------------------------------------------
void vtkImageData::SetScalarComponentFromDouble(int x, int y, int z, int comp, double v)
{
  void *ptr;
  
  if (comp >= this->GetNumberOfScalarComponents() || comp < 0)
    {
    vtkErrorMacro("Bad component index " << comp);
    return;
    }
  
  ptr = this->GetScalarPointer(x, y, z);
  
  if (ptr == NULL)
    {
    // error message will be generated by get scalar pointer
    return;
    }
  
  switch (this->ScalarType)
    {
    case VTK_DOUBLE:
      {
      double* pp = ((double*)ptr) + comp;
      *pp = (double)v;
      break;
      }
    case VTK_FLOAT:
      {
      float* pp = ((float*)ptr) + comp;
      *pp = (float)v;
      break;
      }
    case VTK_INT:
      {
      int* pp = ((int*)ptr) + comp;
      *pp = (int)v;
      break;
      }
    case VTK_UNSIGNED_INT:
      {
      unsigned int* pp = ((unsigned int*)ptr) + comp;
      *pp = (unsigned int)v;
      break;
      }
    case VTK_LONG:
      {
      long* pp = ((long*)ptr) + comp;
      *pp = (long)v;
      break;
      }
    case VTK_UNSIGNED_LONG:
      {
      unsigned long* pp = ((unsigned long*)ptr) + comp;
      *pp = (unsigned long)v;
      break;
      }
    case VTK_SHORT:
      {
      short* pp = ((short*)ptr) + comp;
      *pp = (short)v;
      break;
      }
    case VTK_UNSIGNED_SHORT:
      {
      unsigned short* pp = ((unsigned short*)ptr) + comp;
      *pp = (unsigned short)v;
      break;
      }
    case VTK_UNSIGNED_CHAR:
      {
      unsigned char* pp = ((unsigned char*)ptr) + comp;
      *pp = (unsigned char)v;
      break;
      }
    case VTK_CHAR:
      {
      char* pp = ((char*)ptr) + comp;
      *pp = (char)v;
      break;
      }
    default:
      vtkErrorMacro("Unknown Scalar type");
      break;
    }
}


//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetScalarPointer(int x, int y, int z)
{
  int tmp[3];
  tmp[0] = x;
  tmp[1] = y;
  tmp[2] = z;
  return this->GetScalarPointer(tmp);
}

//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetScalarPointerForExtent(int extent[6])
{
  int tmp[3];
  tmp[0] = extent[0];
  tmp[1] = extent[2];
  tmp[2] = extent[4];
  return this->GetScalarPointer(tmp);
}

//----------------------------------------------------------------------------
void *vtkImageData::GetScalarPointer(int coordinate[3])
{
  vtkDataArray *scalars = this->GetPointData()->GetScalars();

  // Make sure the array has been allocated.
  if (scalars == NULL)
    {
    vtkDebugMacro("Allocating scalars in ImageData");
    this->AllocateScalars();
    scalars = this->PointData->GetScalars();
    }

  if (scalars == NULL)
    {
    vtkErrorMacro("Could not allocate scalars.");
    return NULL;
    }
      
  int extent[6];
  this->GetExtent(extent);
  // error checking: since most acceses will be from pointer arithmetic.
  // this should not waste much time.
  for (int idx = 0; idx < 3; ++idx)
    {
    if (coordinate[idx] < extent[idx*2] ||
        coordinate[idx] > extent[idx*2+1])
      {
      vtkErrorMacro(<< "GetScalarPointer: Pixel (" << coordinate[0] << ", " 
      << coordinate[1] << ", "
      << coordinate[2] << ") not in memory.\n Current extent= ("
      << extent[0] << ", " << extent[1] << ", "
      << extent[2] << ", " << extent[3] << ", "
      << extent[4] << ", " << extent[5] << ")");
      return NULL;
      }
    }
  
  return this->GetArrayPointer(scalars, coordinate);
}

//----------------------------------------------------------------------------
// This Method returns a pointer to the origin of the vtkImageData.
void *vtkImageData::GetScalarPointer()
{
  if (this->PointData->GetScalars() == NULL)
    {
    vtkDebugMacro("Allocating scalars in ImageData");
    this->AllocateScalars();
    }
  return this->PointData->GetScalars()->GetVoidPointer(0);
}

//----------------------------------------------------------------------------
int vtkImageData::GetScalarType()
{
  vtkDataArray *tmp;
  int type = this->ScalarType;
  
  // if we have scalars make sure the type matches our ivar
  tmp = this->GetPointData()->GetScalars();
  if (tmp && tmp->GetDataType() != type)
    {
    // this happens when filters are being bypassed.  Don't error...
    //vtkErrorMacro("ScalarType " << tmp->GetDataType() 
    //                 << " does not match current scalars of type " << type);
    }
  
  return type;
}

//----------------------------------------------------------------------------
void vtkImageData::AllocateScalars()
{
  vtkDataArray *scalars;
  
  // if the scalar type has not been set then we have a problem
  if (this->ScalarType == VTK_VOID)
    {
    vtkErrorMacro("Attempt to allocate scalars before scalar type was set!.");
    return;
    }

  int extent[6];
  this->GetExtent(extent);

  // if we currently have scalars then just adjust the size
  scalars = this->PointData->GetScalars();
  if (scalars && scalars->GetDataType() == this->ScalarType
      && scalars->GetReferenceCount() == 1) 
    {
    scalars->SetNumberOfComponents(this->GetNumberOfScalarComponents());
    scalars->SetNumberOfTuples((extent[1] - extent[0] + 1)*
                               (extent[3] - extent[2] + 1)*
                               (extent[5] - extent[4] + 1));
    // Since the execute method will be modifying the scalars
    // directly.
    scalars->Modified();
    return;
    }
  
  // allocate the new scalars
  switch (this->ScalarType)
    {
    case VTK_BIT:
      scalars = vtkBitArray::New();
      break;
    case VTK_UNSIGNED_CHAR:
      scalars = vtkUnsignedCharArray::New();
      break;
    case VTK_CHAR:           
      scalars = vtkCharArray::New();
      break;
    case VTK_UNSIGNED_SHORT: 
      scalars = vtkUnsignedShortArray::New();
      break;
    case VTK_SHORT:          
      scalars = vtkShortArray::New();
      break;
    case VTK_UNSIGNED_INT:   
      scalars = vtkUnsignedIntArray::New();
      break;
    case VTK_INT:            
      scalars = vtkIntArray::New();
      break;
    case VTK_UNSIGNED_LONG:  
      scalars = vtkUnsignedLongArray::New();
      break;
    case VTK_LONG:           
      scalars = vtkLongArray::New();
      break;
    case VTK_DOUBLE:          
      scalars = vtkDoubleArray::New();
      break;
    case VTK_FLOAT:         
      scalars = vtkFloatArray::New();
      break;
    default:
      vtkErrorMacro("Could not allocate data type.");
      return;
    }
  
  scalars->SetNumberOfComponents(this->GetNumberOfScalarComponents());

  // allocate enough memory
  scalars->
    SetNumberOfTuples((extent[1] - extent[0] + 1)*
                      (extent[3] - extent[2] + 1)*
                      (extent[5] - extent[4] + 1));

  this->PointData->SetScalars(scalars);
  scalars->Delete();
}


//----------------------------------------------------------------------------
int vtkImageData::GetScalarSize()
{
  // allocate the new scalars
  switch (this->ScalarType)
    {
    case VTK_DOUBLE:
      return sizeof(double);
    case VTK_FLOAT:
      return sizeof(float);
    case VTK_INT:
    case VTK_UNSIGNED_INT:
      return sizeof(int);
    case VTK_LONG:
    case VTK_UNSIGNED_LONG:
      return sizeof(long);
    case VTK_SHORT:
    case VTK_UNSIGNED_SHORT:
      return 2;
    case VTK_UNSIGNED_CHAR:
      return 1;
    case VTK_CHAR:
      return 1;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class IT, class OT>
void vtkImageDataCastExecute(vtkImageData *inData, IT *inPtr,
                             vtkImageData *outData, OT *outPtr,
                             int outExt[6])
{
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int rowLength;

  // find the region to loop over
  rowLength = (outExt[1] - outExt[0]+1)*inData->GetNumberOfScalarComponents();
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // Loop through ouput pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; idxY <= maxY; idxY++)
      {
      for (idxR = 0; idxR < rowLength; idxR++)
        {
        // Pixel operation
        *outPtr = (OT)(*inPtr);
        outPtr++;
        inPtr++;
        }
      outPtr += outIncY;
      inPtr += inIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    }
}



//----------------------------------------------------------------------------
template <class T>
void vtkImageDataCastExecute(vtkImageData *inData, T *inPtr,
                             vtkImageData *outData, int outExt[6])
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt);

  if (outPtr == NULL)
    {
    vtkGenericWarningMacro("Scalars not allocated.");
    return;
    }

  switch (outData->GetScalarType())
    {
    vtkTemplateMacro5(vtkImageDataCastExecute, inData, (T *)(inPtr), 
                      outData, (VTK_TT *)(outPtr),outExt);
    default:
      vtkGenericWarningMacro("Execute: Unknown output ScalarType");
      return;
    }
}




//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageData::CopyAndCastFrom(vtkImageData *inData, int extent[6])
{
  void *inPtr = inData->GetScalarPointerForExtent(extent);
  
  if (inPtr == NULL)
    {
    vtkErrorMacro("Scalars not allocated.");
    return;
    }

  switch (inData->ScalarType)
    {
    vtkTemplateMacro4(vtkImageDataCastExecute,inData, (VTK_TT *)(inPtr), 
                      this, extent);
    default:
      vtkErrorMacro(<< "Execute: Unknown input ScalarType");
      return;
    }
}

//----------------------------------------------------------------------------
void vtkImageData::Crop()
{
  int           nExt[6];
  int           idxX, idxY, idxZ;
  int           maxX, maxY, maxZ;
  vtkIdType     outId, inId, inIdY, inIdZ, incZ, incY;
  vtkImageData  *newImage;
  int numPts, numCells, tmp;
  int extent[6];
  this->GetExtent(extent);
  
  // If extents already match, then we need to do nothing.
  if (extent[0] == this->UpdateExtent[0]
      && extent[1] == this->UpdateExtent[1]
      && extent[2] == this->UpdateExtent[2]
      && extent[3] == this->UpdateExtent[3]
      && extent[4] == this->UpdateExtent[4]
      && extent[5] == this->UpdateExtent[5])
    {
    return;
    }

  // Take the intersection of the two extent so that 
  // we are not asking for more than the extent.
  this->GetUpdateExtent(nExt);
  if (nExt[0] < extent[0]) { nExt[0] = extent[0];}
  if (nExt[1] > extent[1]) { nExt[1] = extent[1];}
  if (nExt[2] < extent[2]) { nExt[2] = extent[2];}
  if (nExt[3] > extent[3]) { nExt[3] = extent[3];}
  if (nExt[4] < extent[4]) { nExt[4] = extent[4];}
  if (nExt[5] > extent[5]) { nExt[5] = extent[5];}

  // If the extents are the same just return.
  if (extent[0] == nExt[0] && extent[1] == nExt[1]
      && extent[2] == nExt[2] && extent[3] == nExt[3]
      && extent[4] == nExt[4] && extent[5] == nExt[5])
    {
    vtkDebugMacro("Extents already match.");
    return;
    }

  // How many point/cells.
  numPts = (nExt[1]-nExt[0]+1)*(nExt[3]-nExt[2]+1)*(nExt[5]-nExt[4]+1);
  // Conditional are to handle 3d, 2d, and even 1d images.
  tmp = nExt[1] - nExt[0];
  if (tmp <= 0)
    {
    tmp = 1;
    }
  numCells = tmp;
  tmp = nExt[3] - nExt[2];
  if (tmp <= 0)
    {
    tmp = 1;
    }
  numCells *= tmp;
  tmp = nExt[5] - nExt[4];
  if (tmp <= 0)
    {
    tmp = 1;
    }
  numCells *= tmp;
  
  // Create a new temporary image. 
  newImage = vtkImageData::New();
  newImage->SetScalarType(this->ScalarType);
  newImage->SetNumberOfScalarComponents(this->GetNumberOfScalarComponents());
  newImage->SetExtent(nExt);
  vtkPointData *npd = newImage->GetPointData();
  vtkCellData *ncd = newImage->GetCellData();
  npd->CopyAllocate(this->PointData, numPts);
  ncd->CopyAllocate(this->CellData, numCells);
      
  // Loop through outData points
  incY = extent[1]-extent[0]+1;
  incZ = (extent[3]-extent[2]+1)*incY;
  outId = 0;
  inIdZ = incZ * (nExt[4]-extent[4]) 
          + incY * (nExt[2]-extent[2])
          + (nExt[0]-extent[0]);

  for (idxZ = nExt[4]; idxZ <= nExt[5]; idxZ++)
    {
    inIdY = inIdZ;
    for (idxY = nExt[2]; idxY <= nExt[3]; idxY++)
      {
      inId = inIdY;
      for (idxX = nExt[0]; idxX <= nExt[1]; idxX++)
        {
        npd->CopyData( this->PointData, inId, outId);
        ++inId;
        ++outId;
        }
      inIdY += incY;
      }
    inIdZ += incZ;
    }
         
  // Loop through outData cells
  // Have to handle the 2d and 1d cases.
  maxX = nExt[1];
  maxY = nExt[3];
  maxZ = nExt[5];
  if (maxX == nExt[0])
    {
    ++maxX;
    }
  if (maxY == nExt[2])
    {
    ++maxY;
    }
  if (maxZ == nExt[4])
    {
    ++maxZ;
    }
  incY = extent[1]-extent[0];
  incZ = (extent[3]-extent[2])*incY;
  outId = 0;
  inIdZ = incZ * (nExt[4]-extent[4]) 
          + incY * (nExt[2]-extent[2])
          + (nExt[0]-extent[0]);
  for (idxZ = nExt[4]; idxZ < maxZ; idxZ++)
    {
    inIdY = inIdZ;
    for (idxY = nExt[2]; idxY < maxY; idxY++)
      {
      inId = inIdY;
      for (idxX = nExt[0]; idxX < maxX; idxX++)
        {
        ncd->CopyData(this->CellData, inId, outId);
        ++inId;
        ++outId;
        }
      inIdY += incY;
      }
    inIdZ += incZ;
    }

  this->PointData->ShallowCopy(npd);
  this->CellData->ShallowCopy(ncd);
  this->SetExtent(nExt);
  newImage->Delete();
}



//----------------------------------------------------------------------------
double vtkImageData::GetScalarTypeMin()
{
  switch (this->ScalarType)
    {
    case VTK_DOUBLE:
      return (double)(VTK_DOUBLE_MIN);
    case VTK_FLOAT:
      return (double)(VTK_FLOAT_MIN);
    case VTK_LONG:
      return (double)(VTK_LONG_MIN);
    case VTK_UNSIGNED_LONG:
      return (double)(VTK_UNSIGNED_LONG_MIN);
    case VTK_INT:
      return (double)(VTK_INT_MIN);
    case VTK_UNSIGNED_INT:
      return (double)(VTK_UNSIGNED_INT_MIN);
    case VTK_SHORT:
      return (double)(VTK_SHORT_MIN);
    case VTK_UNSIGNED_SHORT:
      return (double)(VTK_UNSIGNED_SHORT_MIN);
    case VTK_CHAR:
      return (double)(VTK_CHAR_MIN);
    case VTK_UNSIGNED_CHAR:
      return (double)(VTK_UNSIGNED_CHAR_MIN);
    default:
      vtkErrorMacro("Cannot handle scalar type " << this->ScalarType);
      return 0.0;
    }
}


//----------------------------------------------------------------------------
double vtkImageData::GetScalarTypeMax()
{
  switch (this->ScalarType)
    {
    case VTK_DOUBLE:
      return (double)(VTK_DOUBLE_MAX);
    case VTK_FLOAT:
      return (double)(VTK_FLOAT_MAX);
    case VTK_LONG:
      return (double)(VTK_LONG_MAX);
    case VTK_UNSIGNED_LONG:
      return (double)(VTK_UNSIGNED_LONG_MAX);
    case VTK_INT:
      return (double)(VTK_INT_MAX);
    case VTK_UNSIGNED_INT:
      return (double)(VTK_UNSIGNED_INT_MAX);
    case VTK_SHORT:
      return (double)(VTK_SHORT_MAX);
    case VTK_UNSIGNED_SHORT:
      return (double)(VTK_UNSIGNED_SHORT_MAX);
    case VTK_CHAR:
      return (double)(VTK_CHAR_MAX);
    case VTK_UNSIGNED_CHAR:
      return (double)(VTK_UNSIGNED_CHAR_MAX);
    default:
      vtkErrorMacro("Cannot handle scalar type " << this->ScalarType);
      return 0.0;
    }
}

//----------------------------------------------------------------------------
void vtkImageData::SetExtent(int x1, int x2, int y1, int y2, int z1, int z2)
{
  int ext[6];
  ext[0] = x1;
  ext[1] = x2;
  ext[2] = y1;
  ext[3] = y2;
  ext[4] = z1;
  ext[5] = z2;
  this->SetExtent(ext);
}

//----------------------------------------------------------------------------
int* vtkImageData::GetExtent()
{
  return this->Information->Get(vtkDataObject::DATA_EXTENT());
}

//----------------------------------------------------------------------------
void vtkImageData::GetExtent(int& x1, int& x2,
                             int& y1, int& y2,
                             int& z1, int& z2)
{
  int extent[6];
  this->Information->Get(vtkDataObject::DATA_EXTENT(), extent);
  x1 = extent[0];
  x2 = extent[1];
  y1 = extent[2];
  y2 = extent[3];
  z1 = extent[4];
  z2 = extent[5];
}

//----------------------------------------------------------------------------
void vtkImageData::GetExtent(int* extent)
{
  this->Information->Get(vtkDataObject::DATA_EXTENT(), extent);
}

//----------------------------------------------------------------------------
int *vtkImageData::GetDimensions()
{
  int extent[6];
  this->GetExtent(extent);
  this->Dimensions[0] = extent[1] - extent[0] + 1;
  this->Dimensions[1] = extent[3] - extent[2] + 1;
  this->Dimensions[2] = extent[5] - extent[4] + 1;

  return this->Dimensions;
}

//----------------------------------------------------------------------------
void vtkImageData::GetDimensions(int *dOut)
{
  int *dims = this->GetDimensions();
  dOut[0] = dims[0];
  dOut[1] = dims[1];
  dOut[2] = dims[2];  
}

//----------------------------------------------------------------------------
void vtkImageData::SetExtent(int *extent)
{
  int description;

  int newExtent[6];
  this->Information->Get(vtkDataObject::DATA_EXTENT(), newExtent);
  description = vtkStructuredData::SetExtent(extent, newExtent);
  this->Information->Set(vtkDataObject::DATA_EXTENT(), newExtent, 6);
  if ( description < 0 ) //improperly specified
    {
    vtkErrorMacro (<< "Bad Extent, retaining previous values");
    }
  
  if (description == VTK_UNCHANGED)
    {
    return;
    }

  this->DataDescription = description;
  
  this->Modified();
  this->ComputeIncrements();
}


//----------------------------------------------------------------------------
void vtkImageData::SetAxisUpdateExtent(int idx, int min, int max)
{
  int modified = 0;
  
  if (idx > 2)
    {
    vtkWarningMacro("illegal axis!");
    return;
    }
  
  if (this->UpdateExtent[idx*2] != min)
    {
    modified = 1;
    this->UpdateExtent[idx*2] = min;
    }
  if (this->UpdateExtent[idx*2+1] != max)
    {
    modified = 1;
    this->UpdateExtent[idx*2+1] = max;
    }

  this->UpdateExtentInitialized = 1;
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImageData::GetAxisUpdateExtent(int idx, int &min, int &max)
{
  if (idx > 2)
    {
    vtkWarningMacro("illegal axis!");
    return;
    }

  min = this->UpdateExtent[idx*2];
  max = this->UpdateExtent[idx*2+1];
}


//----------------------------------------------------------------------------
unsigned long vtkImageData::GetActualMemorySize()
{
  return this->vtkDataSet::GetActualMemorySize();
}


//----------------------------------------------------------------------------
void vtkImageData::ShallowCopy(vtkDataObject *dataObject)
{
  vtkImageData *imageData = vtkImageData::SafeDownCast(dataObject);

  if ( imageData != NULL )
    {
    this->InternalImageDataCopy(imageData);
    }

  // Do superclass
  this->vtkDataSet::ShallowCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkImageData::DeepCopy(vtkDataObject *dataObject)
{
  vtkImageData *imageData = vtkImageData::SafeDownCast(dataObject);

  if ( imageData != NULL )
    {
    this->InternalImageDataCopy(imageData);
    }

  // Do superclass
  this->vtkDataSet::DeepCopy(dataObject);
}

//----------------------------------------------------------------------------
// This copies all the local variables (but not objects).
void vtkImageData::InternalImageDataCopy(vtkImageData *src)
{
  int idx;

  this->DataDescription = src->DataDescription;
  this->ScalarType = src->ScalarType;
  this->NumberOfScalarComponents = src->NumberOfScalarComponents;
  for (idx = 0; idx < 3; ++idx)
    {
    this->Dimensions[idx] = src->Dimensions[idx];
    this->Increments[idx] = src->Increments[idx];
    this->Origin[idx] = src->Origin[idx];
    this->Spacing[idx] = src->Spacing[idx];
    }
}



//----------------------------------------------------------------------------
vtkIdType vtkImageData::GetNumberOfCells() 
{
  vtkIdType nCells=1;
  int i;
  int *dims = this->GetDimensions();

  for (i=0; i<3; i++)
    {
    if (dims[i] == 0)
      {
      return 0;
      }
    if (dims[i] > 1)
      {
      nCells *= (dims[i]-1);
      }
    }

  return nCells;
}

//============================================================================
// Starting to make some more general methods that deal with any array
// (not just scalars).
//============================================================================


//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void vtkImageData::GetArrayIncrements(vtkDataArray* array, int increments[3])
{
  int extent[6];
  this->GetExtent(extent);
  // We could store tupple increments and just 
  // multiply by the number of componenets...
  increments[0] = array->GetNumberOfComponents();
  increments[1] = increments[0] * (extent[1]-extent[0]+1);
  increments[2] = increments[1] * (extent[3]-extent[2]+1);
}

//----------------------------------------------------------------------------
void *vtkImageData::GetArrayPointerForExtent(vtkDataArray* array, 
                                             int extent[6])
{
  int tmp[3];
  tmp[0] = extent[0];
  tmp[1] = extent[2];
  tmp[2] = extent[4];
  return this->GetArrayPointer(array, tmp);
}

//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetArrayPointer(vtkDataArray* array, int coordinate[3])
{
  int incs[3];
  int idx;
    
  if (array == NULL)
    {
    return NULL;
    }

  int extent[6];
  this->GetExtent(extent);
  // error checking: since most acceses will be from pointer arithmetic.
  // this should not waste much time.
  for (idx = 0; idx < 3; ++idx)
    {
    if (coordinate[idx] < extent[idx*2] ||
        coordinate[idx] > extent[idx*2+1])
      {
      vtkErrorMacro(<< "GetPointer: Pixel (" << coordinate[0] << ", " 
        << coordinate[1] << ", "
        << coordinate[2] << ") not in current extent: ("
        << extent[0] << ", " << extent[1] << ", "
        << extent[2] << ", " << extent[3] << ", "
        << extent[4] << ", " << extent[5] << ")");
      return NULL;
      }
    }
  
  // compute the index of the vector.
  this->GetArrayIncrements(array, incs);
  idx = ((coordinate[0] - extent[0]) * incs[0]
         + (coordinate[1] - extent[2]) * incs[1]
         + (coordinate[2] - extent[4]) * incs[2]);
  // I could check to see if the array has the correct number
  // of tupples for the extent, but that would be an extra multiply.
  if (idx < 0 || idx > array->GetMaxId())
    {
    vtkErrorMacro("Coordinate (" << coordinate[0] << ", " << coordinate[1]
                  << ", " << coordinate[2] << ") out side of array (max = "
                  << array->GetMaxId());
    return NULL;
    }

  return array->GetVoidPointer(idx);
}


void vtkImageData::ComputeInternalExtent(int *intExt, int *tgtExt, int *bnds)
{
  int i;
  int extent[6];
  this->GetExtent(extent);
  for (i = 0; i < 3; ++i)
    {
    intExt[i*2] = tgtExt[i*2];
    if (intExt[i*2] - bnds[i*2] < extent[i*2])
      {
      intExt[i*2] = extent[i*2] + bnds[i*2];
      }
    intExt[i*2+1] = tgtExt[i*2+1];
    if (intExt[i*2+1] + bnds[i*2+1] > extent[i*2+1])
      {
      intExt[i*2+1] = extent[i*2+1] - bnds[i*2+1];
      }
    }
}

//----------------------------------------------------------------------------
void vtkImageData::CopyDownstreamIVarsFromInformation(vtkInformation* info)
{
  this->Superclass::CopyDownstreamIVarsFromInformation(info);
  if (info->Has(vtkDataObject::SCALAR_TYPE()))
    {
    this->ScalarType =
      info->Get(vtkDataObject::SCALAR_TYPE());
    }
  if (info->Has(vtkDataObject::SCALAR_NUMBER_OF_COMPONENTS()))
    {
    this->NumberOfScalarComponents =
      info->Get(vtkDataObject::SCALAR_NUMBER_OF_COMPONENTS());
    }
}

//----------------------------------------------------------------------------
void vtkImageData::CopyDownstreamIVarsToInformation(vtkInformation* info)
{
  this->Superclass::CopyDownstreamIVarsToInformation(info);
  info->Set(vtkDataObject::SCALAR_TYPE(), this->ScalarType);
  info->Set(vtkDataObject::SCALAR_NUMBER_OF_COMPONENTS(),
            this->NumberOfScalarComponents);
}