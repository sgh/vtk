/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataWriter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDataWriter.h"

#include "vtkBitArray.h"
#include "vtkByteSwap.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkShortArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedShortArray.h"

vtkCxxRevisionMacro(vtkDataWriter, "1.109");
vtkStandardNewMacro(vtkDataWriter);

// this undef is required on the hp. vtkMutexLock ends up including
// /usr/inclue/dce/cma_ux.h which has the gall to #define write as cma_write

#ifdef write
#undef write
#endif

// Created object with default header, ASCII format, and default names for 
// scalars, vectors, tensors, normals, and texture coordinates.
vtkDataWriter::vtkDataWriter()
{
  this->FileName = NULL;  
  
  this->Header = new char[257];
  strcpy(this->Header,"vtk output");
  this->FileType = VTK_ASCII;

  this->ScalarsName = 0;
  this->VectorsName = 0;
  this->TensorsName = 0;
  this->NormalsName = 0;
  this->TCoordsName = 0;

  this->LookupTableName = new char[13];
  strcpy(this->LookupTableName,"lookup_table");

  this->FieldDataName = new char[10];
  strcpy(this->FieldDataName,"FieldData");

  this->WriteToOutputString = 0;
  this->OutputString = NULL;
  this->OutputStringAllocatedLength = 0;
  this->OutputStringLength = 0;
}

vtkDataWriter::~vtkDataWriter()
{ 
  if ( this->FileName )
    {
    delete [] this->FileName;
    }
  if ( this->Header )
    {
    delete [] this->Header;
    }
  if ( this->ScalarsName )
    {
    delete [] this->ScalarsName;
    }
  if ( this->VectorsName )
    {
    delete [] this->VectorsName;
    }
  if ( this->TensorsName )
    {
    delete [] this->TensorsName;
    }
  if ( this->NormalsName )
    {
    delete [] this->NormalsName;
    }
  if ( this->TCoordsName )
    {
    delete [] this->TCoordsName;
    }
  if ( this->LookupTableName )
    {
    delete [] this->LookupTableName;
    }
  if ( this->FieldDataName )
    {
    delete [] this->FieldDataName;
    }
  
  if (this->OutputString)
    {
    delete [] this->OutputString;
    this->OutputString = NULL;
    this->OutputStringLength = 0;
    this->OutputStringAllocatedLength = 0;
    }
}


// Open a vtk data file. Returns NULL if error.
ostream *vtkDataWriter::OpenVTKFile()
{
  ostream *fptr;
  vtkDataObject *input = this->GetInput();
  
  if ((!this->WriteToOutputString) && ( !this->FileName ))
    {
    vtkErrorMacro(<< "No FileName specified! Can't write!");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return NULL;
    }
  
  vtkDebugMacro(<<"Opening vtk file for writing...");

  if (this->WriteToOutputString)
    {
    // Get rid of any old output string.
    if (this->OutputString)
      {
      delete [] this->OutputString;
      this->OutputString = NULL;
      this->OutputStringLength = 0;
      this->OutputStringAllocatedLength = 0;
      }
    // Allocate the new output string. (Note: this will only work with binary).
    if (input == NULL)
      {
      vtkErrorMacro(<< "No input! Can't write!");
      return NULL;    
      }
    input->Update();
    this->OutputStringAllocatedLength = (int) (500 
      + 1000 * input->GetActualMemorySize());
    this->OutputString = new char[this->OutputStringAllocatedLength];

    fptr = new ostrstream(this->OutputString, 
                          this->OutputStringAllocatedLength);
    }
  else 
    {
    if ( this->FileType == VTK_ASCII )
      {
      fptr = new ofstream(this->FileName, ios::out);
      }
    else
      { 
#ifdef _WIN32
      fptr = new ofstream(this->FileName, ios::out | ios::binary);
#else
      fptr = new ofstream(this->FileName, ios::out);
#endif
      }
    }

  if (fptr->fail())
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    delete fptr;
    return NULL;
    }

  return fptr;
}

// Write the header of a vtk data file. Returns 0 if error.
int vtkDataWriter::WriteHeader(ostream *fp)
{
  vtkDebugMacro(<<"Writing header...");

  *fp << "# vtk DataFile Version 3.0\n"; 
  *fp << this->Header << "\n";

  if ( this->FileType == VTK_ASCII )
    {
    *fp << "ASCII\n";
    }
  else
    {
    *fp << "BINARY\n";
    }

  fp->flush();
  if (fp->fail())
    {
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return 0;
    }
  
  return 1;
}

