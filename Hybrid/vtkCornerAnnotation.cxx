/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCornerAnnotation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCornerAnnotation.h"

#include "vtkAlgorithmOutput.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkObjectFactory.h"
#include "vtkPropCollection.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"
#include "vtkWindow.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCornerAnnotation);
vtkCxxRevisionMacro(vtkCornerAnnotation, "1.1");

vtkSetObjectImplementationMacro(vtkCornerAnnotation,ImageActor,vtkImageActor);
vtkSetObjectImplementationMacro(vtkCornerAnnotation,WindowLevel,
                                vtkImageMapToWindowLevelColors);
vtkCxxSetObjectMacro(vtkCornerAnnotation,TextProperty,vtkTextProperty);

//----------------------------------------------------------------------------
vtkCornerAnnotation::vtkCornerAnnotation()
{
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
  this->PositionCoordinate->SetValue(0.2,0.85);

  this->LastSize[0] = 0;
  this->LastSize[1] = 0;

  this->MaximumLineHeight = 1.0;
  this->MinimumFontSize = 6;
  this->FontSize = 15;

  this->TextProperty = vtkTextProperty::New();
  this->TextProperty->ShadowOff();

  for (int i = 0; i < 4; i++)
    {
    this->CornerText[i] = NULL;
    this->TextMapper[i] = vtkTextMapper::New();
    this->TextActor[i] = vtkActor2D::New();
    this->TextActor[i]->SetMapper(this->TextMapper[i]);
    }
  
  this->ImageActor = NULL;
  this->LastImageActor = 0;
  this->WindowLevel = NULL;
  
  this->LevelShift = 0;
  this->LevelScale = 1;
  
  this->ShowSliceAndImage = 1;
}

//----------------------------------------------------------------------------
vtkCornerAnnotation::~vtkCornerAnnotation()
{
  this->SetTextProperty(NULL);

  for (int i = 0; i < 4; i++)
    {
    delete [] this->CornerText[i];
    this->TextMapper[i]->Delete();
    this->TextActor[i]->Delete();
    }
  
  this->SetWindowLevel(NULL);
  this->SetImageActor(NULL);
}

//----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkCornerAnnotation::ReleaseGraphicsResources(vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  for (int i = 0; i < 4; i++)
    {
    this->TextActor[i]->ReleaseGraphicsResources(win);
    }
}

//----------------------------------------------------------------------------
void vtkCornerAnnotation::TextReplace(vtkImageActor *ia,
                                      vtkImageMapToWindowLevelColors *wl)
{
  int i;
  char *text, *text2;
  int image=0;
  char *rpos, *tmp;
  float window=0, level=0;
    
  if (ia)
    {
    image = ia->GetSliceNumber();
    }
  if (wl)
    {
    window = wl->GetWindow();
    window *= this->LevelScale;
    level = wl->GetLevel();    
    level = level * this->LevelScale + this->LevelShift;
    }
  
  // search for tokens, replace and then assign to TextMappers
  for (i = 0; i < 4; i++)
    {
    if (this->CornerText[i] && strlen(this->CornerText[i]))
      {
      text = new char [strlen(this->CornerText[i])+1000];
      text2 = new char [strlen(this->CornerText[i])+1000];
      strcpy(text,this->CornerText[i]);
      // now do the replacements
      rpos = strstr(text,"<image>");
      while (rpos)
        {
        *rpos = '\0';
        if (ia && this->ShowSliceAndImage)
          {
          sprintf(text2,"%sImage: %i%s",text,image,rpos+7);
          }
        else
          {
          sprintf(text2,"%s%s",text,rpos+7);
          }
        tmp = text;
        text = text2;
        text2 = tmp;
        rpos = strstr(text,"<image>");
        }
      rpos = strstr(text,"<slice>");
      while (rpos)
        {
        *rpos = '\0';
        if (ia && this->ShowSliceAndImage)
          {
          sprintf(text2,"%sSlice: %i%s",text,image,rpos+7);
          }
        else
          {
          sprintf(text2,"%s%s",text,rpos+7);
          }
        tmp = text;
        text = text2;
        text2 = tmp;
        rpos = strstr(text,"<slice>");
        }
      rpos = strstr(text,"<window>");
      while (rpos)
        {
        *rpos = '\0';
        if (wl)
          {
          sprintf(text2,"%sWindow: %g%s",text,window,rpos+8);
          }
        else
          {
          sprintf(text2,"%s%s",text,rpos+8);
          }
        tmp = text;
        text = text2;
        text2 = tmp;
        rpos = strstr(text,"<window>");
        }
      rpos = strstr(text,"<level>");
      while (rpos)
        {
        *rpos = '\0';
        if (wl)
          {
          sprintf(text2,"%sLevel: %g%s",text,level,rpos+7);
          }
        else
          {
          sprintf(text2,"%s%s",text,rpos+7);
          }
        tmp = text;
        text = text2;
        text2 = tmp;
        rpos = strstr(text,"<level>");
        }
      this->TextMapper[i]->SetInput(text);
      delete [] text;
      delete [] text2;
      }
    else
      {
      this->TextMapper[i]->SetInput("");
      }
    }
}

