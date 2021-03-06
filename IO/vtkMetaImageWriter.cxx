/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMetaImageWriter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMetaImageWriter.h"

#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataSetAttributes.h"

#include <vtkstd/string>

#include <sys/stat.h>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMetaImageWriter, "1.13");
vtkStandardNewMacro(vtkMetaImageWriter);

//----------------------------------------------------------------------------
vtkMetaImageWriter::vtkMetaImageWriter()
{
  this->MHDFileName = 0;
  this->FileLowerLeft = 1;
}

//----------------------------------------------------------------------------
vtkMetaImageWriter::~vtkMetaImageWriter()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void vtkMetaImageWriter::SetFileName(const char* fname)
{
  this->SetMHDFileName(fname);
}

//----------------------------------------------------------------------------
void vtkMetaImageWriter::SetRAWFileName(const char* fname)
{
  this->Superclass::SetFileName(fname);
}

//----------------------------------------------------------------------------
char* vtkMetaImageWriter::GetRAWFileName()
{
  return this->Superclass::GetFileName();
}

//----------------------------------------------------------------------------
int vtkMetaImageWriter::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *input = 
    vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  // Error checking
  if (input == NULL )
    {
    vtkErrorMacro(<<"Write:Please specify an input!");
    return 0;
    }

  if ( !this->MHDFileName )
    {
    vtkErrorMacro("Output file name not specified");
    return 0;
    }

  if ( !this->GetRAWFileName() )
    {
    vtkDebugMacro("Raw file name not specified. Specifying one...");
    // Allocate new file name and leave space for extension
    char* rfname = new char [ strlen(this->MHDFileName) + 10 ]; 
    strcpy(rfname, this->MHDFileName);
    size_t cc;
    for ( cc = strlen(rfname)-1; cc > 0; cc -- )
      {
      if ( rfname[cc] == '.' )
        {
        rfname[cc] = 0;
        break;
        }
      if ( rfname[cc] == '/' || rfname[cc] == '\\' )
        {
        break;
        }
      }
    strcat(rfname, ".raw");
    if ( strcmp(rfname, this->MHDFileName) == 0 )
      {
      strcat(rfname, ".raw");
      }
    this->SetRAWFileName(rfname);
    delete [] rfname;
    }

  ofstream ofs_with_warning_C4701(this->MHDFileName, ios::out);
  if ( !ofs_with_warning_C4701 )
    {
    vtkErrorMacro("Cannot open file: " << this->MHDFileName << " for writing");
    return 0;
    }

  int ndims = 3;
  int ext[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),ext);
  if ( ext[4] == ext[5] )
    {
    ndims = 2;
    if ( ext[2] == ext[3] )
      {
      ndims = 1;
      }
    }
  double origin[3];
  double spacing[3];
  inInfo->Get(vtkDataObject::ORIGIN(),origin);
  inInfo->Get(vtkDataObject::SPACING(),spacing);
  
  const char* scalar_type;
  vtkInformation *scalarInfo = vtkDataObject::GetActiveFieldInformation(inInfo, 
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!scalarInfo)
    {
    vtkErrorMacro( "No active scalar information!" );
    return 0;
    }
  switch ( scalarInfo->Get(vtkDataObject::FIELD_ARRAY_TYPE()) )
    {
    case VTK_CHAR:           scalar_type = "MET_CHAR"; break;
    case VTK_UNSIGNED_CHAR:  scalar_type = "MET_UCHAR"; break;
    case VTK_SHORT:          scalar_type = "MET_SHORT"; break;
    case VTK_UNSIGNED_SHORT: scalar_type = "MET_USHORT"; break;
    case VTK_INT:            scalar_type = "MET_INT"; break;
    case VTK_UNSIGNED_INT:   scalar_type = "MET_UINT"; break;
    case VTK_LONG:           scalar_type = "MET_LONG"; break;
    case VTK_UNSIGNED_LONG:  scalar_type = "MET_ULONG"; break;
    case VTK_FLOAT:          scalar_type = "MET_FLOAT"; break;
    case VTK_DOUBLE:         scalar_type = "MET_DOUBLE"; break;
    default:
      vtkErrorMacro("Unknown scalar type: " 
                    << scalarInfo->Get(vtkDataObject::FIELD_ARRAY_TYPE()));
      return 1;
    }
  
  origin[0] += ext[0] * spacing[0];
  origin[1] += ext[2] * spacing[1];
  origin[2] += ext[4] * spacing[2];

  const char* data_file = this->GetRAWFileName();
  int pos = 0;
  int cc;
  for ( cc = 0; data_file[cc]; cc ++ )
    {
    if ( data_file[cc] == '/' || data_file[cc] == '\\' )
      {
      pos = cc;
      }
    }
  if ( pos > 0 )
    {
    if ( strncmp(data_file, this->GetFileName(), pos) == 0 )
      {
      data_file = this->GetRAWFileName() + pos + 1;
      }
    }

  ofs_with_warning_C4701 
    << "ObjectType = Image" << endl
    << "NDims = " << ndims << endl
    << "BinaryData = True" << endl
#ifdef VTK_WORDS_BIGENDIAN
    << "BinaryDataByteOrderMSB = True" << endl
#else
    << "BinaryDataByteOrderMSB = False" << endl
#endif
    << "ElementSpacing = " 
    << spacing[0] << " " << spacing[1] << " " << spacing[2] << endl
    << "DimSize = " << (ext[1]-ext[0]+1) << " " 
    << (ext[3]-ext[2]+1) << " " << (ext[5]-ext[4]+1) << endl
    << "Position = " 
    << origin[0] << " " << origin[1] << " " << origin[2] << endl
    << "ElementNumberOfChannels = " 
    << scalarInfo->Get(vtkDataObject::FIELD_NUMBER_OF_COMPONENTS()) << endl
    << "ElementType = " << scalar_type 
    << (scalarInfo->Get(vtkDataObject::FIELD_NUMBER_OF_COMPONENTS()) > 1?"_ARRAY":"") << endl
    << "ElementDataFile = " << data_file << endl;
  this->SetFileDimensionality(ndims);
  return this->Superclass::RequestData(request,inputVector,outputVector);
}

//----------------------------------------------------------------------------
void vtkMetaImageWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "MHDFileName: " << (this->MHDFileName?this->MHDFileName:"(none)") << endl;
}