// Write the cell data (e.g., scalars, vectors, ...) of a vtk dataset.
// Returns 0 if error.
int vtkDataWriter::WriteCellData(ostream *fp, vtkDataSet *ds)
{
  int numCells;
  vtkDataArray *scalars;
  vtkDataArray *vectors;
  vtkDataArray *normals;
  vtkDataArray *tcoords;
  vtkDataArray *tensors;
  vtkFieldData *field;
  vtkCellData *cd=ds->GetCellData();

  vtkDebugMacro(<<"Writing cell data...");

  numCells = ds->GetNumberOfCells();
  scalars = cd->GetScalars();
  vectors = cd->GetVectors();
  normals = cd->GetNormals();
  tcoords = cd->GetTCoords();
  tensors = cd->GetTensors();
  field = cd;

  if ( numCells <= 0 || !(scalars || vectors || normals || tcoords || 
                          tensors || field))
    {
    vtkDebugMacro(<<"No cell data to write!");
    return 1;
    }

  *fp << "CELL_DATA " << numCells << "\n";
  //
  // Write scalar data
  //
  if ( scalars && scalars->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteScalarData(fp, scalars, numCells) )
      {
      return 0;
      }
    }
  //
  // Write vector data
  //
  if ( vectors && vectors->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteVectorData(fp, vectors, numCells) )
      {
      return 0;
      }
    }
  //
  // Write normals
  //
  if ( normals && normals->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteNormalData(fp, normals, numCells) )
      {
      return 0;
      }
    }
  //
  // Write texture coords
  //
  if ( tcoords && tcoords->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteTCoordData(fp, tcoords, numCells) )
      {
      return 0;
      }
    }
  //
  // Write tensors
  //
  if ( tensors && tensors->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteTensorData(fp, tensors, numCells) )
      {
      return 0;
      }
    }
  //
  // Write field
  //
  if ( field && field->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteFieldData(fp, field) )
      {
      return 0;
      }
    }

  return 1;
}

// Write the point data (e.g., scalars, vectors, ...) of a vtk dataset.
// Returns 0 if error.
int vtkDataWriter::WritePointData(ostream *fp, vtkDataSet *ds)
{
  int numPts;
  vtkDataArray *scalars;
  vtkDataArray *vectors;
  vtkDataArray *normals;
  vtkDataArray *tcoords;
  vtkDataArray *tensors;
  vtkFieldData *field;
  vtkPointData *pd=ds->GetPointData();

  vtkDebugMacro(<<"Writing point data...");

  numPts = ds->GetNumberOfPoints();
  scalars = pd->GetScalars();
  vectors = pd->GetVectors();
  normals = pd->GetNormals();
  tcoords = pd->GetTCoords();
  tensors = pd->GetTensors();
  field = pd;

  if ( numPts <= 0 || !(scalars || vectors || normals || tcoords || 
                        tensors || field))
    {
    vtkDebugMacro(<<"No point data to write!");
    return 1;
    }

  *fp << "POINT_DATA " << numPts << "\n";
  //
  // Write scalar data
  //
  if ( scalars && scalars->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteScalarData(fp, scalars, numPts) )
      {
      return 0;
      }
    }
  //
  // Write vector data
  //
  if ( vectors && vectors->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteVectorData(fp, vectors, numPts) )
      {
      return 0;
      }
    }
  //
  // Write normals
  //
  if ( normals && normals->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteNormalData(fp, normals, numPts) )
      {
      return 0;
      }
    }
  //
  // Write texture coords
  //
  if ( tcoords && tcoords->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteTCoordData(fp, tcoords, numPts) )
      {
      return 0;
      }
    }
  //
  // Write tensors
  //
  if ( tensors && tensors->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteTensorData(fp, tensors, numPts) )
      {
      return 0;
      }
    }
  //
  // Write field
  //
  if ( field && field->GetNumberOfTuples() > 0 )
    {
    if ( ! this->WriteFieldData(fp, field) )
      {
      return 0;
      }
    }

  return 1;
}

