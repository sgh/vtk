/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInformation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInformation.h"

#include "vtkInformationDataObjectKey.h"
#include "vtkInformationDataObjectVectorKey.h"
#include "vtkInformationInformationKey.h"
#include "vtkInformationInformationVectorKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationKeyVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <vtkstd/map>

vtkCxxRevisionMacro(vtkInformation, "1.1");
vtkStandardNewMacro(vtkInformation);

//----------------------------------------------------------------------------
class vtkInformationInternals
{
public:
  typedef vtkstd::map<vtkInformationKey*,
                      vtkSmartPointer<vtkObjectBase> > MapType;
  MapType Map;
};

//----------------------------------------------------------------------------
vtkInformation::vtkInformation()
{
  this->Internal = new vtkInformationInternals;
}

//----------------------------------------------------------------------------
vtkInformation::~vtkInformation()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkInformationInternals::MapType::const_iterator i;
  for(i=this->Internal->Map.begin(); i != this->Internal->Map.end(); ++i)
    {
    os << indent << i->first << ": " << i->second.GetPointer() << "\n";
    }
}

//----------------------------------------------------------------------------
void vtkInformation::SetAsObjectBase(vtkInformationKey* key,
                                     vtkObjectBase* value)
{
  if(key)
    {
    vtkInformationInternals::MapType::iterator i =
      this->Internal->Map.find(key);
    if(i != this->Internal->Map.end())
      {
      if(value)
        {
        i->second = value;
        }
      else
        {
        this->Internal->Map.erase(i);
        }
      }
    else if(value)
      {
      this->Internal->Map[key] = value;
      }
    }
}

//----------------------------------------------------------------------------
vtkObjectBase* vtkInformation::GetAsObjectBase(vtkInformationKey* key)
{
  if(key)
    {
    vtkInformationInternals::MapType::const_iterator i =
      this->Internal->Map.find(key);
    if(i != this->Internal->Map.end())
      {
      return i->second.GetPointer();
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkInformation::Clear()
{
  this->Copy(0);
}

//----------------------------------------------------------------------------
void vtkInformation::Copy(vtkInformation* from)
{
  if(from)
    {
    this->Internal->Map = from->Internal->Map;
    }
  else
    {
    this->Internal->Map.clear();
    }
}


//----------------------------------------------------------------------------
void vtkInformation::CopyEntry(vtkInformation* from, vtkInformationKey* key)
{
  key->Copy(from, this);
}

//----------------------------------------------------------------------------
void vtkInformation::CopyEntries(vtkInformation* from,
                                 vtkInformationKeyVectorKey* key)
{
  int numberOfKeys = from->Length(key);
  vtkInformationKey** keys = from->Get(key);
  for(int i=0; i < numberOfKeys; ++i)
    {
    this->CopyEntry(from, keys[i]);
    }
}

//----------------------------------------------------------------------------
#define VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(name, type)                  \
  void vtkInformation::Set(vtkInformation##name##Key* key, type value)      \
    {                                                                       \
    key->Set(this, value);                                                  \
    }                                                                       \
  void vtkInformation::Remove(vtkInformation##name##Key* key)               \
    {                                                                       \
    key->Remove(this);                                                      \
    }                                                                       \
  type vtkInformation::Get(vtkInformation##name##Key* key)                  \
    {                                                                       \
    return key->Get(this);                                                  \
    }                                                                       \
  int vtkInformation::Has(vtkInformation##name##Key* key)                   \
    {                                                                       \
    return key->Has(this);                                                  \
    }
VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(Integer, int);
VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(String, const char*);
VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(DataObject, vtkDataObject*);
VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(Information, vtkInformation*);
VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(InformationVector, vtkInformationVector*);
//VTK_INFORMATION_DEFINE_SCALAR_PROPERTY(Float, float);
#undef VTK_INFORMATION_DEFINE_SCALAR_PROPERTY

//----------------------------------------------------------------------------
#define VTK_INFORMATION_DEFINE_VECTOR_PROPERTY(name, type)                  \
  void vtkInformation::Append(vtkInformation##name##VectorKey* key,         \
                              type value)                                   \
    {                                                                       \
    key->Append(this, value);                                               \
    }                                                                       \
  void vtkInformation::Set(vtkInformation##name##VectorKey* key,            \
                           type* value, int length)                         \
    {                                                                       \
    key->Set(this, value, length);                                          \
    }                                                                       \
  type* vtkInformation::Get(vtkInformation##name##VectorKey* key)           \
    {                                                                       \
    return key->Get(this);                                                  \
    }                                                                       \
  void vtkInformation::Get(vtkInformation##name##VectorKey* key,            \
                           type* value)                                     \
    {                                                                       \
    key->Get(this, value);                                                  \
    }                                                                       \
  int vtkInformation::Length(vtkInformation##name##VectorKey* key)          \
    {                                                                       \
    return key->Length(this);                                               \
    }                                                                       \
  void vtkInformation::Remove(vtkInformation##name##VectorKey* key)         \
    {                                                                       \
    key->Remove(this);                                                      \
    }                                                                       \
  int vtkInformation::Has(vtkInformation##name##VectorKey* key)             \
    {                                                                       \
    return key->Has(this);                                                  \
    }
VTK_INFORMATION_DEFINE_VECTOR_PROPERTY(Integer, int);
VTK_INFORMATION_DEFINE_VECTOR_PROPERTY(Key, vtkInformationKey*);
VTK_INFORMATION_DEFINE_VECTOR_PROPERTY(DataObject, vtkDataObject*);
#undef VTK_INFORMATION_DEFINE_VECTOR_PROPERTY

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationDataObjectKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationDataObjectVectorKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationInformationKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationInformationVectorKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationIntegerKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationIntegerVectorKey* key)
{
  return key;
}

//----------------------------------------------------------------------------
vtkInformationKey* vtkInformation::GetKey(vtkInformationStringKey* key)
{
  return key;
}