//----------------------------------------------------------------------------
int vtkCornerAnnotation::RenderOverlay(vtkViewport *viewport)
{
  // Everything is built, just have to render
  // only render if font is at least minimum font
  if (this->FontSize >= this->MinimumFontSize)
    {
    for (int i = 0; i < 4; i++)
      {
      this->TextActor[i]->RenderOverlay(viewport);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkCornerAnnotation::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int fontSize;
  int i;
  
  // Check to see whether we have to rebuild everything
  // If the viewport has changed we may - or may not need
  // to rebuild, it depends on if the projected coords chage
  int viewport_size_has_changed = 0;
  if (viewport->GetMTime() > this->BuildTime ||
      (viewport->GetVTKWindow() && 
       viewport->GetVTKWindow()->GetMTime() > this->BuildTime))
    {
    int *vSize = viewport->GetSize();
    if (this->LastSize[0] != vSize[0] || this->LastSize[1] != vSize[1])
      {
      viewport_size_has_changed = 1;
      }
    }
  
  // Is there an image actor ?
  vtkImageActor *ia = 0;  
  vtkImageMapToWindowLevelColors *wl = this->WindowLevel;
  vtkPropCollection *pc = viewport->GetProps();
  int numProps = pc->GetNumberOfItems();
  for (i = 0; i < numProps; i++)
    {
    ia = vtkImageActor::SafeDownCast(pc->GetItemAsObject(i));
    if (ia)
      {
      if (ia->GetInput() && !wl)
        {
        wl = vtkImageMapToWindowLevelColors::SafeDownCast(
          ia->GetInput()->GetProducerPort()->GetProducer());
        }
      break;
      }
    }  
  
  int tprop_has_changed = (this->TextProperty && 
                           this->TextProperty->GetMTime() > this->BuildTime);

  // Check to see whether we have to rebuild everything
  if (viewport_size_has_changed ||
      tprop_has_changed ||
      (this->GetMTime() > this->BuildTime) ||
      (ia && (ia != this->LastImageActor || 
              ia->GetMTime() > this->BuildTime)) ||
      (wl && wl->GetMTime() > this->BuildTime))
    {
    int *vSize = viewport->GetSize();

    vtkDebugMacro(<<"Rebuilding text");
    
    // Replace text
    this->TextReplace(ia, wl);
    
    // Get the viewport size in display coordinates
    this->LastSize[0] = vSize[0];
    this->LastSize[1] = vSize[1];

    // Only adjust size then the text changes due to non w/l slice reasons
    if (viewport_size_has_changed ||
        tprop_has_changed ||
        this->GetMTime() > this->BuildTime)
      {
      // Rebuid text props.
      // Perform shallow copy here since each individual corner has a
      // different aligment/size but they share the other this->TextProperty
      // attributes.
      fontSize = this->TextMapper[0]->GetTextProperty()->GetFontSize();

      if (tprop_has_changed)
        {
        vtkTextProperty *tprop = this->TextMapper[0]->GetTextProperty();
        tprop->ShallowCopy(this->TextProperty);
        tprop->SetJustificationToLeft();
        tprop->SetVerticalJustificationToBottom();
        tprop->SetFontSize(fontSize);

        tprop = this->TextMapper[1]->GetTextProperty();
        tprop->ShallowCopy(this->TextProperty);
        tprop->SetJustificationToRight();
        tprop->SetVerticalJustificationToBottom();
        tprop->SetFontSize(fontSize);
        
        tprop = this->TextMapper[2]->GetTextProperty();
        tprop->ShallowCopy(this->TextProperty);
        tprop->SetJustificationToLeft();
        tprop->SetVerticalJustificationToTop();
        tprop->SetFontSize(fontSize);
        
        tprop = this->TextMapper[3]->GetTextProperty();
        tprop->ShallowCopy(this->TextProperty);
        tprop->SetJustificationToRight();
        tprop->SetVerticalJustificationToTop();
        tprop->SetFontSize(fontSize);
        }

      // Update all the composing objects to find the best size for the font
      // use the last size as a first guess

      /*  
          +--------+
          |2      3|
          |        |
          |        |
          |0      1|
          +--------+  
      */

      int tempi[8];
      int allZeros = 1;
      for (i = 0; i < 4; i++)
        {
        this->TextMapper[i]->GetSize(viewport, tempi + i * 2);
        if (tempi[2*i] > 0 || tempi[2*i+1] > 0)
          {
          allZeros = 0;
          }
        }

      if (allZeros)
        {
        return 0;
        }
      
      int height_02 = tempi[1] + tempi[5];
      int height_13 = tempi[3] + tempi[7];

      int width_01 = tempi[0] + tempi[2];
      int width_23 = tempi[4] + tempi[6];

      int max_width = (width_01 > width_23) ? width_01 : width_23;
      
      int num_lines_02 = 
        this->TextMapper[0]->GetNumberOfLines() + 
        this->TextMapper[2]->GetNumberOfLines();

      int num_lines_13 = 
        this->TextMapper[1]->GetNumberOfLines() + 
        this->TextMapper[3]->GetNumberOfLines();
      
      int line_max_02 = (int)(vSize[1] * this->MaximumLineHeight) * 
        (num_lines_02 ? num_lines_02 : 1);

      int line_max_13 = (int)(vSize[1] * this->MaximumLineHeight) * 
        (num_lines_13 ? num_lines_13 : 1);
      
      // Target size is to use 90% of x and y

      int tSize[2];
      tSize[0] = (int)(0.9*vSize[0]);
      tSize[1] = (int)(0.9*vSize[1]);    

      // While the size is too small increase it

      while (height_02 < tSize[1] && 
             height_13 < tSize[1] &&
             max_width < tSize[0] &&
             height_02 < line_max_02 &&
             height_13 < line_max_13 &&
             fontSize < 100)
        {
        fontSize++;
        for (i = 0; i < 4; i++)
          {
          this->TextMapper[i]->GetTextProperty()->SetFontSize(fontSize);
          this->TextMapper[i]->GetSize(viewport, tempi + i * 2);
          }
        height_02 = tempi[1] + tempi[5];
        height_13 = tempi[3] + tempi[7];
        width_01 = tempi[0] + tempi[2];
        width_23 = tempi[4] + tempi[6];
        max_width = (width_01 > width_23) ? width_01 : width_23;
        }

      // While the size is too large decrease it

      while ((height_02 > tSize[1] || 
              height_13 > tSize[1] || 
              max_width > tSize[0] ||
              height_02 > line_max_02 || 
              height_13 > line_max_13) && 
             fontSize > 0)
        {
        fontSize--;
        for (i = 0; i < 4; i++)
          {
          this->TextMapper[i]->GetTextProperty()->SetFontSize(fontSize);
          this->TextMapper[i]->GetSize(viewport, tempi + i * 2);
          }
        height_02 = tempi[1] + tempi[5];
        height_13 = tempi[3] + tempi[7];
        width_01 = tempi[0] + tempi[2];
        width_23 = tempi[4] + tempi[6];
        max_width = (width_01 > width_23) ? width_01 : width_23;
        }

      fontSize = static_cast<int>(pow((float)fontSize,0.7f)*pow(10.0f,0.3f));
      this->FontSize = fontSize;
      for (i = 0; i < 4; i++)
        {
        this->TextMapper[i]->GetTextProperty()->SetFontSize(fontSize);
        }

      // Now set the position of the TextActors

      this->TextActor[0]->SetPosition(5,5);
      this->TextActor[1]->SetPosition(vSize[0] - 5,5);
      this->TextActor[2]->SetPosition(5, vSize[1] - 5);
      this->TextActor[3]->SetPosition(vSize[0] - 5, vSize[1] - 5);
      
      for (i = 0; i < 4; i++)
        {
        this->TextActor[i]->SetProperty(this->GetProperty());
        }
      }
    this->BuildTime.Modified();
    this->LastImageActor = ia;
    }

  // Everything is built, just have to render

  if (this->FontSize >= this->MinimumFontSize)
    {
    for (i = 0; i < 4; i++)
      {
      this->TextActor[i]->RenderOpaqueGeometry(viewport);
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkCornerAnnotation::SetText(int i, const char *text)
{
  if (i < 0 || i > 3)
    {
    return;
    }

  if (!text || 
      (this->CornerText[i] && text && (!strcmp(this->CornerText[i],text))))
    { 
    return;
    } 
  delete [] this->CornerText[i];
  this->CornerText[i] = new char [strlen(text)+1];
  strcpy(this->CornerText[i],text);
  this->Modified();
}

//----------------------------------------------------------------------------
char* vtkCornerAnnotation::GetText(int i)
{
  if (i < 0 || i > 3)
    {
    return NULL;
    }

  return this->CornerText[i];
}

//----------------------------------------------------------------------------
void vtkCornerAnnotation::ClearAllTexts()
{
  int i;
  for (i = 0; i < 4; i++)
    {
    this->SetText(i, "");
    }
}

//----------------------------------------------------------------------------
void vtkCornerAnnotation::CopyAllTextsFrom(vtkCornerAnnotation *ca)
{
  int i;
  for (i = 0; i < 4; i++)
    {
    this->SetText(i, ca->GetText(i));
    }
}

//----------------------------------------------------------------------------
void vtkCornerAnnotation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ImageActor: " << this->GetImageActor() << endl;
  os << indent << "MinimumFontSize: " << this->GetMinimumFontSize() << endl;
  os << indent << "WindowLevel: " << this->GetWindowLevel() << endl;
  os << indent << "Mapper: " << this->GetMapper() << endl;
  os << indent << "MaximumLineHeight: " << this->MaximumLineHeight << endl;
  os << indent << "LevelShift: " << this->LevelShift << endl;
  os << indent << "LevelScale: " << this->LevelScale << endl;
  os << indent << "TextProperty: " << this->TextProperty << endl;
  os << indent << "ShowSliceAndImage: " << this->ShowSliceAndImage << endl;
}