// Template to handle writing data in ascii or binary
// We could change the format into C++ io standard ...
template <class T>
void vtkWriteDataArray(ostream *fp, T *data, int fileType, 
                       const char *format, int num, int numComp)
{
  int i, j, idx, sizeT;
  char str[1024];
  
  sizeT = sizeof(T);

  if ( fileType == VTK_ASCII )
    {
    for (j=0; j<num; j++)
      {
      for (i=0; i<numComp; i++)
        {
        idx = i + j*numComp;
        sprintf (str, format, *data++); *fp << str; 
        if ( !((idx+1)%9) )
          {
          *fp << "\n";
          }
        }
      }
    }
  else
    {
    if (num*numComp > 0)
      {
      // need to byteswap ??
      switch (sizeT)
        {
        case 2:
          // typecast doesn't have to be valid here
          vtkByteSwap::SwapWrite2BERange((short *)data,num*numComp, fp);
          break;
        case 4:
          // typecast doesn't have to be valid here
          vtkByteSwap::SwapWrite4BERange((float *)data,num*numComp, fp);
          break;
        case 8:
          // typecast doesn't have to be valid here
          vtkByteSwap::SwapWrite8BERange((double *)data,num*numComp, fp);
          break;
        default:
          fp->write((char *)data, ( sizeof(T))*( num*numComp));

        }
      }
    }
  *fp << "\n";
}

