/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkProperty.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkActor.h"
#include "vtkGraphicsFactory.h"

#include <stdlib.h>

vtkCxxRevisionMacro(vtkProperty, "1.55");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkProperty);
//----------------------------------------------------------------------------

// Construct object with object color, ambient color, diffuse color,
// specular color, and edge color white; ambient coefficient=0; diffuse 
// coefficient=0; specular coefficient=0; specular power=1; Gouraud shading;
// and surface representation. Backface and frontface culling are off.
vtkProperty::vtkProperty()
{
  this->AmbientColor[0] = 1;
  this->AmbientColor[1] = 1;
  this->AmbientColor[2] = 1;

  this->DiffuseColor[0] = 1;
  this->DiffuseColor[1] = 1;
  this->DiffuseColor[2] = 1;

  this->SpecularColor[0] = 1;
  this->SpecularColor[1] = 1;
  this->SpecularColor[2] = 1;

  this->EdgeColor[0] = 1;
  this->EdgeColor[1] = 1;
  this->EdgeColor[2] = 1;

  this->Ambient = 0.0;
  this->Diffuse = 1.0;
  this->Specular = 0.0;
  this->SpecularPower = 1.0;
  this->Opacity = 1.0;
  this->Interpolation = VTK_GOURAUD;
  this->Representation = VTK_SURFACE;
  this->EdgeVisibility = 0;
  this->BackfaceCulling = 0;
  this->FrontfaceCulling = 0;
  this->PointSize = 1.0;
  this->LineWidth = 1.0;
  this->LineStipplePattern = 0xFFFF;
  this->LineStippleRepeatFactor = 1;
}

// Assign one property to another. 
void vtkProperty::DeepCopy(vtkProperty *p)
{
  if ( p != NULL )
    {
    this->SetColor(p->GetColor());
    this->SetAmbientColor(p->GetAmbientColor());
    this->SetDiffuseColor(p->GetDiffuseColor());
    this->SetSpecularColor(p->GetSpecularColor());
    this->SetEdgeColor(p->GetEdgeColor());
    this->SetAmbient(p->GetAmbient());
    this->SetDiffuse(p->GetDiffuse());
    this->SetSpecular(p->GetSpecular());
    this->SetSpecularPower(p->GetSpecularPower());
    this->SetOpacity(p->GetOpacity());
    this->SetInterpolation(p->GetInterpolation());
    this->SetRepresentation(p->GetRepresentation());
    this->SetEdgeVisibility(p->GetEdgeVisibility());
    this->SetBackfaceCulling(p->GetBackfaceCulling());
    this->SetFrontfaceCulling(p->GetFrontfaceCulling());
    this->SetPointSize(p->GetPointSize());
    this->SetLineWidth(p->GetLineWidth());
    this->SetLineStipplePattern(p->GetLineStipplePattern());
    this->SetLineStippleRepeatFactor(p->GetLineStippleRepeatFactor());
    }
}

// return the correct type of Property 
vtkProperty *vtkProperty::New()
{ 
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkGraphicsFactory::CreateInstance("vtkProperty");
  return (vtkProperty*)ret;
}

void vtkProperty::SetColor(double R,double G,double B)
{
  // Use Set macros to insure proper modified time behavior
  this->SetAmbientColor(R,G,B);
  this->SetDiffuseColor(R,G,B);
  this->SetSpecularColor(R,G,B);
}

// Return composite color of object (ambient + diffuse + specular). Return value
// is a pointer to rgb values.
double *vtkProperty::GetColor()
{
  double norm;
  int i;
  
  if ((this->Ambient + this->Diffuse + this->Specular)>0)
    {
    norm = 1.0 / (this->Ambient + this->Diffuse + this->Specular);
    }
  else
    {
    norm = 0.0;
    }
  
  for (i = 0; i < 3; i ++)
    {
    this->Color[i] = this->AmbientColor[i]*this->Ambient*norm;
    this->Color[i] = this->Color[i] + this->DiffuseColor[i]*this->Diffuse*norm;
    this->Color[i] = this->Color[i] + this->SpecularColor[i]*this->Specular*norm;
    }
  
  return this->Color;  
}

// Copy composite color of object (ambient + diffuse + specular) into array 
// provided.
void vtkProperty::GetColor(double rgb[3])
{
  this->GetColor();

  rgb[0] = this->Color[0];
  rgb[1] = this->Color[1];
  rgb[2] = this->Color[2];
}

 
void vtkProperty::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Ambient: " << this->Ambient << "\n";
  os << indent << "Ambient Color: (" << this->AmbientColor[0] << ", " 
    << this->AmbientColor[1] << ", " << this->AmbientColor[2] << ")\n";
  os << indent << "Diffuse: " << this->Diffuse << "\n";
  os << indent << "Diffuse Color: (" << this->DiffuseColor[0] << ", " 
    << this->DiffuseColor[1] << ", " << this->DiffuseColor[2] << ")\n";
  os << indent << "Edge Color: (" << this->EdgeColor[0] << ", " 
    << this->EdgeColor[1] << ", " << this->EdgeColor[2] << ")\n";
  os << indent << "Edge Visibility: " 
    << (this->EdgeVisibility ? "On\n" : "Off\n");
  os << indent << "Interpolation: ";
  switch (this->Interpolation) 
    {
    case VTK_FLAT: os << "VTK_FLAT\n"; break;
    case VTK_GOURAUD: os << "VTK_GOURAUD\n"; break;
    case VTK_PHONG: os << "VTK_PHONG\n"; break;
    default: os << "unknown\n";
    }
  os << indent << "Opacity: " << this->Opacity << "\n";
  os << indent << "Representation: ";
  switch (this->Representation) 
    {
    case VTK_POINTS: os << "VTK_POINTS\n"; break;
    case VTK_WIREFRAME: os << "VTK_WIREFRAME\n"; break;
    case VTK_SURFACE: os << "VTK_SURFACE\n"; break;
    default: os << "unknown\n";
    }
  os << indent << "Specular: " << this->Specular << "\n";
  os << indent << "Specular Color: (" << this->SpecularColor[0] << ", " 
     << this->SpecularColor[1] << ", " << this->SpecularColor[2] << ")\n";
  os << indent << "Specular Power: " << this->SpecularPower << "\n";
  os << indent << "Backface Culling: " 
     << (this->BackfaceCulling ? "On\n" : "Off\n");
  os << indent << "Frontface Culling: " 
     << (this->FrontfaceCulling ? "On\n" : "Off\n");
  os << indent << "Point size: " << this->PointSize << "\n";
  os << indent << "Line width: " << this->LineWidth << "\n";
  os << indent << "Line stipple pattern: " << this->LineStipplePattern << "\n";
  os << indent << "Line stipple repeat factor: " << this->LineStippleRepeatFactor << "\n";

}
