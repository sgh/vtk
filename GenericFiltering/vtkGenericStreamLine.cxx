/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGenericStreamLine.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkGenericStreamLine.h"

#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include "vtkFloatArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"

vtkCxxRevisionMacro(vtkGenericStreamLine, "1.1");
vtkStandardNewMacro(vtkGenericStreamLine);

// Construct object with step size set to 1.0.
vtkGenericStreamLine::vtkGenericStreamLine()
{
  this->StepLength = 1.0;
  this->NumberOfStreamers = 0;
}

void vtkGenericStreamLine::Execute()
{
  if ( !this->GetInput() )
    {
    vtkErrorMacro("Input not set");
    return;
    }

  vtkGenericStreamer::StreamPoint *sPrev, *sPtr;
  vtkPoints *newPts;
  vtkFloatArray *newVectors;
  vtkFloatArray *newScalars=NULL;
  vtkCellArray *newLines;
  vtkIdType ptId, i, id;
  int j;
  vtkIdList *pts;
  double tOffset, x[3], v[3], s, r;
  double theta;
  vtkPolyLine* lineNormalGenerator = NULL;
  vtkFloatArray* normals = NULL;
  vtkFloatArray* rotation = 0;
  vtkPolyData *output=this->GetOutput();

  this->SavePointInterval = this->StepLength;
  this->vtkGenericStreamer::Integrate();
  if ( this->NumberOfStreamers <= 0 ) {return;}

  pts = vtkIdList::New();
  pts->Allocate(2500);

  //
  //  Convert streamer into lines. Lines may be dashed.
  //
  newPts  = vtkPoints::New();
  newPts->Allocate(1000);
  newVectors  = vtkFloatArray::New();
  newVectors->SetNumberOfComponents(3);
  newVectors->Allocate(3000);
  if ( this->Vorticity )
    {
    lineNormalGenerator = vtkPolyLine::New();
    normals = vtkFloatArray::New();
    normals->SetNumberOfComponents(3);
    normals->Allocate(3000);
    rotation = vtkFloatArray::New();
    rotation->SetNumberOfComponents(1);
    rotation->Allocate(1000);
    rotation->SetName("Thetas");
    output->GetPointData()->AddArray(rotation);
    }

//FIXME
//  if ( this->GetInput()->GetPointData()->GetScalars() || this->SpeedScalars
//    || this->OrientationScalars)
//    {
//    newScalars = vtkFloatArray::New();
//    newScalars->Allocate(1000);
//    }
  newLines = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(2*this->NumberOfStreamers,
                                            VTK_CELL_SIZE));
  //
  // Loop over all streamers generating points
  //
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
    if ( this->Streamers[ptId].GetNumberOfPoints() < 2 )
      {
      continue;
      }
    sPrev = this->Streamers[ptId].GetStreamPoint(0);
    sPtr = this->Streamers[ptId].GetStreamPoint(1);

    if ( this->Streamers[ptId].GetNumberOfPoints() == 2 && sPtr->cellId >= 0 )
      {
      continue;
      }

    tOffset = sPrev->t;

    for ( i=1; 
    i < this->Streamers[ptId].GetNumberOfPoints() && sPtr->cellId >= 0;
    i++, sPrev=sPtr, sPtr=this->Streamers[ptId].GetStreamPoint(i) )
      {
      //
      // Create points for line
      //
      while ( tOffset >= sPrev->t && tOffset < sPtr->t )
        {
        r = (tOffset - sPrev->t) / (sPtr->t - sPrev->t);

        for (j=0; j<3; j++)
          {
          x[j] = sPrev->x[j] + r * (sPtr->x[j] - sPrev->x[j]);
          v[j] = sPrev->v[j] + r * (sPtr->v[j] - sPrev->v[j]);
          }

        // add point to line
        id = newPts->InsertNextPoint(x);
        pts->InsertNextId(id);
        newVectors->InsertTuple(id,v);

        if ( newScalars ) 
          {
          s = sPrev->s + r * (sPtr->s - sPrev->s);
          newScalars->InsertTuple(id,&s);
          }
        
        if ( this->Vorticity )
          {
          // Store the rotation values. Used after all the streamlines
          // are generated.
          theta = sPrev->theta + r * (sPtr->theta - sPrev->theta);
          rotation->InsertTuple(id, &theta);
          }

        tOffset += this->StepLength;

        } // while
      } //for this streamer

    if ( pts->GetNumberOfIds() > 1 )
      {
      newLines->InsertNextCell(pts);
      pts->Reset();
      }
    } //for all streamers

  vtkDebugMacro(<<"Created " << newPts->GetNumberOfPoints() << " points, "
               << newLines->GetNumberOfCells() << " lines");

  if (this->Vorticity)
    {
    // Rotate the normal vectors with stream vorticity
    vtkIdType nPts=0;
    vtkIdType *linePts=0;
    double normal[3], local1[3], local2[3], length, costheta, sintheta;

    lineNormalGenerator->GenerateSlidingNormals(newPts,newLines,normals);
    
    //  Loop over all lines, from the above code we are know that each line
    //  will have at least two points and that no points will be shared
    //  between lines. It is important to loop over the points used by the
    //  lines because newPts may actually contain points that are not used by
    //  any lines. The normals are only calculated for points that are used
    //  in lines so referencing normals for all points can lead to UMRs
    for (newLines->InitTraversal(); newLines->GetNextCell(nPts,linePts); )
      {
      for(i=0; i<nPts; i++)
        {
        normals->GetTuple(linePts[i], normal);
        newVectors->GetTuple(linePts[i], v);
        // obtain two unit orthogonal vectors on the plane perpendicular to
        // the streamline
        for(j=0; j<3; j++) 
          { 
          local1[j] = normal[j]; 
          }
        length = vtkMath::Normalize(local1);
        vtkMath::Cross(local1, v, local2);
        vtkMath::Normalize(local2);
        // Rotate the normal with theta
        rotation->GetTuple(linePts[i], &theta);
        costheta = cos(theta);
        sintheta = sin(theta);
        for(j=0; j<3; j++)
          {
          normal[j] = length* (costheta*local1[j] + sintheta*local2[j]);
          }
        normals->SetTuple(linePts[i], normal);
        }
      }
    output->GetPointData()->SetNormals(normals);
    normals->Delete();
    lineNormalGenerator->Delete();
    rotation->Delete();
    }
    
  output->SetPoints(newPts);
  newPts->Delete();

  output->GetPointData()->SetVectors(newVectors);
  newVectors->Delete();

  if ( newScalars ) 
    {
    int idx = output->GetPointData()->AddArray(newScalars);
    output->GetPointData()->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
    }

  pts->Delete();
  output->SetLines(newLines);
  newLines->Delete();

  // Delete the streamers since they are no longer needed
  delete[] this->Streamers;
  this->Streamers = 0;
  this->NumberOfStreamers = 0;

  output->Squeeze();
}

void vtkGenericStreamLine::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Step Length: " << this->StepLength << "\n";

}