// Write out data to file specified.
int vtkDataWriter::WriteArray(ostream *fp, int dataType, vtkDataArray *data,
                              const char *format, int num, int numComp)
{
  int i, j, idx;
  char str[1024];
  
  switch (dataType)
    {
    case VTK_BIT:
      {
      sprintf (str, format, "bit"); *fp << str; 
      if ( this->FileType == VTK_ASCII )
        {
        int s;
        for (j=0; j<num; j++)
          {
          for (i=0; i<numComp; i++)
            {
            idx = i + j*numComp;
            s = ((vtkBitArray *)data)->GetValue(idx);
            *fp << (s!=0.0?1:0); 
            if ( !((idx+1)%8) )
              {
              *fp << "\n";
              }
            else
              {
              *fp << " ";
              }
            }
          }
        }
      else
        {
        unsigned char *cptr=((vtkUnsignedCharArray *)data)->GetPointer(0);
        fp->write((char *)cptr, (sizeof(unsigned char))*((num-1)/8+1));

        }
      *fp << "\n";
      }
    break;

    case VTK_CHAR:
      {
      sprintf (str, format, "char"); *fp << str; 
      char *s=((vtkCharArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%hhd ", num, numComp);
      }
    break;

    case VTK_UNSIGNED_CHAR:
      {
      sprintf (str, format, "unsigned_char"); *fp << str; 
      unsigned char *s=((vtkUnsignedCharArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%hhu ", num, numComp);
      }
    break;
    
    case VTK_SHORT:
      {
      sprintf (str, format, "short"); *fp << str; 
      short *s=((vtkShortArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%hd ", num, numComp);
      }
    break;

    case VTK_UNSIGNED_SHORT:
      {
      sprintf (str, format, "unsigned_short"); *fp << str; 
      unsigned short *s=((vtkUnsignedShortArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%hu ", num, numComp);
      }
    break;

    case VTK_INT:
      {
      sprintf (str, format, "int"); *fp << str; 
      int *s=((vtkIntArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%d ", num, numComp);
      }
    break;

    case VTK_UNSIGNED_INT:
      {
      sprintf (str, format, "unsigned_int"); *fp << str; 
      unsigned int *s=((vtkUnsignedIntArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%u ", num, numComp);
      }
    break;

    case VTK_LONG:
      {
      sprintf (str, format, "long"); *fp << str; 
      long *s=((vtkLongArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%ld ", num, numComp);
      }
    break;

    case VTK_UNSIGNED_LONG:
      {
      sprintf (str, format, "unsigned_long"); *fp << str; 
      unsigned long *s=((vtkUnsignedLongArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%lu ", num, numComp);
      }
    break;

    case VTK_FLOAT:
      {
      sprintf (str, format, "float"); *fp << str; 
      float *s=((vtkFloatArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%g ", num, numComp);
      }
    break;

    case VTK_DOUBLE:
      {
      sprintf (str, format, "double"); *fp << str; 
      double *s=((vtkDoubleArray *)data)->GetPointer(0);
      vtkWriteDataArray(fp, s, this->FileType, "%lg ", num, numComp);
      }
    break;

    case VTK_ID_TYPE:
      {
      // currently writing vtkIdType as int.
      int size = data->GetNumberOfTuples();
      int *intArray = new int[size*numComp];
      sprintf (str, format, "int"); *fp << str; 
      vtkIdType *s=((vtkIdTypeArray *)data)->GetPointer(0);
      for (i = 0; i < size*numComp; i++)
        {
        intArray[i] = s[i];
        }
      vtkWriteDataArray(fp, intArray, this->FileType, "%d ", num, numComp);
      delete [] intArray;
      }
    break;

    default:
      {
      vtkErrorMacro(<<"Type currently not supported");
      return 0;
      }
    }

  fp->flush();
  if (fp->fail())
    {
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return 0;
    }
  return 1;
}

int vtkDataWriter::WritePoints(ostream *fp, vtkPoints *points)
{
  int numPts;
  
  if (points == NULL)
    {
    *fp << "POINTS 0 float\n";
    return 1;
    }

  numPts=points->GetNumberOfPoints();
  *fp << "POINTS " << numPts << " ";
  return this->WriteArray(fp, points->GetDataType(), points->GetData(), "%s\n", numPts, 3);
}

// Write out coordinates for rectilinear grids.
int vtkDataWriter::WriteCoordinates(ostream *fp, vtkDataArray *coords, 
                                    int axes)
{
  int ncoords=coords->GetNumberOfTuples();
  
  if ( axes == 0 )
    {
    *fp << "X_COORDINATES " << ncoords << " ";
    }
  else if ( axes == 1)
    {
    *fp << "Y_COORDINATES " << ncoords << " ";
    }
  else
    {
    *fp << "Z_COORDINATES " << ncoords << " ";
    }

  return this->WriteArray(fp, coords->GetDataType(), coords, "%s\n", ncoords, 1);
}

// Write out scalar data.
int vtkDataWriter::WriteScalarData(ostream *fp, vtkDataArray *scalars, int num)
{
  int i, j, size=0;
  char *name;
  vtkLookupTable *lut;
  int dataType = scalars->GetDataType();
  int numComp = scalars->GetNumberOfComponents();
  
  if ( (lut=scalars->GetLookupTable()) == NULL || 
       (size = lut->GetNumberOfColors()) <= 0 )
    {
    name = (char *) "default";
    }
  else 
    {
    name = this->LookupTableName;
    }

  char* scalarsName;
  // Buffer size is size of array name times four because 
  // in theory there could be array name consisting of only
  // weird symbols.
  if (!this->ScalarsName)
    {
    if (scalars->GetName() && strlen(scalars->GetName()))
      {
      scalarsName = new char[ strlen(scalars->GetName()) * 4 + 1];
      this->EncodeArrayName(scalarsName, scalars->GetName());
      }
    else
      {
      scalarsName = new char[ strlen("scalars") + 1];
      strcpy(scalarsName, "scalars");
      }
    }
  else
    {
    scalarsName = new char[ strlen(this->ScalarsName) * 4 + 1];
    this->EncodeArrayName(scalarsName, this->ScalarsName);
    }
  
  if ( dataType != VTK_UNSIGNED_CHAR )
    {
    char format[1024];
    *fp << "SCALARS ";


    if (numComp == 1) 
      {
      sprintf(format,"%s %%s\nLOOKUP_TABLE %s\n", scalarsName, name);
      } 
    else 
      {
      sprintf(format,"%s %%s %d\nLOOKUP_TABLE %s\n",
              scalarsName, numComp, name);
      }
    delete[] scalarsName;
    if (this->WriteArray(fp, scalars->GetDataType(), scalars, format, 
                         num, numComp) == 0)
      {
      return 0;
      }
    }

  else //color scalars
    {
    int nvs = scalars->GetNumberOfComponents();
    unsigned char *data=((vtkUnsignedCharArray *)scalars)->GetPointer(0);
    *fp << "COLOR_SCALARS " << scalarsName << " " << nvs << "\n";

    if ( this->FileType == VTK_ASCII )
      {
      for (i=0; i<num; i++)
        {
        for (j=0; j<nvs; j++)
          {
          *fp << ((float)data[nvs*i+j]/255.0) << " ";
          }
        if ( i != 0 && !(i%2) )
          {
          *fp << "\n";
          }
        }
      }
    else // binary type
      {
      fp->write((char *)data, (sizeof(unsigned char))*(nvs*num));

      }

    *fp << "\n";
    delete[] scalarsName;

    }

  //if lookup table, write it out
  if ( lut && size > 0 )
    {
    *fp << "LOOKUP_TABLE " << this->LookupTableName << " " << size << "\n";
    if ( this->FileType == VTK_ASCII )
      {
      double *c;
      for (i=0; i<size; i++)
        {
        c = lut->GetTableValue(i);
        *fp << c[0] << " " << c[1] << " " << c[2] << " " << c[3] << "\n";
        }
      }
    else
      {
      unsigned char *colors=lut->GetPointer(0);
      fp->write((char *)colors, (sizeof(unsigned char)*4*size));
      }
    *fp << "\n";
    }

  fp->flush();
  if (fp->fail())
    {
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return 0;
    }
  
  return 1;
}

int vtkDataWriter::WriteVectorData(ostream *fp, vtkDataArray *vectors, int num)
{
  char format[1024];

  *fp << "VECTORS ";

  char* vectorsName;
  // Buffer size is size of array name times four because 
  // in theory there could be array name consisting of only
  // weird symbols.
  if (!this->VectorsName)
    {
    if (vectors->GetName() && strlen(vectors->GetName()))
      {
      vectorsName = new char[ strlen(vectors->GetName()) * 4 + 1];
      this->EncodeArrayName(vectorsName, vectors->GetName());
      }
    else
      {
      vectorsName = new char[ strlen("vectors") + 1];
      strcpy(vectorsName, "vectors");
      }
    }
  else
    {
    vectorsName = new char[ strlen(this->VectorsName) * 4 + 1];
    this->EncodeArrayName(vectorsName, this->VectorsName);
    }

  sprintf(format, "%s %s\n", vectorsName, "%s");
  delete[] vectorsName;

  return this->WriteArray(fp, vectors->GetDataType(), vectors, format, num, 3);
}

int vtkDataWriter::WriteNormalData(ostream *fp, vtkDataArray *normals, int num)
{
  char format[1024];

  char* normalsName;
  // Buffer size is size of array name times four because 
  // in theory there could be array name consisting of only
  // weird symbols.
  if (!this->NormalsName)
    {
    if (normals->GetName() && strlen(normals->GetName()) )
      {
      normalsName = new char[ strlen(normals->GetName()) * 4 + 1];
      this->EncodeArrayName(normalsName, normals->GetName());
      }
    else
      {
      normalsName = new char[ strlen("normals") + 1];
      strcpy(normalsName, "normals");
      }
    }
  else
    {
    normalsName = new char[ strlen(this->NormalsName) * 4 + 1];
    this->EncodeArrayName(normalsName, this->NormalsName);
    }

  *fp << "NORMALS ";
  sprintf(format, "%s %s\n", normalsName, "%s");
  delete[] normalsName;

  return this->WriteArray(fp, normals->GetDataType(), normals, format, num, 3);
}

int vtkDataWriter::WriteTCoordData(ostream *fp, vtkDataArray *tcoords, int num)
{
  int dim=tcoords->GetNumberOfComponents();
  char format[1024];

  char* tcoordsName;
  // Buffer size is size of array name times four because 
  // in theory there could be array name consisting of only
  // weird symbols.
  if (!this->TCoordsName)
    {
    if (tcoords->GetName() && strlen(tcoords->GetName()))
      {
      tcoordsName = new char[ strlen(tcoords->GetName()) * 4 + 1];
      this->EncodeArrayName(tcoordsName, tcoords->GetName());
      }
    else
      {
      tcoordsName = new char[ strlen("tcoords") + 1];
      strcpy(tcoordsName, "tcoords");
      }
    }
  else
    {
    tcoordsName = new char[ strlen(this->TCoordsName) * 4 + 1];
    this->EncodeArrayName(tcoordsName, this->TCoordsName);
    }


  *fp << "TEXTURE_COORDINATES ";
  sprintf(format, "%s %d %s\n", tcoordsName, dim, "%s");
  delete[] tcoordsName;

  return this->WriteArray(fp, tcoords->GetDataType(), tcoords, format, num, 
                          dim);
}

int vtkDataWriter::WriteTensorData(ostream *fp, vtkDataArray *tensors, int num)
{
  char format[1024];

  char* tensorsName;
  // Buffer size is size of array name times four because 
  // in theory there could be array name consisting of only
  // weird symbols.
  if (!this->TensorsName)
    {
    if (tensors->GetName() && strlen(tensors->GetName()))
      {
      tensorsName = new char[ strlen(tensors->GetName()) * 4 + 1];
      this->EncodeArrayName(tensorsName, tensors->GetName());
      }
    else
      {
      tensorsName = new char[ strlen("tensors") + 1];
      strcpy(tensorsName, "tensors");
      }
    }
  else
    {
    tensorsName = new char[ strlen(this->TensorsName) * 4 + 1];
    this->EncodeArrayName(tensorsName, this->TensorsName);
    }

  *fp << "TENSORS "; 
  sprintf(format, "%s %s\n", tensorsName, "%s");
  delete[] tensorsName;

  return this->WriteArray(fp, tensors->GetDataType(), tensors, format, num, 9);
}

int vtkIsInTheList(int index, int* list, int numElem)
{
  for(int i=0; i<numElem; i++)
    {
    if (index == list[i])
      {
      return 1;
      }
    }
  return 0;
}

int vtkDataWriter::WriteFieldData(ostream *fp, vtkFieldData *f)
{
  char format[1024];
  int i, numArrays=f->GetNumberOfArrays(), actNumArrays=0;
  int numComp, numTuples;
  int attributeIndices[vtkDataSetAttributes::NUM_ATTRIBUTES];
  vtkDataArray *array;

  for(i=0; i<vtkDataSetAttributes::NUM_ATTRIBUTES; i++)
    {
    attributeIndices[i] = -1;
    }
  vtkDataSetAttributes* dsa;
  if ((dsa=vtkDataSetAttributes::SafeDownCast(f)))
    {
    dsa->GetAttributeIndices(attributeIndices);
    }

  for (i=0; i < numArrays; i++)
    {
    if (!vtkIsInTheList(i, attributeIndices, 
                        vtkDataSetAttributes::NUM_ATTRIBUTES))
      {
      actNumArrays++;
      }
    }
  if ( actNumArrays < 1 )
    {
    return 1;
    }
  *fp << "FIELD " << this->FieldDataName << " " << actNumArrays << "\n";


  for (i=0; i < numArrays; i++)
    {
    if (!vtkIsInTheList(i, attributeIndices, 
                        vtkDataSetAttributes::NUM_ATTRIBUTES))
      {
      array = f->GetArray(i);
      if ( array != NULL )
        {
        numComp = array->GetNumberOfComponents();
        numTuples = array->GetNumberOfTuples();

        // Buffer size is size of array name times four because 
        // in theory there could be array name consisting of only
        // weird symbols.
        char* buffer;
        if( !array->GetName() || strlen(array->GetName()) == 0)
          {
          buffer = strcpy(new char[strlen("unknown")+1], "unknown");
          }
        else
          {
          buffer = new char[ strlen(array->GetName()) * 4 + 1];
          this->EncodeArrayName(buffer, array->GetName());
          }
        sprintf(format, "%s %d %d %s\n", buffer, numComp, numTuples, 
                "%s");
        this->WriteArray(fp, array->GetDataType(), array, format, numTuples, 
                         numComp);
        delete [] buffer;
        }
      else
        {
        *fp << "NULL_ARRAY"; 
        }
      }
    }

  if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
    return 0;
    }
  
  return 1;
}

int vtkDataWriter::WriteCells(ostream *fp, vtkCellArray *cells, const char *label)
{
  if ( ! cells )
    {
    return 1;
    }
  
  int ncells=cells->GetNumberOfCells();
  int size=cells->GetNumberOfConnectivityEntries();

  if ( ncells < 1 )
    {
    return 1;
    }

  *fp << label << " " << ncells << " " << size << "\n";

  if ( this->FileType == VTK_ASCII )
    {
    int j;
    vtkIdType *pts = 0;
    vtkIdType npts = 0;
    for (cells->InitTraversal(); cells->GetNextCell(npts,pts); )
      {
      // currently writing vtkIdType as int
      *fp << (int)npts << " ";
      for (j=0; j<npts; j++)
        {
        // currently writing vtkIdType as int
        *fp << (int)pts[j] << " ";
        }
      *fp << "\n";
      }
    }
  else
    {
    // swap the bytes if necc
    // currently writing vtkIdType as int
    vtkIdType *tempArray = cells->GetPointer();
    int arraySize = cells->GetNumberOfConnectivityEntries();
    int *intArray = new int[arraySize];
    int i;
    
    for (i = 0; i < arraySize; i++)
      {
      intArray[i] = tempArray[i];
      }
    
    vtkByteSwap::SwapWrite4BERange(intArray,size,fp);
    delete [] intArray;
    }

  *fp << "\n";
  
  fp->flush();
  if (fp->fail())
    {
    this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
    return 0;
    }
  
  return 1;
}

void vtkDataWriter::WriteData()
{
  vtkErrorMacro(<<"WriteData() should be implemented in concrete subclass");
}

// Close a vtk file.
void vtkDataWriter::CloseVTKFile(ostream *fp)
{
  vtkDebugMacro(<<"Closing vtk file\n");
  
  if ( fp != NULL )
    {
    if (this->WriteToOutputString)
      {
      char *tmp;
      ostrstream *ostr = (ostrstream*)(fp);
      this->OutputStringLength = ostr->pcount();

      if (this->OutputStringLength == this->OutputStringAllocatedLength)
        {
        vtkErrorMacro("OutputString was not long enough.");
        }
      // Sanity check.
      tmp = ostr->str();
      if (tmp != this->OutputString)
        {
        vtkErrorMacro("String mismatch");
        }
      this->OutputString = tmp;
      }
    delete fp;
    }

}

char *vtkDataWriter::RegisterAndGetOutputString()
{
  char *tmp = this->OutputString;
  
  this->OutputString = NULL;
  this->OutputStringLength = 0;
  this->OutputStringAllocatedLength = 0;
  
  return tmp;
}


void vtkDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

  if ( this->FileType == VTK_BINARY )
    {
    os << indent << "File Type: BINARY\n";
    }
  else
    {
    os << indent << "File Type: ASCII\n";
    }

  if ( this->Header )
    {
    os << indent << "Header: " << this->Header << "\n";
    }
  else
    {
    os << indent << "Header: (None)\n";
    }
  
  os << indent << "Output String Length: " << this->OutputStringLength << "\n";
  os << indent << "Output String (addr): " << ((void *)this->OutputString) << "\n";
  os << indent << "WriteToOutputString: " << (this->WriteToOutputString ? "On\n" : "Off\n");

  if ( this->ScalarsName )
    {
    os << indent << "Scalars Name: " << this->ScalarsName << "\n";
    }
  else
    {
    os << indent << "Scalars Name: (None)\n";
    }

  if ( this->VectorsName )
    {
    os << indent << "Vectors Name: " << this->VectorsName << "\n";
    }
  else
    {
    os << indent << "Vectors Name: (None)\n";
    }

  if ( this->NormalsName )
    {
    os << indent << "Normals Name: " << this->NormalsName << "\n";
    }
  else
    {
    os << indent << "Normals Name: (None)\n";
    }

  if ( this->TensorsName )
    {
    os << indent << "Tensors Name: " << this->TensorsName << "\n";
    }
  else
    {
    os << indent << "Tensors Name: (None)\n";
    }

  if ( this->TCoordsName )
    {
    os << indent << "Texture Coords Name: " << this->TCoordsName << "\n";
    }
  else
    {
    os << indent << "Texture Coordinates Name: (None)\n";
    }

  if ( this->LookupTableName )
    {
    os << indent << "Lookup Table Name: " << this->LookupTableName << "\n";
    }
  else
    {
    os << indent << "Lookup Table Name: (None)\n";  
    }

  if ( this->FieldDataName )
    {
    os << indent << "Field Data Name: " << this->FieldDataName << "\n";
    }
  else
    {
    os << indent << "Field Data Name: (None)\n";
    }

}

int vtkDataWriter::WriteDataSetData(ostream *fp, vtkDataSet *ds)
{
  vtkFieldData* field = ds->GetFieldData();
  if (field && field->GetNumberOfTuples() > 0) 
    {
    if ( !this->WriteFieldData(fp, field) ) 
      {
      return 0; // we tried to write field data, but we couldn't
      }
    }
  return 1;
}
